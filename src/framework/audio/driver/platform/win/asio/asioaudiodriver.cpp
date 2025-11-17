/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-CLA-applies
 *
 * MuseScore
 * Music Composition & Notation
 *
 * Copyright (C) 2025 MuseScore Limited and others
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
#include "asioaudiodriver.h"

#include "global/async/notification.h"

#undef UNICODE
#include "ASIOSDK/common/asiosys.h"
#include "ASIOSDK/common/asio.h"
#include "ASIOSDK/host/asiodrivers.h"

#include "log.h"

using namespace muse;
using namespace muse::audio;

struct AsioData {
    // Drivers list
    AsioDrivers drivers;

    // ASIOInit()
    ASIODriverInfo driverInfo;

    struct DeviceMetrics {
        // ASIOGetChannels()
        long outputChannels = 0;
        long inputChannels = 0;

        // ASIOGetBufferSize()
        long minSize = 0;
        long maxSize = 0;
        long preferredSize = 0;
        long granularity = 0;

        // ASIOGetSampleRate()
        double sampleRate = 0.0;
    } deviceMetrics;

    // Active
    IAudioDriver::Spec activeSpec;
    // ASIOCreateBuffers ()
    ASIOBufferInfo* bufferInfos = nullptr;
    // ASIOGetChannelInfo()
    ASIOChannelInfo* channelInfos = nullptr;

    // ASIOOutputReady()
    bool postOutput = false;

    // ASIOCallbacks
    ASIOCallbacks callbacks;

    // Reset
    async::Notification resetRequest;
};

static AsioData s_adata;

AsioAudioDriver::AsioAudioDriver()
{
}

void AsioAudioDriver::init()
{
}

std::string AsioAudioDriver::name() const
{
    return "asio";
}

constexpr uint16_t swap16(uint16_t val)
{
    return ((val & 0xFF00) >> 8) | ((val & 0x00FF) << 8);
}

constexpr uint32_t swap32(uint32_t val)
{
    return ((val & 0xFF000000) >> 24)
           | ((val & 0x00FF0000) >> 8)
           | ((val & 0x0000FF00) << 8)
           | ((val & 0x000000FF) << 24);
}

constexpr uint64_t swap64(uint64_t val)
{
    return ((val & 0xFF00000000000000ULL) >> 56)
           | ((val & 0x00FF000000000000ULL) >> 40)
           | ((val & 0x0000FF0000000000ULL) >> 24)
           | ((val & 0x000000FF00000000ULL) >> 8)
           | ((val & 0x00000000FF000000ULL) << 8)
           | ((val & 0x0000000000FF0000ULL) << 24)
           | ((val & 0x000000000000FF00ULL) << 40)
           | ((val & 0x00000000000000FFULL) << 56);
}

