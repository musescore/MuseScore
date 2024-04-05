/* FluidSynth - A Software Synthesizer
 *
 * Copyright (C) 2003  Peter Hanappe and others.
 * Copyright (C) 2018  Carlo Bramini
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

#if WAVEOUT_SUPPORT

#include <mmsystem.h>
#include <mmreg.h>

/* Those two includes are required on Windows 9x/ME */
#include <ks.h>
#include <ksmedia.h>

/**
* The driver handle multiple channels.
* Actually the number maximum of channels is limited to  2 * WAVEOUT_MAX_STEREO_CHANNELS.
* The only reason of this limitation is because we dont know how to define the mapping
* of speakers for stereo output number above WAVEOUT_MAX_STEREO_CHANNELS.
*/
/* Maximum number of stereo outputs */
#define WAVEOUT_MAX_STEREO_CHANNELS 4

static char *fluid_waveout_error(MMRESULT hr);
static int fluid_waveout_write_processed_channels(fluid_synth_t *data, int len,
                               int channels_count,
                               void *channels_out[], int channels_off[],
                               int channels_incr[]);

/* Speakers mapping */
static const DWORD channel_mask_speakers[WAVEOUT_MAX_STEREO_CHANNELS] =
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

    void *synth;
    fluid_audio_func_t func;
    fluid_audio_channels_callback_t write_ptr;
    float **drybuf;

    HWAVEOUT hWaveOut;
    WAVEHDR *waveHeader;

    int periods; /* numbers of internal driver buffers */
    int num_frames;

    HANDLE hThread;
    DWORD  dwThread;

    int    nQuit;
    HANDLE hQuit;
    int channels_count; /* number of channels in audio stream */

} fluid_waveout_audio_driver_t;


/* Thread for playing sample buffers */
static DWORD WINAPI fluid_waveout_synth_thread(void *data)
{
    fluid_waveout_audio_driver_t *dev;
    WAVEHDR                      *pWave;

    MSG msg;
    int code;
    /* pointers table on output first sample channels */
    void *channels_out[WAVEOUT_MAX_STEREO_CHANNELS * 2];
    int channels_off[WAVEOUT_MAX_STEREO_CHANNELS * 2];
    int channels_incr[WAVEOUT_MAX_STEREO_CHANNELS * 2];
    int i;

    dev = (fluid_waveout_audio_driver_t *)data;

    /* initialize write callback constant parameters:
       MME expects interleaved channels in a unique buffer.
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

    /* Forces creation of message queue */
    PeekMessage(&msg, NULL, WM_USER, WM_USER, PM_NOREMOVE);

    for(;;)
    {
        code = GetMessage(&msg, NULL, 0, 0);

        if(code < 0)
        {
            FLUID_LOG(FLUID_ERR, "fluid_waveout_synth_thread: GetMessage() failed: '%s'", fluid_get_windows_error());
            break;
        }

        if(msg.message == WM_CLOSE)
        {
            break;
        }

        switch(msg.message)
        {
        case MM_WOM_DONE:
            pWave = (WAVEHDR *)msg.lParam;
            dev   = (fluid_waveout_audio_driver_t *)pWave->dwUser;

            if(dev->nQuit > 0)
            {
                /* Release the sample buffer */
                waveOutUnprepareHeader((HWAVEOUT)msg.wParam, pWave, sizeof(WAVEHDR));

                if(--dev->nQuit == 0)
                {
                    SetEvent(dev->hQuit);
                }
            }
            else
            {
                /* Before calling write function, finish to initialize
                   channels_out[] table parameter:
                   MME expects interleaved channels in a unique buffer.
                   So, channels_out[] table must be initialized with the address
                   of the same buffer (lpData).
                */
                i = dev->channels_count;

                do
                {
                    channels_out[--i] = pWave->lpData;
                }
                while(i);

                dev->write_ptr(dev->func ? (fluid_synth_t*)dev : dev->synth, dev->num_frames, dev->channels_count,
                               channels_out, channels_off, channels_incr);

                waveOutWrite((HWAVEOUT)msg.wParam, pWave, sizeof(WAVEHDR));
            }

            break;
        }
    }

    return 0;
}

