/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-CLA-applies
 *
 * MuseScore
 * Music Composition & Notation
 *
 * Copyright (C) 2021 MuseScore BVBA and others
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
#include "linuxaudiodriver.h"

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

static constexpr char DEFAULT_DEVICE_ID[] = "default";

using namespace mu::audio;

void LinuxAudioDriver::alsaCleanup()
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

LinuxAudioDriver::LinuxAudioDriver()
{
    m_alsaDriverState = std::make_unique<ALSADriverState>();
    m_deviceId = DEFAULT_DEVICE_ID;
}

LinuxAudioDriver::~LinuxAudioDriver()
{
    alsaCleanup();
}

void LinuxAudioDriver::init()
{
    m_devicesListener.startWithCallback([this]() {
        return availableOutputDevices();
    });

    m_devicesListener.devicesChanged().onNotify(this, [this]() {
        m_availableOutputDevicesChanged.notify();
    });
}

std::string LinuxAudioDriver::name() const
{
    return "MUAUDIO(ALSA)";
}

bool LinuxAudioDriver::open(const Spec& spec, Spec* activeSpec)
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

    m_alsaDriverState->buffer = new float[m_alsaDriverState->samples * m_alsaDriverState->channels];

    if (activeSpec) {
        *activeSpec = spec;
        activeSpec->format = Format::AudioF32;
        activeSpec->sampleRate = aSamplerate;
        m_alsaDriverState->format = *activeSpec;
    }

    m_alsaDriverState->threadHandle = 0;

    LOGD() << "Connected to " << outputDevice();
    return true;
}

void LinuxAudioDriver::close()
{
    alsaCleanup();
}

bool LinuxAudioDriver::isOpened() const
{
    return m_alsaDriverState->alsaDeviceHandle != nullptr;
}

AudioDeviceID LinuxAudioDriver::outputDevice() const
{
    return m_deviceId;
}

bool LinuxAudioDriver::selectOutputDevice(const AudioDeviceID& deviceId)
{
    if (m_deviceId == deviceId) {
        return true;
    }

    bool reopen = isOpened();
    close();
    m_deviceId = deviceId;

    bool ok = true;
    if (reopen) {
        ok = open(m_alsaDriverState->format, &m_alsaDriverState->format);
    }

    if (ok) {
        m_outputDeviceChanged.notify();
    }

    return ok;
}

bool LinuxAudioDriver::resetToDefaultOutputDevice()
{
    return selectOutputDevice(DEFAULT_DEVICE_ID);
}

mu::async::Notification LinuxAudioDriver::outputDeviceChanged() const
{
    return m_outputDeviceChanged;
}

AudioDeviceList LinuxAudioDriver::availableOutputDevices() const
{
    AudioDeviceList devices;
    devices.push_back({ DEFAULT_DEVICE_ID, trc("audio", "System default") });

    return devices;
}

mu::async::Notification LinuxAudioDriver::availableOutputDevicesChanged() const
{
    return m_availableOutputDevicesChanged;
}

unsigned int LinuxAudioDriver::outputDeviceBufferSize() const
{
    return m_alsaDriverState->format.samples;
}

bool LinuxAudioDriver::setOutputDeviceBufferSize(unsigned int bufferSize)
{
    if (m_alsaDriverState->format.samples == bufferSize) {
        return true;
    }

    bool reopen = isOpened();
    close();
    m_alsaDriverState->format.samples = bufferSize;

    bool ok = true;
    if (reopen) {
        ok = open(m_alsaDriverState->format, &m_alsaDriverState->format);
    }

    if (ok) {
        m_bufferSizeChanged.notify();
    }

    return ok;
}

mu::async::Notification LinuxAudioDriver::outputDeviceBufferSizeChanged() const
{
    return m_bufferSizeChanged;
}

std::vector<unsigned int> LinuxAudioDriver::availableOutputDeviceBufferSizes() const
{
    std::vector<unsigned int> result;

    unsigned int n = 4096;
    while (n >= MINIMUM_BUFFER_SIZE) {
        result.push_back(n);
        n /= 2;
    }

    std::sort(result.begin(), result.end());

    return result;
}

void LinuxAudioDriver::resume()
{
}

void LinuxAudioDriver::suspend()
{
}
