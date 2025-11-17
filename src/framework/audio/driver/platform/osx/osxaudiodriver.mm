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

#include "osxaudiodriver.h"

#include <mutex>

#include <AudioToolbox/AudioToolbox.h>

#include "translation.h"
#include "log.h"

typedef AudioDeviceID OSXAudioDeviceID;

using namespace muse;
using namespace muse::audio;

struct OSXAudioDriver::Data {
    Spec format;
    AudioQueueRef audioQueue;
    Callback callback;
};

OSXAudioDriver::OSXAudioDriver()
    : m_data(nullptr)
{
    m_data = std::make_shared<Data>();
    m_data->audioQueue = nullptr;

    initDeviceMapListener();
    updateDeviceMap();
}

OSXAudioDriver::~OSXAudioDriver()
{
    close();
}

void OSXAudioDriver::init()
{
}

std::string OSXAudioDriver::name() const
{
    return "OSX";
}

muse::audio::AudioDeviceID OSXAudioDriver::defaultDevice() const
{
    return DEFAULT_DEVICE_ID;
}

bool OSXAudioDriver::open(const Spec& spec, Spec* activeSpec)
{
    if (!m_data) {
        return 0;
    }

    if (isOpened()) {
        return 0;
    }

    if (activeSpec) {
        *activeSpec = spec;
    }

    m_data->format = spec;
    m_activeSpecChanged.send(m_data->format);

    AudioStreamBasicDescription audioFormat;
    audioFormat.mSampleRate = spec.output.sampleRate;
    audioFormat.mFormatID = kAudioFormatLinearPCM;
    audioFormat.mFramesPerPacket = 1;
    audioFormat.mChannelsPerFrame = spec.output.audioChannelCount;
    audioFormat.mReserved = 0;
    audioFormat.mBitsPerChannel = 32;
    audioFormat.mFormatFlags = kLinearPCMFormatFlagIsFloat;
    audioFormat.mBytesPerPacket = audioFormat.mBitsPerChannel * spec.output.audioChannelCount / 8;
    audioFormat.mBytesPerFrame = audioFormat.mBytesPerPacket * audioFormat.mFramesPerPacket;

    m_data->callback = spec.callback;

    OSStatus result = AudioQueueNewOutput(&audioFormat, OnFillBuffer, m_data.get(), NULL, NULL, 0, &m_data->audioQueue);
    if (result != noErr) {
        logError("Failed to create Audio Queue Output, err: ", result);
        return false;
    }

    audioQueueSetDeviceName(m_data->format.deviceId);

    AudioValueRange bufferSizeRange = { 0, 0 };
    UInt32 bufferSizeRangeSize = sizeof(AudioValueRange);
    AudioObjectPropertyAddress bufferSizeRangeAddress = {
        .mSelector = kAudioDevicePropertyBufferFrameSizeRange,
        .mScope = kAudioObjectPropertyScopeGlobal,
        .mElement = kAudioObjectPropertyElementMaster
    };
    result = AudioObjectGetPropertyData(osxDeviceId(), &bufferSizeRangeAddress, 0, 0, &bufferSizeRangeSize, &bufferSizeRange);
    if (result != noErr) {
        logError("Failed to create Audio Queue Output, err: ", result);
        return false;
    }

    samples_t minBufferSize = static_cast<samples_t>(bufferSizeRange.mMinimum);
    samples_t maxBufferSize = static_cast<samples_t>(bufferSizeRange.mMaximum);
    UInt32 bufferSizeOut = std::min(maxBufferSize, std::max(minBufferSize, spec.output.samplesPerChannel));

    AudioObjectPropertyAddress preferredBufferSizeAddress = {
        .mSelector = kAudioDevicePropertyBufferFrameSize,
        .mScope = kAudioObjectPropertyScopeGlobal,
        .mElement = kAudioObjectPropertyElementMaster
    };

    result = AudioObjectSetPropertyData(osxDeviceId(), &preferredBufferSizeAddress, 0, 0, sizeof(bufferSizeOut), (void*)&bufferSizeOut);
    if (result != noErr) {
        logError("Failed to create Audio Queue Output, err: ", result);
        return false;
    }

    // Allocate 2 audio buffers. At the same time one used for writing, one for reading
    for (unsigned int i = 0; i < 2; ++i) {
        AudioQueueBufferRef buffer;
        result = AudioQueueAllocateBuffer(m_data->audioQueue, spec.output.samplesPerChannel * audioFormat.mBytesPerFrame, &buffer);
        if (result != noErr) {
            logError("Failed to allocate Audio Buffer, err: ", result);
            return false;
        }

        buffer->mAudioDataByteSize = spec.output.samplesPerChannel * audioFormat.mBytesPerFrame;

        memset(buffer->mAudioData, 0, buffer->mAudioDataByteSize);

        AudioQueueEnqueueBuffer(m_data->audioQueue, buffer, 0, NULL);
    }

    // start playback
    result = AudioQueueStart(m_data->audioQueue, NULL);
    if (result != noErr) {
        logError("Failed to start  Audio Queue, err: ", result);
        return false;
    }

    LOGI() << "Connected to " << m_data->format.deviceId
           << " with bufferSize " << bufferSizeOut
           << ", sampleRate " << spec.output.sampleRate;

    return true;
}

