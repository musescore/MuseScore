/*
SoLoud audio engine
Copyright (c) 2013-2015 Jari Komppa

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

#if !defined(WITH_ALSA)

namespace SoLoud
{
    result alsa_init(Soloud *aSoloud, unsigned int aFlags, unsigned int aSamplerate, unsigned int aBuffer)
	{
		return NOT_IMPLEMENTED;
	}
};

#else

#define ALSA_PCM_NEW_HW_PARAMS_API
#include <alsa/asoundlib.h>

#include <fcntl.h>
#include <unistd.h>
#include <string.h>

namespace SoLoud
{
    struct ALSAData
    {
        float *buffer;
        short *sampleBuffer;
        snd_pcm_t *alsaDeviceHandle;
        Soloud *soloud;
        int samples;
        int channels;
        bool audioProcessingDone;
        Thread::ThreadHandle threadHandle;
    };


    static void alsaThread(void *aParam)
    {
        
        ALSAData *data = static_cast<ALSAData*>(aParam);
        while (!data->audioProcessingDone) 
        {            
            data->soloud->mix(data->buffer, data->samples);            
            for (int i=0;i<data->samples*data->channels;++i)
            {
                data->sampleBuffer[i] = static_cast<short>(floor(data->buffer[i] 
                                                          * static_cast<float>(0x7fff)));
            }
            if (snd_pcm_writei(data->alsaDeviceHandle, data->sampleBuffer, data->samples) == -EPIPE)
                snd_pcm_prepare(data->alsaDeviceHandle);
                
        }
        
    }

    static void alsaCleanup(Soloud *aSoloud)
    {
        if (0 == aSoloud->mBackendData)
        {
            return;
        }
        ALSAData *data = static_cast<ALSAData*>(aSoloud->mBackendData);
        data->audioProcessingDone = true;
        if (data->threadHandle)
        {
            Thread::wait(data->threadHandle);
            Thread::release(data->threadHandle);
        }
        snd_pcm_drain(data->alsaDeviceHandle);
        snd_pcm_close(data->alsaDeviceHandle);
        if (0 != data->sampleBuffer)
        {
            delete[] data->sampleBuffer;
        }
        if (0 != data->buffer)
        {
            delete[] data->buffer;
        }
        delete data;
        aSoloud->mBackendData = 0;
    }

    result alsa_init(Soloud *aSoloud, unsigned int aFlags, unsigned int aSamplerate, unsigned int aBuffer, unsigned int aChannels)
    {
        ALSAData *data = new ALSAData;
        memset(data, 0, sizeof(ALSAData));
        aSoloud->mBackendData = data;
        aSoloud->mBackendCleanupFunc = alsaCleanup;
        data->samples = aBuffer;
        data->channels = 2;
        data->soloud = aSoloud;

        int rc;
        snd_pcm_t *handle;
        rc = snd_pcm_open(&handle, "default", SND_PCM_STREAM_PLAYBACK, 0);
        if (rc < 0)
        {
            return UNKNOWN_ERROR;
        }
        
        data->alsaDeviceHandle = handle;
        
        snd_pcm_hw_params_t *params;
        snd_pcm_hw_params_alloca(&params);
        snd_pcm_hw_params_any(handle, params);

        snd_pcm_hw_params_set_access(handle, params, SND_PCM_ACCESS_RW_INTERLEAVED);
        snd_pcm_hw_params_set_format(handle, params, SND_PCM_FORMAT_S16_LE);
        snd_pcm_hw_params_set_channels(handle, params, 2);
        snd_pcm_hw_params_set_buffer_size(handle, params, aBuffer);
        
        unsigned int val = aSamplerate;
        int dir = 0;
        rc = snd_pcm_hw_params_set_rate_near(handle, params, &val, &dir);
        if (rc < 0)
        {
            return UNKNOWN_ERROR;
        }

        rc = snd_pcm_hw_params(handle, params);
        if (rc < 0) 
        {
            return UNKNOWN_ERROR;
        }

        snd_pcm_hw_params_get_rate(params, &val, &dir);
        aSamplerate = val;
        snd_pcm_hw_params_get_channels(params, &val);
        data->channels = val;

        data->buffer = new float[data->samples*data->channels];
        data->sampleBuffer = new short[data->samples*data->channels];
        aSoloud->postinit_internal(aSamplerate, data->samples * data->channels, aFlags, 2);
        data->threadHandle = Thread::createThread(alsaThread, data);
        if (0 == data->threadHandle)
        {
            return UNKNOWN_ERROR;
        }
        aSoloud->mBackendString = "ALSA";
        return 0;
    }
};
#endif