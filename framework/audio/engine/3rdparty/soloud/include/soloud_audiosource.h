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

#ifndef SOLOUD_AUDIOSOURCE_H
#define SOLOUD_AUDIOSOURCE_H

#include "soloud.h"
#include "soloud_fader.h"
#include "soloud_filter.h"

namespace SoLoud
{
	class AudioSource;	
	class AudioSourceInstance;
	class AudioSourceInstance3dData;

	class AudioCollider
	{
	public:
		// Calculate volume multiplier. Assumed to return value between 0 and 1.
		virtual float collide(Soloud *aSoloud, AudioSourceInstance3dData *aAudioInstance3dData,	int aUserData) = 0;
	};

	class AudioAttenuator
	{
	public:
		virtual float attenuate(float aDistance, float aMinDistance, float aMaxDistance, float aRolloffFactor) = 0;
	};

	class AudioSourceInstance3dData
	{
	public:
		// ctor
		AudioSourceInstance3dData();
		// Set settings from audiosource
		void init(AudioSource &aSource);
		// 3d position
		float m3dPosition[3];
		// 3d velocity
		float m3dVelocity[3];
		// 3d cone direction
		/*
		float m3dConeDirection[3];
		// 3d cone inner angle
		float m3dConeInnerAngle;
		// 3d cone outer angle
		float m3dConeOuterAngle;
		// 3d cone outer volume multiplier
		float m3dConeOuterVolume;
		*/
		// 3d min distance
		float m3dMinDistance;
		// 3d max distance
		float m3dMaxDistance;
		// 3d attenuation rolloff factor
		float m3dAttenuationRolloff;
		// 3d attenuation model
		unsigned int m3dAttenuationModel;
		// 3d doppler factor
		float m3dDopplerFactor;
		// Pointer to a custom audio collider object
		AudioCollider *mCollider;
		// Pointer to a custom audio attenuator object
		AudioAttenuator *mAttenuator;
		// User data related to audio collider
		int mColliderData;

		// Doppler sample rate multiplier
		float mDopplerValue;		
		// Overall 3d volume
		float m3dVolume;
		// Channel volume
		float mChannelVolume[MAX_CHANNELS];
		// Copy of flags
		unsigned int mFlags;
		// Latest handle for this voice
		handle mHandle;
	};

	// Base class for audio instances
	class AudioSourceInstance
	{
	public:
		enum FLAGS
		{			
			// This audio instance loops (if supported)
			LOOPING = 1,
			// This audio instance is protected - won't get stopped if we run out of voices
			PROTECTED = 2,
			// This audio instance is paused
			PAUSED = 4,
			// This audio instance is affected by 3d processing
			PROCESS_3D = 8,
			// This audio instance has listener-relative 3d coordinates
			LISTENER_RELATIVE = 16,
			// Currently inaudible
			INAUDIBLE = 32,
			// If inaudible, should be killed (default = don't kill kill)
			INAUDIBLE_KILL = 64,
			// If inaudible, should still be ticked (default = pause)
			INAUDIBLE_TICK = 128
		};
		// Ctor
		AudioSourceInstance();
		// Dtor
		virtual ~AudioSourceInstance();
		// Play index; used to identify instances from handles
		unsigned int mPlayIndex;
		// Loop count
		unsigned int mLoopCount;
		// Flags; see AudioSourceInstance::FLAGS
		unsigned int mFlags;
		// Pan value, for getPan()
		float mPan;
		// Volume for each channel (panning)
		float mChannelVolume[MAX_CHANNELS];
		// Set volume
		float mSetVolume;
		// Overall volume overall = set * 3d
		float mOverallVolume;
		// Base samplerate; samplerate = base samplerate * relative play speed
		float mBaseSamplerate;
		// Samplerate; samplerate = base samplerate * relative play speed
		float mSamplerate;
		// Number of channels this audio source produces
		unsigned int mChannels;
		// Relative play speed; samplerate = base samplerate * relative play speed
		float mSetRelativePlaySpeed;
		// Overall relative plays peed; overall = set * 3d
		float mOverallRelativePlaySpeed;
		// How long this stream has played, in seconds.
		time mStreamTime;
		// Position of this stream, in seconds.
		time mStreamPosition;
		// Fader for the audio panning
		Fader mPanFader;
		// Fader for the audio volume
		Fader mVolumeFader;
		// Fader for the relative play speed
		Fader mRelativePlaySpeedFader;
		// Fader used to schedule pausing of the stream
		Fader mPauseScheduler;
		// Fader used to schedule stopping of the stream
		Fader mStopScheduler;
		// Affected by some fader
		int mActiveFader;
		// Current channel volumes, used to ramp the volume changes to avoid clicks
		float mCurrentChannelVolume[MAX_CHANNELS];
		// ID of the sound source that generated this instance
		unsigned int mAudioSourceID;
		// Handle of the bus this audio instance is playing on. 0 for root.
		unsigned int mBusHandle;
		// Filter pointer
		FilterInstance *mFilter[FILTERS_PER_STREAM];
		// Initialize instance. Mostly internal use.
		void init(AudioSource &aSource, int aPlayIndex);
		// Buffers for the resampler
		AlignedFloatBuffer *mResampleData[2];
		// Sub-sample playhead; 16.16 fixed point
		unsigned int mSrcOffset;
		// Samples left over from earlier pass
		unsigned int mLeftoverSamples;
		// Number of samples to delay streaming
		unsigned int mDelaySamples;
		// When looping, start playing from this time
		time mLoopPoint;