void OSXAudioDriver::close()
{
    if (isOpened()) {
        AudioQueueStop(m_data->audioQueue, true);
        AudioQueueDispose(m_data->audioQueue, true);
        m_data->audioQueue = nullptr;
    }
}

bool OSXAudioDriver::isOpened() const
{
    return m_data->audioQueue != nullptr;
}

const OSXAudioDriver::Spec& OSXAudioDriver::activeSpec() const
{
    return m_data->format;
}

async::Channel<OSXAudioDriver::Spec> OSXAudioDriver::activeSpecChanged() const
{
    return m_activeSpecChanged;
}

AudioDeviceList OSXAudioDriver::availableOutputDevices() const
{
    std::lock_guard lock(m_devicesMutex);

    AudioDeviceList deviceList;
    deviceList.push_back({ DEFAULT_DEVICE_ID, muse::trc("audio", "System default") });

    for (auto& device : m_outputDevices) {
        AudioDevice deviceInfo;
        deviceInfo.id = QString::number(device.first).toStdString();
        deviceInfo.name = device.second;

        deviceList.push_back(deviceInfo);
    }

    return deviceList;
}

async::Notification OSXAudioDriver::availableOutputDevicesChanged() const
{
    return m_availableOutputDevicesChanged;
}

void OSXAudioDriver::updateDeviceMap()
{
    std::lock_guard lock(m_devicesMutex);

    UInt32 propertySize;
    OSStatus result;
    std::vector<AudioObjectID> audioObjects = {};
    m_outputDevices.clear();
    m_inputDevices.clear();

    AudioObjectPropertyAddress devicesPropertyAddress = {
        .mSelector = kAudioHardwarePropertyDevices,
        .mScope = kAudioObjectPropertyScopeGlobal,
        .mElement = kAudioObjectPropertyElementMaster,
    };

    AudioObjectPropertyAddress namePropertyAddress = {
        .mSelector = kAudioDevicePropertyDeviceNameCFString,
        .mScope = kAudioObjectPropertyScopeGlobal,
        .mElement = kAudioObjectPropertyElementMaster,
    };

    auto getStreamsCount
        = [](const AudioObjectID& id, const AudioObjectPropertyScope& scope, const std::string& deviceName) -> unsigned int {
        AudioObjectPropertyAddress propertyAddress = {
            .mSelector  = kAudioDevicePropertyStreamConfiguration,
            .mScope     = scope,
            .mElement   = kAudioObjectPropertyElementWildcard
        };
        UInt32 propertySize = 0;
        OSStatus result = AudioObjectGetPropertyDataSize(id, &propertyAddress, 0, NULL, &propertySize);
        if (result != noErr) {
            logError("Failed to get device's (" + deviceName + ") streams size, err: ", result);
            return 0;
        }

        std::unique_ptr<AudioBufferList> bufferList(reinterpret_cast<AudioBufferList*>(malloc(propertySize)));
        result = AudioObjectGetPropertyData(id, &propertyAddress, 0, NULL, &propertySize, bufferList.get());
        if (result != noErr) {
            logError("Failed to get device's (" + deviceName + ") streams, err: ", result);
            return 0;
        }

        return bufferList->mNumberBuffers;
    };

    result = AudioObjectGetPropertyDataSize(kAudioObjectSystemObject, &devicesPropertyAddress, 0, NULL, &propertySize);
    if (result != noErr) {
        logError("Failed to get devices count, err: ", result);
        return;
    }

    audioObjects.resize(propertySize / sizeof(OSXAudioDeviceID));
    result = AudioObjectGetPropertyData(kAudioObjectSystemObject, &devicesPropertyAddress, 0, NULL, &propertySize, audioObjects.data());
    if (result != noErr) {
        logError("Failed to get devices list, err: ", result);
        return;
    }

    for (auto&& deviceId : audioObjects) {
        CFStringRef nameRef;
        propertySize = sizeof(nameRef);

        result = AudioObjectGetPropertyData(deviceId, &namePropertyAddress, 0, NULL, &propertySize, &nameRef);
        if (result != noErr) {
            logError("Failed to get device's name, err: ", result);
            continue;
        }

        NSString* nsString = (NSString*)nameRef;
        std::string deviceName = [nsString UTF8String];

        if (getStreamsCount(deviceId, kAudioObjectPropertyScopeOutput, deviceName) > 0) {
            m_outputDevices[deviceId] = deviceName;
        }

        if (getStreamsCount(deviceId, kAudioObjectPropertyScopeInput, deviceName) > 0) {
            m_inputDevices[deviceId] = deviceName;
        }

        CFRelease(nameRef);
    }
    m_availableOutputDevicesChanged.notify();
}

