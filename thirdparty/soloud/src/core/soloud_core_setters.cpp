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

#include "soloud_internal.h"

// Setters - set various bits of SoLoud state

namespace SoLoud
{
	void Soloud::setPostClipScaler(float aScaler)
	{
		mPostClipScaler = aScaler;
	}

	void Soloud::setGlobalVolume(float aVolume)
	{
		mGlobalVolumeFader.mActive = 0;
		mGlobalVolume = aVolume;
	}		

	result Soloud::setRelativePlaySpeed(handle aVoiceHandle, float aSpeed)
	{
		result retVal = 0;
		FOR_ALL_VOICES_PRE
			mVoice[ch]->mRelativePlaySpeedFader.mActive = 0;
			retVal = setVoiceRelativePlaySpeed_internal(ch, aSpeed);
			FOR_ALL_VOICES_POST
		return retVal;
	}

	void Soloud::setSamplerate(handle aVoiceHandle, float aSamplerate)
	{
		FOR_ALL_VOICES_PRE
			mVoice[ch]->mBaseSamplerate = aSamplerate;
			updateVoiceRelativePlaySpeed_internal(ch);
		FOR_ALL_VOICES_POST
	}

	void Soloud::setPause(handle aVoiceHandle, bool aPause)
	{
		FOR_ALL_VOICES_PRE
			setVoicePause_internal(ch, aPause);
		FOR_ALL_VOICES_POST
	}

	result Soloud::setMaxActiveVoiceCount(unsigned int aVoiceCount)
	{
		if (aVoiceCount == 0 || aVoiceCount >= VOICE_COUNT)
			return INVALID_PARAMETER;
		lockAudioMutex_internal();
		mMaxActiveVoices = aVoiceCount;
		delete[] mResampleData;
		delete[] mResampleDataOwner;
		mResampleData = new AlignedFloatBuffer[aVoiceCount * 2];
		mResampleDataOwner = new AudioSourceInstance*[aVoiceCount];
		unsigned int i;
		for (i = 0; i < aVoiceCount * 2; i++)
			mResampleData[i].init(SAMPLE_GRANULARITY * MAX_CHANNELS);
		for (i = 0; i < aVoiceCount; i++)
			mResampleDataOwner[i] = NULL;
		mActiveVoiceDirty = true;
		unlockAudioMutex_internal();
		return SO_NO_ERROR;
	}

	void Soloud::setPauseAll(bool aPause)
	{
		lockAudioMutex_internal();
		int ch;
		for (ch = 0; ch < (signed)mHighestVoice; ch++)
		{
			setVoicePause_internal(ch, aPause);
		}
		unlockAudioMutex_internal();
	}

	void Soloud::setProtectVoice(handle aVoiceHandle, bool aProtect)
	{
		FOR_ALL_VOICES_PRE
			if (aProtect)
			{
				mVoice[ch]->mFlags |= AudioSourceInstance::PROTECTED;
			}
			else
			{
				mVoice[ch]->mFlags &= ~AudioSourceInstance::PROTECTED;
			}
		FOR_ALL_VOICES_POST
	}

	void Soloud::setPan(handle aVoiceHandle, float aPan)
	{		
		FOR_ALL_VOICES_PRE
			setVoicePan_internal(ch, aPan);
		FOR_ALL_VOICES_POST
	}

	void Soloud::setPanAbsolute(handle aVoiceHandle, float aLVolume, float aRVolume, float aLBVolume, float aRBVolume, float aCVolume, float aSVolume)
	{
		FOR_ALL_VOICES_PRE
			mVoice[ch]->mPanFader.mActive = 0;	
			mVoice[ch]->mChannelVolume[0] = aLVolume;			
			mVoice[ch]->mChannelVolume[1] = aRVolume;
			if (mVoice[ch]->mChannels == 4)
			{
				mVoice[ch]->mChannelVolume[2] = aLBVolume;
				mVoice[ch]->mChannelVolume[3] = aRBVolume;
			}
			if (mVoice[ch]->mChannels == 6)
			{
				mVoice[ch]->mChannelVolume[2] = aCVolume;
				mVoice[ch]->mChannelVolume[3] = aSVolume;
				mVoice[ch]->mChannelVolume[4] = aLBVolume;
				mVoice[ch]->mChannelVolume[5] = aRBVolume;
			}
			if (mVoice[ch]->mChannels == 8)
			{
				mVoice[ch]->mChannelVolume[2] = aCVolume;
				mVoice[ch]->mChannelVolume[3] = aSVolume;
				mVoice[ch]->mChannelVolume[4] = (aLVolume + aLBVolume) * 0.5f;
				mVoice[ch]->mChannelVolume[5] = (aRVolume + aRBVolume) * 0.5f;
				mVoice[ch]->mChannelVolume[6] = aLBVolume;
				mVoice[ch]->mChannelVolume[7] = aRBVolume;
			}
			FOR_ALL_VOICES_POST
	}

	void Soloud::setInaudibleBehavior(handle aVoiceHandle, bool aMustTick, bool aKill)
	{
		FOR_ALL_VOICES_PRE
			mVoice[ch]->mFlags &= ~(AudioSourceInstance::INAUDIBLE_KILL | AudioSourceInstance::INAUDIBLE_TICK);
			if (aMustTick)
			{
				mVoice[ch]->mFlags |= AudioSourceInstance::INAUDIBLE_TICK;
			}
			if (aKill)
			{
				mVoice[ch]->mFlags |= AudioSourceInstance::INAUDIBLE_KILL;
			}
		FOR_ALL_VOICES_POST
	}

	void Soloud::setLoopPoint(handle aVoiceHandle, time aLoopPoint)
	{
		FOR_ALL_VOICES_PRE
			mVoice[ch]->mLoopPoint = aLoopPoint;
		FOR_ALL_VOICES_POST
	}

	void Soloud::setLooping(handle aVoiceHandle, bool aLooping)
	{
		FOR_ALL_VOICES_PRE
			if (aLooping)
			{
				mVoice[ch]->mFlags |= AudioSourceInstance::LOOPING;
			}
			else
			{
				mVoice[ch]->mFlags &= ~AudioSourceInstance::LOOPING;
			}
		FOR_ALL_VOICES_POST
	}


	void Soloud::setVolume(handle aVoiceHandle, float aVolume)
	{
		FOR_ALL_VOICES_PRE
			mVoice[ch]->mVolumeFader.mActive = 0;
			setVoiceVolume_internal(ch, aVolume);
		FOR_ALL_VOICES_POST
	}

	void Soloud::setDelaySamples(handle aVoiceHandle, unsigned int aSamples)
	{
		FOR_ALL_VOICES_PRE
			mVoice[ch]->mDelaySamples = aSamples;
		FOR_ALL_VOICES_POST
	}

	void Soloud::setVisualizationEnable(bool aEnable)
	{
		if (aEnable)
		{
			mFlags |= ENABLE_VISUALIZATION;
		}
		else
		{
			mFlags &= ~ENABLE_VISUALIZATION;
		}
	}

	result Soloud::setSpeakerPosition(unsigned int aChannel, float aX, float aY, float aZ)
	{
		if (aChannel >= mChannels)
			return INVALID_PARAMETER;
		m3dSpeakerPosition[3 * aChannel + 0] = aX;
		m3dSpeakerPosition[3 * aChannel + 1] = aY;
		m3dSpeakerPosition[3 * aChannel + 2] = aZ;
		return SO_NO_ERROR;
	}

}
