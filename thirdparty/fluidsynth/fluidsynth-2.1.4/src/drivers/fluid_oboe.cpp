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

/* fluid_oboe.c
 *
 * Audio driver for Android Oboe.
 *
 */

extern "C" {

#include "fluid_adriver.h"
#include "fluid_settings.h"

} // extern "C"

#if OBOE_SUPPORT

#include <oboe/Oboe.h>

using namespace oboe;

static const int NUM_CHANNELS = 2;

class OboeAudioStreamCallback;

/** fluid_oboe_audio_driver_t
 *
 * This structure should not be accessed directly. Use audio port
 * functions instead.
 */
typedef struct
{
    fluid_audio_driver_t driver;
    fluid_synth_t *synth;
    int cont;
    OboeAudioStreamCallback *oboe_callback;
    AudioStream *stream;
} fluid_oboe_audio_driver_t;


class OboeAudioStreamCallback : public AudioStreamCallback
{
public:

    OboeAudioStreamCallback(void *userData)
        : user_data(userData)
    {
    }

    DataCallbackResult onAudioReady(AudioStream *stream, void *audioData, int32_t numFrames)
    {
        fluid_oboe_audio_driver_t *dev = static_cast<fluid_oboe_audio_driver_t *>(this->user_data);

        if(!dev->cont)
        {
            return DataCallbackResult::Stop;
        }

        if(stream->getFormat() == AudioFormat::Float)
        {
            fluid_synth_write_float(dev->synth, numFrames, static_cast<float *>(audioData), 0, 2, static_cast<float *>(audioData), 1, 2);
        }
        else
        {
            fluid_synth_write_s16(dev->synth, numFrames, static_cast<short *>(audioData), 0, 2, static_cast<short *>(audioData), 1, 2);
        }

        return DataCallbackResult::Continue;
    }

private:
    void *user_data;
};

void fluid_oboe_audio_driver_settings(fluid_settings_t *settings)
{
    fluid_settings_register_int(settings, "audio.oboe.id", 0, 0, 0x7FFFFFFF, 0);

    fluid_settings_register_str(settings, "audio.oboe.sharing-mode", "Shared", 0);
    fluid_settings_add_option(settings,   "audio.oboe.sharing-mode", "Shared");
    fluid_settings_add_option(settings,   "audio.oboe.sharing-mode", "Exclusive");

    fluid_settings_register_str(settings, "audio.oboe.performance-mode", "None", 0);
    fluid_settings_add_option(settings,   "audio.oboe.performance-mode", "None");
    fluid_settings_add_option(settings,   "audio.oboe.performance-mode", "PowerSaving");
    fluid_settings_add_option(settings,   "audio.oboe.performance-mode", "LowLatency");
}


/*
 * new_fluid_oboe_audio_driver
 */
fluid_audio_driver_t *
new_fluid_oboe_audio_driver(fluid_settings_t *settings, fluid_synth_t *synth)
{
    Result result;
    fluid_oboe_audio_driver_t *dev;
    AudioStreamBuilder builder_obj;
    AudioStreamBuilder *builder = &builder_obj;
    AudioStream *stream;

    int period_frames;
    double sample_rate;
    int is_sample_format_float;
    int device_id;
    int sharing_mode; // 0: Shared, 1: Exclusive
    int performance_mode; // 0: None, 1: PowerSaving, 2: LowLatency

    try
    {
        dev = FLUID_NEW(fluid_oboe_audio_driver_t);

        if(dev == NULL)
        {
            FLUID_LOG(FLUID_ERR, "Out of memory");
            return NULL;
        }

        FLUID_MEMSET(dev, 0, sizeof(fluid_oboe_audio_driver_t));

        dev->synth = synth;
        dev->oboe_callback = new(std::nothrow) OboeAudioStreamCallback(dev);

        if(!dev->oboe_callback)
        {
            FLUID_LOG(FLUID_ERR, "Out of memory");
            goto error_recovery;
        }

        fluid_settings_getint(settings, "audio.period-size", &period_frames);
        fluid_settings_getnum(settings, "synth.sample-rate", &sample_rate);
        is_sample_format_float = fluid_settings_str_equal(settings, "audio.sample-format", "float");
        fluid_settings_getint(settings, "audio.oboe.id", &device_id);
        sharing_mode =
            fluid_settings_str_equal(settings, "audio.oboe.sharing-mode", "Exclusive") ? 1 : 0;
        performance_mode =
            fluid_settings_str_equal(settings, "audio.oboe.performance-mode", "PowerSaving") ? 1 :
            fluid_settings_str_equal(settings, "audio.oboe.performance-mode", "LowLatency") ? 2 : 0;

        builder->setDeviceId(device_id)
        ->setDirection(Direction::Output)
        ->setChannelCount(NUM_CHANNELS)
        ->setSampleRate(sample_rate)
        ->setFramesPerCallback(period_frames)
        ->setFormat(is_sample_format_float ? AudioFormat::Float : AudioFormat::I16)
        ->setSharingMode(sharing_mode == 1 ? SharingMode::Exclusive : SharingMode::Shared)
        ->setPerformanceMode(
            performance_mode == 1 ? PerformanceMode::PowerSaving :
            performance_mode == 2 ? PerformanceMode::LowLatency : PerformanceMode::None)
        ->setUsage(Usage::Media)
        ->setContentType(ContentType::Music)
        ->setCallback(dev->oboe_callback);

        result = builder->openStream(&stream);
        if(result != Result::OK)
        {
            FLUID_LOG(FLUID_ERR, "Unable to open Oboe audio stream");
            goto error_recovery;
        }

        dev->stream = stream;
        dev->cont = 1;

        FLUID_LOG(FLUID_INFO, "Using Oboe driver");

        result = stream->start();
        if(result != Result::OK)
        {
            FLUID_LOG(FLUID_ERR, "Unable to start Oboe audio stream");
            goto error_recovery;
        }

        return reinterpret_cast<fluid_audio_driver_t *>(dev);
    }
    catch(...)
    {
        FLUID_LOG(FLUID_ERR, "Unexpected Oboe driver initialization error");
    }

error_recovery:
    delete_fluid_oboe_audio_driver(reinterpret_cast<fluid_audio_driver_t *>(dev));
    return NULL;
}

void delete_fluid_oboe_audio_driver(fluid_audio_driver_t *p)
{
    fluid_oboe_audio_driver_t *dev = reinterpret_cast<fluid_oboe_audio_driver_t *>(p);

    fluid_return_if_fail(dev != NULL);

    try
    {
        dev->cont = 0;

        if(dev->stream != NULL)
        {
            dev->stream->stop();
            dev->stream->close();
        }
    }
    catch(...) {}

    // the audio stream is silently allocated with new, but neither the API docs nor code examples mention that it should be deleted
    delete dev->stream;

    delete dev->oboe_callback;

    FLUID_FREE(dev);
}

#endif // OBOE_SUPPORT

