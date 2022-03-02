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

#include "osxaudiodriver.h"

#include <mutex>
#include <AudioToolbox/AudioToolbox.h>
#include "log.h"

using namespace mu::audio;

struct OSXAudioDriver::Data {
    Spec format;
    AudioQueueRef audioQueue;
    Callback callback;
    void* mUserData;
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

const std::string OSXAudioDriver::DEFAULT_DEVICE_NAME = "Systems default";

std::string OSXAudioDriver::name() const
{
    return "MUAUDIO(OSX)";
}

bool OSXAudioDriver::open(const Spec& spec, Spec* activeSpec)
{
    if (!m_data) {
        return 0;
    }

    if (isOpened()) {
        return 0;
    }

    *activeSpec = spec;
    activeSpec->format = Format::AudioF32;
    m_data->format = *activeSpec;

    AudioStreamBasicDescription audioFormat;
    audioFormat.mSampleRate = spec.sampleRate;
    audioFormat.mFormatID = kAudioFormatLinearPCM;
    audioFormat.mFramesPerPacket = 1;
    audioFormat.mChannelsPerFrame = spec.channels;
    audioFormat.mReserved = 0;
    switch (activeSpec->format) {
    case Format::AudioF32:
        audioFormat.mBitsPerChannel = 32;
        audioFormat.mFormatFlags = kLinearPCMFormatFlagIsFloat;
        break;
    case Format::AudioS16:
        audioFormat.mBitsPerChannel = 16;
        audioFormat.mFormatFlags = kLinearPCMFormatFlagIsSignedInteger | kAudioFormatFlagIsPacked;
        break;
    }
    audioFormat.mBytesPerPacket = audioFormat.mBitsPerChannel * spec.channels / 8;
    audioFormat.mBytesPerFrame = audioFormat.mBytesPerPacket * audioFormat.mFramesPerPacket;

    m_data->callback = spec.callback;
    m_data->mUserData = spec.userdata;

    OSStatus result = AudioQueueNewOutput(&audioFormat, OnFillBuffer, m_data.get(), NULL, NULL, 0, &m_data->audioQueue);
    if (result != noErr) {
        logError("Failed to create Audio Queue Output, err: ", result);
        return false;
    }

    audioQueueSetDeviceName(m_deviceName);

    // Allocate 3 audio buffers. At the same time one used for writing, one for reading and one for reserve
    for (unsigned int i = 0; i < 3; ++i) {
        AudioQueueBufferRef buffer;
        result = AudioQueueAllocateBuffer(m_data->audioQueue, spec.samples * audioFormat.mBytesPerFrame, &buffer);
        if (result != noErr) {
            logError("Failed to allocate Audio Buffer, err: ", result);
            return false;
        }

        buffer->mAudioDataByteSize = spec.samples * audioFormat.mBytesPerFrame;

        memset(buffer->mAudioData, 0, buffer->mAudioDataByteSize);

        AudioQueueEnqueueBuffer(m_data->audioQueue, buffer, 0, NULL);
    }

    // start playback
    result = AudioQueueStart(m_data->audioQueue, NULL);
    if (result != noErr) {
        logError("Failed to start  Audio Queue, err: ", result);
        return false;
    }

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

std::vector<std::string> OSXAudioDriver::availableOutputDevices() const
{
    std::vector<std::string> deviceList = { DEFAULT_DEVICE_NAME };
    for (auto& device : m_outputDevices) {
        deviceList.push_back(device.second);
    }
    return deviceList;
}

mu::async::Notification OSXAudioDriver::availableOutputDevicesChanged() const
{
    return m_availableOutputDevicesChanged;
}

std::string OSXAudioDriver::outputDevice() const
{
    return m_deviceName;
}

void OSXAudioDriver::updateDeviceMap()
{
    UInt32 propertySize;
    OSStatus result;
    std::vector<AudioObjectID> audioObjects = {};
    m_outputDevices.clear();

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

    audioObjects.resize(propertySize / sizeof(AudioDeviceID));
    result = AudioObjectGetPropertyData(kAudioObjectSystemObject, &devicesPropertyAddress, 0, NULL, &propertySize, audioObjects.data());
    if (result != noErr) {
        logError("Failed to get devices list, err: ", result);
        return;
    }

    for (auto&& deviceId : audioObjects) {
        std::string deviceName;
        CFStringRef nameRef;

        result = AudioObjectGetPropertyData(deviceId, &namePropertyAddress, 0, NULL, &propertySize, &nameRef);
        if (result != noErr) {
            logError("Failed to get device's name, err: ", result);
            continue;
        }
        propertySize = CFStringGetLength(nameRef);
        deviceName.resize(propertySize);
        CFStringGetCString(nameRef, deviceName.data(), sizeof(deviceName), kCFStringEncodingUTF8);

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

bool OSXAudioDriver::audioQueueSetDeviceName(const std::string& deviceName)
{
    if (deviceName.empty() || deviceName == DEFAULT_DEVICE_NAME) {
        return true; //default device used
    }

    auto index = std::find_if(m_outputDevices.begin(), m_outputDevices.end(), [&deviceName](auto& d) { return d.second == deviceName; });
    if (index == m_outputDevices.end()) {
        LOGW() << "device " << deviceName << " not found";
        return false;
    }
    AudioDeviceID deviceId = index->first;

    CFStringRef deviceUID;
    UInt32 deviceUIDSize = sizeof(deviceUID);
    AudioObjectPropertyAddress propertyAddress;
    propertyAddress.mSelector = kAudioDevicePropertyDeviceUID;
    propertyAddress.mScope = kAudioDevicePropertyScopeOutput;
    propertyAddress.mElement = kAudioObjectPropertyElementMaster;

    auto result = AudioObjectGetPropertyData(deviceId, &propertyAddress, 0, NULL, &deviceUIDSize, &deviceUID);
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

bool OSXAudioDriver::selectOutputDevice(const std::string& name)
{
    if (m_deviceName == name) {
        return true;
    }

    bool reopen = isOpened();
    close();
    m_deviceName = name;
    if (reopen) {
        return open(m_data->format, &m_data->format);
    }
    return true;
}

void OSXAudioDriver::resume()
{
}

void OSXAudioDriver::suspend()
{
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
    pData->callback(pData->mUserData, (uint8_t*)buffer->mAudioData, buffer->mAudioDataByteSize);
    AudioQueueEnqueueBuffer(pData->audioQueue, buffer, 0, NULL);
}