		// Get N samples from the stream to the buffer. Report samples written.
		virtual unsigned int getAudio(float *aBuffer, unsigned int aSamplesToRead, unsigned int aBufferSize) = 0;
		// Has the stream ended?
		virtual bool hasEnded() = 0;
		// Seek to certain place in the stream. Base implementation is generic "tape" seek (and slow).
		virtual result seek(time aSeconds, float *mScratch, unsigned int mScratchSize);
		// Rewind stream. Base implementation returns NOT_IMPLEMENTED, meaning it can't rewind.
		virtual result rewind();
		// Get information. Returns 0 by default.
		virtual float getInfo(unsigned int aInfoKey);
	};

	class Soloud;

	// Base class for audio sources
	class AudioSource
	{
	public:
		enum FLAGS
		{
			// The instances from this audio source should loop
			SHOULD_LOOP = 1,
			// Only one instance of this audio source should play at the same time
			SINGLE_INSTANCE = 2,
			// Visualization data gathering enabled. Only for busses.
			VISUALIZATION_DATA = 4,
			// Audio instances created from this source are affected by 3d processing
			PROCESS_3D = 8,
			// Audio instances created from this source have listener-relative 3d coordinates
			LISTENER_RELATIVE = 16,
			// Delay start of sound by the distance from listener
			DISTANCE_DELAY = 32,
			// If inaudible, should be killed (default)
			INAUDIBLE_KILL = 64,
			// If inaudible, should still be ticked (default = pause)
			INAUDIBLE_TICK = 128
		};
		enum ATTENUATION_MODELS
		{
			// No attenuation
			NO_ATTENUATION = 0,
			// Inverse distance attenuation model
			INVERSE_DISTANCE = 1,
			// Linear distance attenuation model
			LINEAR_DISTANCE = 2,
			// Exponential distance attenuation model
			EXPONENTIAL_DISTANCE = 3
		};

		// Flags. See AudioSource::FLAGS
		unsigned int mFlags;
		// Base sample rate, used to initialize instances
		float mBaseSamplerate;
		// Default volume for created instances
		float mVolume;
		// Number of channels this audio source produces
		unsigned int mChannels;
		// Sound source ID. Assigned by SoLoud the first time it's played.
		unsigned int mAudioSourceID;
		// 3d min distance
		float m3dMinDistance;
		// 3d max distance
		float m3dMaxDistance;
		// 3d attenuation rolloff factor
		float m3dAttenuationRolloff;
		// 3d attenuation model
		unsigned int m3dAttenuationModel;
		// 3d doppler factor
		float m3dDopplerFactor;
		// Filter pointer
		Filter *mFilter[FILTERS_PER_STREAM];
		// Pointer to the Soloud object. Needed to stop all instances in dtor.
		Soloud *mSoloud;
		// Pointer to a custom audio collider object
		AudioCollider *mCollider;
		// Pointer to custom attenuator object
		AudioAttenuator *mAttenuator;
		// User data related to audio collider
		int mColliderData;
		// When looping, start playing from this time
		time mLoopPoint;

		// CTor
		AudioSource();
		// Set default volume for instances
		void setVolume(float aVolume);
		// Set the looping of the instances created from this audio source
		void setLooping(bool aLoop);
		// Set whether only one instance of this sound should ever be playing at the same time
		void setSingleInstance(bool aSingleInstance);
		
		// Set the minimum and maximum distances for 3d audio source (closer to min distance = max vol)
		void set3dMinMaxDistance(float aMinDistance, float aMaxDistance);
		// Set attenuation model and rolloff factor for 3d audio source
		void set3dAttenuation(unsigned int aAttenuationModel, float aAttenuationRolloffFactor);
		// Set doppler factor to reduce or enhance doppler effect, default = 1.0
		void set3dDopplerFactor(float aDopplerFactor);
		// Set the coordinates for this audio source to be relative to listener's coordinates.
		void set3dListenerRelative(bool aListenerRelative);
		// Enable delaying the start of the sound based on the distance.
		void set3dDistanceDelay(bool aDistanceDelay);

		// Set a custom 3d audio collider. Set to NULL to disable.
		void set3dCollider(AudioCollider *aCollider, int aUserData = 0);
		// Set a custom attenuator. Set to NULL to disable.
		void set3dAttenuator(AudioAttenuator *aAttenuator);

		// Set behavior for inaudible sounds
		void setInaudibleBehavior(bool aMustTick, bool aKill);

		// Set time to jump to when looping
		void setLoopPoint(time aLoopPoint);
		// Get current loop point value
		time getLoopPoint();

		// Set filter. Set to NULL to clear the filter.
		virtual void setFilter(unsigned int aFilterId, Filter *aFilter);
		// DTor
		virtual ~AudioSource();
		// Create instance from the audio source. Called from within Soloud class.
		virtual AudioSourceInstance *createInstance() = 0;
		// Stop all instances of this audio source
		void stop();
	};
};

#endif