static void s_bufferSwitch(long index, ASIOBool /*processNow*/)
{
    const IAudioDriver::Spec& active = s_adata.activeSpec;

    const size_t channels = active.output.audioChannelCount;
    const size_t procSamplesTotal = active.output.audioChannelCount * active.output.samplesPerChannel;

    static std::vector<float> proc_buf;
    if (proc_buf.size() != procSamplesTotal) {
        proc_buf.resize(procSamplesTotal);
    }

    uint8_t* stream = reinterpret_cast<uint8_t*>(&proc_buf[0]);
    active.callback(stream, (int)(procSamplesTotal * sizeof(float)));

    constexpr float MAX_S16 = 32767.0f;
    constexpr float MAX_S18 = 131071.0f;
    constexpr float MAX_S20 = 524287.0f;
    constexpr float MAX_S24 = 8388607.0f;
    constexpr float MAX_S32 = 2147483647.0f;

    size_t outIndex = 0;
    for (size_t s = 0; s < procSamplesTotal; s += channels) {
        for (size_t c = 0; c < channels; c++) {
            const float sample = proc_buf.at(s + c);
            void* out = s_adata.bufferInfos[c].buffers[index];

            const ASIOSampleType type = s_adata.channelInfos[c].type;
            switch (type) {
            // MSB
            case ASIOSTInt16MSB: {
                int16_t* out16 = (int16_t*)out;
                int16_t val16LSB = static_cast<int16_t>(std::clamp(sample, -1.0f, 1.0f) * MAX_S16);
                out16[outIndex] = swap16(val16LSB); // to big-endian
            } break;
            case ASIOSTInt24MSB: {
                uint8_t* out8 = (uint8_t*)out;
                int32_t val = static_cast<int32_t>(std::clamp(sample, -1.0f, 1.0f) * MAX_S24);
                size_t out8Index = outIndex * 3; // 3 bytes
                out8[out8Index] = (val >> 16) & 0xFF;
                out8[out8Index] = (val >> 8) & 0xFF;
                out8[out8Index] = val & 0xFF;
            } break;
            case ASIOSTInt32MSB: {
                int32_t* out32 = (int32_t*)out;
                int32_t val32LSB = static_cast<int32_t>(std::clamp(sample, -1.0f, 1.0f) * MAX_S32);
                out32[outIndex] = swap32(val32LSB); // to big-endian
            } break;
            case ASIOSTFloat32MSB: {
                float* outF32 = (float*)out;
                union {
                    float f;
                    uint32_t i;
                } converter;

                converter.f = sample;
                converter.i = swap32(converter.i);
                outF32[outIndex] = converter.f;
            } break;
            case ASIOSTFloat64MSB: {
                double* outF64 = (double*)out;
                union {
                    double d;
                    uint64_t i;
                } converter;

                converter.d = static_cast<double>(sample);
                converter.i = swap64(converter.i);
                outF64[outIndex] = converter.d;
            } break;

            // MSB 32 with alignment
            case ASIOSTInt32MSB16: {
                int32_t* out32 = (int32_t*)out;
                int32_t val32LSB = static_cast<int32_t>(std::clamp(sample, -1.0f, 1.0f) * MAX_S16);
                out32[outIndex] = swap32(val32LSB << 16); // 16 alignment and to big-endian
            } break;
            case ASIOSTInt32MSB18: {
                int32_t* out32 = (int32_t*)out;
                int32_t val32LSB = static_cast<int32_t>(std::clamp(sample, -1.0f, 1.0f) * MAX_S18);
                out32[outIndex] = swap32(val32LSB << 14); // 18 alignment and to big-endian
            } break;
            case ASIOSTInt32MSB20: {
                int32_t* out32 = (int32_t*)out;
                int32_t val32LSB = static_cast<int32_t>(std::clamp(sample, -1.0f, 1.0f) * MAX_S20);
                out32[outIndex] = swap32(val32LSB << 12); // 20 alignment and to big-endian
            } break;
            case ASIOSTInt32MSB24: {
                int32_t* out32 = (int32_t*)out;
                int32_t val32LSB = static_cast<int32_t>(std::clamp(sample, -1.0f, 1.0f) * MAX_S24);
                out32[outIndex] = swap32(val32LSB << 8); // 24 alignment and to big-endian
            } break;

            // LSB
            case ASIOSTInt16LSB: {
                int16_t* out16 = (int16_t*)out;
                out16[outIndex] = static_cast<int16_t>(std::clamp(sample, -1.0f, 1.0f) * MAX_S16);
            } break;
            case ASIOSTInt24LSB: {
                uint8_t* out8 = (uint8_t*)out;
                int32_t val = static_cast<int32_t>(std::clamp(sample, -1.0f, 1.0f) * MAX_S24);
                size_t out8Index = outIndex * 3; // 3 bytes
                out8[out8Index]=(val) & 0xff;
                out8[out8Index + 1]=(val >> 8) & 0xff;
                out8[out8Index + 2]=(val >> 16) & 0xff;
            } break;
            case ASIOSTInt32LSB: {
                int32_t* out32 = (int32_t*)out;
                out32[outIndex] = static_cast<int32_t>(std::clamp(sample, -1.0f, 1.0f) * MAX_S32);
            } break;
            case ASIOSTFloat32LSB: {
                float* outF32 = (float*)out;
                outF32[outIndex] = sample;
            } break;
            case ASIOSTFloat64LSB: {
                double* outF64 = (double*)out;
                outF64[outIndex] = static_cast<double>(sample);
            } break;

            // LSB 32 with alignment
            case ASIOSTInt32LSB16: {
                int32_t* out32 = (int32_t*)out;
                int32_t val = static_cast<int32_t>(std::clamp(sample, -1.0f, 1.0f) * MAX_S16);
                out32[outIndex] = val & 0x0000FFFF;
            } break;
            case ASIOSTInt32LSB18: {
                int32_t* out32 = (int32_t*)out;
                int32_t val = static_cast<int32_t>(std::clamp(sample, -1.0f, 1.0f) * MAX_S18);
                out32[outIndex] = val & 0x0003FFFF;
            } break;
            case ASIOSTInt32LSB20: {
                int32_t* out32 = (int32_t*)out;
                int32_t val = static_cast<int32_t>(std::clamp(sample, -1.0f, 1.0f) * MAX_S20);
                out32[outIndex] = val & 0x000FFFFF;
            } break;
            case ASIOSTInt32LSB24: {
                int32_t* out32 = (int32_t*)out;
                int32_t val = static_cast<int32_t>(std::clamp(sample, -1.0f, 1.0f) * MAX_S20);
                out32[outIndex] = val & 0x00FFFFFF;
            } break;
            default:
                // not supported yet
                //LOGD() << "not supported yet type: " << type;
                continue;
            }
        }
        ++outIndex;
    }

    // finally if the driver supports the ASIOOutputReady() optimization, do it here, all data are in place
    if (s_adata.postOutput) {
        ASIOOutputReady();
    }
}

