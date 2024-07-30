/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-CLA-applies
 *
 * MuseScore
 * Music Composition & Notation
 *
 * Copyright (C) 2021 MuseScore Limited and others
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

#include "translation.h"
#include "log.h"
#include "runtime.h"

#define ALSA_PCM_NEW_HW_PARAMS_API
#include <alsa/asoundlib.h>

#include <fcntl.h>
#include <unistd.h>
#include <pthread.h>

#include <sys/mman.h>
#include <sched.h>

#include <algorithm>
#include <cmath>
#include <cstring>

#define ALSA_REALTIME

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

#ifdef ALSA_REALTIME
static bool alsaAcquireRealtime(pthread_t th)
{
    const int min_prio = sched_get_priority_min(SCHED_FIFO);
    const int max_prio = sched_get_priority_max(SCHED_FIFO);
    // priority is normally anything from 0 to 99
    // No need to have a too high value though as it can make problems in the kernel scheduling.
    const int desired_prio = 20;
    const int sched_prio = std::max(min_prio, std::min(max_prio, desired_prio));
    struct sched_param param = {
        .sched_priority = sched_prio,
    };
    LOGD() << "Requesting RT priority " << sched_prio;

    /* Set scheduler policy and priority of pthread */
    int ret = pthread_setschedparam(th, SCHED_FIFO, &param);
    if (ret) {
        LOGW() << "pthread setschedparam failed: " << strerror(ret);
        return false;
    }

    // Pinning to RAM the data accessed during the realtime thread
    // (optional and require CAP_IPC_LOCK capability)
    const auto bufLen = s_alsaData->samples * s_alsaData->channels;
    if (mlock(reinterpret_cast<void*>(s_alsaData), sizeof(ALSAData))) {
        LOGW() << "Could not lock ALSA data to RAM: " << strerror(errno);
    }
    if (mlock(reinterpret_cast<void*>(s_alsaData->buffer), bufLen * sizeof(float))) {
        LOGW() << "Could not lock ALSA buffer to RAM: " << strerror(errno);
    }

    return true;
}
#endif

static void* alsaThread(void* aParam)
{
    muse::runtime::setThreadName("audio_driver");
    ALSAData* data = static_cast<ALSAData*>(aParam);

    int ret = snd_pcm_wait(data->alsaDeviceHandle, 1000);
    IF_ASSERT_FAILED(ret > 0) {
        return nullptr;
    }

#ifdef ALSA_REALTIME
    if (!alsaAcquireRealtime(data->threadHandle)) {
        LOGW() << "Could not acquire realtime priority";
    } else {
        LOGI() << "Successfully acquired realtime priority";
    }
#endif

    while (!data->audioProcessingDone)
    {
        uint8_t* stream = (uint8_t*)data->buffer;
        int len = data->samples * data->channels * sizeof(float);

        data->callback(stream, len);

        snd_pcm_uframes_t frames = data->samples;
        while (frames > 0) {
            auto ret = snd_pcm_writei(data->alsaDeviceHandle, data->buffer, frames);
            if (ret == -EPIPE) {
                snd_pcm_prepare(data->alsaDeviceHandle);
                break;
            }
            frames -= ret;
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
    return "ALSA";
}

AudioDeviceID AlsaAudioDriver::defaultDevice() const
{
    return DEFAULT_DEVICE_ID;
}

bool AlsaAudioDriver::open(const Spec& spec, Spec* activeSpec)
{
    IF_ASSERT_FAILED(!spec.deviceId.empty()) {
        return false;
    }

    s_alsaData = new ALSAData();
    s_alsaData->samples = spec.output.samplesPerChannel;
    s_alsaData->channels = spec.output.audioChannelCount;
    s_alsaData->callback = spec.callback;

    snd_pcm_t* handle;
    int rc = snd_pcm_open(&handle, spec.deviceId.c_str(), SND_PCM_STREAM_PLAYBACK, 0);
    if (rc < 0) {
        LOGE() << "Unable to open device: " << spec.deviceId << ", err code: " << rc;
        return false;
    }

    s_alsaData->alsaDeviceHandle = handle;

    snd_pcm_hw_params_t* params;
    snd_pcm_hw_params_alloca(&params);
    snd_pcm_hw_params_any(handle, params);

    snd_pcm_hw_params_set_access(handle, params, SND_PCM_ACCESS_RW_INTERLEAVED);
    snd_pcm_hw_params_set_format(handle, params, SND_PCM_FORMAT_FLOAT_LE);
    snd_pcm_hw_params_set_channels(handle, params, spec.output.audioChannelCount);

    unsigned int aSamplerate = spec.output.sampleRate;
    unsigned int val = aSamplerate;
    int dir = 0;
    rc = snd_pcm_hw_params_set_rate_near(handle, params, &val, &dir);
    if (rc < 0) {
        LOGE() << "Unable to set sample rate: " << val << ", err code: " << rc;
        return false;
    }

    snd_pcm_uframes_t ringBufferSize = s_alsaData->samples * 2;
    rc = snd_pcm_hw_params_set_buffer_size_near(handle, params, &ringBufferSize);
    if (rc < 0) {
        LOGE() << "Unable to set ring buffer size, err code: " << rc;
        return false;
    }

    rc = snd_pcm_hw_params_set_period_size_near(handle, params, &s_alsaData->samples, 0);
    if (rc < 0) {
        LOGE() << "Unable to set period buffer size, err code: " << rc;
        return false;
    }

    rc = snd_pcm_hw_params(handle, params);
    if (rc < 0) {
        LOGE() << "Unable to set params, err code: " << rc;
        return false;
    }

    snd_pcm_hw_params_get_rate(params, &val, &dir);
    aSamplerate = val;

    const auto bufLen = s_alsaData->samples * s_alsaData->channels;
    s_alsaData->buffer = new float[bufLen];

    s_format = spec;
    s_format.output.sampleRate = aSamplerate;
    m_activeSpecChanged.send(s_format);

    if (activeSpec) {
        *activeSpec = s_format;
    }

    s_alsaData->threadHandle = 0;

    rc = pthread_create(&s_alsaData->threadHandle, nullptr, alsaThread, (void*)s_alsaData);

    if (0 != rc) {
        LOGE() << "Could not start ALSA thread: " << strerror(errno);
        return false;
    }

    LOGI() << "Connected to " << spec.deviceId
           << " with bufferSize " << s_format.output.samplesPerChannel
           << ", sampleRate " << s_format.output.sampleRate
           << ", channels:  " << s_format.output.audioChannelCount;

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

async::Channel<IAudioDriver::Spec> AlsaAudioDriver::activeSpecChanged() const
{
    return m_activeSpecChanged;
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

std::vector<samples_t> AlsaAudioDriver::availableOutputDeviceBufferSizes() const
{
    std::vector<samples_t> result;

    samples_t n = MAXIMUM_BUFFER_SIZE;
    while (n >= MINIMUM_BUFFER_SIZE) {
        result.push_back(n);
        n /= 2;
    }

    std::sort(result.begin(), result.end());

    return result;
}

std::vector<sample_rate_t> AlsaAudioDriver::availableOutputDeviceSampleRates() const
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
