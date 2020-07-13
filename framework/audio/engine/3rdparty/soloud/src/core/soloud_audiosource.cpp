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

namespace SoLoud
{

	AudioSourceInstance3dData::AudioSourceInstance3dData()
	{
		m3dAttenuationModel = 0;
		m3dAttenuationRolloff = 1;
		m3dDopplerFactor = 1.0;
		m3dMaxDistance = 1000000.0f;
		m3dMinDistance = 0.0f;
		m3dPosition[0] = 0;
		m3dPosition[1] = 0;
		m3dPosition[2] = 0;
		m3dVelocity[0] = 0;
		m3dVelocity[1] = 0;
		m3dVelocity[2] = 0;
		m3dVolume = 0;
		mCollider = 0;
		mColliderData = 0;
		mAttenuator = 0;
		mDopplerValue = 0;
		mFlags = 0;
		mHandle = 0;
		for (int i = 0; i < MAX_CHANNELS; i++)
			mChannelVolume[i] = 0;
	}

	void AudioSourceInstance3dData::init(AudioSource &aSource)
	{
		m3dAttenuationModel = aSource.m3dAttenuationModel;
		m3dAttenuationRolloff = aSource.m3dAttenuationRolloff;
		m3dDopplerFactor = aSource.m3dDopplerFactor;
		m3dMaxDistance = aSource.m3dMaxDistance;
		m3dMinDistance = aSource.m3dMinDistance;
		mCollider = aSource.mCollider;
		mColliderData = aSource.mColliderData;
		mAttenuator = aSource.mAttenuator;
		m3dVolume = 1.0f;
		mDopplerValue = 1.0f;
	}

	AudioSourceInstance::AudioSourceInstance()
	{
		mPlayIndex = 0;
		mFlags = 0;
		mPan = 0;
		// Default all volumes to 1.0 so sound behind N mix busses isn't super quiet.
		int i;
		for (i = 0; i < MAX_CHANNELS; i++)
			mChannelVolume[i] = 1.0f;		
		mSetVolume = 1.0f;
		mBaseSamplerate = 44100.0f;
		mSamplerate = 44100.0f;
		mSetRelativePlaySpeed = 1.0f;
		mStreamTime = 0.0f;
		mStreamPosition = 0.0f;
		mAudioSourceID = 0;
		mActiveFader = 0;
		mChannels = 1;
		mBusHandle = ~0u;
		mLoopCount = 0;
		mLoopPoint = 0;
		for (i = 0; i < FILTERS_PER_STREAM; i++)
		{
			mFilter[i] = NULL;
		}
		for (i = 0; i < MAX_CHANNELS; i++)
		{
			mCurrentChannelVolume[i] = 0;
		}
		// behind pointers because we swap between the two buffers
		mResampleData[0] = 0;
		mResampleData[1] = 0;
		mSrcOffset = 0;
		mLeftoverSamples = 0;
		mDelaySamples = 0;
		mOverallVolume = 0;
		mOverallRelativePlaySpeed = 1;
	}

	AudioSourceInstance::~AudioSourceInstance()
	{
		int i;
		for (i = 0; i < FILTERS_PER_STREAM; i++)
		{
			delete mFilter[i];
		}		
	}

	void AudioSourceInstance::init(AudioSource &aSource, int aPlayIndex)
	{
		mPlayIndex = aPlayIndex;
		mBaseSamplerate = aSource.mBaseSamplerate;
		mSamplerate = mBaseSamplerate;
		mChannels = aSource.mChannels;
		mStreamTime = 0.0f;
		mStreamPosition = 0.0f;
		mLoopPoint = aSource.mLoopPoint;

		if (aSource.mFlags & AudioSource::SHOULD_LOOP)
		{
			mFlags |= AudioSourceInstance::LOOPING;
		}
		if (aSource.mFlags & AudioSource::PROCESS_3D)
		{
			mFlags |= AudioSourceInstance::PROCESS_3D;
		}
		if (aSource.mFlags & AudioSource::LISTENER_RELATIVE)
		{
			mFlags |= AudioSourceInstance::LISTENER_RELATIVE;
		}
		if (aSource.mFlags & AudioSource::INAUDIBLE_KILL)
		{
			mFlags |= AudioSourceInstance::INAUDIBLE_KILL;
		}
		if (aSource.mFlags & AudioSource::INAUDIBLE_TICK)
		{
			mFlags |= AudioSourceInstance::INAUDIBLE_TICK;
		}
	}

	result AudioSourceInstance::rewind()
	{
		return NOT_IMPLEMENTED;
	}

