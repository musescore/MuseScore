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

using namespace muse;
using namespace muse::audio;

namespace {
struct ALSAData
{
    float* buffer = nullptr;
    snd_pcm_t* alsaDeviceHandle = nullptr;
    unsigned long samples = 0;
    int channels = 0;
    bool audioProcessingDone = false;
    pthread_t threadHandle = 0;
    IAudioDriver::Callback callback;
    void* userdata = nullptr;
};

static ALSAData* s_alsaData{ nullptr };
static muse::audio::IAudioDriver::Spec s_format;

static void* alsaThread(void* aParam)
{
    muse::runtime::setThreadName("audio_driver");
    ALSAData* data = static_cast<ALSAData*>(aParam);

    int ret = snd_pcm_wait(data->alsaDeviceHandle, 1000);
    IF_ASSERT_FAILED(ret > 0) {
        return nullptr;
    }

    while (!data->audioProcessingDone)
    {
        uint8_t* stream = (uint8_t*)data->buffer;
        int len = data->samples * data->channels * sizeof(float);

        data->callback(data->userdata, stream, len);

        snd_pcm_sframes_t pcm = snd_pcm_writei(data->alsaDeviceHandle, data->buffer, data->samples);
        if (pcm != -EPIPE) {
        } else {
            snd_pcm_prepare(data->alsaDeviceHandle);
        }
    }

    LOGI() << "exit";
    return nullptr;
}

static void alsaCleanup()
{
    if (!s_alsaData) {
        return;
    }

    s_alsaData->audioProcessingDone = true;
    if (s_alsaData->threadHandle) {
        pthread_join(s_alsaData->threadHandle, nullptr);
    }
    if (nullptr != s_alsaData->alsaDeviceHandle) {
        snd_pcm_drain(s_alsaData->alsaDeviceHandle);
        snd_pcm_close(s_alsaData->alsaDeviceHandle);
    }

    if (nullptr != s_alsaData->buffer) {
        delete[] s_alsaData->buffer;
    }

    delete s_alsaData;
    s_alsaData = nullptr;
}
}

AlsaAudioDriver::AlsaAudioDriver()
{
    m_deviceId = DEFAULT_DEVICE_ID;
}

AlsaAudioDriver::~AlsaAudioDriver()
{
    alsaCleanup();
}

void AlsaAudioDriver::init()
{
    m_devicesListener.startWithCallback([this]() {
        return availableOutputDevices();
    });

    m_devicesListener.devicesChanged().onNotify(this, [this]() {
        m_availableOutputDevicesChanged.notify();
    });
}

std::string AlsaAudioDriver::name() const
{
    return "MUAUDIO(ALSA)";
}

bool AlsaAudioDriver::open(const Spec& spec, Spec* activeSpec)
{
    s_alsaData = new ALSAData();
    s_alsaData->samples = spec.samples;
    s_alsaData->channels = spec.channels;
    s_alsaData->callback = spec.callback;
    s_alsaData->userdata = spec.userdata;

    snd_pcm_t* handle;
    int rc = snd_pcm_open(&handle, outputDevice().c_str(), SND_PCM_STREAM_PLAYBACK, 0);
    if (rc < 0) {
        LOGE() << "Unable to open device: " << outputDevice() << ", err code: " << rc;
        return false;
    }

    s_alsaData->alsaDeviceHandle = handle;

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

    rc = snd_pcm_hw_params_set_buffer_size_near(handle, params, &s_alsaData->samples);
    if (rc < 0) {
        LOGE() << "Unable to set buffer size: " << s_alsaData->samples << ", err code: " << rc;
    }

    rc = snd_pcm_hw_params(handle, params);
    if (rc < 0) {
        LOGE() << "Unable to set params, err code: " << rc;
        return false;
    }

    snd_pcm_hw_params_get_rate(params, &val, &dir);
    aSamplerate = val;

    s_alsaData->buffer = new float[s_alsaData->samples * s_alsaData->channels];
    //_alsaData->sampleBuffer = new short[_alsaData->samples * _alsaData->channels];

    if (activeSpec) {
        *activeSpec = spec;
        activeSpec->format = Format::AudioF32;
        activeSpec->sampleRate = aSamplerate;
        s_format = *activeSpec;
    }

    s_alsaData->threadHandle = 0;
    int ret = pthread_create(&s_alsaData->threadHandle, NULL, alsaThread, (void*)s_alsaData);

    if (0 != ret) {
        LOGE() << "Unable to create audio thread, err code: " << ret;
        return false;
    }

    LOGI() << "Connected to " << outputDevice()
           << " with bufferSize " << s_format.samples
           << ", sampleRate " << s_format.sampleRate
           << ", channels:  " << s_format.channels;

    return true;
}

