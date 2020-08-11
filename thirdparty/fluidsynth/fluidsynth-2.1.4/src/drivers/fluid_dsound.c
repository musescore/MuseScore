/* FluidSynth - A Software Synthesizer
 *
 * Copyright (C) 2003  Peter Hanappe and others.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public License
 * as published by the Free Software Foundation; either version 2.1 of
 * the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free
 * Software Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301, USA
 */


#include "fluid_synth.h"
#include "fluid_adriver.h"
#include "fluid_settings.h"

#if DSOUND_SUPPORT

#define INITGUID

#include <mmsystem.h>
#include <dsound.h>

#define NOBITMAP
#include <mmreg.h>

static DWORD WINAPI fluid_dsound_audio_run(LPVOID lpParameter);

static char *fluid_win32_error(HRESULT hr);

typedef struct
{
    fluid_audio_driver_t driver;
    LPDIRECTSOUND direct_sound;
    LPDIRECTSOUNDBUFFER prim_buffer;
    LPDIRECTSOUNDBUFFER sec_buffer;
    HANDLE thread;
    DWORD threadID;
    fluid_synth_t *synth;
    fluid_audio_callback_t write;
    HANDLE quit_ev;
    int   bytes_per_second;
    DWORD buffer_byte_size;
    DWORD queue_byte_size;
    DWORD frame_size;
} fluid_dsound_audio_driver_t;

typedef struct
{
    LPGUID devGUID;
    char *devname;
} fluid_dsound_devsel_t;

BOOL CALLBACK
fluid_dsound_enum_callback(LPGUID guid, LPCTSTR description, LPCTSTR module, LPVOID context)
{
    fluid_settings_t *settings = (fluid_settings_t *) context;
    fluid_settings_add_option(settings, "audio.dsound.device", (const char *)description);

    return TRUE;
}

BOOL CALLBACK
fluid_dsound_enum_callback2(LPGUID guid, LPCTSTR description, LPCTSTR module, LPVOID context)
{
    fluid_dsound_devsel_t *devsel = (fluid_dsound_devsel_t *) context;
    FLUID_LOG(FLUID_DBG, "Testing audio device: %s", description);

    if(FLUID_STRCASECMP(devsel->devname, description) == 0)
    {
        devsel->devGUID = FLUID_NEW(GUID);

        if(devsel->devGUID)
        {
            memcpy(devsel->devGUID, guid, sizeof(GUID));
            FLUID_LOG(FLUID_DBG, "Selected audio device GUID: %p", devsel->devGUID);
        }
    }

    return TRUE;
}

void fluid_dsound_audio_driver_settings(fluid_settings_t *settings)
{
    fluid_settings_register_str(settings, "audio.dsound.device", "default", 0);
    fluid_settings_add_option(settings, "audio.dsound.device", "default");
    DirectSoundEnumerate((LPDSENUMCALLBACK) fluid_dsound_enum_callback, settings);
}


/*
 * new_fluid_dsound_audio_driver
 */