std::vector<samples_t> OSXAudioDriver::availableOutputDeviceBufferSizes() const
{
    OSXAudioDeviceID osxDeviceId = this->osxDeviceId();
    AudioObjectPropertyAddress bufferFrameSizePropertyAddress = {
        .mSelector = kAudioDevicePropertyBufferFrameSizeRange,
        .mScope = kAudioObjectPropertyScopeGlobal,
        .mElement = kAudioObjectPropertyElementMaster
    };

    AudioValueRange range = { 0, 0 };
    UInt32 dataSize = sizeof(AudioValueRange);
    OSStatus rangeResult = AudioObjectGetPropertyData(osxDeviceId, &bufferFrameSizePropertyAddress, 0, NULL, &dataSize, &range);
    if (rangeResult != noErr) {
        logError("Failed to get device " + m_data->format.deviceId + " bufferFrameSize, err: ", rangeResult);
        return {};
    }

    samples_t minimum = std::max(static_cast<samples_t>(range.mMinimum), MINIMUM_BUFFER_SIZE);
    samples_t maximum = std::min(static_cast<samples_t>(range.mMaximum), MAXIMUM_BUFFER_SIZE);

    std::vector<samples_t> result;
    for (samples_t bufferSize = maximum; bufferSize >= minimum;) {
        result.push_back(bufferSize);
        bufferSize /= 2;
    }

    std::sort(result.begin(), result.end());

    return result;
}

std::vector<sample_rate_t> OSXAudioDriver::availableOutputDeviceSampleRates() const
{
    return {
        44100,
        48000,
        88200,
        96000,
    };
}

bool OSXAudioDriver::audioQueueSetDeviceName(const AudioDeviceID& deviceId)
{
    if (deviceId.empty() || deviceId == DEFAULT_DEVICE_ID) {
        return true; //default device used
    }

    std::lock_guard lock(m_devicesMutex);

    uint deviceIdInt = QString::fromStdString(deviceId).toInt();
    auto index = std::find_if(m_outputDevices.begin(), m_outputDevices.end(), [&deviceIdInt](auto& d) {
        return d.first == deviceIdInt;
    });

    if (index == m_outputDevices.end()) {
        LOGW() << "device " << deviceId << " not found";
        return false;
    }

    OSXAudioDeviceID osxDeviceId = index->first;

    CFStringRef deviceUID;
    UInt32 deviceUIDSize = sizeof(deviceUID);
    AudioObjectPropertyAddress propertyAddress;
    propertyAddress.mSelector = kAudioDevicePropertyDeviceUID;
    propertyAddress.mScope = kAudioDevicePropertyScopeOutput;
    propertyAddress.mElement = kAudioObjectPropertyElementMaster;

    auto result = AudioObjectGetPropertyData(osxDeviceId, &propertyAddress, 0, NULL, &deviceUIDSize, &deviceUID);
    if (result != noErr) {
        logError("Failed to get device UID, err: ", result);
        return false;
    }
    result = AudioQueueSetProperty(m_data->audioQueue, kAudioQueueProperty_CurrentDevice, &deviceUID, deviceUIDSize);
    if (result != noErr) {
        logError("Failed to set device by UID, err: ", result);
        return false;
    }
    return true;
}