static ASIOTime* s_bufferSwitchTimeInfo(ASIOTime* /*params*/, long index, ASIOBool processNow)
{
    s_bufferSwitch(index, processNow);
    return nullptr;
}

static void s_resetRequest()
{
    s_adata.resetRequest.notify();
}

static void s_sampleRateChanged(ASIOSampleRate rate)
{
    LOGI() << "rate: " << rate;
    s_resetRequest();
}

static long s_asioMessages(long selector, long value, void* /*message*/, double* /*opt*/)
{
    LOGI() << "selector: " << selector
           << " value: " << value;

    long ret = 0;
    switch (selector) {
    case kAsioSelectorSupported:
        if (value == kAsioEngineVersion
            || value == kAsioResetRequest
            || value == kAsioResyncRequest
            || value == kAsioLatenciesChanged
            ) {
            ret = 1L;
        }
        break;
    case kAsioResetRequest:
        // defer the task and perform the reset of the driver during the next "safe" situation
        // You cannot reset the driver right now, as this code is called from the driver.
        // Reset the driver is done by completely destruct is. I.e. ASIOStop(), ASIODisposeBuffers(), Destruction
        // Afterwards you initialize the driver again.
        s_resetRequest();
        ret = 1L;
        break;
    case kAsioBufferSizeChange:
        s_resetRequest();
        ret = 1L;
    case kAsioResyncRequest:
        // This informs the application, that the driver encountered some non fatal data loss.
        // It is used for synchronization purposes of different media.
        // Added mainly to work around the Win16Mutex problems in Windows 95/98 with the
        // Windows Multimedia system, which could loose data because the Mutex was hold too long
        // by another thread.
        // However a driver can issue it in other situations, too.
        s_resetRequest();
        ret = 1L;
        break;
    case kAsioLatenciesChanged:
        // This will inform the host application that the drivers were latencies changed.
        // Beware, it this does not mean that the buffer sizes have changed!
        // You might need to update internal delay data.
        s_resetRequest();
        ret = 1L;
        break;
    case kAsioEngineVersion:
        // return the supported ASIO version of the host application
        // If a host applications does not implement this selector, ASIO 1.0 is assumed
        // by the driver
        ret = 1L;
        break;
    case kAsioSupportsTimeInfo:
        // informs the driver wether the asioCallbacks.bufferSwitchTimeInfo() callback
        // is supported.
        // For compatibility with ASIO 1.0 drivers the host application should always support
        // the "old" bufferSwitch method, too.
        ret = 0L;
        break;
    case kAsioSupportsTimeCode:
        // informs the driver wether application is interested in time code info.
        // If an application does not need to know about time code, the driver has less work
        // to do.
        ret = 0L;
        break;
    }
    return ret;
}

