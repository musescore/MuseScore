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


#include <mmsystem.h>
#include <dsound.h>
#include <mmreg.h>

/* Those two includes are required on Windows 9x/ME */
#include <ks.h>
#include <ksmedia.h>

static DWORD WINAPI fluid_dsound_audio_run(LPVOID lpParameter);
static int fluid_dsound_write_processed_channels(fluid_synth_t *data, int len,
                               int channels_count,
                               void *channels_out[], int channels_off[],
                               int channels_incr[]);

static char *fluid_win32_error(HRESULT hr);

/**
* The driver handle multiple channels.
* Actually the number maximum of channels is limited to  2 * DSOUND_MAX_STEREO_CHANNELS.
* The only reason of this limitation is because we dont know how to define the mapping
* of speakers for stereo output number above DSOUND_MAX_STEREO_CHANNELS.
*/
/* Maximum number of stereo outputs */
#define DSOUND_MAX_STEREO_CHANNELS 4
/* Speakers mapping */
static const DWORD channel_mask_speakers[DSOUND_MAX_STEREO_CHANNELS] =
{
    /* 1 stereo output */
    SPEAKER_FRONT_LEFT | SPEAKER_FRONT_RIGHT,
  
    /* 2 stereo outputs */
    SPEAKER_FRONT_LEFT | SPEAKER_FRONT_RIGHT |
    SPEAKER_BACK_LEFT | SPEAKER_BACK_RIGHT,

    /* 3 stereo outputs */
    SPEAKER_FRONT_LEFT | SPEAKER_FRONT_RIGHT |
    SPEAKER_FRONT_CENTER | SPEAKER_LOW_FREQUENCY |
    SPEAKER_BACK_LEFT | SPEAKER_BACK_RIGHT,

    /* 4 stereo outputs */
    SPEAKER_FRONT_LEFT | SPEAKER_FRONT_RIGHT |
    SPEAKER_FRONT_CENTER | SPEAKER_LOW_FREQUENCY |
    SPEAKER_BACK_LEFT | SPEAKER_BACK_RIGHT |
    SPEAKER_SIDE_LEFT | SPEAKER_SIDE_RIGHT
};

typedef struct
{
    fluid_audio_driver_t driver;
    LPDIRECTSOUND direct_sound;      /* dsound instance */
    LPDIRECTSOUNDBUFFER prim_buffer; /* dsound buffer*/
    LPDIRECTSOUNDBUFFER sec_buffer;  /* dsound buffer */

    HANDLE thread;        /* driver task */
    DWORD threadID;
    void *synth; /* fluidsynth instance, or user pointer if custom processing function is defined */
    fluid_audio_func_t func;
    /* callback called by the task for audio rendering in dsound buffers */
    fluid_audio_channels_callback_t write;
    HANDLE quit_ev;       /* Event object to request the audio task to stop */
    float **drybuf;

    int   bytes_per_second; /* number of bytes per second */
    DWORD buffer_byte_size; /* size of one buffer in bytes */
    DWORD queue_byte_size;  /* total size of all buffers in bytes */
    DWORD frame_size;       /* frame size in bytes */
    int channels_count; /* number of channels in audio stream */
} fluid_dsound_audio_driver_t;

typedef struct
{
    LPGUID devGUID;
    char *devname;
} fluid_dsound_devsel_t;

/* enumeration callback to add "device name" option on setting "audio.dsound.device" */
BOOL CALLBACK
fluid_dsound_enum_callback(LPGUID guid, LPCTSTR description, LPCTSTR module, LPVOID context)
{
    fluid_settings_t *settings = (fluid_settings_t *) context;
    fluid_settings_add_option(settings, "audio.dsound.device", (const char *)description);

    return TRUE;
}