fluid_audio_driver_t *
new_fluid_dsound_audio_driver(fluid_settings_t *settings, fluid_synth_t *synth)
{
    HRESULT hr;
    DSBUFFERDESC desc;
    fluid_dsound_audio_driver_t *dev = NULL;
    DSCAPS caps;
    char *buf1;
    DWORD bytes1;
    double sample_rate;
    int periods, period_size;
    fluid_dsound_devsel_t devsel;
    WAVEFORMATEX format;

    /* create and clear the driver data */
    dev = FLUID_NEW(fluid_dsound_audio_driver_t);

    if(dev == NULL)
    {
        FLUID_LOG(FLUID_ERR, "Out of memory");
        return NULL;
    }

    FLUID_MEMSET(dev, 0, sizeof(fluid_dsound_audio_driver_t));
    dev->synth = synth;

    fluid_settings_getnum(settings, "synth.sample-rate", &sample_rate);
    fluid_settings_getint(settings, "audio.periods", &periods);
    fluid_settings_getint(settings, "audio.period-size", &period_size);

    /* Clear the buffer format */
    ZeroMemory(&format, sizeof(WAVEFORMATEX));

    /* check the format */
    if(fluid_settings_str_equal(settings, "audio.sample-format", "float"))
    {
        FLUID_LOG(FLUID_DBG, "Selected 32 bit sample format");

        dev->frame_size = 2 * sizeof(float);
        dev->write = fluid_synth_write_float;

        format.wFormatTag = WAVE_FORMAT_IEEE_FLOAT;
    }
    else if(fluid_settings_str_equal(settings, "audio.sample-format", "16bits"))
    {
        FLUID_LOG(FLUID_DBG, "Selected 16 bit sample format");

        dev->frame_size = 2 * sizeof(short);
        dev->write = fluid_synth_write_s16;

        format.wFormatTag = WAVE_FORMAT_PCM;
    }
    else
    {
        FLUID_LOG(FLUID_ERR, "Unhandled sample format");
        goto error_recovery;
    }

    dev->buffer_byte_size = period_size * dev->frame_size;
    dev->queue_byte_size = periods * dev->buffer_byte_size;
    dev->bytes_per_second = sample_rate * dev->frame_size;

    /* Finish to initialize the buffer format */
    format.nChannels = 2;
    format.wBitsPerSample = dev->frame_size * 4;
    format.nSamplesPerSec = (DWORD) sample_rate;
    format.nBlockAlign = (WORD) dev->frame_size;
    format.nAvgBytesPerSec = dev->bytes_per_second;

    devsel.devGUID = NULL;

    /* get the selected device name. if none is specified, use NULL for the default device. */
    if(fluid_settings_dupstr(settings, "audio.dsound.device", &devsel.devname) == FLUID_OK /* ++ alloc device name */
            && devsel.devname && strlen(devsel.devname) > 0)
    {
        /* look for the GUID of the selected device */
        DirectSoundEnumerate((LPDSENUMCALLBACK) fluid_dsound_enum_callback2, (void *)&devsel);
    }

    if(devsel.devname)
    {
        FLUID_FREE(devsel.devname);    /* -- free device name */
    }

    /* open DirectSound */
    hr = DirectSoundCreate(devsel.devGUID, &dev->direct_sound, NULL);

    if(hr != DS_OK)
    {
        FLUID_LOG(FLUID_ERR, "Failed to create the DirectSound object");
        goto error_recovery;
    }

    hr = IDirectSound_SetCooperativeLevel(dev->direct_sound, GetDesktopWindow(), DSSCL_PRIORITY);

    if(hr != DS_OK)
    {
        FLUID_LOG(FLUID_ERR, "Failed to set the cooperative level");
        goto error_recovery;
    }

    caps.dwSize = sizeof(caps);
    hr = IDirectSound_GetCaps(dev->direct_sound, &caps);

    if(hr != DS_OK)
    {
        FLUID_LOG(FLUID_ERR, "Failed to query the device capacities");
        goto error_recovery;
    }

    /* create primary buffer */

    ZeroMemory(&desc, sizeof(DSBUFFERDESC));
    desc.dwSize = sizeof(DSBUFFERDESC);
    desc.dwFlags = DSBCAPS_PRIMARYBUFFER;

    if(caps.dwFreeHwMixingStreamingBuffers > 0)
    {
        desc.dwFlags |= DSBCAPS_LOCHARDWARE;
    }

    hr = IDirectSound_CreateSoundBuffer(dev->direct_sound, &desc, &dev->prim_buffer, NULL);

    if(hr != DS_OK)
    {
        FLUID_LOG(FLUID_ERR, "Failed to allocate the primary buffer");
        goto error_recovery;
    }

    /* set the primary sound buffer to this format. if it fails, just
       print a warning. */
    hr = IDirectSoundBuffer_SetFormat(dev->prim_buffer, &format);

    if(hr != DS_OK)
    {
        FLUID_LOG(FLUID_WARN, "Can't set format of primary sound buffer: %s", fluid_win32_error(hr));
    }

    /* initialize the buffer description */

    ZeroMemory(&desc, sizeof(DSBUFFERDESC));
    desc.dwSize = sizeof(DSBUFFERDESC);
    desc.dwFlags = DSBCAPS_GLOBALFOCUS | DSBCAPS_GETCURRENTPOSITION2;
    desc.lpwfxFormat = &format;
    desc.dwBufferBytes = dev->queue_byte_size;

    if(caps.dwFreeHwMixingStreamingBuffers > 0)
    {
        desc.dwFlags |= DSBCAPS_LOCHARDWARE;
    }

    /* create the secondary sound buffer */

    hr = IDirectSound_CreateSoundBuffer(dev->direct_sound, &desc, &dev->sec_buffer, NULL);

    if(hr != DS_OK)
    {
        FLUID_LOG(FLUID_ERR, "dsound: Can't create sound buffer: %s", fluid_win32_error(hr));
        goto error_recovery;
    }


    /* Lock */
    hr = IDirectSoundBuffer_Lock(dev->sec_buffer, 0, 0, (void *) &buf1, &bytes1, 0, 0, DSBLOCK_ENTIREBUFFER);

    if((hr != DS_OK) || (buf1 == NULL))
    {
        FLUID_LOG(FLUID_PANIC, "Failed to lock the audio buffer. Exiting.");
        goto error_recovery;
    }

    /* fill the buffer with silence */
    memset(buf1, 0, bytes1);

    /* Unlock */
    IDirectSoundBuffer_Unlock(dev->sec_buffer, buf1, bytes1, 0, 0);

    /* Create object to signal thread exit */
    dev->quit_ev = CreateEvent(NULL, FALSE, FALSE, NULL);

    if(dev->quit_ev == NULL)
    {
        goto error_recovery;
    }

    /* start the audio thread */
    dev->thread = CreateThread(NULL, 0, fluid_dsound_audio_run, (LPVOID) dev, 0, &dev->threadID);

    if(dev->thread == NULL)
    {
        goto error_recovery;
    }

    return (fluid_audio_driver_t *) dev;

error_recovery:
    delete_fluid_dsound_audio_driver((fluid_audio_driver_t *) dev);
    return NULL;
}


