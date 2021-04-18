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

/* fluid_portaudio.c
 *
 * Drivers for the PortAudio API : www.portaudio.com
 * Implementation files for PortAudio on each platform have to be added
 *
 * Stephane Letz  (letz@grame.fr)  Grame
 * 12/20/01 Adapdation for new audio drivers
 *
 * Josh Green <jgreen@users.sourceforge.net>
 * 2009-01-28 Overhauled for PortAudio 19 API and current FluidSynth API (was broken)
 */

#include "fluid_synth.h"
#include "fluid_settings.h"
#include "fluid_adriver.h"

#if PORTAUDIO_SUPPORT

#include <portaudio.h>


/** fluid_portaudio_driver_t
 *
 * This structure should not be accessed directly. Use audio port
 * functions instead.
 */
typedef struct
{
    fluid_audio_driver_t driver;
    fluid_synth_t *synth;
    fluid_audio_callback_t read;
    PaStream *stream;
} fluid_portaudio_driver_t;

static int
fluid_portaudio_run(const void *input, void *output, unsigned long frameCount,
                    const PaStreamCallbackTimeInfo *timeInfo,
                    PaStreamCallbackFlags statusFlags, void *userData);

#define PORTAUDIO_DEFAULT_DEVICE "PortAudio Default"

/**
 * Checks if device_num is a valid device and returns the name of the portaudio device.
 * A device is valid if it is an output device with at least 2 channels.
 *
 * @param device_num index of the portaudio device to check.
 * @param name_ptr if device_num is valid, set to a unique device name, ignored otherwise
 *
 * The name returned is unique for each num_device index, so this
 * name is useful to identify any available host audio device.
 * This name is convenient for audio.portaudio.device setting.
 *
 * The format of the name is: device_index:host_api_name:host_device_name
 *
 *   example: 5:MME:SB PCI
 *
 *   5: is the portaudio device index.
 *   MME: is the host API name.
 *   SB PCI: is the host device name.
 *
 * @return #FLUID_OK if device_num is a valid output device, #FLUID_FAILED otherwise.
 * When #FLUID_OK, the name is returned in allocated memory. The caller must check
 * the name pointer for a valid memory allocation and should free the memory.
 */
static int fluid_portaudio_get_device_name(int device_num, char **name_ptr)
{
    const PaDeviceInfo *deviceInfo =  Pa_GetDeviceInfo(device_num);

    if(deviceInfo->maxOutputChannels >= 2)
    {
        const PaHostApiInfo *hostInfo = Pa_GetHostApiInfo(deviceInfo->hostApi);
        /* The size of the buffer name for the following format:
           device_index:host_api_name:host_device_name.
        */
        int i =  device_num;
        size_t size = 0;

        do
        {
            size++;
            i = i / 10 ;
        }
        while(i);		/*  index size */

        /* host API size +  host device size + 2 separators + zero termination */
        size += FLUID_STRLEN(hostInfo->name) + FLUID_STRLEN(deviceInfo->name) + 3u;
        *name_ptr = FLUID_MALLOC(size);

        if(*name_ptr)
        {
            /* the name is filled if allocation is successful */
            FLUID_SPRINTF(*name_ptr, "%d:%s:%s", device_num,
                          hostInfo->name, deviceInfo->name);
        }

        return FLUID_OK; /* device_num is a valid device */
    }
    else
    {
        return FLUID_FAILED;    /* device_num is an invalid device */
    }
}

/**
 * Initializes "audio.portaudio.device" setting with an options list of unique device names
 * of available sound card devices.
 * @param settings pointer to settings.
 */
void
fluid_portaudio_driver_settings(fluid_settings_t *settings)
{
    int numDevices;
    PaError err;
    int i;

    fluid_settings_register_str(settings, "audio.portaudio.device", PORTAUDIO_DEFAULT_DEVICE, 0);
    fluid_settings_add_option(settings, "audio.portaudio.device", PORTAUDIO_DEFAULT_DEVICE);

    err = Pa_Initialize();

    if(err != paNoError)
    {
        FLUID_LOG(FLUID_ERR, "Error initializing PortAudio driver: %s",
                  Pa_GetErrorText(err));
        return;
    }

    numDevices = Pa_GetDeviceCount();

    if(numDevices < 0)
    {
        FLUID_LOG(FLUID_ERR, "PortAudio returned unexpected device count %d", numDevices);
    }
    else
    {
        for(i = 0; i < numDevices; i++)
        {
            char *name;

            if(fluid_portaudio_get_device_name(i, &name) == FLUID_OK)
            {
                /* the device i is a valid output device */
                if(name)
                {
                    /* registers this name in the option list */
                    fluid_settings_add_option(settings, "audio.portaudio.device", name);
                    FLUID_FREE(name);
                }
                else
                {
                    FLUID_LOG(FLUID_ERR, "Out of memory");
                    break;
                }
            }
        }
    }

    /* done with PortAudio for now, may get reopened later */
    err = Pa_Terminate();

    if(err != paNoError)
    {
        printf("PortAudio termination error: %s\n", Pa_GetErrorText(err));
    }
}

/**
 * Creates the portaudio driver and opens the portaudio device
 * indicated by audio.portaudio.device setting.
 *
 * @param settings pointer to settings
 * @param synth the synthesizer instance
 * @return pointer to the driver on success, NULL otherwise.
 */
