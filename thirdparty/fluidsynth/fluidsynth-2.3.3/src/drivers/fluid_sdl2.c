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

#if SDL2_SUPPORT

#include "SDL.h"

typedef struct
{
    fluid_audio_driver_t driver;

    fluid_synth_t *synth;
    fluid_audio_callback_t write_ptr;

    SDL_AudioDeviceID devid;

    int frame_size;

} fluid_sdl2_audio_driver_t;


static void
SDLAudioCallback(void *data, void *stream, int len)
{
    fluid_sdl2_audio_driver_t *dev = (fluid_sdl2_audio_driver_t *)data;

    len /= dev->frame_size;

    dev->write_ptr(dev->synth, len, stream, 0, 2, stream, 1, 2);
}

void fluid_sdl2_audio_driver_settings(fluid_settings_t *settings)
{
    int n, nDevs;

    fluid_settings_register_str(settings, "audio.sdl2.device", "default", 0);
    fluid_settings_add_option(settings, "audio.sdl2.device", "default");

    if(!SDL_WasInit(SDL_INIT_AUDIO))
    {
#if FLUID_VERSION_CHECK(FLUIDSYNTH_VERSION_MAJOR, FLUIDSYNTH_VERSION_MINOR, FLUIDSYNTH_VERSION_MICRO) < FLUID_VERSION_CHECK(2,2,0)
        FLUID_LOG(FLUID_WARN, "SDL2 not initialized, SDL2 audio driver won't be usable");
#endif
        return;
    }

    nDevs = SDL_GetNumAudioDevices(0);

    for(n = 0; n < nDevs; n++)
    {
        const char *dev_name = SDL_GetAudioDeviceName(n, 0);

        if(dev_name != NULL)
        {
            FLUID_LOG(FLUID_DBG, "SDL2 driver testing audio device: %s", dev_name);
            fluid_settings_add_option(settings, "audio.sdl2.device", dev_name);
        }
    }
}


/*
 * new_fluid_sdl2_audio_driver
 */
fluid_audio_driver_t *
new_fluid_sdl2_audio_driver(fluid_settings_t *settings, fluid_synth_t *synth)
{
    fluid_sdl2_audio_driver_t *dev = NULL;
    fluid_audio_callback_t write_ptr;
    double sample_rate;
    int period_size, sample_size;
    SDL_AudioSpec aspec, rspec;
    char *device;
    const char *dev_name;

    /* Check if SDL library has been started */
    if(!SDL_WasInit(SDL_INIT_AUDIO))
    {
        FLUID_LOG(FLUID_ERR, "Failed to create SDL2 audio driver, because the audio subsystem of SDL2 is not initialized.");
        return NULL;
    }

    /* Retrieve the settings */
    fluid_settings_getnum(settings, "synth.sample-rate", &sample_rate);
    fluid_settings_getint(settings, "audio.period-size", &period_size);

    /* Lower values do not seem to give good results */
    if(period_size < 1024)
    {
        period_size = 1024;
    }
    else
    {
        /* According to documentation, it MUST be a power of two */
        if((period_size & (period_size - 1)) != 0)
        {
            FLUID_LOG(FLUID_ERR, "\"audio.period-size\" must be a power of 2 for SDL2");
            return NULL;
        }
    }
    /* Clear the format buffer */
    FLUID_MEMSET(&aspec, 0, sizeof(aspec));

    /* Setup mixing frequency */
    aspec.freq = (int)sample_rate;

    /* Check the format */
    if(fluid_settings_str_equal(settings, "audio.sample-format", "float"))
    {
        FLUID_LOG(FLUID_DBG, "Selected 32 bit sample format");

        sample_size = sizeof(float);
        write_ptr   = fluid_synth_write_float;

        aspec.format = AUDIO_F32SYS;
    }
    else if(fluid_settings_str_equal(settings, "audio.sample-format", "16bits"))
    {
        FLUID_LOG(FLUID_DBG, "Selected 16 bit sample format");

        sample_size = sizeof(short);
        write_ptr   = fluid_synth_write_s16;

        aspec.format = AUDIO_S16SYS;
    }
    else
    {
        FLUID_LOG(FLUID_ERR, "Unhandled sample format");
        return NULL;
    }

    /* Compile the format buffer */
    aspec.channels   = 2;
    aspec.samples    = aspec.channels * ((period_size + 7) & ~7);
    aspec.callback   = (SDL_AudioCallback)SDLAudioCallback;

    /* Set default device to use */
    device   = NULL;
    dev_name = NULL;

    /* get the selected device name. if none is specified, use default device. */
    if(fluid_settings_dupstr(settings, "audio.sdl2.device", &device) == FLUID_OK
            && device != NULL && device[0] != '\0')
    {
        int n, nDevs = SDL_GetNumAudioDevices(0);

        for(n = 0; n < nDevs; n++)
        {
            dev_name = SDL_GetAudioDeviceName(n, 0);

            if(FLUID_STRCASECMP(dev_name, device) == 0)
            {
                FLUID_LOG(FLUID_DBG, "Selected audio device GUID: %s", dev_name);
                break;
            }
        }

        if(n >= nDevs)
        {
            FLUID_LOG(FLUID_DBG, "Audio device %s, using \"default\"", device);
            dev_name = NULL;
        }
    }

    if(device != NULL)
    {
        FLUID_FREE(device);
    }

    do
    {
        /* create and clear the driver data */
        dev = FLUID_NEW(fluid_sdl2_audio_driver_t);

        if(dev == NULL)
        {
            FLUID_LOG(FLUID_ERR, "Out of memory");
            break;
        }

        FLUID_MEMSET(dev, 0, sizeof(fluid_sdl2_audio_driver_t));

        /* set device pointer to userdata */
        aspec.userdata = dev;

        /* Save copy of synth */
        dev->synth = synth;

        /* Save copy of other variables */
        dev->write_ptr = write_ptr;
        dev->frame_size = sample_size * aspec.channels;

        /* Open audio device */
        dev->devid = SDL_OpenAudioDevice(dev_name, 0, &aspec, &rspec, 0);

        if(!dev->devid)
        {
            FLUID_LOG(FLUID_ERR, "Failed to open audio device");
            break;
        }

        /* Start to play */
        SDL_PauseAudioDevice(dev->devid, 0);

        return (fluid_audio_driver_t *) dev;
    }
    while(0);

    delete_fluid_sdl2_audio_driver(&dev->driver);
    return NULL;
}


void delete_fluid_sdl2_audio_driver(fluid_audio_driver_t *d)
{
    fluid_sdl2_audio_driver_t *dev = (fluid_sdl2_audio_driver_t *) d;

    if(dev != NULL)
    {
        if(dev->devid)
        {
            /* Stop audio and close */
            SDL_PauseAudioDevice(dev->devid, 1);
            SDL_CloseAudioDevice(dev->devid);
        }

        FLUID_FREE(dev);
    }
}

#endif /* SDL2_SUPPORT */
