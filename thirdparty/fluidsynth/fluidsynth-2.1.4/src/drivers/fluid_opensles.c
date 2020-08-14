/* FluidSynth - A Software Synthesizer
 *
 * Copyright (C) 2003  Peter Hanappe and others.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public License
 * as published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this library; if not, write to the Free
 * Software Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301, USA
 */

/* fluid_opensles.c
 *
 * Audio driver for OpenSLES.
 *
 */

#include "fluid_adriver.h"

#if OPENSLES_SUPPORT

#include <SLES/OpenSLES.h>
#include <SLES/OpenSLES_Android.h>

static const int NUM_CHANNELS = 2;

/** fluid_opensles_audio_driver_t
 *
 * This structure should not be accessed directly. Use audio port
 * functions instead.
 */
typedef struct
{
    fluid_audio_driver_t driver;
    SLObjectItf engine;
    SLObjectItf output_mix_object;
    SLObjectItf audio_player;
    SLPlayItf audio_player_interface;
    SLAndroidSimpleBufferQueueItf player_buffer_queue_interface;

    void *synth;
    int period_frames;

    int is_sample_format_float;

    /* used only by callback mode */
    short *sles_buffer_short;
    float *sles_buffer_float;

    int cont;

    double sample_rate;
} fluid_opensles_audio_driver_t;


static void opensles_callback(SLAndroidSimpleBufferQueueItf caller, void *pContext);
static void process_fluid_buffer(fluid_opensles_audio_driver_t *dev);

void fluid_opensles_audio_driver_settings(fluid_settings_t *settings)
{
}


/*
 * new_fluid_opensles_audio_driver
 */
