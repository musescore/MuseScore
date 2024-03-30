/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-CLA-applies
 *
 * MuseScore
 * Music Composition & Notation
 *
 * Copyright (C) 2023 MuseScore BVBA and others
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 3 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */
#include "alsaaudiodriver.h"

#define ALSA_PCM_NEW_HW_PARAMS_API
#include <alsa/asoundlib.h>

#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <math.h>
#include <pthread.h>

#include "translation.h"
#include "log.h"
#include "runtime.h"

static constexpr char DEFAULT_DEVICE_ID[] = "alsa";

using namespace muse::audio;

namespace {
static void* alsaThread(void* aParam)
{
    muse::runtime::setThreadName("audio_driver");
    ALSADriverState* data = static_cast<ALSADriverState*>(aParam);

    int ret = snd_pcm_wait(static_cast<snd_pcm_t*>(data->alsaDeviceHandle), 1000);
    IF_ASSERT_FAILED(ret > 0) {
        return nullptr;
    }

    while (!data->audioProcessingDone)
    {
        uint8_t* stream = (uint8_t*)data->buffer;
        int len = data->samples * data->channels * sizeof(float);

        data->callback(data->userdata, stream, len);

        snd_pcm_sframes_t pcm = snd_pcm_writei(static_cast<snd_pcm_t*>(data->alsaDeviceHandle), data->buffer, data->samples);
        if (pcm != -EPIPE) {
        } else {
            snd_pcm_prepare(static_cast<snd_pcm_t*>(data->alsaDeviceHandle));
        }
    }

    LOGI() << "exit";
    return nullptr;
}
}

void AlsaAudioDriver::alsaCleanup()
{
    m_alsaDriverState->audioProcessingDone = true;
    if (m_alsaDriverState->threadHandle) {
        pthread_join(m_alsaDriverState->threadHandle, nullptr);
    }
    if (m_alsaDriverState->alsaDeviceHandle != nullptr) {
        snd_pcm_t* alsaDeviceHandle = static_cast<snd_pcm_t*>(m_alsaDriverState->alsaDeviceHandle);
        snd_pcm_drain(alsaDeviceHandle);
        snd_pcm_close(alsaDeviceHandle);
        m_alsaDriverState->alsaDeviceHandle = nullptr;
    }

    delete[] m_alsaDriverState->buffer;
}

AlsaAudioDriver::AlsaAudioDriver()
{
    m_alsaDriverState = std::make_shared<ALSADriverState>();
    m_deviceId = DEFAULT_DEVICE_ID;
}

AlsaAudioDriver::~AlsaAudioDriver()
{
    alsaCleanup();
}

std::string AlsaAudioDriver::name() const
{
    return "MUAUDIO(ALSA)";
}

bool AlsaAudioDriver::open(const Spec& spec, Spec* activeSpec)
{
    m_alsaDriverState->samples = spec.samples;
    m_alsaDriverState->channels = spec.channels;
    m_alsaDriverState->callback = spec.callback;
    m_alsaDriverState->userdata = spec.userdata;

    int rc;
    snd_pcm_t* handle;
    rc = snd_pcm_open(&handle, outputDevice().c_str(), SND_PCM_STREAM_PLAYBACK, 0);
    if (rc < 0) {
        return false;
    }

    m_alsaDriverState->alsaDeviceHandle = handle;

    snd_pcm_hw_params_t* params;
    snd_pcm_hw_params_alloca(&params);
    snd_pcm_hw_params_any(handle, params);

    snd_pcm_hw_params_set_access(handle, params, SND_PCM_ACCESS_RW_INTERLEAVED);
    snd_pcm_hw_params_set_format(handle, params, SND_PCM_FORMAT_FLOAT_LE);
    snd_pcm_hw_params_set_channels(handle, params, spec.channels);

    unsigned int aSamplerate = spec.sampleRate;
    unsigned int val = aSamplerate;
    int dir = 0;
    rc = snd_pcm_hw_params_set_rate_near(handle, params, &val, &dir);
    if (rc < 0) {
        return false;
    }

    snd_pcm_hw_params_set_buffer_size_near(handle, params, &m_alsaDriverState->samples);

    rc = snd_pcm_hw_params(handle, params);
    if (rc < 0) {
        return false;
    }

    snd_pcm_hw_params_get_rate(params, &val, &dir);
    aSamplerate = val;

    if (m_alsaDriverState->buffer != nullptr) {
        LOGW() << "open before close";
        delete[] m_alsaDriverState->buffer;
    }

    m_alsaDriverState->buffer = new float[m_alsaDriverState->samples * m_alsaDriverState->channels];
    //m_alsaDriverState->sampleBuffer = new short[m_alsaDriverState->samples * m_alsaDriverState->channels];

    if (activeSpec) {
        *activeSpec = spec;
        activeSpec->format = Format::AudioF32;
        activeSpec->sampleRate = aSamplerate;
        m_alsaDriverState->format = *activeSpec;
    }

    m_alsaDriverState->threadHandle = 0;
    int ret = pthread_create(&m_alsaDriverState->threadHandle, NULL, alsaThread, (void*)m_alsaDriverState.get());

    if (0 != ret) {
        return false;
    }

    LOGD() << "Connected to " << outputDevice();
    return true;
}

void AlsaAudioDriver::close()
{
    alsaCleanup();
}

bool AlsaAudioDriver::isOpened() const
{
    return m_alsaDriverState->alsaDeviceHandle != nullptr;
}