fluid_audio_driver_t *
new_fluid_portaudio_driver(fluid_settings_t *settings, fluid_synth_t *synth)
{
    fluid_portaudio_driver_t *dev = NULL;
    PaStreamParameters outputParams;
    char *device = NULL; /* the portaudio device name to work with */
    double sample_rate;  /* intended sample rate */
    int period_size;     /* intended buffer size */
    PaError err;

    dev = FLUID_NEW(fluid_portaudio_driver_t);

    if(dev == NULL)
    {
        FLUID_LOG(FLUID_ERR, "Out of memory");
        return NULL;
    }

    err = Pa_Initialize();

    if(err != paNoError)
    {
        FLUID_LOG(FLUID_ERR, "Error initializing PortAudio driver: %s",
                  Pa_GetErrorText(err));
        FLUID_FREE(dev);
        return NULL;
    }

    FLUID_MEMSET(dev, 0, sizeof(fluid_portaudio_driver_t));

    dev->synth = synth;

    /* gets audio parameters from the settings */
    fluid_settings_getint(settings, "audio.period-size", &period_size);
    fluid_settings_getnum(settings, "synth.sample-rate", &sample_rate);
    fluid_settings_dupstr(settings, "audio.portaudio.device", &device);   /* ++ alloc device name */

    memset(&outputParams, 0, sizeof(outputParams));
    outputParams.channelCount = 2; /* For stereo output */
    outputParams.suggestedLatency = (PaTime)period_size / sample_rate;

    /* Locate the device if specified */
    if(FLUID_STRCMP(device, PORTAUDIO_DEFAULT_DEVICE) != 0)
    {
        /* The intended device is not the default device name, so we search
        a device among available devices */
        int numDevices;
        int i;

        numDevices = Pa_GetDeviceCount();

        if(numDevices < 0)
        {
            FLUID_LOG(FLUID_ERR, "PortAudio returned unexpected device count %d", numDevices);
            goto error_recovery;
        }

        for(i = 0; i < numDevices; i++)
        {
            char *name;

            if(fluid_portaudio_get_device_name(i, &name) == FLUID_OK)
            {
                /* the device i is a valid output device */
                if(name)
                {
                    /* We see if the name corresponds to audio.portaudio.device */
                    char found = (FLUID_STRCMP(device, name) == 0);
                    FLUID_FREE(name);

                    if(found)
                    {
                        /* the device index is found */
                        outputParams.device = i;
                        /* The search is finished */
                        break;
                    }
                }
                else
                {
                    FLUID_LOG(FLUID_ERR, "Out of memory");
                    goto error_recovery;
                }
            }
        }

        if(i == numDevices)
        {
            FLUID_LOG(FLUID_ERR, "PortAudio device '%s' was not found", device);
            goto error_recovery;
        }
    }
    else
    {
        /* the default device will be used */
        outputParams.device = Pa_GetDefaultOutputDevice();
    }

    /* The device is found. We set the sample format and the audio rendering
       function suited to this format.
    */
    if(fluid_settings_str_equal(settings, "audio.sample-format", "16bits"))
    {
        outputParams.sampleFormat = paInt16;
        dev->read = fluid_synth_write_s16;
    }
    else if(fluid_settings_str_equal(settings, "audio.sample-format", "float"))
    {
        outputParams.sampleFormat = paFloat32;
        dev->read = fluid_synth_write_float;
    }
    else
    {
        FLUID_LOG(FLUID_ERR, "Unknown sample format");
        goto error_recovery;
    }

    /* PortAudio section */

    /* Open an audio I/O stream. */
    err = Pa_OpenStream(&dev->stream,
                        NULL,              /* Input parameters */
                        &outputParams,     /* Output parameters */
                        sample_rate,
                        period_size,
                        paNoFlag,
                        fluid_portaudio_run, /* callback */
                        dev);

    if(err != paNoError)
    {
        FLUID_LOG(FLUID_ERR, "Error opening PortAudio stream: %s",
                  Pa_GetErrorText(err));
        goto error_recovery;
    }

    err = Pa_StartStream(dev->stream);  /* starts the I/O stream */

    if(err != paNoError)
    {
        FLUID_LOG(FLUID_ERR, "Error starting PortAudio stream: %s",
                  Pa_GetErrorText(err));
        goto error_recovery;
    }

    if(device)
    {
        FLUID_FREE(device);    /* -- free device name */
    }

    return (fluid_audio_driver_t *)dev;

error_recovery:

    if(device)
    {
        FLUID_FREE(device);    /* -- free device name */
    }

    delete_fluid_portaudio_driver((fluid_audio_driver_t *)dev);
    return NULL;
}

/* PortAudio callback
 * fluid_portaudio_run
 */
static int
fluid_portaudio_run(const void *input, void *output, unsigned long frameCount,
                    const PaStreamCallbackTimeInfo *timeInfo,
                    PaStreamCallbackFlags statusFlags, void *userData)
{
    fluid_portaudio_driver_t *dev = (fluid_portaudio_driver_t *)userData;
    /* it's as simple as that: */
    dev->read(dev->synth, frameCount, output, 0, 2, output, 1, 2);
    return 0;
}

/*
 * delete_fluid_portaudio_driver
 */
void
delete_fluid_portaudio_driver(fluid_audio_driver_t *p)
{
    fluid_portaudio_driver_t *dev = (fluid_portaudio_driver_t *)p;
    PaError err;
    fluid_return_if_fail(dev != NULL);

    /* PortAudio section */
    if(dev->stream)
    {
        Pa_CloseStream(dev->stream);
    }

    err = Pa_Terminate();

    if(err != paNoError)
    {
        printf("PortAudio termination error: %s\n", Pa_GetErrorText(err));
    }

    FLUID_FREE(dev);
}

#endif /* PORTAUDIO_SUPPORT */