static ASIOError create_asio_buffers(long bufferSize, long outputChannels, long inputChannels = 0)
{
    // Create buffer info array
    long totalChannels = inputChannels + outputChannels;
    s_adata.bufferInfos = new ASIOBufferInfo[totalChannels];

    // Setup output buffers
    for (long i = 0; i < outputChannels; i++) {
        s_adata.bufferInfos[i].isInput = ASIOFalse;
        s_adata.bufferInfos[i].channelNum = i;
        s_adata.bufferInfos[i].buffers[0] = s_adata.bufferInfos[i].buffers[1] = nullptr;
    }

    // Setup input buffers
    for (long i = 0; i < inputChannels; i++) {
        s_adata.bufferInfos[outputChannels + i].isInput = ASIOTrue;
        s_adata.bufferInfos[outputChannels + i].channelNum = i;
        s_adata.bufferInfos[outputChannels + i].buffers[0] = s_adata.bufferInfos[outputChannels + i].buffers[1] = nullptr;
    }

    // Create buffers
    ASIOError result = ASIOCreateBuffers(s_adata.bufferInfos, totalChannels, bufferSize, &s_adata.callbacks);
    if (result != ASE_OK) {
        LOGE() << "failed create buffers";
        delete[] s_adata.bufferInfos;
        s_adata.bufferInfos = nullptr;
        return result;
    }

    // Get channels info
    s_adata.channelInfos = new ASIOChannelInfo[totalChannels];
    for (long i = 0; i < totalChannels; i++) {
        s_adata.channelInfos[i].channel = s_adata.bufferInfos[i].channelNum;
        s_adata.channelInfos[i].isInput = s_adata.bufferInfos[i].isInput;
        result = ASIOGetChannelInfo(&s_adata.channelInfos[i]);
        if (result != ASE_OK) {
            LOGE() << "failed get channels info";
            delete[] s_adata.channelInfos;
            s_adata.channelInfos = nullptr;
            return result;
        }
    }

    return ASE_OK;
}

AudioDeviceID AsioAudioDriver::defaultDevice() const
{
    AudioDeviceList devices = availableOutputDevices();
    if (devices.empty()) {
        return AudioDeviceID();
    }
    return devices.at(0).id;
}

bool AsioAudioDriver::open(const Spec& spec, Spec* activeSpec)
{
    LOGI() << "try open: " << spec.deviceId;
    IF_ASSERT_FAILED(!spec.deviceId.empty()) {
        return false;
    }

    const char* name = spec.deviceId.c_str();
    bool ok = s_adata.drivers.loadDriver(const_cast<char*>(name));
    if (!ok) {
        LOGE() << "failed load driver: " << name;
        return ok;
    }

    ok = ASIOInit(&s_adata.driverInfo) == ASE_OK;
    if (!ok) {
        LOGE() << "failed init driver: " << name << ", error: " << s_adata.driverInfo.errorMessage;
        return ok;
    }

    LOGI() << "asioVersion: " << s_adata.driverInfo.asioVersion
           << " driverVersion: " << s_adata.driverInfo.driverVersion
           << " driverName: " << s_adata.driverInfo.name;

    // Get device metrics
    AsioData::DeviceMetrics& metrics = s_adata.deviceMetrics;
    ok =  ASIOGetChannels(&metrics.inputChannels, &metrics.outputChannels) == ASE_OK;
    if (!ok) {
        LOGE() << "failed get num of channels, driver: " << name;
        return ok;
    }

    LOGI() << "device outputChannels: " << metrics.outputChannels;

    ok = ASIOGetBufferSize(&metrics.minSize, &metrics.maxSize,
                           &metrics.preferredSize, &metrics.granularity) == ASE_OK;
    if (!ok) {
        LOGE() << "failed get buffer size, driver: " << name;
        return ok;
    }

    LOGI() << "ASIOGetBufferSize"
           << " min: " << metrics.minSize
           << " max: " << metrics.maxSize
           << " preferred: " << metrics.preferredSize
           << " granularity: " << metrics.granularity;

    ok = ASIOGetSampleRate(&metrics.sampleRate) == ASE_OK;
    if (!ok) {
        LOGE() << "failed get sample rate, driver: " << name;
        return ok;
    }

    // Set active
    s_adata.activeSpec = spec;
    OutputSpec& active = s_adata.activeSpec.output;
    active.audioChannelCount = 2;

    active.samplesPerChannel = std::clamp(spec.output.samplesPerChannel,
                                          (samples_t)metrics.minSize,
                                          (samples_t)metrics.maxSize);

    ok = ASIOSetSampleRate(static_cast<double>(spec.output.sampleRate)) == ASE_OK;
    if (!ok) {
        LOGW() << "failed set sample rate: " << spec.output.sampleRate << ", driver: " << name;
    }
    active.sampleRate = ok ? spec.output.sampleRate : static_cast<sample_rate_t>(metrics.sampleRate);

    LOGI() << "active spec"
           << " audioChannelCount: " << active.audioChannelCount
           << " samplesPerChannel: " << active.samplesPerChannel
           << " sampleRate: " << active.sampleRate;

    m_activeSpecChanged.send(s_adata.activeSpec);
    if (activeSpec) {
        *activeSpec = s_adata.activeSpec;
    }

    if (ASIOOutputReady() == ASE_OK) {
        s_adata.postOutput = true;
    } else {
        s_adata.postOutput = false;
    }

    LOGI() << "ASIOOutputReady - " << (s_adata.postOutput ? "Supported" : "Not supported");

    s_adata.callbacks.bufferSwitch = &s_bufferSwitch;
    s_adata.callbacks.bufferSwitchTimeInfo = s_bufferSwitchTimeInfo;
    s_adata.callbacks.sampleRateDidChange = &s_sampleRateChanged;
    s_adata.callbacks.asioMessage = &s_asioMessages;

    ok = create_asio_buffers((long)active.samplesPerChannel, (long)active.audioChannelCount) == ASE_OK;
    if (!ok) {
        LOGE() << "failed create asio buffers, driver: " << name;
        return ok;
    }

    s_adata.resetRequest.onNotify(this, [this]() {
        reset();
    }, async::Asyncable::Mode::SetReplace);

    m_running = true;
    m_thread = std::thread([this]() {
        bool ok = ASIOStart() == ASE_OK;
        if (!ok) {
            LOGE() << "failed asio start";
            return;
        }

        LOGI() << "ASIO thread started: " << std::this_thread::get_id();

        // Main thread loop
        while (m_running) {
            // ASIO callbacks will be called in this thread
            // We just need to keep the thread alive
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }

        // Stop ASIO
        ASIOStop();
        LOGI() << "ASIO thread stopped";
    });

    return true;
}

