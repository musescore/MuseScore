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

using namespace muse::audio;

static void* alsaThread(void* aParam)
{
    muse::runtime::setThreadName("audio_driver");
    AlsaDriverState* data = static_cast<AlsaDriverState*>(aParam);

    int ret = snd_pcm_wait(static_cast<snd_pcm_t*>(data->m_alsaDeviceHandle), 1000);
    IF_ASSERT_FAILED(ret > 0) {
        return nullptr;
    }

    while (!data->m_audioProcessingDone)
    {
        uint8_t* stream = (uint8_t*)data->m_buffer;
        int len = data->m_samples * data->m_channels * sizeof(float);

        data->m_callback(data->m_userdata, stream, len);

        snd_pcm_sframes_t pcm = snd_pcm_writei(static_cast<snd_pcm_t*>(data->m_alsaDeviceHandle), data->m_buffer, data->m_samples);
        if (pcm != -EPIPE) {
        } else {
            snd_pcm_prepare(static_cast<snd_pcm_t*>(data->m_alsaDeviceHandle));
        }
    }

    LOGI() << "exit";
    return nullptr;
}

void AlsaDriverState::alsaCleanup()
{
    m_audioProcessingDone = true;
    if (m_threadHandle) {
        pthread_join(m_threadHandle, nullptr);
    }
    if (m_alsaDeviceHandle != nullptr) {
        snd_pcm_t* aDevice = static_cast<snd_pcm_t*>(m_alsaDeviceHandle);
        snd_pcm_drain(aDevice);
        snd_pcm_close(aDevice);
        m_alsaDeviceHandle = nullptr;
    }

    delete[] m_buffer;
}

AlsaDriverState::AlsaDriverState()
{
    m_deviceId = "alsa";
}

AlsaDriverState::~AlsaDriverState()
{
    alsaCleanup();
}

std::string AlsaDriverState::name() const
{
    return "MUAUDIO(ALSA)";
}

bool AlsaDriverState::open(const IAudioDriver::Spec& spec, IAudioDriver::Spec* activeSpec)
{
    m_samples = spec.samples;
    m_channels = spec.channels;
    m_callback = spec.callback;
    m_userdata = spec.userdata;

    int rc;
    snd_pcm_t* handle;
    rc = snd_pcm_open(&handle, name().c_str(), SND_PCM_STREAM_PLAYBACK, 0);
    if (rc < 0) {
        return false;
    }

    m_alsaDeviceHandle = handle;

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

    snd_pcm_hw_params_set_buffer_size_near(handle, params, &m_samples);

    rc = snd_pcm_hw_params(handle, params);
    if (rc < 0) {
        return false;
    }

    snd_pcm_hw_params_get_rate(params, &val, &dir);
    aSamplerate = val;

    if (m_buffer != nullptr) {
        LOGW() << "open before close";
        delete[] m_buffer;
    }

    m_buffer = new float[m_samples * m_channels];

    if (activeSpec) {
        *activeSpec = spec;
        activeSpec->format = IAudioDriver::Format::AudioF32;
        activeSpec->sampleRate = aSamplerate;
        m_format = *activeSpec;
    }

    m_threadHandle = 0;
    int ret = pthread_create(&m_threadHandle, NULL, alsaThread, (void*)this);

    if (0 != ret) {
        return false;
    }

    LOGD() << "Connected to " << name();
    return true;
}

void AlsaDriverState::close()
{
    alsaCleanup();
}

bool AlsaDriverState::isOpened() const
{
    return m_alsaDeviceHandle != nullptr;
}