void fluid_waveout_audio_driver_settings(fluid_settings_t *settings)
{
    UINT n, nDevs = waveOutGetNumDevs();
#ifdef _UNICODE
    char dev_name[MAXPNAMELEN];
#endif

    fluid_settings_register_str(settings, "audio.waveout.device", "default", 0);
    fluid_settings_add_option(settings, "audio.waveout.device", "default");

    for(n = 0; n < nDevs; n++)
    {
        WAVEOUTCAPS caps;
        MMRESULT    res;

        res = waveOutGetDevCaps(n, &caps, sizeof(caps));

        if(res == MMSYSERR_NOERROR)
        {
#ifdef _UNICODE
            WideCharToMultiByte(CP_UTF8, 0, caps.szPname, -1, dev_name, MAXPNAMELEN, 0, 0);
            FLUID_LOG(FLUID_DBG, "Testing audio device: %s", dev_name);
            fluid_settings_add_option(settings, "audio.waveout.device", dev_name);
#else
            FLUID_LOG(FLUID_DBG, "Testing audio device: %s", caps.szPname);
            fluid_settings_add_option(settings, "audio.waveout.device", caps.szPname);
#endif
        }
    }
}


/*
 * new_fluid_waveout_audio_driver
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
 *  "synth.audio-channels", the number of internal mixer stereo buffer.
 *  "audio.periods",the number of buffers.
 *  "audio.period-size",the size of one buffer.
 *  "audio.sample-format",the sample format, 16bits or float.
 *
 * @param synth, fluidsynth synth instance to associate to the driver.
 *
 * Note: The number of internal mixer stereo buffer is indicated by "synth.audio-channels".
 * If the audio device cannot handle the format or do not have enough channels,
 * the driver fails and return NULL.
 */
fluid_audio_driver_t *
new_fluid_waveout_audio_driver(fluid_settings_t *settings, fluid_synth_t *synth)
{
    return new_fluid_waveout_audio_driver2(settings, NULL, synth);
}

