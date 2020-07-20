/*
SoLoud audio engine
Copyright (c) 2015 Jari Komppa

Core Audio backend for Mac OS X
Copyright (c) 2015 Petri Häkkinen

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

#include <stdlib.h>
#include <math.h>

#include "soloud.h"
#include "soloud_thread.h"

#if !defined(WITH_COREAUDIO)

namespace SoLoud
{
	result coreaudio_init(SoLoud::Soloud *aSoloud, unsigned int aFlags, unsigned int aSamplerate, unsigned int aBuffer)
	{
		return NOT_IMPLEMENTED;
	}
}
#else

#include <AudioToolbox/AudioToolbox.h>

#define NUM_BUFFERS 2

static AudioQueueRef audioQueue = 0;

namespace SoLoud
{
	void soloud_coreaudio_deinit(SoLoud::Soloud *aSoloud)
	{
		AudioQueueStop(audioQueue, true);
		AudioQueueDispose(audioQueue, false);
	}
	
	static void coreaudio_mutex_lock(void *mutex)
	{
		Thread::lockMutex(mutex);
	}

	static void coreaudio_mutex_unlock(void *mutex)
	{
		Thread::unlockMutex(mutex);
	}

	static void coreaudio_fill_buffer(void *context, AudioQueueRef queue, AudioQueueBufferRef buffer)
	{
		SoLoud::Soloud *aSoloud = (SoLoud::Soloud*)context;
		aSoloud->mixSigned16((short*)buffer->mAudioData, buffer->mAudioDataByteSize / 4);
		AudioQueueEnqueueBuffer(queue, buffer, 0, NULL);
	}

	result coreaudio_init(SoLoud::Soloud *aSoloud, unsigned int aFlags, unsigned int aSamplerate, unsigned int aBuffer, unsigned int aChannels)
	{
		aSoloud->postinit_internal(aSamplerate, aBuffer, aFlags, 2);
		aSoloud->mBackendCleanupFunc = soloud_coreaudio_deinit;

		AudioStreamBasicDescription audioFormat;
		audioFormat.mSampleRate = aSamplerate;
		audioFormat.mFormatID = kAudioFormatLinearPCM;
		audioFormat.mFormatFlags = kLinearPCMFormatFlagIsSignedInteger | kAudioFormatFlagIsPacked;
		audioFormat.mBytesPerPacket = 4;
		audioFormat.mFramesPerPacket = 1;
		audioFormat.mBytesPerFrame = 4;
		audioFormat.mChannelsPerFrame = 2;
		audioFormat.mBitsPerChannel = 16;
		audioFormat.mReserved = 0;

		// create the audio queue
		OSStatus result = AudioQueueNewOutput(&audioFormat, coreaudio_fill_buffer, aSoloud, NULL, NULL, 0, &audioQueue);
		if(result)
		{
			//printf("AudioQueueNewOutput failed (%d)\n", result);
			return UNKNOWN_ERROR;
		}

		// allocate and prime audio buffers
		for(int i = 0; i < NUM_BUFFERS; ++i)
		{
			AudioQueueBufferRef buffer;
			result = AudioQueueAllocateBuffer(audioQueue, aBuffer * 4, &buffer);
			if(result)
			{
				//printf("AudioQueueAllocateBuffer failed (%d)\n", result);
				return UNKNOWN_ERROR;
			}
			buffer->mAudioDataByteSize = aBuffer * 4;
			memset(buffer->mAudioData, 0, buffer->mAudioDataByteSize);
			AudioQueueEnqueueBuffer(audioQueue, buffer, 0, NULL);
		}

		// start playback
		result = AudioQueueStart(audioQueue, NULL);
		if(result)
		{
			//printf("AudioQueueStart failed (%d)\n", result);
			return UNKNOWN_ERROR;
		}

        aSoloud->mBackendString = "CoreAudio";
		return 0;
	}	
};
#endif
