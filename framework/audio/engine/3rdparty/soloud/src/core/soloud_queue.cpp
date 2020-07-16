/*
SoLoud audio engine
Copyright (c) 2013-2018 Jari Komppa

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

namespace SoLoud
{
	QueueInstance::QueueInstance(Queue *aParent)
	{
		mParent = aParent;
		mFlags |= PROTECTED;
	}
	
	unsigned int QueueInstance::getAudio(float *aBuffer, unsigned int aSamplesToRead, unsigned int aBufferSize)
	{
		if (mParent->mCount == 0)
		{
			return 0;			
		}
		unsigned int copycount = aSamplesToRead;
		unsigned int copyofs = 0;
		while (copycount && mParent->mCount)
		{
			int readcount = mParent->mSource[mParent->mReadIndex]->getAudio(aBuffer + copyofs, copycount, aBufferSize);
			copyofs += readcount;
			copycount -= readcount;
			if (mParent->mSource[mParent->mReadIndex]->hasEnded())
			{
				delete mParent->mSource[mParent->mReadIndex];
				mParent->mSource[mParent->mReadIndex] = 0;
				mParent->mReadIndex = (mParent->mReadIndex + 1) % SOLOUD_QUEUE_MAX;
				mParent->mCount--;
				mLoopCount++;
			}
		}
		return copyofs;
	}

	bool QueueInstance::hasEnded()
	{
		return mLoopCount != 0 && mParent->mCount == 0;
	}

	QueueInstance::~QueueInstance()
	{
	}

	Queue::Queue()
	{
		mQueueHandle = 0;
		mInstance = 0;
		mReadIndex = 0;
		mWriteIndex = 0;
		mCount = 0;
		int i;
		for (i = 0; i < SOLOUD_QUEUE_MAX; i++)
			mSource[i] = 0;
	}
	
	QueueInstance * Queue::createInstance()
	{
		if (mInstance)
		{
			stop();
			mInstance = 0;
		}
		mInstance = new QueueInstance(this);
		return mInstance;
	}

	void Queue::findQueueHandle()
	{
		// Find the channel the queue is playing on to calculate handle..
		int i;
		for (i = 0; mQueueHandle == 0 && i < (signed)mSoloud->mHighestVoice; i++)
		{
			if (mSoloud->mVoice[i] == mInstance)
			{
				mQueueHandle = mSoloud->getHandleFromVoice_internal(i);
			}
		}
	}

	result Queue::play(AudioSource &aSound)
	{
		if (!mSoloud)
		{
			return INVALID_PARAMETER;
		}
	
		findQueueHandle();

		if (mQueueHandle == 0)
			return INVALID_PARAMETER;

		if (mCount >= SOLOUD_QUEUE_MAX)
			return OUT_OF_MEMORY;

		if (!aSound.mAudioSourceID)
		{
			aSound.mAudioSourceID = mSoloud->mAudioSourceID;
			mSoloud->mAudioSourceID++;
		}

		SoLoud::AudioSourceInstance *instance = aSound.createInstance();

		if (instance == 0)
		{
			return OUT_OF_MEMORY;
		}
		instance->init(aSound, 0);
		instance->mAudioSourceID = aSound.mAudioSourceID;

		mSoloud->lockAudioMutex_internal();
		mSource[mWriteIndex] = instance;
		mWriteIndex = (mWriteIndex + 1) % SOLOUD_QUEUE_MAX;
		mCount++;
		mSoloud->unlockAudioMutex_internal();

		return SO_NO_ERROR;
	}


	unsigned int Queue::getQueueCount()
	{
		unsigned int count;
		mSoloud->lockAudioMutex_internal();
		count = mCount;
		mSoloud->unlockAudioMutex_internal();
		return count;
	}

	bool Queue::isCurrentlyPlaying(AudioSource &aSound)
	{
		if (mCount == 0 || aSound.mAudioSourceID == 0)
			return false;
		mSoloud->lockAudioMutex_internal();
		bool res = mSource[mReadIndex]->mAudioSourceID == aSound.mAudioSourceID;
		mSoloud->unlockAudioMutex_internal();
		return res;
	}

	result Queue::setParamsFromAudioSource(AudioSource &aSound)
	{
		mChannels = aSound.mChannels;
		mBaseSamplerate = aSound.mBaseSamplerate;

	    return SO_NO_ERROR;
	}
	
	result Queue::setParams(float aSamplerate, unsigned int aChannels)
	{
        if (aChannels < 1 || aChannels > MAX_CHANNELS) {
	        return INVALID_PARAMETER;
        }
		mChannels = aChannels;
		mBaseSamplerate = aSamplerate;
	    return SO_NO_ERROR;
	}
}
