//=============================================================================
//  MusE Score
//  Linux Music Score Editor
//
//  Copyright (C) 2020 Musescore BVBA
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

/*
 * Thanks to PortAudio, whose CoreAudio implementation inspired portions of this code.
 */

#include "mididriver.h"

#include "libmscore/score.h"

#include "mscore/preferences.h"
#include "mscore/seq.h"
#include "audio/midi/msynthesizer.h"

#include "coreaudio.h"

#include <CoreAudio/CoreAudio.h>
#include <AudioUnit/AudioUnit.h>
#include <AudioToolbox/AudioToolbox.h>

namespace Ms {
AudioUnit audioUnit = NULL;

//---------------------------------------------------------
//   CoreAudio
//---------------------------------------------------------

CoreAudio::CoreAudio(Seq* s)
    : Driver(s)
{
    _sampleRate = 48000;      // will be replaced by device default sample rate
    initialized = false;
    state       = Transport::STOP;
    seekflag    = false;
    pos = 0;
    startTime = 0.0;
    midiDriver  = 0;
}

//---------------------------------------------------------
//   ~CoreAudio
//---------------------------------------------------------

CoreAudio::~CoreAudio()
{
    if (initialized) {
        OSStatus status = AudioUnitUninitialize(audioUnit);
        if (status != noErr) {
            qDebug("CoreAudio uninitialize failed: %d", status);
        }

        AudioComponentInstanceDispose(audioUnit);
    }
}

OSStatus getDefaultOutputDevice(AudioDeviceID* deviceId)
{
    if (deviceId == nullptr) {
        return kAudio_ParamError;
    }

    UInt32 size = sizeof(AudioDeviceID);

    AudioObjectPropertyAddress devAddress = {
        kAudioHardwarePropertyDefaultOutputDevice,
        kAudioObjectPropertyScopeGlobal,
        kAudioObjectPropertyElementMaster
    };

    return AudioObjectGetPropertyData(kAudioObjectSystemObject, &devAddress, 0, nullptr, &size, deviceId);
}

Float64 getSampleRate(AudioDeviceID devId)
{
    Float64 sampleRate;
    UInt32 size = sizeof(Float64);

    AudioObjectPropertyAddress theAddress = {
        kAudioDevicePropertyActualSampleRate,
        kAudioObjectPropertyScopeInput,
        kAudioObjectPropertyElementMaster
    };

    OSStatus status = AudioObjectGetPropertyData(devId, &theAddress, 0, NULL, &size, &sampleRate);

    if (status == noErr) {
        return sampleRate;
    } else {
        return 0.0f;
    }
}

static OSStatus CoreAudioIOProc(void* inRefCon,AudioUnitRenderActionFlags* ioActionFlags,const AudioTimeStamp* inTimeStamp,
                                UInt32 inBusNumber,UInt32 inNumberFrames,AudioBufferList* ioData);

static OSStatus changeDesiredStreamFormat(AudioUnit* audioUnit, float sampleRate)
{
    AudioStreamBasicDescription desiredFormat;

    qDebug("Setting sample rate of %f, %d", sampleRate, seq->canStart());

    MScore::sampleRate = sampleRate;

    if (seq->canStart()) {
        seq->synti()->setSampleRate(sampleRate);
    }

    memset(&desiredFormat, 0, sizeof(AudioStreamBasicDescription));
    desiredFormat.mFormatID = kAudioFormatLinearPCM;
    desiredFormat.mFormatFlags = kAudioFormatFlagsNativeFloatPacked;
    desiredFormat.mFramesPerPacket = 1;
    desiredFormat.mBitsPerChannel = sizeof(float) * 8;
    desiredFormat.mSampleRate = sampleRate;
    desiredFormat.mBytesPerPacket = sizeof(float) * 2;
    desiredFormat.mBytesPerFrame = sizeof(float) * 2;
    desiredFormat.mChannelsPerFrame = 2;
    return AudioUnitSetProperty(*audioUnit, kAudioUnitProperty_StreamFormat, kAudioUnitScope_Input, 0, &desiredFormat,
                                sizeof(desiredFormat));
}

// Substantial portions of this taken from Portaudio.
bool openAudioUnit(AudioUnit* audioUnit, AudioDeviceID* audioDevice, CoreAudio* ca)
{
    AudioComponent comp;
    AudioComponentDescription desc;
    AURenderCallbackStruct renderCallbackData;
    UInt32 converterQuality;

    desc.componentType = kAudioUnitType_Output;
    desc.componentSubType = kAudioUnitSubType_HALOutput;
    desc.componentManufacturer = kAudioUnitManufacturer_Apple;
    desc.componentFlags = 0;
    desc.componentFlagsMask = 0;

    comp = AudioComponentFindNext(NULL, &desc);
    if (!comp) {
        qDebug("AUHAL component not found.");
        *audioUnit = NULL;
        *audioDevice = kAudioDeviceUnknown;
        return false;
    }

    OSStatus status = AudioComponentInstanceNew(comp, audioUnit);
    if (status) {
        qDebug("Failed to open AUHAL component.");
        *audioUnit = NULL;
        *audioDevice = kAudioDeviceUnknown;
        return false;
    }

    status
        = AudioUnitSetProperty(*audioUnit, kAudioOutputUnitProperty_CurrentDevice, kAudioUnitScope_Global, 0, audioDevice,
                               sizeof(AudioDeviceID));
    if (status) {
        qDebug("Failed to set current device.");
        goto error;
    }

    converterQuality = kAudioConverterQuality_Max;
    status
        = AudioUnitSetProperty(*audioUnit, kAudioUnitProperty_RenderQuality, kAudioUnitScope_Global, 0, &converterQuality,
                               sizeof(converterQuality));
    if (status) {
        qDebug("Failed to set render quality");
        goto error;
    }

    status = changeDesiredStreamFormat(audioUnit, ca->sampleRate());
    if (status) {
        qDebug("Failed to set stream format");
        goto error;
    }

    renderCallbackData.inputProc = CoreAudioIOProc;
    renderCallbackData.inputProcRefCon = ca;

    status
        = AudioUnitSetProperty(*audioUnit, kAudioUnitProperty_SetRenderCallback, kAudioUnitScope_Output, 0, &renderCallbackData,
                               sizeof(renderCallbackData));
    if (status) {
        qDebug("Failed to set render callback");
        goto error;
    }

    AudioUnitInitialize(*audioUnit);

    return true;
error:
    if (*audioUnit) {
        AudioComponentInstanceDispose(*audioUnit);
        *audioUnit = NULL;
    }

    *audioDevice = kAudioDeviceUnknown;
    return false;
}

static OSStatus CoreAudioIOProc(void* inRefCon,
                                AudioUnitRenderActionFlags* ioActionFlags,
                                const AudioTimeStamp* inTimeStamp,
                                UInt32 inBusNumber,
                                UInt32 inNumberFrames,
                                AudioBufferList* ioData)
{
    Q_UNUSED(inRefCon);
    Q_UNUSED(ioActionFlags);
    Q_UNUSED(inTimeStamp);
    Q_UNUSED(inBusNumber);
    Q_UNUSED(inNumberFrames);

    seq->setInitialMillisecondTimestampWithLatency();

    UInt32 bytesPerFrame = sizeof(float) * ioData->mBuffers[0].mNumberChannels;

    float* buffer = static_cast<float*>(ioData->mBuffers[0].mData);
    UInt32 frames = ioData->mBuffers[0].mDataByteSize / bytesPerFrame;

    if (seq->driver()) {
        seq->process(frames, buffer);
    }

    return noErr;
}

bool CoreAudio::init(bool)
{
    CFRunLoopRef theRunLoop = NULL;
    AudioObjectPropertyAddress theAddress
        = { kAudioHardwarePropertyRunLoop, kAudioObjectPropertyScopeGlobal, kAudioObjectPropertyElementMaster };
    OSStatus osErr = AudioObjectSetPropertyData(kAudioObjectSystemObject, &theAddress, 0, NULL, sizeof(CFRunLoopRef), &theRunLoop);
    if (osErr != noErr) {
        initialized = false;
        return false;
    }

    AudioObjectPropertyAddress outputDeviceAddress = {
        kAudioHardwarePropertyDefaultOutputDevice,
        kAudioObjectPropertyScopeGlobal,
        kAudioObjectPropertyElementMaster
    };

    AudioObjectAddPropertyListener(kAudioObjectSystemObject, &outputDeviceAddress, &CoreAudio::onDefaultOutputDeviceChanged, this);

    AudioDeviceID deviceId = 0;
    OSStatus status = getDefaultOutputDevice(&deviceId);
    if (status != noErr) {
        qDebug("Error occurred while getting default output device: %d", status);
        return false;
    }

    this->currentOutputDevice = deviceId;

    outputDeviceAddress = {
        kAudioDevicePropertyActualSampleRate,
        kAudioObjectPropertyScopeInput,
        kAudioObjectPropertyElementMaster
    };

    AudioObjectAddPropertyListener(deviceId, &outputDeviceAddress, &CoreAudio::onSampleRateChanged, this);

    return openAudioUnit(&audioUnit, &deviceId, this);
}

bool CoreAudio::start(bool)
{
    OSStatus status = AudioOutputUnitStart(audioUnit);
    if (status) {
        return false;
    } else {
        return true;
    }
}

bool CoreAudio::stop()
{
    OSStatus status = AudioOutputUnitStop(audioUnit);

    if (status) {
        return false;
    } else {
        return true;
    }
}

void CoreAudio::stopTransport()
{
    state = Transport::STOP;
}

void CoreAudio::startTransport()
{
    state = Transport::PLAY;
}

Transport CoreAudio::getState()
{
    return state;
}

void CoreAudio::midiRead()
{
    return;
}

// FIXME: Do we want to keep this around? Will there be a need to give people the option to switch devices in the CoreAudio back end?
QStringList CoreAudio::deviceList()
{
    QStringList dl;

    AudioObjectPropertyAddress deviceListAddress = {
        kAudioHardwarePropertyDevices,
        kAudioObjectPropertyScopeGlobal,
        kAudioObjectPropertyElementMaster
    };

    UInt32 dataSize = 0;

    OSStatus status = AudioObjectGetPropertyDataSize(kAudioObjectSystemObject, &deviceListAddress, 0, NULL, &dataSize);
    if (status != kAudioHardwareNoError) {
        qDebug("AudioObjectGetPropertyDataSize (kAudioHardwarePropertyDevices) failed: %i\n", status);
        return dl;
    }

    UInt32 deviceCount = static_cast<UInt32>(dataSize / sizeof(AudioDeviceID));

    AudioDeviceID* audioDevices = static_cast<AudioDeviceID*>(malloc(dataSize));
    if (audioDevices == nullptr) {
        qDebug("Could not allocate enough memory for audioDevices");
        return dl;
    }

    status = AudioObjectGetPropertyData(kAudioObjectSystemObject, &deviceListAddress, 0, NULL, &dataSize, audioDevices);
    if (status != kAudioHardwareNoError) {
        qDebug("AudioObjectGetPropertyData (kAudioHardwarePropertyDevices) failed: %i\n", status);
        free(audioDevices);
        audioDevices = nullptr;
        return dl;
    }

    CFMutableArrayRef inputDeviceArray = CFArrayCreateMutable(kCFAllocatorDefault, deviceCount, &kCFTypeArrayCallBacks);
    if (NULL == inputDeviceArray) {
        qDebug("CFArrayCreateMutable failed");
        free(audioDevices), audioDevices = nullptr;
        return dl;
    }

    // Iterate through all the devices and determine which are input-capable
    deviceListAddress.mScope = kAudioDevicePropertyScopeOutput;
    for (UInt32 i = 0; i < deviceCount; i++) {
        // Query device UID
        CFStringRef deviceUID = NULL;
        dataSize = sizeof(deviceUID);
        deviceListAddress.mSelector = kAudioDevicePropertyDeviceUID;
        status = AudioObjectGetPropertyData(audioDevices[i], &deviceListAddress, 0, NULL, &dataSize, &deviceUID);
        if (kAudioHardwareNoError != status) {
            qDebug("AudioObjectGetPropertyData (kAudioDevicePropertyDeviceUID) failed: %i\n", status);
            continue;
        }

        // Query device name
        CFStringRef deviceName = NULL;
        dataSize = sizeof(deviceName);
        deviceListAddress.mSelector = kAudioDevicePropertyDeviceNameCFString;
        status = AudioObjectGetPropertyData(audioDevices[i], &deviceListAddress, 0, NULL, &dataSize, &deviceName);
        if (kAudioHardwareNoError != status) {
            qDebug("AudioObjectGetPropertyData (kAudioDevicePropertyDeviceNameCFString) failed: %i\n", status);
            continue;
        }

        if (deviceName != nullptr) {
            dl.append(QString::fromCFString(deviceName));
        }
    }

    return dl;
}

OSStatus CoreAudio::onSampleRateChanged(AudioObjectID objId, UInt32 inNumberAddresses, const AudioObjectPropertyAddress inAddresses[],
                                        void* paVoid)
{
    Q_UNUSED(inNumberAddresses);
    Q_UNUSED(paVoid);

    Float64 sampleRate = 0.0f;
    UInt32 size = sizeof(Float64);

    OSStatus status;
    if ((status = AudioObjectGetPropertyData(objId, inAddresses, 0, nullptr, &size, &sampleRate)) != 0) {
        qDebug("Error happened while fetching sample rate: %d\n", status);
        return status;
    }

    status = changeDesiredStreamFormat(&audioUnit, sampleRate);
    if (!status) {
        qDebug("Error occurred while setting new sample rate: %d\n", status);
    }

    return status;
}

OSStatus CoreAudio::onDefaultOutputDeviceChanged(AudioObjectID inObjectID, UInt32 inNumberAddresses,
                                                 const AudioObjectPropertyAddress inAddresses[], void* paVoid)
{
    Q_UNUSED(inNumberAddresses);

    CoreAudio* ca = static_cast<CoreAudio*>(paVoid);

    UInt32 defaultOut = 0;
    UInt32 size = sizeof(UInt32);

    AudioObjectPropertyAddress addr = {
        kAudioDevicePropertyActualSampleRate,
        kAudioObjectPropertyScopeInput,
        kAudioObjectPropertyElementMaster
    };

    OSStatus status;

    // Need to keep the property listener updated for the default output device.
    status = AudioObjectRemovePropertyListener(ca->currentOutputDevice, &addr, &CoreAudio::onSampleRateChanged, paVoid);
    if (status) {
        qDebug("Error occurred while removing property listener: %d", status);
        return status;
    }

    if ((status = AudioObjectGetPropertyData(inObjectID, inAddresses, 0, nullptr, &size, &defaultOut)) != 0) {
        qDebug("Error occurred while fetching default output device: %d", status);
        return status;
    }

    ca->_sampleRate = getSampleRate(defaultOut);
    ca->currentOutputDevice = defaultOut;

    // Need to keep the property listener updated for the default output device.
    status = AudioObjectAddPropertyListener(ca->currentOutputDevice, &addr, &CoreAudio::onSampleRateChanged, paVoid);
    if (status) {
        qDebug("Error occurred while adding property listener: %d", status);
        return status;
    }

    status
        = AudioUnitSetProperty(audioUnit, kAudioOutputUnitProperty_CurrentDevice, kAudioUnitScope_Global, 0, &defaultOut,
                               sizeof(AudioDeviceID));
    if (status) {
        qDebug("Failed to set current device: %d", status);
        return status;
    }

    status = changeDesiredStreamFormat(&audioUnit, ca->_sampleRate);
    if (status) {
        qDebug("Failed to change desired stream format: %d", status);
        return status;
    }

    return noErr;
}
}
