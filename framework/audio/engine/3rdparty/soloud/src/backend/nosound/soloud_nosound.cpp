/*
SoLoud audio engine
Copyright (c) 2013-2020 Jari Komppa

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

#if !defined(WITH_NOSOUND)

namespace SoLoud
{
	result nosound_init(Soloud *aSoloud, unsigned int aFlags, unsigned int aSamplerate, unsigned int aBuffer)
	{
		return NOT_IMPLEMENTED;
	}
};

#else

namespace SoLoud
{
    struct SoLoudNosoundData
    {
        AlignedFloatBuffer mBuffer;
        Soloud *mSoloud;
        int mSamples;
		int mSamplerate;
        Thread::ThreadHandle mThreadHandle;
        bool mRunning;
        SoLoudNosoundData()
        {
            mBuffer.clear();
            mSoloud = 0;
            mSamples = 0;
            mSamplerate = 0;
            mThreadHandle = 0;
            mRunning = 0;
        }
    };

    static void nosoundThread(LPVOID aParam)
    {
        SoLoudNosoundData *data = static_cast<SoLoudNosoundData*>(aParam);
		int delay = (1000 * data->mSamples) / data->mSamplerate;
		int overflow = 0;
        while (data->mRunning) 
        {			
			data->mSoloud->mix(data->mBuffer.mData, data->mSamples);            
			int startwait = Thread::getTimeMillis() - overflow;
			int t;
			do
			{
				Thread::sleep(1);
				t = Thread::getTimeMillis() - startwait;
			} 
			while (t < delay);
			overflow = t - delay;
        }
    }

    static void nosoundCleanup(Soloud *aSoloud)
    {
        if (0 == aSoloud->mBackendData)
        {
            return;
        }
		SoLoudNosoundData*data = static_cast<SoLoudNosoundData*>(aSoloud->mBackendData);
		data->mRunning = false;
		if (data->mThreadHandle)
		{
			Thread::wait(data->mThreadHandle);
			Thread::release(data->mThreadHandle);
		}
        delete data;
        aSoloud->mBackendData = 0;
    }

	result nosound_init(Soloud *aSoloud, unsigned int aFlags, unsigned int aSamplerate, unsigned int aBuffer, unsigned int aChannels)
    {
		SoLoudNosoundData*data = new SoLoudNosoundData;		
		aSoloud->mBackendData = data;
        aSoloud->mBackendCleanupFunc = nosoundCleanup;
        data->mSamples = aBuffer;
		data->mSamplerate = aSamplerate;
        data->mSoloud = aSoloud;
        data->mBuffer.init(data->mSamples * aChannels);
		data->mRunning = true;
        aSoloud->postinit_internal(aSamplerate, data->mSamples * aChannels, aFlags, aChannels);
        data->mThreadHandle = Thread::createThread(nosoundThread, data);
        if (0 == data->mThreadHandle)
        {
            return UNKNOWN_ERROR;
        }
        aSoloud->mBackendString = "NoSound";
        return 0;
    }
};

#endif