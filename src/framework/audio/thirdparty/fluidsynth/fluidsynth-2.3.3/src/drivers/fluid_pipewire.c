/* FluidSynth - A Software Synthesizer
 *
 * Copyright (C) 2003  Peter Hanappe and others.
 * Copyright (C) 2021  E. "sykhro" Melucci
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

/* fluid_pipewire.c
 *
 * Audio driver for PipeWire.
 *
 */

#include "fluid_synth.h"
#include "fluid_adriver.h"
#include "fluid_settings.h"

#if PIPEWIRE_SUPPORT

#include <pipewire/pipewire.h>
#include <spa/param/audio/format-utils.h>

/* At the moment, only stereo is supported */
#define NUM_CHANNELS 2
static const int stride = sizeof(float) * NUM_CHANNELS;

typedef struct
{
    fluid_audio_driver_t driver;
    fluid_audio_func_t user_callback;
    void *data;

    /* Used only with the user-provided callback */
    float *lbuf, *rbuf;

    int buffer_period;
    struct spa_pod_builder *builder;

    struct pw_thread_loop *pw_loop;
    struct pw_stream *pw_stream;
    struct pw_stream_events *events;

} fluid_pipewire_audio_driver_t;


/* Fast-path rendering routine with no user processing callbacks */
static void fluid_pipewire_event_process(void *data)
{
    fluid_pipewire_audio_driver_t *drv = data;
    struct pw_buffer *pwb;
    struct spa_buffer *buf;
    float *dest;

    pwb = pw_stream_dequeue_buffer(drv->pw_stream);

    if(!pwb)
    {
        FLUID_LOG(FLUID_WARN, "No buffers!");
        return;
    }

    buf = pwb->buffer;
    dest = buf->datas[0].data;

    if(!dest)
    {
        return;
    }

    fluid_synth_write_float(drv->data, drv->buffer_period, dest, 0, 2, dest, 1, 2);
    buf->datas[0].chunk->offset = 0;
    buf->datas[0].chunk->stride = stride;
    buf->datas[0].chunk->size = drv->buffer_period * stride;

    pw_stream_queue_buffer(drv->pw_stream, pwb);
}

/* Rendering routine with support for user-defined audio manipulation */
static void fluid_pipewire_event_process2(void *data)
{
    fluid_pipewire_audio_driver_t *drv = data;
    struct pw_buffer *pwb;
    struct spa_buffer *buf;
    float *dest;
    float *channels[NUM_CHANNELS] = { drv->lbuf, drv->rbuf };
    int i;

    pwb = pw_stream_dequeue_buffer(drv->pw_stream);

    if(!pwb)
    {
        FLUID_LOG(FLUID_WARN, "No buffers!");
        return;
    }

    buf = pwb->buffer;
    dest = buf->datas[0].data;

    if(!dest)
    {
        return;
    }

    FLUID_MEMSET(drv->lbuf, 0, drv->buffer_period * sizeof(float));
    FLUID_MEMSET(drv->rbuf, 0, drv->buffer_period * sizeof(float));

    (*drv->user_callback)(drv->data, drv->buffer_period, 0, NULL, NUM_CHANNELS, channels);

    /* Interleave the floating point data */
    for(i = 0; i < drv->buffer_period; i++)
    {
        dest[i * 2] = drv->lbuf[i];
        dest[i * 2 + 1] = drv->rbuf[i];
    }

    buf->datas[0].chunk->offset = 0;
    buf->datas[0].chunk->stride = stride;
    buf->datas[0].chunk->size = drv->buffer_period * stride;

    pw_stream_queue_buffer(drv->pw_stream, pwb);
}

fluid_audio_driver_t *new_fluid_pipewire_audio_driver(fluid_settings_t *settings, fluid_synth_t *synth)
{
    return new_fluid_pipewire_audio_driver2(settings, NULL, synth);
}

