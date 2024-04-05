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

/* fluid_pulse.c
 *
 * Audio driver for PulseAudio.
 *
 */

#include "fluid_synth.h"
#include "fluid_adriver.h"
#include "fluid_settings.h"

#if PULSE_SUPPORT

#include <pulse/simple.h>
#include <pulse/error.h>

/** fluid_pulse_audio_driver_t
 *
 * This structure should not be accessed directly. Use audio port
 * functions instead.
 */
typedef struct
{
    fluid_audio_driver_t driver;
    pa_simple *pa_handle;
    fluid_audio_func_t callback;
    void *data;
    int buffer_size;
    fluid_thread_t *thread;
    int cont;

    float *left;
    float *right;
    float *buf;
} fluid_pulse_audio_driver_t;


static fluid_thread_return_t fluid_pulse_audio_run(void *d);
static fluid_thread_return_t fluid_pulse_audio_run2(void *d);


void fluid_pulse_audio_driver_settings(fluid_settings_t *settings)
{
    fluid_settings_register_str(settings, "audio.pulseaudio.server", "default", 0);
    fluid_settings_register_str(settings, "audio.pulseaudio.device", "default", 0);
    fluid_settings_register_str(settings, "audio.pulseaudio.media-role", "music", 0);
    fluid_settings_register_int(settings, "audio.pulseaudio.adjust-latency", 1, 0, 1,
                                FLUID_HINT_TOGGLED);
}


fluid_audio_driver_t *
new_fluid_pulse_audio_driver(fluid_settings_t *settings,
                             fluid_synth_t *synth)
{
    return new_fluid_pulse_audio_driver2(settings, NULL, synth);
}

fluid_audio_driver_t *
new_fluid_pulse_audio_driver2(fluid_settings_t *settings,
                              fluid_audio_func_t func, void *data)
{
    fluid_pulse_audio_driver_t *dev;
    pa_sample_spec samplespec;
    pa_buffer_attr bufattr;
    double sample_rate;
    int period_size, period_bytes, adjust_latency, periods;
    char *server = NULL;
    char *device = NULL;
    char *media_role = NULL;
    int realtime_prio = 0;
    int err = 0;
    float *left = NULL,
           *right = NULL,
            *buf = NULL;

    dev = FLUID_NEW(fluid_pulse_audio_driver_t);

    if(dev == NULL)
    {
        FLUID_LOG(FLUID_ERR, "Out of memory");
        return NULL;
    }

    FLUID_MEMSET(dev, 0, sizeof(fluid_pulse_audio_driver_t));

    fluid_settings_getint(settings, "audio.periods", &periods);
    fluid_settings_getint(settings, "audio.period-size", &period_size);
    fluid_settings_getnum(settings, "synth.sample-rate", &sample_rate);
    fluid_settings_dupstr(settings, "audio.pulseaudio.server", &server);  /* ++ alloc server string */
    fluid_settings_dupstr(settings, "audio.pulseaudio.device", &device);  /* ++ alloc device string */
    fluid_settings_dupstr(settings, "audio.pulseaudio.media-role", &media_role);  /* ++ alloc media-role string */
    fluid_settings_getint(settings, "audio.realtime-prio", &realtime_prio);
    fluid_settings_getint(settings, "audio.pulseaudio.adjust-latency", &adjust_latency);

    if(media_role != NULL)
    {
        if(FLUID_STRCMP(media_role, "") != 0)
        {
            g_setenv("PULSE_PROP_media.role", media_role, TRUE);
        }

        FLUID_FREE(media_role);       /* -- free media_role string */
    }

    if(server && FLUID_STRCMP(server, "default") == 0)
    {
        FLUID_FREE(server);         /* -- free server string */
        server = NULL;
    }

    if(device && FLUID_STRCMP(device, "default") == 0)
    {
        FLUID_FREE(device);         /* -- free device string */
        device = NULL;
    }

    dev->data = data;
    dev->callback = func;
    dev->cont = 1;
    dev->buffer_size = period_size;

    samplespec.format = PA_SAMPLE_FLOAT32NE;
    samplespec.channels = 2;
    samplespec.rate = sample_rate;

    period_bytes = period_size * sizeof(float) * 2;
    bufattr.maxlength = adjust_latency ? -1 : period_bytes * periods;
    bufattr.tlength = period_bytes;
    bufattr.minreq = -1;
    bufattr.prebuf = -1;    /* Just initialize to same value as tlength */
    bufattr.fragsize = -1;  /* Not used */

    dev->pa_handle = pa_simple_new(server, "FluidSynth", PA_STREAM_PLAYBACK,
                                   device, "FluidSynth output", &samplespec,
                                   NULL, /* pa_channel_map */
                                   &bufattr,
                                   &err);

    if(!dev->pa_handle || err != PA_OK)
    {
        FLUID_LOG(FLUID_ERR, "Failed to create PulseAudio connection, because pa_simple_new() failed with error: %s", pa_strerror(err));
        goto error_recovery;
    }

    FLUID_LOG(FLUID_INFO, "Using PulseAudio driver");

    if(func != NULL)
    {
        left = FLUID_ARRAY(float, period_size);
        right = FLUID_ARRAY(float, period_size);

        if(left == NULL || right == NULL)
        {
            FLUID_LOG(FLUID_ERR, "Out of memory.");
            goto error_recovery;
        }
    }

    buf = FLUID_ARRAY(float, period_size * 2);

    if(buf == NULL)
    {
        FLUID_LOG(FLUID_ERR, "Out of memory.");
        goto error_recovery;
    }

    dev->left = left;
    dev->right = right;
    dev->buf = buf;

    /* Create the audio thread */
    dev->thread = new_fluid_thread("pulse-audio", func ? fluid_pulse_audio_run2 : fluid_pulse_audio_run,
                                   dev, realtime_prio, FALSE);

    if(!dev->thread)
    {
        goto error_recovery;
    }

    FLUID_FREE(server);    /* -- free server string */
    FLUID_FREE(device);    /* -- free device string */

    return (fluid_audio_driver_t *) dev;

error_recovery:
    FLUID_FREE(server);    /* -- free server string */
    FLUID_FREE(device);    /* -- free device string */
    FLUID_FREE(left);
    FLUID_FREE(right);
    FLUID_FREE(buf);

    delete_fluid_pulse_audio_driver((fluid_audio_driver_t *) dev);
    return NULL;
}