void AsioAudioDriver::close()
{
    m_running = false;
    if (m_thread.joinable()) {
        m_thread.join();
    }

    ASIODisposeBuffers();

    delete[] s_adata.bufferInfos;
    s_adata.bufferInfos = nullptr;
    delete[] s_adata.channelInfos;
    s_adata.channelInfos = nullptr;

    // don't use
    // ASIOExit();

    s_adata.drivers.removeCurrentDriver();
}

void AsioAudioDriver::reset()
{
    LOGI() << "! driver reset called";
    if (!isOpened()) {
        return;
    }

    Spec spec = s_adata.activeSpec;
    close();
    open(spec, nullptr);
}

bool AsioAudioDriver::isOpened() const
{
    return m_running;
}

const AsioAudioDriver::Spec& AsioAudioDriver::activeSpec() const
{
    return s_adata.activeSpec;
}

async::Channel<AsioAudioDriver::Spec> AsioAudioDriver::activeSpecChanged() const
{
    return m_activeSpecChanged;
}

std::vector<samples_t> AsioAudioDriver::availableOutputDeviceBufferSizes() const
{
    if (!isOpened()) {
        return {
            128,
            256,
            512,
            1024,
            2048,
        };
    }

    std::vector<samples_t> result;

    samples_t n = s_adata.deviceMetrics.maxSize;
    samples_t min = s_adata.deviceMetrics.minSize;

    while (n >= min) {
        result.push_back(n);
        n /= 2;
    }

    return result;
}

std::vector<sample_rate_t> AsioAudioDriver::availableOutputDeviceSampleRates() const
{
    return {
        44100,
        48000
    };
}

AudioDeviceList AsioAudioDriver::availableOutputDevices() const
{
    char names[16][32];
    char* pointers[16];

    for (int i = 0; i < 16; i++) {
        pointers[i] = names[i];
    }

    long count = s_adata.drivers.getDriverNames(pointers, 16);

    AudioDeviceList devices;
    devices.reserve(count);
    for (long i = 0; i < count; i++) {
        AudioDevice d;
        d.id = names[i];
        d.name = d.id;
        devices.push_back(std::move(d));
    }

    return devices;
}

async::Notification AsioAudioDriver::availableOutputDevicesChanged() const
{
    return m_availableOutputDevicesChanged;
}
