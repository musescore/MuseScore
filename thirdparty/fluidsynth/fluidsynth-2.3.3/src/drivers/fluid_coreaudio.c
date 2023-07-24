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
    unsigned int buffer_count;
    float **buffers;
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

#if __MAC_OS_X_VERSION_MAX_ALLOWED < 120000
#define kAudioObjectPropertyElementMain (kAudioObjectPropertyElementMaster)
#endif

int
get_num_outputs(AudioDeviceID deviceID)
{
    int i, total = 0;
    UInt32 size;
    AudioObjectPropertyAddress pa;
    pa.mSelector = kAudioDevicePropertyStreamConfiguration;
    pa.mScope = kAudioDevicePropertyScopeOutput;
    pa.mElement = kAudioObjectPropertyElementMain;

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
set_channel_map(AudioUnit outputUnit, int audio_channels, const char *map_string)
{
    OSStatus status;
    long int number_of_channels;
    int i, *channel_map;
    UInt32 property_size;
    Boolean writable = false;

    status = AudioUnitGetPropertyInfo(outputUnit,
                                      kAudioOutputUnitProperty_ChannelMap,
                                      kAudioUnitScope_Output,
                                      0,
                                      &property_size, &writable);
    if(status != noErr)
    {
        FLUID_LOG(FLUID_ERR, "Failed to get the channel map size. Status=%ld\n", (long int) status);
        return;
    }

    number_of_channels = property_size / sizeof(int);
    if(!number_of_channels)
    {
        return;
    }

    channel_map = FLUID_ARRAY(int, number_of_channels);
    if(channel_map == NULL)
    {
        FLUID_LOG(FLUID_ERR, "Out of memory.\n");
        return;
    }

    FLUID_MEMSET(channel_map, 0xff, property_size);

    status = AudioUnitGetProperty(outputUnit,
                                  kAudioOutputUnitProperty_ChannelMap,
                                  kAudioUnitScope_Output,
                                  0,
                                  channel_map, &property_size);
    if(status != noErr)
    {
        FLUID_LOG(FLUID_ERR, "Failed to get the existing channel map. Status=%ld\n", (long int) status);
        FLUID_FREE(channel_map);
        return;
    }

    fluid_settings_split_csv(map_string, channel_map, (int) number_of_channels);
    for(i = 0; i < number_of_channels; i++)
    {
        if(channel_map[i] < -1 || channel_map[i] >= audio_channels)
        {
            FLUID_LOG(FLUID_DBG, "Channel map of output channel %d is out-of-range. Silencing.", i);
            channel_map[i] = -1;
        }
    }

    status = AudioUnitSetProperty(outputUnit,
                                  kAudioOutputUnitProperty_ChannelMap,
                                  kAudioUnitScope_Output,
                                  0,
                                  channel_map, property_size);
    if(status != noErr)
    {
        FLUID_LOG(FLUID_ERR, "Failed to set the channel map. Status=%ld\n", (long int) status);
    }

    FLUID_FREE(channel_map);
}

void
fluid_core_audio_driver_settings(fluid_settings_t *settings)
{
    int i;
    UInt32 size;
    AudioObjectPropertyAddress pa;
    pa.mSelector = kAudioHardwarePropertyDevices;
    pa.mScope = kAudioObjectPropertyScopeWildcard;
    pa.mElement = kAudioObjectPropertyElementMain;

    fluid_settings_register_str(settings, "audio.coreaudio.device", "default", 0);
    fluid_settings_register_str(settings, "audio.coreaudio.channel-map", "", 0);
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
    char *devname = NULL, *channel_map = NULL;
    fluid_core_audio_driver_t *dev = NULL;
    int period_size, periods, audio_channels = 1;
    double sample_rate;
    OSStatus status;
    UInt32 size;
    int i;
#if MAC_OS_X_VERSION_MIN_REQUIRED < 1060
    ComponentDescription desc;
    Component comp;
#else
    AudioComponentDescription desc;
    AudioComponent comp;
#endif
    AURenderCallbackStruct render;

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
    desc.componentType = kAudioUnitType_Output;
    desc.componentSubType = kAudioUnitSubType_HALOutput; //kAudioUnitSubType_DefaultOutput;
    desc.componentManufacturer = kAudioUnitManufacturer_Apple;
    desc.componentFlags = 0;
    desc.componentFlagsMask = 0;

#if MAC_OS_X_VERSION_MIN_REQUIRED < 1060
    comp = FindNextComponent(NULL, &desc);
#else
    comp = AudioComponentFindNext(NULL, &desc);
#endif

    if(comp == NULL)
    {
        FLUID_LOG(FLUID_ERR, "Failed to get the default audio device");
        goto error_recovery;
    }

#if MAC_OS_X_VERSION_MIN_REQUIRED < 1060
    status = OpenAComponent(comp, &dev->outputUnit);
#else
    status = AudioComponentInstanceNew(comp, &dev->outputUnit);
#endif

    if(status != noErr)
    {
        FLUID_LOG(FLUID_ERR, "Failed to open the default audio device. Status=%ld\n", (long int)status);
        goto error_recovery;
    }

    // Set up a callback function to generate output
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

    fluid_settings_getint(settings, "synth.audio-channels", &audio_channels);
    fluid_settings_getnum(settings, "synth.sample-rate", &sample_rate);
    fluid_settings_getint(settings, "audio.periods", &periods);
    fluid_settings_getint(settings, "audio.period-size", &period_size);

    /* audio channels are in stereo, with a minimum of one pair */
    audio_channels = (audio_channels > 0) ? (2 * audio_channels) : 2;

    /* get the selected device name. if none is specified, use NULL for the default device. */
    if(fluid_settings_dupstr(settings, "audio.coreaudio.device", &devname) == FLUID_OK   /* alloc device name */
            && devname && strlen(devname) > 0)
    {
        AudioObjectPropertyAddress pa;
        pa.mSelector = kAudioHardwarePropertyDevices;
        pa.mScope = kAudioObjectPropertyScopeWildcard;
        pa.mElement = kAudioObjectPropertyElementMain;

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
    dev->format.mFormatFlags = kLinearPCMFormatFlagIsFloat | kAudioFormatFlagIsNonInterleaved;
    dev->format.mBytesPerPacket = sizeof(float);
    dev->format.mFramesPerPacket = 1;
    dev->format.mBytesPerFrame = sizeof(float);
    dev->format.mChannelsPerFrame = audio_channels;
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

    if(fluid_settings_dupstr(settings, "audio.coreaudio.channel-map", &channel_map) == FLUID_OK   /* alloc channel map */
            && channel_map && strlen(channel_map) > 0)
    {
        set_channel_map(dev->outputUnit, audio_channels, channel_map);
    }
    FLUID_FREE(channel_map);  /* free channel map */

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

    dev->buffers = FLUID_ARRAY(float *, audio_channels);

    if(dev->buffers == NULL)
    {
        FLUID_LOG(FLUID_ERR, "Out of memory.");
        goto error_recovery;
    }

    dev->buffer_count = (unsigned int) audio_channels;

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

#if MAC_OS_X_VERSION_MIN_REQUIRED < 1060
    CloseComponent(dev->outputUnit);
#else
    AudioComponentInstanceDispose(dev->outputUnit);
#endif

    if(dev->buffers != NULL)
    {
        FLUID_FREE(dev->buffers);
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
    fluid_core_audio_driver_t *dev = (fluid_core_audio_driver_t *) data;
    int len = inNumberFrames;
    UInt32 i, nBuffers = ioData->mNumberBuffers;
    fluid_audio_func_t callback = (dev->callback != NULL) ? dev->callback : (fluid_audio_func_t) fluid_synth_process;

    for(i = 0; i < ioData->mNumberBuffers && i < dev->buffer_count; i++)
    {
        dev->buffers[i] = ioData->mBuffers[i].mData;
        FLUID_MEMSET(dev->buffers[i], 0, len * sizeof(float));
    }

    callback(dev->data, len, nBuffers, dev->buffers, nBuffers, dev->buffers);

    return noErr;
}


#endif /* #if COREAUDIO_SUPPORT */