fluid_audio_driver_t *
new_fluid_opensles_audio_driver(fluid_settings_t *settings, fluid_synth_t *synth)
{
    SLresult result;
    fluid_opensles_audio_driver_t *dev;
    double sample_rate;
    int period_size;
    int realtime_prio = 0;
    int is_sample_format_float;
    SLEngineItf engine_interface;

    dev = FLUID_NEW(fluid_opensles_audio_driver_t);

    if(dev == NULL)
    {
        FLUID_LOG(FLUID_ERR, "Out of memory");
        return NULL;
    }

    FLUID_MEMSET(dev, 0, sizeof(*dev));

    fluid_settings_getint(settings, "audio.period-size", &period_size);
    fluid_settings_getnum(settings, "synth.sample-rate", &sample_rate);
    fluid_settings_getint(settings, "audio.realtime-prio", &realtime_prio);
    is_sample_format_float = fluid_settings_str_equal(settings, "audio.sample-format", "float");

    dev->synth = synth;
    dev->is_sample_format_float = is_sample_format_float;
    dev->period_frames = period_size;
    dev->sample_rate = sample_rate;
    dev->cont = 1;

    result = slCreateEngine(&(dev->engine), 0, NULL, 0, NULL, NULL);

    if(!dev->engine)
    {
        FLUID_LOG(FLUID_ERR, "Failed to create the OpenSL ES engine, error code 0x%lx", (unsigned long)result);
        goto error_recovery;
    }

    result = (*dev->engine)->Realize(dev->engine, SL_BOOLEAN_FALSE);

    if(result != SL_RESULT_SUCCESS)
    {
        FLUID_LOG(FLUID_ERR, "Failed to realize the OpenSL ES engine, error code 0x%lx", (unsigned long)result);
        goto error_recovery;
    }

    result = (*dev->engine)->GetInterface(dev->engine, SL_IID_ENGINE, &engine_interface);

    if(result != SL_RESULT_SUCCESS)
    {
        FLUID_LOG(FLUID_ERR, "Failed to retrieve the OpenSL ES engine interface, error code 0x%lx", (unsigned long)result);
        goto error_recovery;
    }

    result = (*engine_interface)->CreateOutputMix(engine_interface, &dev->output_mix_object, 0, 0, 0);

    if(result != SL_RESULT_SUCCESS)
    {
        FLUID_LOG(FLUID_ERR, "Failed to create the OpenSL ES output mix object, error code 0x%lx", (unsigned long)result);
        goto error_recovery;
    }

    result = (*dev->output_mix_object)->Realize(dev->output_mix_object, SL_BOOLEAN_FALSE);

    if(result != SL_RESULT_SUCCESS)
    {
        FLUID_LOG(FLUID_ERR, "Failed to realize the OpenSL ES output mix object, error code 0x%lx", (unsigned long)result);
        goto error_recovery;
    }

    {
        SLDataLocator_AndroidSimpleBufferQueue loc_buffer_queue =
        {
            SL_DATALOCATOR_ANDROIDSIMPLEBUFFERQUEUE,
            2 /* number of buffers */
        };
        SLAndroidDataFormat_PCM_EX format_pcm =
        {
            SL_ANDROID_DATAFORMAT_PCM_EX,
            NUM_CHANNELS,
            ((SLuint32) sample_rate) * 1000,
            is_sample_format_float ? SL_PCMSAMPLEFORMAT_FIXED_32 : SL_PCMSAMPLEFORMAT_FIXED_16,
            is_sample_format_float ? SL_PCMSAMPLEFORMAT_FIXED_32 : SL_PCMSAMPLEFORMAT_FIXED_16,
            SL_SPEAKER_FRONT_LEFT | SL_SPEAKER_FRONT_RIGHT,
            SL_BYTEORDER_LITTLEENDIAN,
            is_sample_format_float ? SL_ANDROID_PCM_REPRESENTATION_FLOAT : SL_ANDROID_PCM_REPRESENTATION_SIGNED_INT
        };
        SLDataSource audio_src =
        {
            &loc_buffer_queue,
            &format_pcm
        };

        SLDataLocator_OutputMix loc_outmix =
        {
            SL_DATALOCATOR_OUTPUTMIX,
            dev->output_mix_object
        };
        SLDataSink audio_sink = {&loc_outmix, NULL};

        const SLInterfaceID ids1[] = {SL_IID_ANDROIDSIMPLEBUFFERQUEUE};
        const SLboolean req1[] = {SL_BOOLEAN_TRUE};
        result = (*engine_interface)->CreateAudioPlayer(engine_interface,
                &(dev->audio_player), &audio_src, &audio_sink, 1, ids1, req1);
    }

    if(result != SL_RESULT_SUCCESS)
    {
        FLUID_LOG(FLUID_ERR, "Failed to create the OpenSL ES audio player object, error code 0x%lx", (unsigned long)result);
        goto error_recovery;
    }

    result = (*dev->audio_player)->Realize(dev->audio_player, SL_BOOLEAN_FALSE);

    if(result != SL_RESULT_SUCCESS)
    {
        FLUID_LOG(FLUID_ERR, "Failed to realize the OpenSL ES audio player object, error code 0x%lx", (unsigned long)result);
        goto error_recovery;
    }

    result = (*dev->audio_player)->GetInterface(dev->audio_player,
             SL_IID_PLAY, &(dev->audio_player_interface));

    if(result != SL_RESULT_SUCCESS)
    {
        FLUID_LOG(FLUID_ERR, "Failed to retrieve the OpenSL ES audio player interface, error code 0x%lx", (unsigned long)result);
        goto error_recovery;
    }

    result = (*dev->audio_player)->GetInterface(dev->audio_player,
             SL_IID_ANDROIDSIMPLEBUFFERQUEUE, &(dev->player_buffer_queue_interface));

    if(result != SL_RESULT_SUCCESS)
    {
        FLUID_LOG(FLUID_ERR, "Failed to retrieve the OpenSL ES buffer queue interface, error code 0x%lx", (unsigned long)result);
        goto error_recovery;
    }

    if(dev->is_sample_format_float)
    {
        dev->sles_buffer_float = FLUID_ARRAY(float, dev->period_frames * NUM_CHANNELS);
    }
    else
    {
        dev->sles_buffer_short = FLUID_ARRAY(short, dev->period_frames * NUM_CHANNELS);
    }

    if(dev->sles_buffer_float == NULL && dev->sles_buffer_short == NULL)
    {
        FLUID_LOG(FLUID_ERR, "Out of memory.");
        goto error_recovery;
    }

    result = (*dev->player_buffer_queue_interface)->RegisterCallback(dev->player_buffer_queue_interface, opensles_callback, dev);

    if(result != SL_RESULT_SUCCESS)
    {
        FLUID_LOG(FLUID_ERR, "Failed to register the opensles_callback, error code 0x%lx", (unsigned long)result);
        goto error_recovery;
    }

    if(dev->is_sample_format_float)
    {
        result = (*dev->player_buffer_queue_interface)->Enqueue(dev->player_buffer_queue_interface, dev->sles_buffer_float, dev->period_frames * NUM_CHANNELS * sizeof(float));
    }
    else
    {
        result = (*dev->player_buffer_queue_interface)->Enqueue(dev->player_buffer_queue_interface, dev->sles_buffer_short, dev->period_frames * NUM_CHANNELS * sizeof(short));
    }

    if(result != SL_RESULT_SUCCESS)
    {
        FLUID_LOG(FLUID_ERR, "Failed to add a buffer to the queue, error code 0x%lx", (unsigned long)result);
        goto error_recovery;
    }

    result = (*dev->audio_player_interface)->SetCallbackEventsMask(dev->audio_player_interface, SL_PLAYEVENT_HEADATEND);

    if(result != SL_RESULT_SUCCESS)
    {
        FLUID_LOG(FLUID_ERR, "Failed to set OpenSL ES audio player callback events, error code 0x%lx", (unsigned long)result);
        goto error_recovery;
    }

    result = (*dev->audio_player_interface)->SetPlayState(dev->audio_player_interface, SL_PLAYSTATE_PLAYING);

    if(result != SL_RESULT_SUCCESS)
    {
        FLUID_LOG(FLUID_ERR, "Failed to set OpenSL ES audio player play state to playing, error code 0x%lx", (unsigned long)result);
        goto error_recovery;
    }

    FLUID_LOG(FLUID_INFO, "Using OpenSLES driver.");

    return (fluid_audio_driver_t *) dev;

error_recovery:

    delete_fluid_opensles_audio_driver((fluid_audio_driver_t *) dev);
    return NULL;
}