fluid_audio_driver_t *
new_fluid_pipewire_audio_driver2(fluid_settings_t *settings, fluid_audio_func_t func, void *data)
{
    fluid_pipewire_audio_driver_t *drv;
    int period_size;
    int buffer_length;
    int res;
    int pw_flags;
    int realtime_prio = 0;
    double sample_rate;
    char *media_role = NULL;
    char *media_type = NULL;
    char *media_category = NULL;
    float *buffer = NULL;
    const struct spa_pod *params[1];

    drv = FLUID_NEW(fluid_pipewire_audio_driver_t);

    if(!drv)
    {
        FLUID_LOG(FLUID_ERR, "Out of memory");
        return NULL;
    }

    FLUID_MEMSET(drv, 0, sizeof(*drv));

    fluid_settings_getint(settings, "audio.period-size", &period_size);
    fluid_settings_getint(settings, "audio.realtime-prio", &realtime_prio);
    fluid_settings_getnum(settings, "synth.sample-rate", &sample_rate);
    fluid_settings_dupstr(settings, "audio.pipewire.media-role", &media_role);
    fluid_settings_dupstr(settings, "audio.pipewire.media-type", &media_type);
    fluid_settings_dupstr(settings, "audio.pipewire.media-category", &media_category);

    drv->data = data;
    drv->user_callback = func;
    drv->buffer_period = period_size;

    drv->events = FLUID_NEW(struct pw_stream_events);

    if(!drv->events)
    {
        FLUID_LOG(FLUID_ERR, "Out of memory");
        goto driver_cleanup;
    }

    FLUID_MEMSET(drv->events, 0, sizeof(*drv->events));
    drv->events->version = PW_VERSION_STREAM_EVENTS;
    drv->events->process = func ? fluid_pipewire_event_process2 : fluid_pipewire_event_process;

    drv->pw_loop = pw_thread_loop_new("fluid_pipewire", NULL);

    if(!drv->pw_loop)
    {
        FLUID_LOG(FLUID_ERR, "Failed to allocate PipeWire loop. Have you called pw_init() ?");
        goto driver_cleanup;
    }

    struct pw_properties *props = pw_properties_new(PW_KEY_MEDIA_TYPE, media_type, PW_KEY_MEDIA_CATEGORY, media_category, PW_KEY_MEDIA_ROLE, media_role, NULL);

    pw_properties_setf(props, PW_KEY_NODE_LATENCY, "%d/%d", period_size, (int) sample_rate);
    pw_properties_setf(props, PW_KEY_NODE_RATE, "1/%d", (int) sample_rate);

    drv->pw_stream = pw_stream_new_simple(
                         pw_thread_loop_get_loop(drv->pw_loop),
                         "FluidSynth",
                         props,
                         drv->events,
                         drv);

    if(!drv->pw_stream)
    {
        FLUID_LOG(FLUID_ERR, "Failed to allocate PipeWire stream");
        goto driver_cleanup;
    }

    buffer = FLUID_ARRAY(float, NUM_CHANNELS * period_size);

    if(!buffer)
    {
        FLUID_LOG(FLUID_ERR, "Out of memory");
        goto driver_cleanup;
    }

    buffer_length = period_size * sizeof(float) * NUM_CHANNELS;

    drv->builder = FLUID_NEW(struct spa_pod_builder);

    if(!drv->builder)
    {
        FLUID_LOG(FLUID_ERR, "Out of memory");
        goto driver_cleanup;
    }

    FLUID_MEMSET(drv->builder, 0, sizeof(*drv->builder));
    drv->builder->data = buffer;
    drv->builder->size = buffer_length;

    if(func)
    {
        drv->lbuf = FLUID_ARRAY(float, period_size);
        drv->rbuf = FLUID_ARRAY(float, period_size);

        if(!drv->lbuf || !drv->rbuf)
        {
            FLUID_LOG(FLUID_ERR, "Out of memory");
            goto driver_cleanup;
        }
    }

    params[0] = spa_format_audio_raw_build(drv->builder,
                                           SPA_PARAM_EnumFormat,
                                           &SPA_AUDIO_INFO_RAW_INIT(.format = SPA_AUDIO_FORMAT_F32,
                                                   .channels = 2,
                                                   .rate = sample_rate));

    pw_flags = PW_STREAM_FLAG_AUTOCONNECT | PW_STREAM_FLAG_MAP_BUFFERS;
    pw_flags |= realtime_prio ? PW_STREAM_FLAG_RT_PROCESS : 0;
    res = pw_stream_connect(drv->pw_stream, PW_DIRECTION_OUTPUT, PW_ID_ANY, pw_flags, params, 1);

    if(res < 0)
    {
        FLUID_LOG(FLUID_ERR, "PipeWire stream connection failed");
        goto driver_cleanup;
    }

    res = pw_thread_loop_start(drv->pw_loop);

    if(res != 0)
    {
        FLUID_LOG(FLUID_ERR, "Failed starting PipeWire loop");
        goto driver_cleanup;
    }

    FLUID_LOG(FLUID_INFO, "Using PipeWire audio driver");

    FLUID_FREE(media_role);
    FLUID_FREE(media_type);
    FLUID_FREE(media_category);

    return (fluid_audio_driver_t *)drv;

driver_cleanup:
    FLUID_FREE(media_role);
    FLUID_FREE(media_type);
    FLUID_FREE(media_category);

    delete_fluid_pipewire_audio_driver((fluid_audio_driver_t *)drv);
    return NULL;
}

void delete_fluid_pipewire_audio_driver(fluid_audio_driver_t *p)
{
    fluid_pipewire_audio_driver_t *drv = (fluid_pipewire_audio_driver_t *)p;
    fluid_return_if_fail(drv);

    if(drv->pw_stream)
    {
        pw_stream_destroy(drv->pw_stream);
    }

    if(drv->pw_loop)
    {
        pw_thread_loop_destroy(drv->pw_loop);
    }

    FLUID_FREE(drv->lbuf);
    FLUID_FREE(drv->rbuf);

    if(drv->builder)
    {
        FLUID_FREE(drv->builder->data);
    }

    FLUID_FREE(drv->builder);

    FLUID_FREE(drv->events);

    FLUID_FREE(drv);
}

void fluid_pipewire_audio_driver_settings(fluid_settings_t *settings)
{
    fluid_settings_register_str(settings, "audio.pipewire.media-role", "Music", 0);
    fluid_settings_register_str(settings, "audio.pipewire.media-type", "Audio", 0);
    fluid_settings_register_str(settings, "audio.pipewire.media-category", "Playback", 0);
}

#endif
