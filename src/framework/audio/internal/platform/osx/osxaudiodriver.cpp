//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2020 MuseScore BVBA and others
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
//=============================================================================

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

    activeSpec->format = Format::AudioS16;
    m_data->format = *activeSpec;

    AudioStreamBasicDescription audioFormat;
    audioFormat.mSampleRate = spec.freq;
    audioFormat.mFormatID = kAudioFormatLinearPCM;
    audioFormat.mFormatFlags = kLinearPCMFormatFlagIsSignedInteger | kAudioFormatFlagIsPacked;
    audioFormat.mBytesPerPacket = 2 * spec.channels;
    audioFormat.mFramesPerPacket = 1;
    audioFormat.mBytesPerFrame = 2 * spec.channels;
    audioFormat.mChannelsPerFrame = spec.channels;
    audioFormat.mBitsPerChannel = 16;
    audioFormat.mReserved = 0;

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

std::vector<std::string> OSXAudioDriver::availableDevices() const
{
    std::vector<std::string> deviceList = { DEFAULT_DEVICE_NAME };
    for (auto& device : m_devices) {
        deviceList.push_back(device.second);
    }
    return deviceList;
}

mu::async::Notification OSXAudioDriver::deviceListChanged() const
{
    return m_deviceListChanged;
}

std::string OSXAudioDriver::device() const
{
    return m_deviceName;
}

void OSXAudioDriver::updateDeviceMap()
{
    AudioObjectPropertyAddress propertyAddress;
    UInt32 propertySize;
    OSStatus result;
    std::vector<AudioObjectID> audioObjects = {};
    m_devices.clear();

    propertyAddress.mSelector = kAudioHardwarePropertyDevices;
    propertyAddress.mScope = kAudioObjectPropertyScopeOutput;
    propertyAddress.mElement = kAudioObjectPropertyElementMaster;

    result = AudioObjectGetPropertyDataSize(kAudioObjectSystemObject, &propertyAddress, 0, NULL, &propertySize);
    if (result != noErr) {
        logError("Failed to get devices count, err: ", result);
        return;
    }

    audioObjects.resize(propertySize / sizeof(AudioDeviceID));
    result = AudioObjectGetPropertyData(kAudioObjectSystemObject, &propertyAddress, 0, NULL, &propertySize, audioObjects.data());
    if (result != noErr) {
        logError("Failed to get devices list, err: ", result);
        return;
    }

    propertyAddress.mSelector = kAudioDevicePropertyDeviceNameCFString;
    propertyAddress.mScope = kAudioObjectPropertyScopeOutput;
    propertyAddress.mElement = kAudioObjectPropertyElementMaster;

    for (auto&& d : audioObjects) {
        std::string deviceName;
        CFStringRef nameRef;

        result = AudioObjectGetPropertyData(d, &propertyAddress, 0, NULL, &propertySize, &nameRef);
        if (result != noErr) {
            logError("Failed to get device's name, err: ", result);
            continue;
        }
        propertySize = CFStringGetLength(nameRef);
        deviceName.resize(propertySize);
        CFStringGetCString(nameRef, deviceName.data(), sizeof(deviceName), kCFStringEncodingUTF8);
        m_devices[d] = deviceName;
        CFRelease(nameRef);
    }
    m_deviceListChanged.notify();
}

bool OSXAudioDriver::audioQueueSetDeviceName(std::string deviceName)
{
    if (deviceName.empty() || deviceName == DEFAULT_DEVICE_NAME) {
        return true; //default device used
    }

    auto index = std::find_if(m_devices.begin(), m_devices.end(), [&deviceName](auto& d) { return d.second == deviceName; });
    if (index == m_devices.end()) {
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

bool OSXAudioDriver::selectDevice(std::string name)
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