void delete_fluid_dsound_audio_driver(fluid_audio_driver_t *d)
{
    fluid_dsound_audio_driver_t *dev = (fluid_dsound_audio_driver_t *) d;
    fluid_return_if_fail(dev != NULL);

    /* First tell dsound to stop playing its buffer now.
       Doing this before stopping audio thread avoid dsound playing transient
       glitches between the time audio task is stopped and dsound will be released.
       These glitches are particularly audible when a reverb is connected
       on output.
    */
    if(dev->sec_buffer != NULL)
    {
        IDirectSoundBuffer_Stop(dev->sec_buffer);
    }

    /* request the audio task to stop and wait till the audio thread exits */
    if(dev->thread != NULL)
    {
        /* tell the audio thread to stop its loop */
        SetEvent(dev->quit_ev);

        if(WaitForSingleObject(dev->thread, 2000) != WAIT_OBJECT_0)
        {
            /* on error kill the thread mercilessly */
            FLUID_LOG(FLUID_DBG, "Couldn't join the audio thread. killing it.");
            TerminateThread(dev->thread, 0);
        }

        /* Release the thread object */
        CloseHandle(dev->thread);
    }

    /* Release the event object */
    if(dev->quit_ev != NULL)
    {
        CloseHandle(dev->quit_ev);
    }

    /* Finish to release all the dsound allocated resources */

    if(dev->sec_buffer != NULL)
    {
        IDirectSoundBuffer_Release(dev->sec_buffer);
    }

    if(dev->prim_buffer != NULL)
    {
        IDirectSoundBuffer_Release(dev->prim_buffer);
    }

    if(dev->direct_sound != NULL)
    {
        IDirectSound_Release(dev->direct_sound);
    }

    FLUID_FREE(dev);
}

