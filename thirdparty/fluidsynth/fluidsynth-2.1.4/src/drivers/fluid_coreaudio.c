/* FluidSynth - A Software Synthesizer
 *
 * Copyright (C) 2003  Peter Hanappe and others.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public License
 * as published by the Free Software Foundation; either version 2.1 of
 * the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free
 * Software Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301, USA
 */

/* fluid_coreaudio.c
 *
 * Driver for the Apple's CoreAudio on MacOS X
 *
 */

#include "fluid_adriver.h"
#include "fluid_settings.h"

/* 
 * !!! Make sure that no include above includes <netinet/tcp.h> !!!
 * It #defines some macros that collide with enum definitions of OpenTransportProviders.h, which is included from OSServices.h, included from CoreServices.h
 * 
 * https://trac.macports.org/ticket/36962
 */

#if COREAUDIO_SUPPORT
#include <CoreServices/CoreServices.h>
#include <CoreAudio/CoreAudioTypes.h>
#include <CoreAudio/AudioHardware.h>
#include <AudioUnit/AudioUnit.h>

/*
 * fluid_core_audio_driver_t
 *
 */
typedef struct
{
    fluid_audio_driver_t driver;
    AudioUnit outputUnit;
    AudioStreamBasicDescription format;
    fluid_audio_func_t callback;
    void *data;
    unsigned int buffer_size;
    float *buffers[2];
    double phase;
} fluid_core_audio_driver_t;


OSStatus fluid_core_audio_callback(void *data,
                                   AudioUnitRenderActionFlags *ioActionFlags,
                                   const AudioTimeStamp *inTimeStamp,
                                   UInt32 inBusNumber,
                                   UInt32 inNumberFrames,
                                   AudioBufferList *ioData);


/**************************************************************
 *
 *        CoreAudio audio driver
 *
 */

#define OK(x) (x == noErr)

int
get_num_outputs(AudioDeviceID deviceID)
{
    int i, total = 0;
    UInt32 size;
    AudioObjectPropertyAddress pa;
    pa.mSelector = kAudioDevicePropertyStreamConfiguration;
    pa.mScope = kAudioDevicePropertyScopeOutput;
    pa.mElement = kAudioObjectPropertyElementMaster;

    if(OK(AudioObjectGetPropertyDataSize(deviceID, &pa, 0, 0, &size)) && size > 0)
    {
        AudioBufferList *bufList = FLUID_MALLOC(size);

        if(bufList == NULL)
        {
            FLUID_LOG(FLUID_ERR, "Out of memory");
            return 0;
        }

        if(OK(AudioObjectGetPropertyData(deviceID, &pa, 0, 0, &size, bufList)))
        {
            int numStreams = bufList->mNumberBuffers;

            for(i = 0; i < numStreams; ++i)
            {
                AudioBuffer b = bufList->mBuffers[i];
                total += b.mNumberChannels;
            }
        }

        FLUID_FREE(bufList);
    }

    return total;
}

void
fluid_core_audio_driver_settings(fluid_settings_t *settings)
{
    int i;
    UInt32 size;
    AudioObjectPropertyAddress pa;
    pa.mSelector = kAudioHardwarePropertyDevices;
    pa.mScope = kAudioObjectPropertyScopeWildcard;
    pa.mElement = kAudioObjectPropertyElementMaster;

    fluid_settings_register_str(settings, "audio.coreaudio.device", "default", 0);
    fluid_settings_add_option(settings, "audio.coreaudio.device", "default");

    if(OK(AudioObjectGetPropertyDataSize(kAudioObjectSystemObject, &pa, 0, 0, &size)))
    {
        int num = size / (int) sizeof(AudioDeviceID);
        AudioDeviceID devs [num];

        if(OK(AudioObjectGetPropertyData(kAudioObjectSystemObject, &pa, 0, 0, &size, devs)))
        {
            for(i = 0; i < num; ++i)
            {
                char name [1024];
                size = sizeof(name);
                pa.mSelector = kAudioDevicePropertyDeviceName;

                if(OK(AudioObjectGetPropertyData(devs[i], &pa, 0, 0, &size, name)))
                {
                    if(get_num_outputs(devs[i]) > 0)
                    {
                        fluid_settings_add_option(settings, "audio.coreaudio.device", name);
                    }
                }
            }
        }
    }
}

/*
 * new_fluid_core_audio_driver
 */
fluid_audio_driver_t *
new_fluid_core_audio_driver(fluid_settings_t *settings, fluid_synth_t *synth)
{
    return new_fluid_core_audio_driver2(settings,
                                        NULL,
                                        synth);
}

/*
 * new_fluid_core_audio_driver2
 */