/* enumeration callback to look if a certain device exists and return its GUID.
   @context, (fluid_dsound_devsel_t *) context->devname provide the device name to look for.
             (fluid_dsound_devsel_t *) context->devGUID pointer to return device GUID.
   @return TRUE to continue enumeration, FALSE otherwise.
*/
BOOL CALLBACK
fluid_dsound_enum_callback2(LPGUID guid, LPCTSTR description, LPCTSTR module, LPVOID context)
{
    fluid_dsound_devsel_t *devsel = (fluid_dsound_devsel_t *) context;
    FLUID_LOG(FLUID_DBG, "Testing audio device: %s", description);

    if(FLUID_STRCASECMP(devsel->devname, description) == 0)
    {
        /* The device exists, return a copy of its GUID */
        devsel->devGUID = FLUID_NEW(GUID);

        if(devsel->devGUID)
        {
            /* return GUID */
            memcpy(devsel->devGUID, guid, sizeof(GUID));
            FLUID_LOG(FLUID_DBG, "Selected audio device GUID: %p", devsel->devGUID);
            return FALSE;
        }
    }

    return TRUE;
}

/*
   - register setting "audio.dsound.device".
   - add list of dsound device name as option of "audio.dsound.device" setting.
*/
void fluid_dsound_audio_driver_settings(fluid_settings_t *settings)
{
    fluid_settings_register_str(settings, "audio.dsound.device", "default", 0);
    fluid_settings_add_option(settings, "audio.dsound.device", "default");
    DirectSoundEnumerate((LPDSENUMCALLBACK) fluid_dsound_enum_callback, settings);
}


/*
 * new_fluid_dsound_audio_driver
 * The driver handle the case of multiple stereo buffers provided by fluidsynth
 * mixer.
 * Each stereo buffers (left, right) are written to respective channels pair
 * of the audio device card.
 * For example, if the number of internal mixer buffer is 2, the audio device
 * must have at least 4 channels:
 * - buffer 0 (left, right) will be written to channel pair (0, 1).
 * - buffer 1 (left, right) will be written to channel pair (2, 3).
 *
 * @param setting. The settings the driver looks for:
 *  "synth.sample-rate", the sample rate.
 *  "audio.periods", the number of buffers and
 *  "audio.period-size", the size of each buffer.
 *  "audio.sample-format",the sample format, 16bits or float.

 * @param synth, fluidsynth synth instance to associate to the driver.
 *
 * Note: The number of internal mixer buffer is indicated by synth->audio_channels.
 * If the audio device cannot handle the format or do not have enough channels,
 * the driver fails and return NULL.
*/
fluid_audio_driver_t *
new_fluid_dsound_audio_driver(fluid_settings_t *settings, fluid_synth_t *synth)
{
    return new_fluid_dsound_audio_driver2(settings, NULL, synth);
}