void delete_fluid_pulse_audio_driver(fluid_audio_driver_t *p)
{
    fluid_pulse_audio_driver_t *dev = (fluid_pulse_audio_driver_t *) p;
    fluid_return_if_fail(dev != NULL);

    dev->cont = 0;

    if(dev->thread)
    {
        fluid_thread_join(dev->thread);
        delete_fluid_thread(dev->thread);
    }

    if(dev->pa_handle)
    {
        pa_simple_free(dev->pa_handle);
    }

    FLUID_FREE(dev->left);
    FLUID_FREE(dev->right);
    FLUID_FREE(dev->buf);

    FLUID_FREE(dev);
}

/* Thread without audio callback, more efficient */
static fluid_thread_return_t
fluid_pulse_audio_run(void *d)
{
    fluid_pulse_audio_driver_t *dev = (fluid_pulse_audio_driver_t *) d;
    float *buf = dev->buf;
    int buffer_size = dev->buffer_size;
    int err = 0;

    while(dev->cont)
    {
        fluid_synth_write_float(dev->data, buffer_size, buf, 0, 2, buf, 1, 2);

        if(pa_simple_write(dev->pa_handle, buf,
                           buffer_size * sizeof(float) * 2, &err) < 0)
        {
            FLUID_LOG(FLUID_ERR, "Error writing to PulseAudio connection: %s", pa_strerror(err));
            break;
        }

#if 0
        {
            pa_usec_t pa_latency = pa_simple_get_latency(dev->pa_handle, &err);

            if(err == PA_OK)
            {
                FLUID_LOG(FLUID_DBG, "PulseAudio latency: %d ms", (int) pa_latency / 1000);
            }
        }
#endif
    }	/* while (dev->cont) */

    return FLUID_THREAD_RETURN_VALUE;
}

static fluid_thread_return_t
fluid_pulse_audio_run2(void *d)
{
    fluid_pulse_audio_driver_t *dev = (fluid_pulse_audio_driver_t *) d;
    fluid_synth_t *synth = (fluid_synth_t *)(dev->data);
    float *left = dev->left,
           *right = dev->right,
            *buf = dev->buf;
    float *handle[2];
    int buffer_size = dev->buffer_size;
    int err = 0;
    int i;

    handle[0] = left;
    handle[1] = right;

    while(dev->cont)
    {
        FLUID_MEMSET(left, 0, buffer_size * sizeof(float));
        FLUID_MEMSET(right, 0, buffer_size * sizeof(float));

        (*dev->callback)(synth, buffer_size, 0, NULL, 2, handle);

        /* Interleave the floating point data */
        for(i = 0; i < buffer_size; i++)
        {
            buf[i * 2] = left[i];
            buf[i * 2 + 1] = right[i];
        }

        if(pa_simple_write(dev->pa_handle, buf,
                           buffer_size * sizeof(float) * 2, &err) < 0)
        {
            FLUID_LOG(FLUID_ERR, "Error writing to PulseAudio connection: %s", pa_strerror(err));
            break;
        }

#if 0
        {
            pa_usec_t pa_latency = pa_simple_get_latency(dev->pa_handle, &err);

            if(err == PA_OK)
            {
                FLUID_LOG(FLUID_DBG, "PulseAudio latency: %d ms", (int) pa_latency / 1000);
            }
        }
#endif
    }	/* while (dev->cont) */

    return FLUID_THREAD_RETURN_VALUE;
}

#endif /* PULSE_SUPPORT */