fluid_audio_driver_t *
new_fluid_core_audio_driver2(fluid_settings_t *settings, fluid_audio_func_t func, void *data)
{
    char *devname = NULL;
    fluid_core_audio_driver_t *dev = NULL;
    int period_size, periods;
    double sample_rate;
    OSStatus status;
    UInt32 size;
    int i;

    dev = FLUID_NEW(fluid_core_audio_driver_t);

    if(dev == NULL)
    {
        FLUID_LOG(FLUID_ERR, "Out of memory");
        return NULL;
    }

    FLUID_MEMSET(dev, 0, sizeof(fluid_core_audio_driver_t));

    dev->callback = func;
    dev->data = data;

    // Open the default output unit
    ComponentDescription desc;
    desc.componentType = kAudioUnitType_Output;
    desc.componentSubType = kAudioUnitSubType_HALOutput; //kAudioUnitSubType_DefaultOutput;
    desc.componentManufacturer = kAudioUnitManufacturer_Apple;
    desc.componentFlags = 0;
    desc.componentFlagsMask = 0;

    Component comp = FindNextComponent(NULL, &desc);

    if(comp == NULL)
    {
        FLUID_LOG(FLUID_ERR, "Failed to get the default audio device");
        goto error_recovery;
    }

    status = OpenAComponent(comp, &dev->outputUnit);

    if(status != noErr)
    {
        FLUID_LOG(FLUID_ERR, "Failed to open the default audio device. Status=%ld\n", (long int)status);
        goto error_recovery;
    }

    // Set up a callback function to generate output
    AURenderCallbackStruct render;
    render.inputProc = fluid_core_audio_callback;
    render.inputProcRefCon = (void *) dev;
    status = AudioUnitSetProperty(dev->outputUnit,
                                  kAudioUnitProperty_SetRenderCallback,
                                  kAudioUnitScope_Input,
                                  0,
                                  &render,
                                  sizeof(render));

    if(status != noErr)
    {
        FLUID_LOG(FLUID_ERR, "Error setting the audio callback. Status=%ld\n", (long int)status);
        goto error_recovery;
    }

    fluid_settings_getnum(settings, "synth.sample-rate", &sample_rate);
    fluid_settings_getint(settings, "audio.periods", &periods);
    fluid_settings_getint(settings, "audio.period-size", &period_size);

    /* get the selected device name. if none is specified, use NULL for the default device. */
    if(fluid_settings_dupstr(settings, "audio.coreaudio.device", &devname) == FLUID_OK   /* alloc device name */
            && devname && strlen(devname) > 0)
    {
        AudioObjectPropertyAddress pa;
        pa.mSelector = kAudioHardwarePropertyDevices;
        pa.mScope = kAudioObjectPropertyScopeWildcard;
        pa.mElement = kAudioObjectPropertyElementMaster;

        if(OK(AudioObjectGetPropertyDataSize(kAudioObjectSystemObject, &pa, 0, 0, &size)))
        {
            int num = size / (int) sizeof(AudioDeviceID);
            AudioDeviceID devs [num];

            if(OK(AudioObjectGetPropertyData(kAudioObjectSystemObject, &pa, 0, 0, &size, devs)))
            {
                for(i = 0; i < num; ++i)
                {
                    char name [1024];
                    size = sizeof(name);
                    pa.mSelector = kAudioDevicePropertyDeviceName;

                    if(OK(AudioObjectGetPropertyData(devs[i], &pa, 0, 0, &size, name)))
                    {
                        if(get_num_outputs(devs[i]) > 0 && FLUID_STRCASECMP(devname, name) == 0)
                        {
                            AudioDeviceID selectedID = devs[i];
                            status = AudioUnitSetProperty(dev->outputUnit,
                                                          kAudioOutputUnitProperty_CurrentDevice,
                                                          kAudioUnitScope_Global,
                                                          0,
                                                          &selectedID,
                                                          sizeof(AudioDeviceID));

                            if(status != noErr)
                            {
                                FLUID_LOG(FLUID_ERR, "Error setting the selected output device. Status=%ld\n", (long int)status);
                                goto error_recovery;
                            }
                        }
                    }
                }
            }
        }
    }

    FLUID_FREE(devname);  /* free device name */

    dev->buffer_size = period_size * periods;

    // The DefaultOutputUnit should do any format conversions
    // necessary from our format to the device's format.
    dev->format.mSampleRate = sample_rate; // sample rate of the audio stream
    dev->format.mFormatID = kAudioFormatLinearPCM; // encoding type of the audio stream
    dev->format.mFormatFlags = kLinearPCMFormatFlagIsFloat;
    dev->format.mBytesPerPacket = 2 * sizeof(float);
    dev->format.mFramesPerPacket = 1;
    dev->format.mBytesPerFrame = 2 * sizeof(float);
    dev->format.mChannelsPerFrame = 2;
    dev->format.mBitsPerChannel = 8 * sizeof(float);

    FLUID_LOG(FLUID_DBG, "mSampleRate %g", dev->format.mSampleRate);
    FLUID_LOG(FLUID_DBG, "mFormatFlags %08X", dev->format.mFormatFlags);
    FLUID_LOG(FLUID_DBG, "mBytesPerPacket %d", dev->format.mBytesPerPacket);
    FLUID_LOG(FLUID_DBG, "mFramesPerPacket %d", dev->format.mFramesPerPacket);
    FLUID_LOG(FLUID_DBG, "mChannelsPerFrame %d", dev->format.mChannelsPerFrame);
    FLUID_LOG(FLUID_DBG, "mBytesPerFrame %d", dev->format.mBytesPerFrame);
    FLUID_LOG(FLUID_DBG, "mBitsPerChannel %d", dev->format.mBitsPerChannel);

    status = AudioUnitSetProperty(dev->outputUnit,
                                  kAudioUnitProperty_StreamFormat,
                                  kAudioUnitScope_Input,
                                  0,
                                  &dev->format,
                                  sizeof(AudioStreamBasicDescription));

    if(status != noErr)
    {
        FLUID_LOG(FLUID_ERR, "Error setting the audio format. Status=%ld\n", (long int)status);
        goto error_recovery;
    }

    status = AudioUnitSetProperty(dev->outputUnit,
                                  kAudioUnitProperty_MaximumFramesPerSlice,
                                  kAudioUnitScope_Input,
                                  0,
                                  &dev->buffer_size,
                                  sizeof(unsigned int));

    if(status != noErr)
    {
        FLUID_LOG(FLUID_ERR, "Failed to set the MaximumFramesPerSlice. Status=%ld\n", (long int)status);
        goto error_recovery;
    }

    FLUID_LOG(FLUID_DBG, "MaximumFramesPerSlice = %d", dev->buffer_size);

    dev->buffers[0] = FLUID_ARRAY(float, dev->buffer_size);
    dev->buffers[1] = FLUID_ARRAY(float, dev->buffer_size);
    
    if(dev->buffers[0] == NULL || dev->buffers[1] == NULL)
    {
        FLUID_LOG(FLUID_ERR, "Out of memory.");
        goto error_recovery;
    }

    // Initialize the audio unit
    status = AudioUnitInitialize(dev->outputUnit);

    if(status != noErr)
    {
        FLUID_LOG(FLUID_ERR, "Error calling AudioUnitInitialize(). Status=%ld\n", (long int)status);
        goto error_recovery;
    }

    // Start the rendering
    status = AudioOutputUnitStart(dev->outputUnit);

    if(status != noErr)
    {
        FLUID_LOG(FLUID_ERR, "Error calling AudioOutputUnitStart(). Status=%ld\n", (long int)status);
        goto error_recovery;
    }

    return (fluid_audio_driver_t *) dev;

error_recovery:

    delete_fluid_core_audio_driver((fluid_audio_driver_t *) dev);
    return NULL;
}