fluid_audio_driver_t *
new_fluid_dsound_audio_driver2(fluid_settings_t *settings, fluid_audio_func_t func, void *data)
{
    HRESULT hr;
    DSBUFFERDESC desc;
    fluid_dsound_audio_driver_t *dev = NULL;
    DSCAPS caps;
    char *buf1;
    DWORD bytes1;
    double sample_rate;
    int periods, period_size;
    int audio_channels;
    int i;
    fluid_dsound_devsel_t devsel;
    WAVEFORMATEXTENSIBLE format;

    /* create and clear the driver data */
    dev = FLUID_NEW(fluid_dsound_audio_driver_t);

    if(dev == NULL)
    {
        FLUID_LOG(FLUID_ERR, "Out of memory");
        return NULL;
    }

    FLUID_MEMSET(dev, 0, sizeof(fluid_dsound_audio_driver_t));
    dev->synth = data;
    dev->func = func;

    fluid_settings_getnum(settings, "synth.sample-rate", &sample_rate);
    fluid_settings_getint(settings, "audio.periods", &periods);
    fluid_settings_getint(settings, "audio.period-size", &period_size);
    fluid_settings_getint(settings, "synth.audio-channels", &audio_channels);

    /* Clear format structure*/
    ZeroMemory(&format, sizeof(WAVEFORMATEXTENSIBLE));

    /* Set this early so that if buffer allocation failed we can free the memory */
    dev->channels_count = audio_channels * 2;

    /* check the format */
    if(!func)
    {
        if(fluid_settings_str_equal(settings, "audio.sample-format", "float"))
        {
            FLUID_LOG(FLUID_DBG, "Selected 32 bit sample format");
            dev->write = fluid_synth_write_float_channels;
            /* sample container size in bits: 32 bits */
            format.Format.wBitsPerSample = 8 * sizeof(float);
            format.SubFormat = KSDATAFORMAT_SUBTYPE_IEEE_FLOAT;
            format.Format.wFormatTag = WAVE_FORMAT_IEEE_FLOAT;
        }
        else if(fluid_settings_str_equal(settings, "audio.sample-format", "16bits"))
        {
            FLUID_LOG(FLUID_DBG, "Selected 16 bit sample format");
            dev->write = fluid_synth_write_s16_channels;
            /* sample container size in bits: 16bits */
            format.Format.wBitsPerSample = 8 * sizeof(short);
            format.SubFormat = KSDATAFORMAT_SUBTYPE_PCM;
            format.Format.wFormatTag = WAVE_FORMAT_PCM;
        }
        else
        {
            FLUID_LOG(FLUID_ERR, "Unhandled sample format");
            goto error_recovery;
        }
    }
    else
    {
        FLUID_LOG(FLUID_DBG, "Selected 32 bit sample format");
        dev->write = fluid_dsound_write_processed_channels;
        /* sample container size in bits: 32 bits */
        format.Format.wBitsPerSample = 8 * sizeof(float);
        format.SubFormat = KSDATAFORMAT_SUBTYPE_IEEE_FLOAT;
        format.Format.wFormatTag = WAVE_FORMAT_IEEE_FLOAT;
        dev->drybuf = FLUID_ARRAY(float*, audio_channels * 2);
        if(dev->drybuf == NULL)
        {
            FLUID_LOG(FLUID_ERR, "Out of memory");
            goto error_recovery;
        }
        FLUID_MEMSET(dev->drybuf, 0, sizeof(float*) * audio_channels * 2);
        for(i = 0; i < audio_channels * 2; ++i)
        {
            dev->drybuf[i] = FLUID_ARRAY(float, periods * period_size);
            if(dev->drybuf[i] == NULL)
            {
                FLUID_LOG(FLUID_ERR, "Out of memory");
                goto error_recovery;
            }
        }
    }

    /* Finish to initialize the format structure */
    /* number of channels in a frame */
    format.Format.nChannels = audio_channels * 2;

    if(audio_channels > DSOUND_MAX_STEREO_CHANNELS)
    {
        FLUID_LOG(FLUID_ERR, "Channels number %d exceed internal limit %d",
                  format.Format.nChannels, DSOUND_MAX_STEREO_CHANNELS * 2);
        goto error_recovery;
    }

    /* size of frame in bytes */
    format.Format.nBlockAlign = format.Format.nChannels * format.Format.wBitsPerSample / 8;
    format.Format.nSamplesPerSec = (DWORD) sample_rate;
    format.Format.nAvgBytesPerSec = format.Format.nBlockAlign * format.Format.nSamplesPerSec;

    /* extension */
    if(format.Format.nChannels > 2)
    {
        format.Format.wFormatTag = WAVE_FORMAT_EXTENSIBLE;
        format.Format.cbSize = 22;
        format.Samples.wValidBitsPerSample = format.Format.wBitsPerSample;

        /* CreateSoundBuffer accepts only format.dwChannelMask compatible with
           format.Format.nChannels
        */
        format.dwChannelMask = channel_mask_speakers[audio_channels - 1];
    }

    /* Finish to initialize dev structure */
    dev->frame_size = format.Format.nBlockAlign;
    dev->buffer_byte_size = period_size * dev->frame_size;
    dev->queue_byte_size = periods * dev->buffer_byte_size;
    dev->bytes_per_second = format.Format.nAvgBytesPerSec;

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

    if(devsel.devGUID)
    {
        FLUID_FREE(devsel.devGUID);    /* -- free device GUID */
    }

    if(hr != DS_OK)
    {
        FLUID_LOG(FLUID_ERR, "Failed to create the DirectSound object: '%s'", fluid_win32_error(hr));
        goto error_recovery;
    }

    hr = IDirectSound_SetCooperativeLevel(dev->direct_sound, GetDesktopWindow(), DSSCL_PRIORITY);

    if(hr != DS_OK)
    {
        FLUID_LOG(FLUID_ERR, "Failed to set the cooperative level: '%s'", fluid_win32_error(hr));
        goto error_recovery;
    }

    caps.dwSize = sizeof(caps);
    hr = IDirectSound_GetCaps(dev->direct_sound, &caps);

    if(hr != DS_OK)
    {
        FLUID_LOG(FLUID_ERR, "Failed to query the device capacities: '%s'", fluid_win32_error(hr));
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
        FLUID_LOG(FLUID_ERR, "Failed to allocate the primary buffer: '%s'", fluid_win32_error(hr));
        goto error_recovery;
    }

    /* set the primary sound buffer to this format. if it fails, just
       print a warning. */
    hr = IDirectSoundBuffer_SetFormat(dev->prim_buffer, (WAVEFORMATEX *)&format);

    if(hr != DS_OK)
    {
        FLUID_LOG(FLUID_WARN, "Can't set format of primary sound buffer: '%s'", fluid_win32_error(hr));
    }

    /* initialize the buffer description */

    ZeroMemory(&desc, sizeof(DSBUFFERDESC));
    desc.dwSize = sizeof(DSBUFFERDESC);
    desc.dwFlags = DSBCAPS_GLOBALFOCUS | DSBCAPS_GETCURRENTPOSITION2;

    /* CreateSoundBuffer accepts only format.dwChannelMask compatible with
       format.Format.nChannels
    */
    desc.lpwfxFormat = (WAVEFORMATEX *)&format;
    desc.dwBufferBytes = dev->queue_byte_size;

    if(caps.dwFreeHwMixingStreamingBuffers > 0)
    {
        desc.dwFlags |= DSBCAPS_LOCHARDWARE;
    }

    /* create the secondary sound buffer */

    hr = IDirectSound_CreateSoundBuffer(dev->direct_sound, &desc, &dev->sec_buffer, NULL);

    if(hr != DS_OK)
    {
        FLUID_LOG(FLUID_ERR, "dsound: Can't create sound buffer: '%s'", fluid_win32_error(hr));
        goto error_recovery;
    }


    /* Lock and get dsound buffer */
    hr = IDirectSoundBuffer_Lock(dev->sec_buffer, 0, 0, (void *) &buf1, &bytes1, 0, 0, DSBLOCK_ENTIREBUFFER);

    if((hr != DS_OK) || (buf1 == NULL))
    {
        FLUID_LOG(FLUID_PANIC, "Failed to lock the audio buffer: '%s'", fluid_win32_error(hr));
        goto error_recovery;
    }

    /* fill the buffer with silence */
    memset(buf1, 0, bytes1);

    /* Unlock dsound buffer */
    IDirectSoundBuffer_Unlock(dev->sec_buffer, buf1, bytes1, 0, 0);

    /* Create object to signal thread exit */
    dev->quit_ev = CreateEvent(NULL, FALSE, FALSE, NULL);

    if(dev->quit_ev == NULL)
    {
        FLUID_LOG(FLUID_ERR, "Failed to create quit Event: '%s'", fluid_get_windows_error());
        goto error_recovery;
    }

    /* start the audio thread */
    dev->thread = CreateThread(NULL, 0, fluid_dsound_audio_run, (LPVOID) dev, 0, &dev->threadID);

    if(dev->thread == NULL)
    {
        FLUID_LOG(FLUID_ERR, "Failed to create DSound audio thread: '%s'", fluid_get_windows_error());
        goto error_recovery;
    }

    return (fluid_audio_driver_t *) dev;

error_recovery:
    delete_fluid_dsound_audio_driver((fluid_audio_driver_t *) dev);
    return NULL;
}


