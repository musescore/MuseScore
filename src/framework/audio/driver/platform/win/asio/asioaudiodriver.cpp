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

#include "ASIOSDK/common/asio.h"
#include "ASIOSDK/host/asiodrivers.h"

#include "log.h"

using namespace muse;
using namespace muse::audio;

struct Data {
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
};

static Data s_data;

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

static void s_bufferSwitch(long index, ASIOBool /*processNow*/)
{
    const IAudioDriver::Spec& active = s_data.activeSpec;

    const size_t channels = active.output.audioChannelCount;
    const size_t procSamplesTotal = active.output.audioChannelCount * active.output.samplesPerChannel;

    static std::vector<float> proc_buf;
    if (proc_buf.size() != procSamplesTotal) {
        proc_buf.resize(procSamplesTotal);
    }

    uint8_t* stream = reinterpret_cast<uint8_t*>(&proc_buf[0]);
    active.callback(nullptr, stream, (int)(procSamplesTotal * sizeof(float)));

    size_t preChannelSample = 0;
    for (size_t s = 0; s < procSamplesTotal; s += channels) {
        for (size_t c = 0; c < channels; c++) {
            const float sample = proc_buf.at(s + c);
            uint8_t* out = (uint8_t*)s_data.bufferInfos[c].buffers[index];

            const ASIOSampleType type = s_data.channelInfos[c].type;
            switch (type) {
            case ASIOSTInt24LSB: { // 3 bytes
                int32_t val = std::clamp(sample, -1.0f, 1.0f) * 8388608.0f;
                size_t outIndex = preChannelSample * 3;
                out[outIndex]=(val) & 0xff;
                out[outIndex + 1]=(val >> 8) & 0xff;
                out[outIndex + 2]=(val >> 16) & 0xff;
            } break;
            default:
                // not supported yet
                continue;
            }
        }
        ++preChannelSample;
    }

    // finally if the driver supports the ASIOOutputReady() optimization, do it here, all data are in place
    if (s_data.postOutput) {
        ASIOOutputReady();
    }
}

static ASIOTime* s_bufferSwitchTimeInfo(ASIOTime* /*params*/, long index, ASIOBool processNow)
{
    s_bufferSwitch(index, processNow);
    return nullptr;
}

static void s_sampleRateChanged(ASIOSampleRate rate)
{
    LOGI() << "rate: " << rate;
}

