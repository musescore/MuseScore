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

#define NOBITMAP
#include <mmreg.h>

/* Number of buffers in the chain */
#define NB_SOUND_BUFFERS    4

/* Milliseconds of a single sound buffer */
#define MS_BUFFER_LENGTH    20

typedef struct
{
    fluid_audio_driver_t driver;

    fluid_synth_t *synth;
    fluid_audio_callback_t write_ptr;

    HWAVEOUT hWaveOut;
    WAVEHDR  waveHeader[NB_SOUND_BUFFERS];

    int sample_size;
    int num_frames;

    HANDLE hThread;
    DWORD  dwThread;

    int    nQuit;
    HANDLE hQuit;

} fluid_waveout_audio_driver_t;


/* Thread for playing sample buffers */
static DWORD WINAPI fluid_waveout_synth_thread(void *data)
{
    fluid_waveout_audio_driver_t *dev;
    WAVEHDR                      *pWave;

    MSG msg;
    int code;

    /* Forces creation of message queue */
    PeekMessage(&msg, NULL, WM_USER, WM_USER, PM_NOREMOVE);

    for(;;)
    {
        code = GetMessage(&msg, NULL, 0, 0);

        if(code < 0)
        {
            FLUID_LOG(FLUID_ERR, "fluid_waveout_synth_thread: GetMessage() failed.");
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
                dev->write_ptr(dev->synth, dev->num_frames, pWave->lpData, 0, 2, pWave->lpData, 1, 2);

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
 */
fluid_audio_driver_t *
new_fluid_waveout_audio_driver(fluid_settings_t *settings, fluid_synth_t *synth)
{
    fluid_waveout_audio_driver_t *dev = NULL;
    fluid_audio_callback_t write_ptr;
    double sample_rate;
    int periods, period_size, frequency, sample_size;
    LPSTR ptrBuffer;
    int lenBuffer;
    int device;
    int i;
    WAVEFORMATEX wfx;
    char dev_name[MAXPNAMELEN];
    MMRESULT errCode;

    /* Retrieve the settings */
    fluid_settings_getnum(settings, "synth.sample-rate", &sample_rate);
    fluid_settings_getint(settings, "audio.periods", &periods);
    fluid_settings_getint(settings, "audio.period-size", &period_size);

    /* Clear the format buffer */
    ZeroMemory(&wfx, sizeof(WAVEFORMATEX));

    /* check the format */
    if(fluid_settings_str_equal(settings, "audio.sample-format", "float"))
    {
        FLUID_LOG(FLUID_DBG, "Selected 32 bit sample format");

        sample_size = sizeof(float);
        write_ptr   = fluid_synth_write_float;

        wfx.wFormatTag = WAVE_FORMAT_IEEE_FLOAT;
    }
    else if(fluid_settings_str_equal(settings, "audio.sample-format", "16bits"))
    {
        FLUID_LOG(FLUID_DBG, "Selected 16 bit sample format");

        sample_size = sizeof(short);
        write_ptr   = fluid_synth_write_s16;

        wfx.wFormatTag = WAVE_FORMAT_PCM;
    }
    else
    {
        FLUID_LOG(FLUID_ERR, "Unhandled sample format");
        return NULL;
    }

    /* Set frequency to integer */
    frequency = (int)sample_rate;

    /* Compile the format buffer */
    wfx.nChannels       = 2;
    wfx.wBitsPerSample  = sample_size * 8;
    wfx.nSamplesPerSec  = frequency;
    wfx.nBlockAlign     = sample_size * wfx.nChannels;
    wfx.nAvgBytesPerSec = frequency * wfx.nBlockAlign;

    /* Calculate the length of a single buffer */
    lenBuffer = (MS_BUFFER_LENGTH * wfx.nAvgBytesPerSec + 999) / 1000;

    /* Round to 8-bytes size */
    lenBuffer = (lenBuffer + 7) & ~7;

    /* create and clear the driver data */
    dev = HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY,
                    sizeof(fluid_waveout_audio_driver_t) + lenBuffer * NB_SOUND_BUFFERS);

    if(dev == NULL)
    {
        FLUID_LOG(FLUID_ERR, "Out of memory");
        return NULL;
    }

    /* Save copy of synth */
    dev->synth = synth;

    /* Save copy of other variables */
    dev->write_ptr = write_ptr;
    dev->sample_size = sample_size;

    /* Calculate the number of frames in a block */
    dev->num_frames = lenBuffer / wfx.nBlockAlign;

    /* Set default device to use */
    device = WAVE_MAPPER;

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
            FLUID_LOG(FLUID_ERR, "Failed to create quit event");
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
            FLUID_LOG(FLUID_ERR, "Failed to create waveOut thread");
            break;
        }

        errCode = waveOutOpen(&dev->hWaveOut,
                              device,
                              &wfx,
                              (DWORD_PTR)dev->dwThread,
                              0,
                              CALLBACK_THREAD);

        if(errCode != MMSYSERR_NOERROR)
        {
            FLUID_LOG(FLUID_ERR, "Failed to open waveOut device");
            break;
        }

        /* Get pointer to sound buffer memory */
        ptrBuffer = (LPSTR)(dev + 1);

        /* Setup the sample buffers */
        for(i = 0; i < NB_SOUND_BUFFERS; i++)
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
        for(i = 0; i < NB_SOUND_BUFFERS; i++)
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
    fluid_waveout_audio_driver_t *dev = (fluid_waveout_audio_driver_t *) d;
    fluid_return_if_fail(dev != NULL);

    /* release all the allocated resources */
    if(dev->hWaveOut != NULL)
    {
        dev->nQuit = NB_SOUND_BUFFERS;
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

    HeapFree(GetProcessHeap(), 0, dev);
}

#endif /* WAVEOUT_SUPPORT */
