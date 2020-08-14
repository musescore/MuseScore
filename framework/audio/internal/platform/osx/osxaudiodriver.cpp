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
    AudioQueueRef audioQueue;
    Callback callback;
    void* mUserData;
};

OSXAudioDriver::OSXAudioDriver()
    : m_data(nullptr)
{
    m_data = std::make_shared<Data>();
    m_data->audioQueue = nullptr;
}

OSXAudioDriver::~OSXAudioDriver()
{
    close();
}

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
    if (isprint(errorString[0]) && isprint(errorString[0]) && isprint(errorString[0]) && isprint(errorString[0])) {
        LOGE() << message << errorString << "(" << error << ")";
    } else {
        LOGE() << message << error;
    }
}

/*static*/
void OSXAudioDriver::OnFillBuffer(void* context, AudioQueueRef, AudioQueueBufferRef buffer)
{
    Data* pData = (Data*)context;
    pData->callback(pData->mUserData, (uint8_t*)buffer->mAudioData, buffer->mAudioDataByteSize);
    AudioQueueEnqueueBuffer(pData->audioQueue, buffer, 0, NULL);
}