muse::audio::AudioDeviceID OSXAudioDriver::defaultDeviceId() const
{
    OSXAudioDeviceID osxDeviceId = kAudioObjectUnknown;
    UInt32 deviceIdSize = sizeof(osxDeviceId);

    AudioObjectPropertyAddress deviceNamePropertyAddress = {
        .mSelector = kAudioHardwarePropertyDefaultOutputDevice,
        .mScope = kAudioDevicePropertyScopeOutput,
        .mElement = kAudioObjectPropertyElementMaster
    };

    OSStatus result = AudioObjectGetPropertyData(kAudioObjectSystemObject, &deviceNamePropertyAddress, 0, 0, &deviceIdSize, &osxDeviceId);
    if (result != noErr) {
        logError("Failed to get default device ID, err: ", result);
        return AudioDeviceID();
    }

    return QString::number(osxDeviceId).toStdString();
}

UInt32 OSXAudioDriver::osxDeviceId() const
{
    AudioDeviceID deviceId = m_data->format.deviceId;
    if (deviceId == DEFAULT_DEVICE_ID) {
        deviceId = defaultDeviceId();
    }

    return QString::fromStdString(deviceId).toInt();
}

void OSXAudioDriver::logError(const std::string message, OSStatus error)
{
    if (error == noErr) {
        return;
    }

    char errorString[5];

    UInt32 errorBigEndian = CFSwapInt32HostToBig(error);
    errorString[0] = errorBigEndian & 0xFF;
    errorString[1] = (errorBigEndian >> 8) & 0xFF;
    errorString[2] = (errorBigEndian >> 16) & 0xFF;
    errorString[3] = (errorBigEndian >> 24) & 0xFF;
    errorString[4] = '\0';
    if (isprint(errorString[0]) && isprint(errorString[1]) && isprint(errorString[2]) && isprint(errorString[3])) {
        LOGE() << message << errorString << "(" << error << ")";
    } else {
        LOGE() << message << error;
    }
}

static OSStatus onDeviceListChanged(AudioObjectID inObjectID, UInt32 inNumberAddresses, const AudioObjectPropertyAddress* inAddresses,
                                    void* inClientData)
{
    UNUSED(inObjectID);
    UNUSED(inNumberAddresses);
    UNUSED(inAddresses);
    auto driver = reinterpret_cast<OSXAudioDriver*>(inClientData);
    driver->updateDeviceMap();

    return noErr;
}

void OSXAudioDriver::initDeviceMapListener()
{
    AudioObjectPropertyAddress propertyAddress;
    propertyAddress.mSelector = kAudioHardwarePropertyDevices;
    propertyAddress.mScope = kAudioObjectPropertyScopeGlobal;
    propertyAddress.mElement = kAudioObjectPropertyElementMaster;

    auto result = AudioObjectAddPropertyListener(kAudioObjectSystemObject, &propertyAddress, &onDeviceListChanged, this);
    if (result != noErr) {
        logError("Failed to add devices list listener, err: ", result);
    }
}

/*static*/
void OSXAudioDriver::OnFillBuffer(void* context, AudioQueueRef, AudioQueueBufferRef buffer)
{
    Data* pData = (Data*)context;
    pData->callback((uint8_t*)buffer->mAudioData, buffer->mAudioDataByteSize);
    AudioQueueEnqueueBuffer(pData->audioQueue, buffer, 0, NULL);
}