fluid_audio_driver_t *
new_fluid_waveout_audio_driver2(fluid_settings_t *settings, fluid_audio_func_t func, void *data)
{
    fluid_waveout_audio_driver_t *dev = NULL;
    fluid_audio_channels_callback_t write_ptr;
    double sample_rate;
    int frequency, sample_size;
    int periods, period_size;
    int audio_channels;
    LPSTR ptrBuffer;
    int lenBuffer;
    int device;
    int i;
    WAVEFORMATEXTENSIBLE wfx;
    char dev_name[MAXPNAMELEN];
    MMRESULT errCode;

    /* Retrieve the settings */
    fluid_settings_getnum(settings, "synth.sample-rate", &sample_rate);
    fluid_settings_getint(settings, "audio.periods", &periods);
    fluid_settings_getint(settings, "audio.period-size", &period_size);
    fluid_settings_getint(settings, "synth.audio-channels", &audio_channels);

    /* Clear format structure */
    ZeroMemory(&wfx, sizeof(WAVEFORMATEXTENSIBLE));

    /* check the format */
    if(fluid_settings_str_equal(settings, "audio.sample-format", "float") || func)
    {
        FLUID_LOG(FLUID_DBG, "Selected 32 bit sample format");

        sample_size = sizeof(float);
        write_ptr = func ? fluid_waveout_write_processed_channels : fluid_synth_write_float_channels;
        wfx.SubFormat = KSDATAFORMAT_SUBTYPE_IEEE_FLOAT;
        wfx.Format.wFormatTag = WAVE_FORMAT_IEEE_FLOAT;
    }
    else if(fluid_settings_str_equal(settings, "audio.sample-format", "16bits"))
    {
        FLUID_LOG(FLUID_DBG, "Selected 16 bit sample format");

        sample_size = sizeof(short);
        write_ptr = fluid_synth_write_s16_channels;
        wfx.SubFormat = KSDATAFORMAT_SUBTYPE_PCM;
        wfx.Format.wFormatTag = WAVE_FORMAT_PCM;
    }
    else
    {
        FLUID_LOG(FLUID_ERR, "Unhandled sample format");
        return NULL;
    }

    /* Set frequency to integer */
    frequency = (int)sample_rate;

    /* Initialize the format structure */
    wfx.Format.nChannels  = audio_channels * 2;

    if(audio_channels > WAVEOUT_MAX_STEREO_CHANNELS)
    {
        FLUID_LOG(FLUID_ERR, "Channels number %d exceed internal limit %d",
                  wfx.Format.nChannels, WAVEOUT_MAX_STEREO_CHANNELS * 2);
        return NULL;
    }

    wfx.Format.wBitsPerSample  = sample_size * 8;
    wfx.Format.nBlockAlign     = sample_size * wfx.Format.nChannels;
    wfx.Format.nSamplesPerSec  = frequency;
    wfx.Format.nAvgBytesPerSec = frequency * wfx.Format.nBlockAlign;
    /* WAVEFORMATEXTENSIBLE extension is used only when channels number
       is above 2.
       When channels number is below 2, only WAVEFORMATEX structure
       will be used by the Windows driver. This ensures compatibility with
       Windows 9X/NT in the case these versions does not accept the
       WAVEFORMATEXTENSIBLE structure.
    */
    if(wfx.Format.nChannels > 2)
    {
        wfx.Format.wFormatTag = WAVE_FORMAT_EXTENSIBLE;
        wfx.Format.cbSize = 22;
        wfx.Samples.wValidBitsPerSample = wfx.Format.wBitsPerSample;
        wfx.dwChannelMask = channel_mask_speakers[audio_channels - 1];
    }

    /* allocate the internal waveout buffers:
      The length of a single buffer in bytes is dependent of period_size.
    */
    lenBuffer = wfx.Format.nBlockAlign * period_size;
    /* create and clear the driver data */
    dev = HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY,
                    sizeof(fluid_waveout_audio_driver_t) +
                    lenBuffer * periods +
                    sizeof(WAVEHDR) * periods);
    if(dev == NULL)
    {
        FLUID_LOG(FLUID_ERR, "Out of memory");
        return NULL;
    }

    /* Assign extra memory to WAVEHDR */
    dev->waveHeader = (WAVEHDR *)((uintptr_t)(dev + 1) + lenBuffer * periods);

    /* Save copy of synth */
    dev->synth = data;
    dev->func = func;

    /* Save copy of other variables */
    dev->write_ptr = write_ptr;
    /* number of frames in a buffer */
    dev->num_frames = period_size;
    /* number of buffers */
    dev->periods = periods;
    dev->channels_count = wfx.Format.nChannels;

    /* Set default device to use */
    device = WAVE_MAPPER;

    if(func)
    {
        /* allocate extra buffer used by fluid_waveout_write_processed_channels().
           These buffers are buffer adaptation between the rendering
           API fluid_synth_process() and the waveout internal buffers
           Note: the size (in bytes) of these extra buffer (drybuf[]) must be the
           same that the size of internal waveout buffers.
        */
        dev->drybuf = FLUID_ARRAY(float*, audio_channels * 2);
        if(dev->drybuf == NULL)
        {
            FLUID_LOG(FLUID_ERR, "Out of memory");
            delete_fluid_waveout_audio_driver(&dev->driver);
            return NULL;
        }
        FLUID_MEMSET(dev->drybuf, 0, sizeof(float*) * audio_channels * 2);
        for(i = 0; i < audio_channels * 2; ++i)
        {
            /* The length of a single buffer drybuf[i] is dependent of period_size */
            dev->drybuf[i] = FLUID_ARRAY(float, period_size);
            if(dev->drybuf[i] == NULL)
            {
                FLUID_LOG(FLUID_ERR, "Out of memory");
                delete_fluid_waveout_audio_driver(&dev->driver);
                return NULL;
            }
        }
    }

    /* get the selected device name. if none is specified, use default device. */
    if(fluid_settings_copystr(settings, "audio.waveout.device", dev_name, MAXPNAMELEN) == FLUID_OK
            && dev_name[0] != '\0')
    {
        UINT nDevs = waveOutGetNumDevs();
        UINT n;
#ifdef _UNICODE
        WCHAR lpwDevName[MAXPNAMELEN];

        MultiByteToWideChar(CP_UTF8, 0, dev_name, -1, lpwDevName, MAXPNAMELEN);
#endif

        for(n = 0; n < nDevs; n++)
        {
            WAVEOUTCAPS caps;
            MMRESULT    res;

            res = waveOutGetDevCaps(n, &caps, sizeof(caps));

            if(res == MMSYSERR_NOERROR)
            {
#ifdef _UNICODE

                if(wcsicmp(lpwDevName, caps.szPname) == 0)
#else
                if(FLUID_STRCASECMP(dev_name, caps.szPname) == 0)
#endif
                {
                    FLUID_LOG(FLUID_DBG, "Selected audio device GUID: %s", dev_name);
                    device = n;
                    break;
                }
            }
        }
    }

    do
    {

        dev->hQuit = CreateEvent(NULL, FALSE, FALSE, NULL);

        if(dev->hQuit == NULL)
        {
            FLUID_LOG(FLUID_ERR, "Failed to create quit event: '%s'", fluid_get_windows_error());
            break;
        }

        /* Create thread which processes re-adding SYSEX buffers */
        dev->hThread = CreateThread(
                           NULL,
                           0,
                           (LPTHREAD_START_ROUTINE)
                           fluid_waveout_synth_thread,
                           dev,
                           0,
                           &dev->dwThread);

        if(dev->hThread == NULL)
        {
            FLUID_LOG(FLUID_ERR, "Failed to create waveOut thread: '%s'", fluid_get_windows_error());
            break;
        }

        errCode = waveOutOpen(&dev->hWaveOut,
                              device,
                              (WAVEFORMATEX *)&wfx,
                              (DWORD_PTR)dev->dwThread,
                              0,
                              CALLBACK_THREAD);

        if(errCode != MMSYSERR_NOERROR)
        {
            FLUID_LOG(FLUID_ERR, "Failed to open waveOut device: '%s'", fluid_waveout_error(errCode));
            break;
        }

        /* Get pointer to sound buffer memory */
        ptrBuffer = (LPSTR)(dev + 1);

        /* Setup the sample buffers */
        for(i = 0; i < periods; i++)
        {
            /* Clear the sample buffer */
            memset(ptrBuffer, 0, lenBuffer);

            /* Clear descriptor buffer */
            memset(dev->waveHeader + i, 0, sizeof(WAVEHDR));

            /* Compile descriptor buffer */
            dev->waveHeader[i].lpData         = ptrBuffer;
            dev->waveHeader[i].dwBufferLength = lenBuffer;
            dev->waveHeader[i].dwUser         = (DWORD_PTR)dev;

            waveOutPrepareHeader(dev->hWaveOut, &dev->waveHeader[i], sizeof(WAVEHDR));

            ptrBuffer += lenBuffer;
        }

        /* Play the sample buffers */
        for(i = 0; i < periods; i++)
        {
            waveOutWrite(dev->hWaveOut, &dev->waveHeader[i], sizeof(WAVEHDR));
        }

        return (fluid_audio_driver_t *) dev;

    }
    while(0);

    delete_fluid_waveout_audio_driver(&dev->driver);
    return NULL;
}