static DWORD WINAPI fluid_dsound_audio_run(LPVOID lpParameter)
{
    fluid_dsound_audio_driver_t *dev = (fluid_dsound_audio_driver_t *) lpParameter;
    short *buf1, *buf2;
    DWORD bytes1, bytes2;
    DWORD cur_position, frames, play_position, write_position, bytes;
    HRESULT res;
    int     ms;

    cur_position = 0;

    /* boost the priority of the audio thread */
    SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_TIME_CRITICAL);

    IDirectSoundBuffer_Play(dev->sec_buffer, 0, 0, DSBPLAY_LOOPING);

    for(;;)
    {
        IDirectSoundBuffer_GetCurrentPosition(dev->sec_buffer, &play_position, &write_position);

        if(cur_position <= play_position)
        {
            bytes = play_position - cur_position;
        }
        else if((play_position < cur_position) && (write_position <= cur_position))
        {
            bytes = dev->queue_byte_size + play_position - cur_position;
        }
        else
        {
            bytes = 0;
        }

        if(bytes >= dev->buffer_byte_size)
        {

            /* Lock */
            res = IDirectSoundBuffer_Lock(dev->sec_buffer, cur_position, bytes, (void *) &buf1, &bytes1, (void *) &buf2, &bytes2, 0);

            if((res != DS_OK) || (buf1 == NULL))
            {
                FLUID_LOG(FLUID_PANIC, "Failed to lock the audio buffer. System lockup might follow. Exiting.");
                ExitProcess(0);
            }

            /* fill the first part of the buffer */
            if(bytes1 > 0)
            {
                frames = bytes1 / dev->frame_size;
                dev->write(dev->synth, frames, buf1, 0, 2, buf1, 1, 2);
                cur_position += frames * dev->frame_size;
            }

            /* fill the second part of the buffer */
            if((buf2 != NULL) && (bytes2 > 0))
            {
                frames = bytes2 / dev->frame_size;
                dev->write(dev->synth, frames, buf2, 0, 2, buf2, 1, 2);
                cur_position += frames * dev->frame_size;
            }

            /* Unlock */
            IDirectSoundBuffer_Unlock(dev->sec_buffer, buf1, bytes1, buf2, bytes2);

            if(cur_position >= dev->queue_byte_size)
            {
                cur_position -= dev->queue_byte_size;
            }

            /* 1 ms of wait */
            ms = 1;
        }
        else
        {
            /* Calculate how many milliseconds to sleep (minus 1 for safety) */
            ms = (dev->buffer_byte_size - bytes) * 1000 / dev->bytes_per_second - 1;

            if(ms < 1)
            {
                ms = 1;
            }
        }

        /* Wait quit event or timeout */
        if(WaitForSingleObject(dev->quit_ev, ms) == WAIT_OBJECT_0)
        {
            break;
        }
    }

    return 0;
}


static char *fluid_win32_error(HRESULT hr)
{
    char *s = "Don't know why";

    switch(hr)
    {
    case E_NOINTERFACE:
        s = "No such interface";
        break;

    case DSERR_GENERIC:
        s = "Generic error";
        break;

    case DSERR_ALLOCATED:
        s = "Required resources already allocated";
        break;

    case DSERR_BADFORMAT:
        s = "The format is not supported";
        break;

    case DSERR_INVALIDPARAM:
        s = "Invalid parameter";
        break;

    case DSERR_NOAGGREGATION:
        s = "No aggregation";
        break;

    case DSERR_OUTOFMEMORY:
        s = "Out of memory";
        break;

    case DSERR_UNINITIALIZED:
        s = "Uninitialized";
        break;

    case DSERR_UNSUPPORTED:
        s = "Function not supported";
        break;
    }

    return s;
}

#endif /* DSOUND_SUPPORT */