static long s_asioMessages(long selector, long value, void* message, double* /*opt*/)
{
    LOGI() << "selector: " << selector
           << "value: " << value
           << "message: " << (const char*)message;

    long ret = 0;
    switch (selector) {
    case kAsioSelectorSupported:
        if (value == kAsioEngineVersion
            // || value == kAsioResetRequest
            // || value == kAsioResyncRequest
            // || value == kAsioLatenciesChanged
            // // the following three were added for ASIO 2.0, you don't necessarily have to support them
            // || value == kAsioSupportsTimeInfo
            // || value == kAsioSupportsTimeCode
            // || value == kAsioSupportsInputMonitor
            ) {
            ret = 1L;
        }
        break;
    case kAsioResetRequest:
        // defer the task and perform the reset of the driver during the next "safe" situation
        // You cannot reset the driver right now, as this code is called from the driver.
        // Reset the driver is done by completely destruct is. I.e. ASIOStop(), ASIODisposeBuffers(), Destruction
        // Afterwards you initialize the driver again.
        ret = 0L;
        break;
    case kAsioResyncRequest:
        // This informs the application, that the driver encountered some non fatal data loss.
        // It is used for synchronization purposes of different media.
        // Added mainly to work around the Win16Mutex problems in Windows 95/98 with the
        // Windows Multimedia system, which could loose data because the Mutex was hold too long
        // by another thread.
        // However a driver can issue it in other situations, too.
        ret = 1L;
        break;
    case kAsioLatenciesChanged:
        // This will inform the host application that the drivers were latencies changed.
        // Beware, it this does not mean that the buffer sizes have changed!
        // You might need to update internal delay data.
        ret = 0L;
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
    s_data.bufferInfos = new ASIOBufferInfo[totalChannels];

    // Setup output buffers
    for (long i = 0; i < outputChannels; i++) {
        s_data.bufferInfos[i].isInput = ASIOFalse;
        s_data.bufferInfos[i].channelNum = i;
        s_data.bufferInfos[i].buffers[0] = s_data.bufferInfos[i].buffers[1] = nullptr;
    }

    // Setup input buffers
    for (long i = 0; i < inputChannels; i++) {
        s_data.bufferInfos[outputChannels + i].isInput = ASIOTrue;
        s_data.bufferInfos[outputChannels + i].channelNum = i;
        s_data.bufferInfos[outputChannels + i].buffers[0] = s_data.bufferInfos[outputChannels + i].buffers[1] = nullptr;
    }

    // Create buffers
    ASIOError result = ASIOCreateBuffers(s_data.bufferInfos, totalChannels, bufferSize, &s_data.callbacks);
    if (result != ASE_OK) {
        LOGE() << "failed create buffers";
        delete[] s_data.bufferInfos;
        s_data.bufferInfos = nullptr;
        return result;
    }

    // Get channels info
    s_data.channelInfos = new ASIOChannelInfo[totalChannels];
    for (long i = 0; i < totalChannels; i++) {
        s_data.channelInfos[i].channel = s_data.bufferInfos[i].channelNum;
        s_data.channelInfos[i].isInput = s_data.bufferInfos[i].isInput;
        result = ASIOGetChannelInfo(&s_data.channelInfos[i]);
        if (result != ASE_OK) {
            LOGE() << "failed get channels info";
            delete[] s_data.channelInfos;
            s_data.channelInfos = nullptr;
            return result;
        }
    }

    return ASE_OK;
}

bool AsioAudioDriver::open(const Spec& spec, Spec* activeSpec)
{
    if (m_deviceId.empty()) {
        AudioDeviceList devices = availableOutputDevices();
        if (devices.empty()) {
            return false;
        }
        m_deviceId = devices.at(1).id;
    }

    return doOpen(m_deviceId, spec, activeSpec);
}

bool AsioAudioDriver::doOpen(const AudioDeviceID& device, const Spec& spec, Spec* activeSpec)
{
    const char* name = device.c_str();
    bool ok = s_data.drivers.loadDriver(const_cast<char*>(name));
    if (!ok) {
        LOGE() << "failed load driver: " << name;
        return ok;
    }

    ok = ASIOInit(&s_data.driverInfo) == ASE_OK;
    if (!ok) {
        LOGE() << "failed init driver: " << name << ", error: " << s_data.driverInfo.errorMessage;
        return ok;
    }

    LOGI() << "asioVersion: " << s_data.driverInfo.asioVersion
           << " driverVersion: " << s_data.driverInfo.driverVersion
           << " name: " << s_data.driverInfo.name;

    // Get device metrics
    Data::DeviceMetrics& metrics = s_data.deviceMetrics;
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
    s_data.activeSpec = spec;
    OutputSpec& active = s_data.activeSpec.output;
    active.audioChannelCount = 2;

    active.samplesPerChannel = std::clamp(spec.output.samplesPerChannel,
                                          (samples_t)metrics.minSize,
                                          (samples_t)metrics.maxSize);

    ok = ASIOSetSampleRate(spec.output.sampleRate) == ASE_OK;
    if (!ok) {
        LOGW() << "failed set sample rate: " << spec.output.sampleRate << ", driver: " << name;
    }
    active.sampleRate = ok ? spec.output.sampleRate : metrics.sampleRate;

    LOGI() << "active spec"
           << " audioChannelCount: " << active.audioChannelCount
           << " samplesPerChannel: " << active.samplesPerChannel
           << " sampleRate: " << active.sampleRate;

    m_activeSpecChanged.send(s_data.activeSpec);
    if (activeSpec) {
        *activeSpec = s_data.activeSpec;
    }

    if (ASIOOutputReady() == ASE_OK) {
        s_data.postOutput = true;
    } else {
        s_data.postOutput = false;
    }

    LOGI() << "ASIOOutputReady - " << (s_data.postOutput ? "Supported" : "Not supported");

    s_data.callbacks.bufferSwitch = &s_bufferSwitch;
    s_data.callbacks.bufferSwitchTimeInfo = s_bufferSwitchTimeInfo;
    s_data.callbacks.sampleRateDidChange = &s_sampleRateChanged;
    s_data.callbacks.asioMessage = &s_asioMessages;

    ok = create_asio_buffers(active.samplesPerChannel, active.audioChannelCount) == ASE_OK;
    if (!ok) {
        LOGE() << "failed create asio buffers, driver: " << name;
        return ok;
    }

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
    m_thread.join();

    ASIODisposeBuffers();

    delete[] s_data.bufferInfos;
    s_data.bufferInfos = nullptr;
    delete[] s_data.channelInfos;
    s_data.channelInfos = nullptr;
}

bool AsioAudioDriver::isOpened() const
{
    return m_running;
}

const AsioAudioDriver::Spec& AsioAudioDriver::activeSpec() const
{
    return s_data.activeSpec;
}

async::Channel<AsioAudioDriver::Spec> AsioAudioDriver::activeSpecChanged() const
{
    return m_activeSpecChanged;
}

bool AsioAudioDriver::setOutputDeviceBufferSize(unsigned int bufferSize)
{
    return false;
}

async::Notification AsioAudioDriver::outputDeviceBufferSizeChanged() const
{
    return m_outputDeviceBufferSizeChanged;
}

bool AsioAudioDriver::setOutputDeviceSampleRate(unsigned int sampleRate)
{
    return false;
}

async::Notification AsioAudioDriver::outputDeviceSampleRateChanged() const
{
    return m_outputDeviceSampleRateChanged;
}

std::vector<unsigned int> AsioAudioDriver::availableOutputDeviceBufferSizes() const
{
    return {};
}

std::vector<unsigned int> AsioAudioDriver::availableOutputDeviceSampleRates() const
{
    return {};
}

AudioDeviceID AsioAudioDriver::outputDevice() const
{
    return m_deviceId;
}

bool AsioAudioDriver::selectOutputDevice(const AudioDeviceID& id)
{
    return false;
}

bool AsioAudioDriver::resetToDefaultOutputDevice()
{
    return false;
}

async::Notification AsioAudioDriver::outputDeviceChanged() const
{
    return m_outputDeviceChanged;
}

AudioDeviceList AsioAudioDriver::availableOutputDevices() const
{
    char names[16][32];
    char* pointers[16];

    for (int i = 0; i < 16; i++) {
        pointers[i] = names[i];
    }

    long count = s_data.drivers.getDriverNames(pointers, 16);

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

void AsioAudioDriver::resume()
{
}

void AsioAudioDriver::suspend()
{
}