/*
 * delete_fluid_core_audio_driver
 */
void
delete_fluid_core_audio_driver(fluid_audio_driver_t *p)
{
    fluid_core_audio_driver_t *dev = (fluid_core_audio_driver_t *) p;
    fluid_return_if_fail(dev != NULL);

    CloseComponent(dev->outputUnit);

    if(dev->buffers[0])
    {
        FLUID_FREE(dev->buffers[0]);
    }

    if(dev->buffers[1])
    {
        FLUID_FREE(dev->buffers[1]);
    }

    FLUID_FREE(dev);
}

OSStatus
fluid_core_audio_callback(void *data,
                          AudioUnitRenderActionFlags *ioActionFlags,
                          const AudioTimeStamp *inTimeStamp,
                          UInt32 inBusNumber,
                          UInt32 inNumberFrames,
                          AudioBufferList *ioData)
{
    int i, k;
    fluid_core_audio_driver_t *dev = (fluid_core_audio_driver_t *) data;
    int len = inNumberFrames;
    float *buffer = ioData->mBuffers[0].mData;

    if(dev->callback)
    {
        float *left = dev->buffers[0];
        float *right = dev->buffers[1];

        FLUID_MEMSET(left, 0, len * sizeof(float));
        FLUID_MEMSET(right, 0, len * sizeof(float));

        (*dev->callback)(dev->data, len, 0, NULL, 2, dev->buffers);

        for(i = 0, k = 0; i < len; i++)
        {
            buffer[k++] = left[i];
            buffer[k++] = right[i];
        }
    }
    else
        fluid_synth_write_float((fluid_synth_t *) dev->data, len, buffer, 0, 2,
                                buffer, 1, 2);

    return noErr;
}


#endif /* #if COREAUDIO_SUPPORT */
