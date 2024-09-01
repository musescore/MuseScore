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

#define ALSA_DEFAULT_DEVICE_ID "default"

using namespace muse::audio;

static void* alsaThread(void* aParam)
{
    muse::runtime::setThreadName("audio_driver");
    AlsaDriverState* state = static_cast<AlsaDriverState*>(aParam);

    int ret = snd_pcm_wait(static_cast<snd_pcm_t*>(state->alsaDeviceHandle), 1000);
    IF_ASSERT_FAILED(ret > 0) {
        return nullptr;
    }

    while (!state->audioProcessingDone)
    {
        uint8_t* stream = (uint8_t*)state->buffer;
        int len = state->deviceSpec.samples * state->deviceSpec.channels * sizeof(float);

        state->deviceSpec.callback(state->deviceSpec.userdata, stream, len);

        snd_pcm_sframes_t pcm = snd_pcm_writei(static_cast<snd_pcm_t*>(state->alsaDeviceHandle), state->buffer, state->deviceSpec.samples);
        if (pcm != -EPIPE) {
        } else {
            snd_pcm_prepare(static_cast<snd_pcm_t*>(state->alsaDeviceHandle));
        }
    }

    LOGI() << "exit";
    return nullptr;
}

AlsaDriverState::AlsaDriverState()
{
    deviceId = "alsa";
    m_deviceName = ALSA_DEFAULT_DEVICE_ID;
}

AlsaDriverState::~AlsaDriverState()
{
    alsaCleanup();
}

void AlsaDriverState::alsaCleanup()
{
    audioProcessingDone = true;
    if (m_threadHandle) {
        pthread_join(m_threadHandle, nullptr);
    }
    if (alsaDeviceHandle != nullptr) {
        snd_pcm_t* aDevice = static_cast<snd_pcm_t*>(alsaDeviceHandle);
        snd_pcm_drain(aDevice);
        snd_pcm_close(aDevice);
        alsaDeviceHandle = nullptr;
    }
    if (buffer) {
        delete[] buffer;
    }
    buffer = nullptr;
}

std::string AlsaDriverState::name() const
{
    return deviceId;
}

std::string AlsaDriverState::deviceName() const
{
    return m_deviceName;
}

void AlsaDriverState::deviceName(const std::string newDeviceName)
{
    m_deviceName = newDeviceName;
}

void AlsaDriverState::setAudioDelayCompensate([[maybe_unused]] const int frames)
{
}

bool AlsaDriverState::open(const IAudioDriver::Spec& spec, IAudioDriver::Spec* activeSpec)
{
    deviceSpec.samples = spec.samples;
    deviceSpec.channels = spec.channels;
    deviceSpec.callback = spec.callback;
    deviceSpec.userdata = spec.userdata;

    snd_pcm_t* handle;
    int rc = snd_pcm_open(&handle, m_deviceName.c_str(), SND_PCM_STREAM_PLAYBACK, 0);
    if (rc < 0) {
        LOGE() << "Unable to open device: " << outputDevice() << ", err code: " << rc;
        return false;
    }

    alsaDeviceHandle = handle;

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
        LOGE() << "Unable to set sample rate: " << val << ", err code: " << rc;
        return false;
    }

    long unsigned int samples = deviceSpec.samples;
    rc = snd_pcm_hw_params_set_buffer_size_near(handle, params, &samples);
    if (rc < 0) {
        LOGE() << "Unable to set buffer size: " << samples << ", err code: " << rc;
    }
    deviceSpec.samples = (int)samples;

    rc = snd_pcm_hw_params(handle, params);
    if (rc < 0) {
        return false;
    }

    snd_pcm_hw_params_get_rate(params, &val, &dir);
    aSamplerate = val;
    deviceSpec.sampleRate = aSamplerate;

    if (buffer != nullptr) {
        LOGW() << "open before close";
        delete[] buffer;
        buffer = nullptr;
    }

    buffer = new float[deviceSpec.samples * deviceSpec.channels];

    if (activeSpec) {
        *activeSpec = spec;
        activeSpec->format = IAudioDriver::Format::AudioF32;
        activeSpec->sampleRate = aSamplerate;
        deviceSpec = *activeSpec;
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
    return alsaDeviceHandle != nullptr;
}

void AlsaDriverState::changedPlaying() const
{
}

void AlsaDriverState::changedPosition(muse::audio::secs_t secs, muse::midi::tick_t tick) const
{
}

bool AlsaDriverState::pushMidiEvent(muse::midi::Event&)
{
    return true; // dummy
}

void AlsaDriverState::registerMidiInputQueue(async::Channel<muse::midi::tick_t, muse::midi::Event > midiInputQueue)
{
    m_eventReceived = midiInputQueue;
}

std::vector<muse::midi::MidiDevice> AlsaDriverState::availableMidiDevices([[maybe_unused]] muse::midi::MidiPortDirection direction) const
{
    std::vector<muse::midi::MidiDevice> x;
    return x; // dummy
}