void delete_fluid_waveout_audio_driver(fluid_audio_driver_t *d)
{
    int i;

    fluid_waveout_audio_driver_t *dev = (fluid_waveout_audio_driver_t *) d;
    fluid_return_if_fail(dev != NULL);

    /* release all the allocated resources */
    if(dev->hWaveOut != NULL)
    {
        dev->nQuit = dev->periods;
        WaitForSingleObject(dev->hQuit, INFINITE);

        waveOutClose(dev->hWaveOut);
    }

    if(dev->hThread != NULL)
    {
        PostThreadMessage(dev->dwThread, WM_CLOSE, 0, 0);
        WaitForSingleObject(dev->hThread, INFINITE);

        CloseHandle(dev->hThread);
    }

    if(dev->hQuit != NULL)
    {
        CloseHandle(dev->hQuit);
    }

    if(dev->drybuf != NULL)
    {
        for(i = 0; i < dev->channels_count; ++i)
        {
            FLUID_FREE(dev->drybuf[i]);
        }
    }

    FLUID_FREE(dev->drybuf);

    HeapFree(GetProcessHeap(), 0, dev);
}

static int fluid_waveout_write_processed_channels(fluid_synth_t *data, int len,
                               int channels_count,
                               void *channels_out[], int channels_off[],
                               int channels_incr[])
{
    int i, ch;
    int ret;
    fluid_waveout_audio_driver_t *drv = (fluid_waveout_audio_driver_t*) data;
    float *optr[WAVEOUT_MAX_STEREO_CHANNELS * 2];
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

static char *fluid_waveout_error(MMRESULT hr)
{
    char *s = "Don't know why";

    switch(hr)
    {
    case MMSYSERR_NOERROR:
        s = "The operation completed successfully :)";
        break;

    case MMSYSERR_ALLOCATED:
        s = "Specified resource is already allocated.";
        break;

    case MMSYSERR_BADDEVICEID:
        s = "Specified device identifier is out of range";
        break;

    case MMSYSERR_NODRIVER:
        s = "No device driver is present";
        break;

    case MMSYSERR_NOMEM:
        s = "Unable to allocate or lock memory";
        break;

    case WAVERR_BADFORMAT:
        s = "Attempted to open with an unsupported waveform-audio format";
        break;

    case WAVERR_SYNC:
        s = "The device is synchronous but waveOutOpen was called without using the WAVE_ALLOWSYNC flag";
        break;
    }

    return s;
}

#endif /* WAVEOUT_SUPPORT */