void AlsaAudioDriver::close()
{
    alsaCleanup();
}

bool AlsaAudioDriver::isOpened() const
{
    return s_alsaData != nullptr;
}

const AlsaAudioDriver::Spec& AlsaAudioDriver::activeSpec() const
{
    return s_format;
}

AudioDeviceID AlsaAudioDriver::outputDevice() const
{
    return m_deviceId;
}

bool AlsaAudioDriver::selectOutputDevice(const AudioDeviceID& deviceId)
{
    if (m_deviceId == deviceId) {
        return true;
    }

    bool reopen = isOpened();
    close();
    m_deviceId = deviceId;

    bool ok = true;
    if (reopen) {
        ok = open(s_format, &s_format);
    }

    if (ok) {
        m_outputDeviceChanged.notify();
    }

    return ok;
}

bool AlsaAudioDriver::resetToDefaultOutputDevice()
{
    return selectOutputDevice(DEFAULT_DEVICE_ID);
}

async::Notification AlsaAudioDriver::outputDeviceChanged() const
{
    return m_outputDeviceChanged;
}

AudioDeviceList AlsaAudioDriver::availableOutputDevices() const
{
    AudioDeviceList devices;
    devices.push_back({ DEFAULT_DEVICE_ID, muse::trc("audio", "System default") });

    return devices;
}

async::Notification AlsaAudioDriver::availableOutputDevicesChanged() const
{
    return m_availableOutputDevicesChanged;
}

unsigned int AlsaAudioDriver::outputDeviceBufferSize() const
{
    return s_format.samples;
}

bool AlsaAudioDriver::setOutputDeviceBufferSize(unsigned int bufferSize)
{
    if (s_format.samples == bufferSize) {
        return true;
    }

    bool reopen = isOpened();
    close();
    s_format.samples = bufferSize;

    bool ok = true;
    if (reopen) {
        ok = open(s_format, &s_format);
    }

    if (ok) {
        m_bufferSizeChanged.notify();
    }

    return ok;
}

async::Notification AlsaAudioDriver::outputDeviceBufferSizeChanged() const
{
    return m_bufferSizeChanged;
}

std::vector<unsigned int> AlsaAudioDriver::availableOutputDeviceBufferSizes() const
{
    std::vector<unsigned int> result;

    unsigned int n = MAXIMUM_BUFFER_SIZE;
    while (n >= MINIMUM_BUFFER_SIZE) {
        result.push_back(n);
        n /= 2;
    }

    std::sort(result.begin(), result.end());

    return result;
}

unsigned int AlsaAudioDriver::outputDeviceSampleRate() const
{
    return s_format.sampleRate;
}

bool AlsaAudioDriver::setOutputDeviceSampleRate(unsigned int sampleRate)
{
    if (s_format.sampleRate == sampleRate) {
        return true;
    }

    bool reopen = isOpened();
    close();
    s_format.sampleRate = sampleRate;

    bool ok = true;
    if (reopen) {
        ok = open(s_format, &s_format);
    }

    if (ok) {
        m_sampleRateChanged.notify();
    }

    return ok;
}

async::Notification AlsaAudioDriver::outputDeviceSampleRateChanged() const
{
    return m_sampleRateChanged;
}

std::vector<unsigned int> AlsaAudioDriver::availableOutputDeviceSampleRates() const
{
    // ALSA API is not of any help to get sample rates supported by the driver.
    // (snd_pcm_hw_params_get_rate_[min|max] will return 1 to 384000 Hz)
    // So just returning a sensible hard-coded list.
    return {
        44100,
        48000,
        88200,
        96000,
    };
}

void AlsaAudioDriver::resume()
{
}

void AlsaAudioDriver::suspend()
{
}
