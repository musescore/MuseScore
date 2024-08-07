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
#include <array>
#include <cmath>
#include <cstring>
#include <vector>

#define DEBUG_AUDIO
// #define ALSA_POLL

#ifdef DEBUG_AUDIO
#define LOG_AUDIO LOGD
#else
#define LOG_AUDIO LOGN
#endif

static constexpr char DEFAULT_DEVICE_ID[] = "default";

using namespace muse;
using namespace muse::audio;

namespace {
struct AlsaParams
{
    snd_pcm_access_t access;
    snd_pcm_format_t format;
    unsigned int channels;
    unsigned int sampleRate;
    snd_pcm_uframes_t periodSize;
    unsigned int periods; // num periods in the ring buffer
    snd_pcm_uframes_t ringBufferSize;
};

struct AlsaData
{
    snd_pcm_t* alsaDeviceHandle = nullptr;
    AlsaParams params;
    bool audioProcessingDone = false;
    pthread_t threadHandle = 0;
    IAudioDriver::Callback callback;
    void* userdata = nullptr;
};

static AlsaData* s_alsaData{ nullptr };
static muse::audio::IAudioDriver::Spec s_format;

inline bool accessIsSupported(snd_pcm_access_t access)
{
#ifdef ALSA_POLL
    return access == SND_PCM_ACCESS_RW_INTERLEAVED || access == SND_PCM_ACCESS_MMAP_INTERLEAVED;
#else
    return access == SND_PCM_ACCESS_RW_INTERLEAVED;
#endif
}

static bool alsaConfigureDevice(snd_pcm_t* deviceHandle, AlsaParams& params)
{
    snd_pcm_hw_params_t* hwparams;
    snd_pcm_hw_params_alloca(&hwparams);

    int rc = snd_pcm_hw_params_any(deviceHandle, hwparams);
    if (rc < 0) {
        LOGE() << "Broken playback configuration. No configuration available: " << snd_strerror(rc);
        return false;
    }

    if (!accessIsSupported(params.access)) {
        LOGE() << "Unsupported requested access type: " << snd_pcm_access_name(params.access);
        return false;
    }

#ifdef ALSA_POLL
    const auto supportedAccess = std::array<snd_pcm_access_t, 2> {
        params.access,
        params.access == SND_PCM_ACCESS_MMAP_INTERLEAVED
        ? SND_PCM_ACCESS_RW_INTERLEAVED
        : SND_PCM_ACCESS_MMAP_INTERLEAVED,
    };
#else
    const auto supportedAccess = std::array<snd_pcm_access_t, 1> { params.access };
#endif

    bool accessSet = false;
    for (const auto access : supportedAccess) {
        rc = snd_pcm_hw_params_set_access(deviceHandle, hwparams, access);
        if (rc >= 0) {
            params.access = access;
            accessSet = true;
            break;
        }
        LOGW() << "Access type not available for playback: " << snd_pcm_access_name(access) << ": " << snd_strerror(rc);
    }
    if (!accessSet) {
        LOGE() << "Could not find suitable access for playback";
        return false;
    }

    rc = snd_pcm_hw_params_set_format(deviceHandle, hwparams, SND_PCM_FORMAT_FLOAT_LE);
    if (rc < 0) {
        LOGE() << "Sample format not available for playback: " << snd_strerror(rc);
        return false;
    }

    rc = snd_pcm_hw_params_set_channels(deviceHandle, hwparams, params.channels);
    if (rc < 0) {
        LOGE() << "Channel count (" << params.channels << ") not available for playback: " << snd_strerror(rc);
        return false;
    }

    unsigned int rate = params.sampleRate;
    rc = snd_pcm_hw_params_set_rate_near(deviceHandle, hwparams, &rate, nullptr);
    if (rc < 0) {
        LOGE() << "Sample rate " << params.sampleRate << "Hz not available for playback: " << snd_strerror(rc);
        return false;
    }
    if (rate != params.sampleRate) {
        LOGW() << "Sample rate doesn't match request. Requested " << params.sampleRate << "Hz, obtained " << rate << "Hz.";
        params.sampleRate = rate;
    }

    rc = snd_pcm_hw_params_set_period_size_near(deviceHandle, hwparams, &params.periodSize, 0);
    if (rc < 0) {
        LOGE() << "Could not set period size (" << params.periodSize << "): " << snd_strerror(rc);
        return false;
    }

    unsigned int nperiods = params.periods;
    snd_pcm_hw_params_get_periods_min(hwparams, &nperiods, 0);
    nperiods = std::max(params.periods, nperiods);

    rc = snd_pcm_hw_params_set_periods_near(deviceHandle, hwparams, &nperiods, 0);
    if (rc < 0) {
        LOGE() << "Could not set periods (" << nperiods << "): " << snd_strerror(rc);
        return false;
    }
    if (nperiods < params.periods) {
        LOGE() << "Could not get requested periods";
        return false;
    }
    params.periods = nperiods;

    params.ringBufferSize = params.periods * params.periodSize;
    rc = snd_pcm_hw_params_set_buffer_size(deviceHandle, hwparams, params.ringBufferSize);
    if (rc < 0) {
        LOGE() << "Could not set ring buffer size (" << params.ringBufferSize << "): " << snd_strerror(rc);
        return false;
    }

    rc = snd_pcm_hw_params(deviceHandle, hwparams);
    if (rc < 0) {
        LOGE() << "Could not set hardware parameters: " << snd_strerror(rc);
        return false;
    }

    snd_pcm_sw_params_t* swparams;
    snd_pcm_sw_params_alloca(&swparams);

    /* get the current swparams */
    rc = snd_pcm_sw_params_current(deviceHandle, swparams);
    if (rc < 0) {
        LOGE() << "Unable to determine current swparams for playback: " << snd_strerror(rc);
        return false;
    }

    rc = snd_pcm_sw_params_set_start_threshold(deviceHandle, swparams, params.periodSize);
    if (rc < 0) {
        LOGE() << "Unable to set start threshold for playback: " << snd_strerror(rc);
        return false;
    }

    rc = snd_pcm_sw_params_set_stop_threshold(deviceHandle, swparams, params.ringBufferSize);
    if (rc < 0) {
        LOGE() << "Unable to set stop threshold for playback: " << snd_strerror(rc);
        return false;
    }

    /* allow the transfer when at least periodSize samples can be processed */
    const snd_pcm_uframes_t availMin = params.periodSize;
    rc = snd_pcm_sw_params_set_avail_min(deviceHandle, swparams, availMin);
    if (rc < 0) {
        LOGE() << "Unable to set avail min for playback: " << snd_strerror(rc);
        return false;
    }

    /* write the parameters to the playback device */
    rc = snd_pcm_sw_params(deviceHandle, swparams);
    if (rc < 0) {
        LOGE() << "Unable to set sw params for playback: " << snd_strerror(rc);
        return false;
    }

    return true;
}

/*
 *   Underrun and suspend recovery
 */
static int xrunRecovery(snd_pcm_t* deviceHandle, int err)
{
    LOG_AUDIO() << "xrun recovery";

    switch (err) {
    case -EPIPE:
        // underrun
        err = snd_pcm_prepare(deviceHandle);
        if (err < 0) {
            LOGE() << "Can't recovery from underrun, prepare failed: " << snd_strerror(err);
        }
        return 0;
    case -ESTRPIPE:
        // hardware is suspended, possibly due to low battery
        LOGW() << "Suspended, waiting for resume";
        while ((err = snd_pcm_resume(deviceHandle)) == -EAGAIN) {
            sleep(1);       // wait until the suspend flag is released
        }
        if (err < 0) {
            err = snd_pcm_prepare(deviceHandle);
            if (err < 0) {
                LOGE() << "Can't recovery from suspend, prepare failed: " << snd_strerror(err);
            }
        }
        return 0;
    default:
        LOGE() << "Unexpected error: " << snd_strerror(err);
        return err;
    }
}

#ifdef ALSA_POLL
// returns true if success
static bool waitForPoll(snd_pcm_t* deviceHandle, struct pollfd* ufds,
                        unsigned int count, int timeout = -1)
{
    unsigned short revents;

    while (true) {
        if (poll(ufds, count, timeout) == 0) {
            LOG_AUDIO() << "poll timeout " << timeout;
        }
        snd_pcm_poll_descriptors_revents(deviceHandle, ufds, count, &revents);

        if (revents & POLLERR) {
            const auto state = snd_pcm_state(deviceHandle);
            switch (state) {
            case SND_PCM_STATE_XRUN:
                xrunRecovery(deviceHandle, -EPIPE);
                break;
            case SND_PCM_STATE_SUSPENDED:
                xrunRecovery(deviceHandle, -ESTRPIPE);
                break;
            default:
                break;
            }
            return false;
        }

        if (revents & POLLOUT) {
            return true;
        }
    }
}

static void alsaPollMmapLoop(AlsaData* data, std::vector<struct pollfd> ufds)
{
    LOGI() << "Entering ALSA poll mmap loop";

    auto deviceHandle = data->alsaDeviceHandle;
    auto callback = data->callback;
    auto userdata = data->userdata;
    const auto periodSize = data->params.periodSize;
    const auto channels = data->params.channels;

    const auto avail = snd_pcm_avail_update(deviceHandle);
    if (static_cast<snd_pcm_uframes_t>(avail) != data->params.ringBufferSize) {
        LOGE() << "Full buffer not available at start";
        return;
    }

    const snd_pcm_channel_area_t* areas = nullptr;
    snd_pcm_uframes_t offset = 0;
    snd_pcm_uframes_t frames = avail;
    snd_pcm_mmap_begin(deviceHandle, &areas, &offset, &frames);

    // Double check the area
    const void* expectedAddr = areas[0].addr;
    unsigned int expectedFirst = 0;
    const unsigned expectedStep = sizeof(float) * 8 * channels;
    for (unsigned int c = 0; c < channels; c++) {
        if (areas[c].first != expectedFirst || areas[c].step != expectedStep || areas[c].addr != expectedAddr) {
            LOGE() << "MMAP areas mismatch, not interleaved as expected";
            return;
        }
        expectedFirst += sizeof(float) * 8;
    }

    // Zero the buffer
    float* addr = static_cast<float*>(areas[0].addr) + offset * channels;
    size_t bytes = frames * channels * sizeof(float);
    std::memset(addr, 0, bytes);

    snd_pcm_mmap_commit(deviceHandle, offset, frames);

    int err = snd_pcm_start(deviceHandle);
    if (err < 0) {
        LOGE() << "Unable to start device: " << snd_strerror(err);
        return;
    }

    bool restartPcm = false;

    int waitTimeout = 4 * 1000 * data->params.periodSize / data->params.sampleRate;

    // Start the loop
    while (!data->audioProcessingDone) {
        const auto avail = snd_pcm_avail_update(deviceHandle);
        if (avail < 0) {
            LOG_AUDIO() << "Error in snd_pcm_avail_update: " << snd_strerror(avail);
            xrunRecovery(deviceHandle, avail);
            continue;
        }
        if (static_cast<snd_pcm_uframes_t>(avail) < periodSize) {
            if (restartPcm) {
                LOG_AUDIO() << "Restarting PCM";
                err = snd_pcm_start(deviceHandle);
                if (err < 0) {
                    LOGE() << "Unable to restart device: " << snd_strerror(err);
                }
                restartPcm = false;
            }
            waitForPoll(deviceHandle, &ufds[0], static_cast<unsigned int>(ufds.size()), waitTimeout);
            continue;
        }

        frames = periodSize;
        while (frames) {
            snd_pcm_uframes_t contiguous = frames;
            if ((err = snd_pcm_mmap_begin(deviceHandle, &areas, &offset, &contiguous)) < 0) {
                xrunRecovery(deviceHandle, err);
                restartPcm = true;
                break;
            }

            uint8_t* stream = reinterpret_cast<uint8_t*>(areas[0].addr) + offset * channels * sizeof(float);
            const auto len = contiguous * channels * sizeof(float);

            callback(userdata, stream, len);

            snd_pcm_sframes_t commitres = snd_pcm_mmap_commit(deviceHandle, offset, contiguous);
            if (commitres < 0) {
                xrunRecovery(deviceHandle, static_cast<int>(commitres));
                restartPcm = true;
                break;
            }
            frames -= contiguous;
        }
    }
}

static void alsaPollWriteLoop(AlsaData* data, std::vector<struct pollfd> ufds)
{
    LOGI() << "Entering ALSA poll write loop";

    auto deviceHandle = data->alsaDeviceHandle;
    auto callback = data->callback;
    auto userdata = data->userdata;
    const auto periodSize = data->params.periodSize;
    const auto channels = data->params.channels;

    int err = snd_pcm_start(deviceHandle);
    if (err < 0) {
        LOGE() << "Unable to start device: " << snd_strerror(err);
        return;
    }

    auto buffer = std::vector<float>(periodSize * channels);

    int waitTimeout = 4 * 1000 * data->params.periodSize / data->params.sampleRate;

    // Start the loop
    while (!data->audioProcessingDone) {
        uint8_t* stream = reinterpret_cast<uint8_t*>(&buffer[0]);
        const auto len = buffer.size() * sizeof(float);

        callback(userdata, stream, len);

        snd_pcm_uframes_t frames = periodSize;
        float* ptr = &buffer[0];

        while (frames > 0) {
            const auto rc = snd_pcm_writei(deviceHandle, ptr, frames);
            if (rc < 0) {
                xrunRecovery(deviceHandle, rc);
                break;
            }
            frames -= rc;
            ptr += rc * channels;

            if (frames == 0) {
                break;
            }

            waitForPoll(deviceHandle, &ufds[0], static_cast<unsigned int>(ufds.size()), waitTimeout);
        }

        if (snd_pcm_state(deviceHandle) == SND_PCM_STATE_RUNNING) {
            waitForPoll(deviceHandle, &ufds[0], static_cast<unsigned int>(ufds.size()), waitTimeout);
        }
    }
}

void alsaPollLoop(AlsaData* data)
{
    auto deviceHandle = data->alsaDeviceHandle;

    const auto count = snd_pcm_poll_descriptors_count(deviceHandle);
    if (count <= 0) {
        LOGE() << "Invalid poll descriptors count";
        return;
    }
    auto ufds = std::vector<struct pollfd>(count);

    int err = snd_pcm_poll_descriptors(deviceHandle, &ufds[0], count);
    if (err < 0) {
        LOGE() << "Unable to obtain poll descriptors for playback: " << snd_strerror(err);
        return;
    }

    switch (data->params.access) {
    case SND_PCM_ACCESS_MMAP_INTERLEAVED:
        alsaPollMmapLoop(data, std::move(ufds));
        break;
    case SND_PCM_ACCESS_RW_INTERLEAVED:
        alsaPollWriteLoop(data, std::move(ufds));
        break;
    default:
        LOGE() << "Unsupported access type: " << data->params.access;
        break;
    }
}

#endif

void alsaWriteLoop(AlsaData* data)
{
    LOGI() << "Entering ALSA write loop";

    auto deviceHandle = data->alsaDeviceHandle;
    auto callback = data->callback;
    auto userdata = data->userdata;
    const auto periodSize = data->params.periodSize;
    const auto channels = data->params.channels;

    int err = snd_pcm_start(deviceHandle);
    if (err < 0) {
        LOGE() << "Unable to start device: " << snd_strerror(err);
        return;
    }

    auto buffer = std::vector<float>(periodSize * channels);

    while (!data->audioProcessingDone) {
        uint8_t* stream = reinterpret_cast<uint8_t*>(&buffer[0]);
        const auto len = buffer.size() * sizeof(float);

        callback(userdata, stream, len);

        auto count = periodSize;
        while (count > 0) {
            const auto rc = snd_pcm_writei(deviceHandle, stream, count);
            if (rc < 0) {
                xrunRecovery(deviceHandle, rc);
                break;
            }
            count -= rc;
            stream += rc * channels;
        }
    }
}

static void* alsaThread(void* aParam)
{
    muse::runtime::setThreadName("audio_driver");
    AlsaData* data = static_cast<AlsaData*>(aParam);
    auto deviceHandle = data->alsaDeviceHandle;

    int err = snd_pcm_prepare(deviceHandle);
    if (err < 0) {
        LOGE() << "Unable to prepare device: " << snd_strerror(err);
        return nullptr;
    }

#ifdef ALSA_POLL
    alsaPollLoop(data);
#else
    alsaWriteLoop(data);
#endif

    LOGI() << "Exiting ALSA thread";
    return nullptr;
}

static void alsaCleanup()
{
    if (!s_alsaData) {
        return;
    }

    s_alsaData->audioProcessingDone = true;

    if (s_alsaData->threadHandle) {
        struct timespec ts;
        if (clock_gettime(CLOCK_REALTIME, &ts)) {
            LOGE() << "Unable to get current time: " << strerror(errno);
            pthread_join(s_alsaData->threadHandle, nullptr);
        } else {
            ts.tv_nsec += 200 * 1000 * 1000;
            int rt = pthread_timedjoin_np(s_alsaData->threadHandle, nullptr, &ts);
            if (rt == ETIMEDOUT) {
                LOGE() << "Timeout while waiting for ALSA thread to join";
                pthread_cancel(s_alsaData->threadHandle);
            }
        }
    }
    if (nullptr != s_alsaData->alsaDeviceHandle) {
        snd_pcm_drain(s_alsaData->alsaDeviceHandle);
        snd_pcm_close(s_alsaData->alsaDeviceHandle);
    }

    delete s_alsaData;
    s_alsaData = nullptr;
}
}