	result AudioSourceInstance::seek(double aSeconds, float *mScratch, unsigned int mScratchSize)
	{
		double offset = aSeconds - mStreamPosition;
		if (offset <= 0)
		{
			if (rewind() != SO_NO_ERROR)
			{
				// can't do generic seek backwards unless we can rewind.
				return NOT_IMPLEMENTED;
			}
			offset = aSeconds;
		}
		int samples_to_discard = (int)floor(mSamplerate * offset);

		while (samples_to_discard)
		{
			int samples = mScratchSize / mChannels;
			if (samples > samples_to_discard)
				samples = samples_to_discard;
			getAudio(mScratch, samples, samples);
			samples_to_discard -= samples;
		}
		mStreamPosition = offset;
		return SO_NO_ERROR;
	}


	AudioSource::AudioSource() 
	{ 
		int i;
		for (i = 0; i < FILTERS_PER_STREAM; i++)
		{
			mFilter[i] = 0;
		}
		mFlags = 0; 
		mBaseSamplerate = 44100; 
		mAudioSourceID = 0;
		mSoloud = 0;
		mChannels = 1;
		m3dMinDistance = 1;
		m3dMaxDistance = 1000000.0f;
		m3dAttenuationRolloff = 1.0f;
		m3dAttenuationModel = NO_ATTENUATION;
		m3dDopplerFactor = 1.0f;
		mCollider = 0;
		mAttenuator = 0;
		mColliderData = 0;
		mVolume = 1;
		mLoopPoint = 0;
	}

	AudioSource::~AudioSource() 
	{
		stop();
	}

	void AudioSource::setVolume(float aVolume)
	{
		mVolume = aVolume;
	}

	void AudioSource::setLoopPoint(time aLoopPoint)
	{
		mLoopPoint = aLoopPoint;
	}

	time AudioSource::getLoopPoint()
	{
		return mLoopPoint;
	}

	void AudioSource::setLooping(bool aLoop)
	{
		if (aLoop)
		{
			mFlags |= SHOULD_LOOP;
		}
		else
		{
			mFlags &= ~SHOULD_LOOP;
		}
	}

	void AudioSource::setSingleInstance(bool aSingleInstance)
	{
		if (aSingleInstance)
		{
			mFlags |= SINGLE_INSTANCE;
		}
		else
		{
			mFlags &= ~SINGLE_INSTANCE;
		}
	}

	void AudioSource::setFilter(unsigned int aFilterId, Filter *aFilter)
	{
		if (aFilterId >= FILTERS_PER_STREAM)
			return;
		mFilter[aFilterId] = aFilter;
	}

	void AudioSource::stop()
	{
		if (mSoloud)
		{
			mSoloud->stopAudioSource(*this);
		}
	}

	void AudioSource::set3dMinMaxDistance(float aMinDistance, float aMaxDistance)
	{
		m3dMinDistance = aMinDistance;
		m3dMaxDistance = aMaxDistance;
	}

	void AudioSource::set3dAttenuation(unsigned int aAttenuationModel, float aAttenuationRolloffFactor)
	{
		m3dAttenuationModel = aAttenuationModel;
		m3dAttenuationRolloff = aAttenuationRolloffFactor;
	}

	void AudioSource::set3dDopplerFactor(float aDopplerFactor)
	{
		m3dDopplerFactor = aDopplerFactor;
	}

	void AudioSource::set3dListenerRelative(bool aListenerRelative)
	{
		if (aListenerRelative)
		{
			mFlags |= LISTENER_RELATIVE;
		}
		else
		{
			mFlags &= ~LISTENER_RELATIVE;
		}
	}


	void AudioSource::set3dDistanceDelay(bool aDistanceDelay)
	{
		if (aDistanceDelay)
		{
			mFlags |= DISTANCE_DELAY;
		}
		else
		{
			mFlags &= ~DISTANCE_DELAY;
		}
	}

	void AudioSource::set3dCollider(AudioCollider *aCollider, int aUserData)
	{
		mCollider = aCollider;
		mColliderData = aUserData;
	}

	void AudioSource::set3dAttenuator(AudioAttenuator *aAttenuator)
	{
		mAttenuator = aAttenuator;
	}

	void AudioSource::setInaudibleBehavior(bool aMustTick, bool aKill)
	{
		mFlags &= ~(AudioSource::INAUDIBLE_KILL | AudioSource::INAUDIBLE_TICK);
		if (aMustTick)
		{
			mFlags |= AudioSource::INAUDIBLE_TICK;
		}
		if (aKill)
		{
			mFlags |= AudioSource::INAUDIBLE_KILL;
		}
	}


	float AudioSourceInstance::getInfo(unsigned int /*aInfoKey*/)
	{
	    return 0;
	}


};