void delete_fluid_dsound_audio_driver(fluid_audio_driver_t *d)
{
    int i;

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

    if(dev->drybuf != NULL)
    {
        for(i = 0; i < dev->channels_count; ++i)
        {
            FLUID_FREE(dev->drybuf[i]);
        }
    }

    FLUID_FREE(dev->drybuf);

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

    /* pointers table on output first sample channels */
    void *channels_out[DSOUND_MAX_STEREO_CHANNELS * 2];
    int channels_off[DSOUND_MAX_STEREO_CHANNELS * 2];
    int channels_incr[DSOUND_MAX_STEREO_CHANNELS * 2];
    int i;

    /* initialize write callback constant parameters:
       dsound expects interleaved channels in a unique buffer.
       For example 4 channels (c1, c2, c3, c4) and n samples:
       { s1:c1, s1:c2, s1:c3, s1:c4,  s2:c1, s2:c2, s2:c3, s2:c4,...
         sn:c1, sn:c2, sn:c3, sn:c4 }.

       So, channels_off[], channnel_incr[] tables should initialized like this:
         channels_off[0] = 0    channels_incr[0] = 4
         channels_off[1] = 1    channels_incr[1] = 4
         channels_off[2] = 2    channels_incr[2] = 4
         channels_off[3] = 3    channels_incr[3] = 4

       channels_out[], table will be initialized later, just before calling
       the write callback function.
         channels_out[0] = address of dsound buffer
         channels_out[1] = address of dsound buffer
         channels_out[2] = address of dsound buffer
         channels_out[3] = address of dsound buffer
    */
    for(i = 0; i < dev->channels_count; i++)
    {
        channels_off[i] = i;
        channels_incr[i] = dev->channels_count;
    }

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
                ExitProcess((UINT)-1);
            }

            /* fill the first part of the buffer */
            if(bytes1 > 0)
            {
                frames = bytes1 / dev->frame_size;
                /* Before calling write function, finish to initialize
                   channels_out[] table parameter:
                   dsound expects interleaved channels in a unique buffer.
                   So, channels_out[] table must be initialized with the address
                   of the same buffer (buf1).
                */
                i = dev->channels_count;

                do
                {
                    channels_out[--i] = buf1;
                }
                while(i);

                /* calling write function */
                if(dev->func == NULL)
                {
                    dev->write(dev->synth, frames, dev->channels_count,
                            channels_out, channels_off, channels_incr);
                }
                else
                {
                    dev->write((fluid_synth_t*)dev, frames, dev->channels_count,
                            channels_out, channels_off, channels_incr);
                }
                cur_position += frames * dev->frame_size;
            }

            /* fill the second part of the buffer */
            if((buf2 != NULL) && (bytes2 > 0))
            {
                frames = bytes2 / dev->frame_size;
                /* Before calling write function, finish to initialize
                   channels_out[] table parameter:
                   dsound expects interleaved channels in a unique buffer.
                   So, channels_out[] table must be initialized with the address
                   of the same buffer (buf2).
                */
                i = dev->channels_count;

                do
                {
                    channels_out[--i] = buf2;
                }
                while(i);

                /* calling write function */
                if(dev->func == NULL)
                {
                    dev->write(dev->synth, frames, dev->channels_count,
                            channels_out, channels_off, channels_incr);
                }
                else
                {
                    dev->write((fluid_synth_t*)dev, frames, dev->channels_count,
                            channels_out, channels_off, channels_incr);
                }
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

static int fluid_dsound_write_processed_channels(fluid_synth_t *data, int len,
                               int channels_count,
                               void *channels_out[], int channels_off[],
                               int channels_incr[])
{
    int i, ch;
    int ret;
    fluid_dsound_audio_driver_t *drv = (fluid_dsound_audio_driver_t*) data;
    float *optr[DSOUND_MAX_STEREO_CHANNELS * 2];
    for(ch = 0; ch < drv->channels_count; ++ch)
    {
        FLUID_MEMSET(drv->drybuf[ch], 0, len * sizeof(float));
        optr[ch] = (float*)channels_out[ch] + channels_off[ch];
    }
    ret = drv->func(drv->synth, len, 0, NULL, drv->channels_count, drv->drybuf);
    for(ch = 0; ch < drv->channels_count; ++ch)
    {
        for(i = 0; i < len; ++i)
        {
            *optr[ch] = drv->drybuf[ch][i];
            optr[ch] += channels_incr[ch];
        }
    }
    return ret;
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