LinuxAudioDriver::LinuxAudioDriver()
{
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
#ifdef ALSA_POLL
    const auto flags = SND_PCM_NONBLOCK;
    const auto preferredAccess = SND_PCM_ACCESS_MMAP_INTERLEAVED;
#else
    const auto flags = 0;
    const auto preferredAccess = SND_PCM_ACCESS_RW_INTERLEAVED;
#endif

    snd_pcm_t* deviceHandle;
    int rc = snd_pcm_open(&deviceHandle, outputDevice().c_str(), SND_PCM_STREAM_PLAYBACK, flags);
    if (rc < 0) {
        LOGE() << "Could not open ALSA device (" << outputDevice() << "): " << snd_strerror(rc);
        return false;
    }

    AlsaParams params {
        .access = preferredAccess,
        .format = SND_PCM_FORMAT_FLOAT_LE,
        .channels = spec.channels,
        .sampleRate = spec.sampleRate,
        .periodSize = spec.samples,
        .periods = 2u,
        .ringBufferSize = 0u,
    };

    if (!alsaConfigureDevice(deviceHandle, params)) {
        LOGE() << "Could not configure ALSA device";
        return false;
    }

    std::ostringstream configStr;
    configStr << "    access         = " << snd_pcm_access_name(params.access) << "\n";
    configStr << "    format         = " << snd_pcm_format_name(params.format) << "\n";
    configStr << "    channels       = " << params.channels << "\n";
    configStr << "    periodSize     = " << params.periodSize << "\n";
    configStr << "    periods        = " << params.periods << "\n";
    configStr << "    ringBufferSize = " << params.ringBufferSize;

    LOGI() << "Configured ALSA device " << outputDevice() << ":\n" << configStr.str();

    s_alsaData = new AlsaData {};
    s_alsaData->params = params;
    s_alsaData->callback = spec.callback;
    s_alsaData->userdata = spec.userdata;
    s_alsaData->alsaDeviceHandle = deviceHandle;

    s_alsaData->threadHandle = 0;

    rc = pthread_create(&s_alsaData->threadHandle, nullptr, alsaThread, (void*)s_alsaData);

    if (0 != rc) {
        LOGE() << "Could not start ALSA thread: " << strerror(errno);
        return false;
    }

    if (activeSpec) {
        *activeSpec = spec;
        activeSpec->format = Format::AudioF32;
        activeSpec->samples = params.periodSize;
        activeSpec->sampleRate = params.sampleRate;
        s_format = *activeSpec;
    }

    LOGD() << "Connected to " << outputDevice();
    return true;
}

