/*
SoLoud audio engine
Copyright (c) 2013-2014 Jari Komppa

This software is provided 'as-is', without any express or implied
warranty. In no event will the authors be held liable for any damages
arising from the use of this software.

Permission is granted to anyone to use this software for any purpose,
including commercial applications, and to alter it and redistribute it
freely, subject to the following restrictions:

   1. The origin of this software must not be misrepresented; you must not
   claim that you wrote the original software. If you use this software
   in a product, an acknowledgment in the product documentation would be
   appreciated but is not required.

   2. Altered source versions must be plainly marked as such, and must not be
   misrepresented as being the original software.

   3. This notice may not be removed or altered from any source
   distribution.
*/

#include "soloud.h"
#include "soloud_thread.h"

#if !defined(WITH_OSS)

namespace SoLoud
{
    result oss_init(Soloud *aSoloud, unsigned int aFlags, unsigned int aSamplerate, unsigned int aBuffer)
	{
		return NOT_IMPLEMENTED;
	}
};

#else
#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/soundcard.h>
#include <unistd.h>
#include <string.h>

namespace SoLoud
{
    static const int OSS_DEVICE_COUNT = 4;
    static const char *OSS_DEVICES[OSS_DEVICE_COUNT] = 
    { 
        "/dev/dsp", 
        "/dev/dsp0.0", 
        "/dev/dsp1.0", 
        "/dev/dsp2.0" 
    };

    struct OSSData
    {
        float *buffer;
        short *sampleBuffer;
        int ossDeviceHandle;
        Soloud *soloud;
        int samples;
        int channels;
        bool audioProcessingDone;
        Thread::ThreadHandle threadHandle;
    };

    static void ossThread(void *aParam)
    {
        OSSData *data = static_cast<OSSData*>(aParam);
        while (!data->audioProcessingDone) 
        {
            data->soloud->mix(data->buffer, data->samples);
            for (int i=0;i<data->samples*data->channels;++i)
            {
                data->sampleBuffer[i] = static_cast<short>(floor(data->buffer[i] 
                                                                 * static_cast<float>(0x7fff)));
            }
            write(data->ossDeviceHandle, data->sampleBuffer, 
                  sizeof(short)*data->samples*data->channels);
        }
    }

    static void ossCleanup(Soloud *aSoloud)
    {
        if (0 == aSoloud->mBackendData)
        {
            return;
        }
        OSSData *data = static_cast<OSSData*>(aSoloud->mBackendData);
        data->audioProcessingDone = true;
        if (data->threadHandle)
        {
            Thread::wait(data->threadHandle);
            Thread::release(data->threadHandle);
        }
        ioctl(data->ossDeviceHandle, SNDCTL_DSP_RESET, 0);       
        if (0 != data->sampleBuffer)
        {
            delete[] data->sampleBuffer;
        }
        if (0 != data->buffer)
        {
            delete[] data->buffer;
        }
        close(data->ossDeviceHandle);
        delete data;
        aSoloud->mBackendData = 0;
    }

    result oss_init(Soloud *aSoloud, unsigned int aFlags, unsigned int aSamplerate, unsigned int aBuffer, unsigned int aChannels)
    {
        OSSData *data = new OSSData;
        memset(data, 0, sizeof(OSSData));
        aSoloud->mBackendData = data;
        aSoloud->mBackendCleanupFunc = ossCleanup;
        data->samples = aBuffer;
        data->soloud = aSoloud;
        bool deviceIsOpen = false;
        for (int i=0;i<OSS_DEVICE_COUNT;++i)
        {
            data->ossDeviceHandle = open(OSS_DEVICES[i], O_WRONLY, 0);
            if (-1 != data->ossDeviceHandle)
            {
                deviceIsOpen = true;
                break;
            }
        }
        if (!deviceIsOpen)
        {
            return UNKNOWN_ERROR;
        }
        int flags = fcntl(data->ossDeviceHandle, F_GETFL);
        flags &= ~O_NONBLOCK;
        if (-1 == fcntl(data->ossDeviceHandle, F_SETFL, flags))
        {
            return UNKNOWN_ERROR;
        }        
        int format = AFMT_S16_NE;
        if (-1 == ioctl(data->ossDeviceHandle, SNDCTL_DSP_SETFMT, &format))
        {
            return UNKNOWN_ERROR;
        }
        if (format != AFMT_S16_NE)
        {
            return UNKNOWN_ERROR;
        }
        int channels = 2;
        data->channels = channels;
        if (-1 == ioctl(data->ossDeviceHandle, SNDCTL_DSP_CHANNELS, &data->channels))
        {
            return UNKNOWN_ERROR;
        }
        if (channels != data->channels)
        {
            return UNKNOWN_ERROR;
        }
        int speed = aSamplerate;
        if (-1 == ioctl(data->ossDeviceHandle, SNDCTL_DSP_SPEED, &speed))
        {
            return UNKNOWN_ERROR;
        }
        if (speed != aSamplerate)
        {
            return UNKNOWN_ERROR;
        }
        data->buffer = new float[data->samples*data->channels];
        data->sampleBuffer = new short[data->samples*data->channels];
        aSoloud->postinit_internal(aSamplerate, data->samples * data->channels, aFlags, 2);
        data->threadHandle = Thread::createThread(ossThread, data);
        if (0 == data->threadHandle)
        {
            return UNKNOWN_ERROR;
        }
        aSoloud->mBackendString = "OSS";
        return 0;
    }
};
#endif