void delete_fluid_opensles_audio_driver(fluid_audio_driver_t *p)
{
    fluid_opensles_audio_driver_t *dev = (fluid_opensles_audio_driver_t *) p;

    fluid_return_if_fail(dev != NULL);

    dev->cont = 0;

    if(dev->audio_player)
    {
        (*dev->audio_player)->Destroy(dev->audio_player);
    }

    if(dev->output_mix_object)
    {
        (*dev->output_mix_object)->Destroy(dev->output_mix_object);
    }

    if(dev->engine)
    {
        (*dev->engine)->Destroy(dev->engine);
    }

    if(dev->sles_buffer_float)
    {
        FLUID_FREE(dev->sles_buffer_float);
    }

    if(dev->sles_buffer_short)
    {
        FLUID_FREE(dev->sles_buffer_short);
    }

    FLUID_FREE(dev);
}

void opensles_callback(SLAndroidSimpleBufferQueueItf caller, void *pContext)
{
    fluid_opensles_audio_driver_t *dev = (fluid_opensles_audio_driver_t *) pContext;
    SLresult result;

    process_fluid_buffer(dev);

    if(dev->is_sample_format_float)
    {
        result = (*caller)->Enqueue(
                     dev->player_buffer_queue_interface, dev->sles_buffer_float, dev->period_frames * sizeof(float) * NUM_CHANNELS);
    }
    else
    {
        result = (*caller)->Enqueue(
                     dev->player_buffer_queue_interface, dev->sles_buffer_short, dev->period_frames * sizeof(short) * NUM_CHANNELS);
    }

    /*
    if (result != SL_RESULT_SUCCESS)
    {
      // Do not simply break at just one single insufficient buffer. Go on.
    }
    */
}

void process_fluid_buffer(fluid_opensles_audio_driver_t *dev)
{
    short *out_short = dev->sles_buffer_short;
    float *out_float = dev->sles_buffer_float;
    int period_frames = dev->period_frames;

    if(dev->is_sample_format_float)
    {
        fluid_synth_write_float(dev->synth, period_frames, out_float, 0, 2, out_float, 1, 2);
    }
    else
    {
        fluid_synth_write_s16(dev->synth, period_frames, out_short, 0, 2, out_short, 1, 2);
    }
}

#endif