void LinuxAudioDriver::close()
{
    alsaCleanup();
}

bool LinuxAudioDriver::isOpened() const
{
    return s_alsaData != nullptr;
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
        ok = open(s_format, &s_format);
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

async::Notification LinuxAudioDriver::outputDeviceChanged() const
{
    return m_outputDeviceChanged;
}

AudioDeviceList LinuxAudioDriver::availableOutputDevices() const
{
    AudioDeviceList devices;
    devices.push_back({ DEFAULT_DEVICE_ID, muse::trc("audio", "System default") });

    return devices;
}

async::Notification LinuxAudioDriver::availableOutputDevicesChanged() const
{
    return m_availableOutputDevicesChanged;
}

unsigned int LinuxAudioDriver::outputDeviceBufferSize() const
{
    return s_format.samples;
}

bool LinuxAudioDriver::setOutputDeviceBufferSize(unsigned int bufferSize)
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

async::Notification LinuxAudioDriver::outputDeviceBufferSizeChanged() const
{
    return m_bufferSizeChanged;
}

std::vector<unsigned int> LinuxAudioDriver::availableOutputDeviceBufferSizes() const
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

unsigned int LinuxAudioDriver::outputDeviceSampleRate() const
{
    return s_format.sampleRate;
}

bool LinuxAudioDriver::setOutputDeviceSampleRate(unsigned int sampleRate)
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

async::Notification LinuxAudioDriver::outputDeviceSampleRateChanged() const
{
    return m_sampleRateChanged;
}

std::vector<unsigned int> LinuxAudioDriver::availableOutputDeviceSampleRates() const
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

void LinuxAudioDriver::resume()
{
}

void LinuxAudioDriver::suspend()
{
}
