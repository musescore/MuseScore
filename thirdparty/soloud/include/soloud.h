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

#ifndef SOLOUD_H
#define SOLOUD_H

#include <stdlib.h> // rand
#include <math.h> // sin

#ifdef SOLOUD_NO_ASSERTS
#define SOLOUD_ASSERT(x)
#else
#ifdef _MSC_VER
#include <stdio.h> // for sprintf in asserts
#ifndef VC_EXTRALEAN
#define VC_EXTRALEAN
#endif
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h> // only needed for OutputDebugStringA, should be solved somehow.
#define SOLOUD_ASSERT(x) if (!(x)) { char temp[200]; sprintf(temp, "%s(%d): assert(%s) failed.\n", __FILE__, __LINE__, #x); OutputDebugStringA(temp); __debugbreak(); }
#else
#include <assert.h> // assert
#define SOLOUD_ASSERT(x) assert(x)
#endif
#endif

#ifdef WITH_SDL
#undef WITH_SDL2
#undef WITH_SDL1
#define WITH_SDL1
#define WITH_SDL2
#endif

#ifdef WITH_SDL_STATIC
#undef WITH_SDL1_STATIC
#define WITH_SDL1_STATIC
#endif

#ifndef M_PI
#define M_PI 3.14159265359
#endif

#if defined(_WIN32)||defined(_WIN64)
#define WINDOWS_VERSION
#endif

#if !defined(DISABLE_SIMD)
#if defined(__x86_64__) || defined( _M_X64 ) || defined( __i386 ) || defined( _M_IX86 )
#define SOLOUD_SSE_INTRINSICS
#endif
#endif

#define SOLOUD_VERSION 202002

/////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////
// Configuration defines

// Maximum number of filters per stream
#define FILTERS_PER_STREAM 1

// Number of samples to process on one go
#define SAMPLE_GRANULARITY 1024

// Maximum number of concurrent voices (hard limit is 4095)
#define VOICE_COUNT 96

// Use linear resampler
#define RESAMPLER_LINEAR

// 1)mono, 2)stereo 4)quad 6)5.1 8)7.1
#define MAX_CHANNELS 2

//
/////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////

// Typedefs have to be made before the includes, as the
// includes depend on them.
namespace SoLoud
{
	class Soloud;
	typedef void (*mutexCallFunction)(void *aMutexPtr);
	typedef void (*soloudCallFunction)(Soloud *aSoloud);
	typedef unsigned int result;
	typedef unsigned int handle;
	typedef double time;
};

namespace SoLoud
{
	// Class that handles aligned allocations to support vectorized operations
	class AlignedFloatBuffer
	{
	public:
		float *mData; // aligned pointer
		unsigned char *mBasePtr; // raw allocated pointer (for delete)
		int mFloats; // size of buffer (w/out padding)

		// ctor
		AlignedFloatBuffer();
		// Allocate and align buffer
		result init(unsigned int aFloats);
		// Clear data to zero.
		void clear();
		// dtor
		~AlignedFloatBuffer();
	};

	// Lightweight class that handles small aligned buffer to support vectorized operations
	class TinyAlignedFloatBuffer
	{
	public:
		float *mData; // aligned pointer
		unsigned char mActualData[sizeof(float) * 16 + 16];

		// ctor
		TinyAlignedFloatBuffer();
	};
};

#include "soloud_filter.h"
#include "soloud_fader.h"
#include "soloud_audiosource.h"
#include "soloud_bus.h"
#include "soloud_queue.h"
#include "soloud_error.h"

namespace SoLoud
{

	// Soloud core class.
	class Soloud
	{
	public:
		// Back-end data; content is up to the back-end implementation.
		void * mBackendData;
		// Pointer for the audio thread mutex.
		void * mAudioThreadMutex;
		// Flag for when we're inside the mutex, used for debugging.
		bool mInsideAudioThreadMutex;
		// Called by SoLoud to shut down the back-end. If NULL, not called. Should be set by back-end.
		soloudCallFunction mBackendCleanupFunc;

		// CTor
		Soloud();
		// DTor
		~Soloud();

		enum BACKENDS
		{
			AUTO = 0,
            MUAUDIO,
			SDL1,
			SDL2,
			PORTAUDIO,
			WINMM,
			XAUDIO2,
			WASAPI,
			ALSA,
			JACK,
			OSS,
			OPENAL,
			COREAUDIO,
			OPENSLES,
			VITA_HOMEBREW,
			MINIAUDIO,
			NOSOUND,
			NULLDRIVER,
			BACKEND_MAX,
		};

		enum FLAGS
		{
			// Use round-off clipper
			CLIP_ROUNDOFF = 1,
			ENABLE_VISUALIZATION = 2,
			LEFT_HANDED_3D = 4,
			NO_FPU_REGISTER_CHANGE = 8
		};

		// Initialize SoLoud. Must be called before SoLoud can be used.
		result init(unsigned int aFlags = Soloud::CLIP_ROUNDOFF, unsigned int aBackend = Soloud::AUTO, unsigned int aSamplerate = Soloud::AUTO, unsigned int aBufferSize = Soloud::AUTO, unsigned int aChannels = 2);

		// Deinitialize SoLoud. Must be called before shutting down.
		void deinit();

		// Query SoLoud version number (should equal to SOLOUD_VERSION macro)
		unsigned int getVersion() const;

		// Translate error number to an asciiz string
		const char * getErrorString(result aErrorCode) const;

		// Returns current backend ID (BACKENDS enum)
		unsigned int getBackendId();
		// Returns current backend string. May be NULL.
		const char * getBackendString();
		// Returns current backend channel count (1 mono, 2 stereo, etc)
		unsigned int getBackendChannels();
		// Returns current backend sample rate
		unsigned int getBackendSamplerate();
		// Returns current backend buffer size
		unsigned int getBackendBufferSize();

		// Set speaker position in 3d space
		result setSpeakerPosition(unsigned int aChannel, float aX, float aY, float aZ);
		// Get speaker position in 3d space
		result getSpeakerPosition(unsigned int aChannel, float &aX, float &aY, float &aZ);

		// Start playing a sound. Returns voice handle, which can be ignored or used to alter the playing sound's parameters. Negative volume means to use default.
		handle play(AudioSource &aSound, float aVolume = -1.0f, float aPan = 0.0f, bool aPaused = 0, unsigned int aBus = 0);
		// Start playing a sound delayed in relation to other sounds called via this function. Negative volume means to use default.
		handle playClocked(time aSoundTime, AudioSource &aSound, float aVolume = -1.0f, float aPan = 0.0f, unsigned int aBus = 0);
		// Start playing a 3d audio source
		handle play3d(AudioSource &aSound, float aPosX, float aPosY, float aPosZ, float aVelX = 0.0f, float aVelY = 0.0f, float aVelZ = 0.0f, float aVolume = 1.0f, bool aPaused = 0, unsigned int aBus = 0);
		// Start playing a 3d audio source, delayed in relation to other sounds called via this function.
		handle play3dClocked(time aSoundTime, AudioSource &aSound, float aPosX, float aPosY, float aPosZ, float aVelX = 0.0f, float aVelY = 0.0f, float aVelZ = 0.0f, float aVolume = 1.0f, unsigned int aBus = 0);
		// Start playing a sound without any panning. It will be played at full volume.
		handle playBackground(AudioSource &aSound, float aVolume = -1.0f, bool aPaused = 0, unsigned int aBus = 0);

		// Seek the audio stream to certain point in time. Some streams can't seek backwards. Relative play speed affects time.
		result seek(handle aVoiceHandle, time aSeconds);
		// Stop the sound.
		void stop(handle aVoiceHandle);
		// Stop all voices.
		void stopAll();
		// Stop all voices that play this sound source
		void stopAudioSource(AudioSource &aSound);
		// Count voices that play this audio source
		int countAudioSource(AudioSource &aSound);

		// Set a live filter parameter. Use 0 for the global filters.
		void setFilterParameter(handle aVoiceHandle, unsigned int aFilterId, unsigned int aAttributeId, float aValue);
		// Get a live filter parameter. Use 0 for the global filters.
		float getFilterParameter(handle aVoiceHandle, unsigned int aFilterId, unsigned int aAttributeId);
		// Fade a live filter parameter. Use 0 for the global filters.
		void fadeFilterParameter(handle aVoiceHandle, unsigned int aFilterId, unsigned int aAttributeId, float aTo, time aTime);
		// Oscillate a live filter parameter. Use 0 for the global filters.
		void oscillateFilterParameter(handle aVoiceHandle, unsigned int aFilterId, unsigned int aAttributeId, float aFrom, float aTo, time aTime);

		// Get current play time, in seconds.
		time getStreamTime(handle aVoiceHandle);
		// Get current sample position, in seconds.
		time getStreamPosition(handle aVoiceHandle);
		// Get current pause state.
		bool getPause(handle aVoiceHandle);
		// Get current volume.
		float getVolume(handle aVoiceHandle);
		// Get current overall volume (set volume * 3d volume)
		float getOverallVolume(handle aVoiceHandle);
		// Get current pan.
		float getPan(handle aVoiceHandle);
		// Get current sample rate.
		float getSamplerate(handle aVoiceHandle);
		// Get current voice protection state.
		bool getProtectVoice(handle aVoiceHandle);
		// Get the current number of busy voices.
		unsigned int getActiveVoiceCount();
		// Get the current number of voices in SoLoud
		unsigned int getVoiceCount();
		// Check if the handle is still valid, or if the sound has stopped.
		bool isValidVoiceHandle(handle aVoiceHandle);
		// Get current relative play speed.
		float getRelativePlaySpeed(handle aVoiceHandle);
		// Get current post-clip scaler value.
		float getPostClipScaler() const;
		// Get current global volume
		float getGlobalVolume() const;
		// Get current maximum active voice setting
		unsigned int getMaxActiveVoiceCount() const;
		// Query whether a voice is set to loop.
		bool getLooping(handle aVoiceHandle);
		// Get voice loop point value
		time getLoopPoint(handle aVoiceHandle);

		// Set voice loop point value
		void setLoopPoint(handle aVoiceHandle, time aLoopPoint);
		// Set voice's loop state
		void setLooping(handle aVoiceHandle, bool aLooping);
		// Set current maximum active voice setting
		result setMaxActiveVoiceCount(unsigned int aVoiceCount);
		// Set behavior for inaudible sounds
		void setInaudibleBehavior(handle aVoiceHandle, bool aMustTick, bool aKill);
		// Set the global volume
		void setGlobalVolume(float aVolume);
		// Set the post clip scaler value
		void setPostClipScaler(float aScaler);
		// Set the pause state
		void setPause(handle aVoiceHandle, bool aPause);
		// Pause all voices
		void setPauseAll(bool aPause);
		// Set the relative play speed
		result setRelativePlaySpeed(handle aVoiceHandle, float aSpeed);
		// Set the voice protection state
		void setProtectVoice(handle aVoiceHandle, bool aProtect);
		// Set the sample rate
		void setSamplerate(handle aVoiceHandle, float aSamplerate);
		// Set panning value; -1 is left, 0 is center, 1 is right
		void setPan(handle aVoiceHandle, float aPan);
		// Set absolute left/right volumes
		void setPanAbsolute(handle aVoiceHandle, float aLVolume, float aRVolume, float aLBVolume = 0, float aRBVolume = 0, float aCVolume = 0, float aSVolume = 0);
		// Set overall volume
		void setVolume(handle aVoiceHandle, float aVolume);
		// Set delay, in samples, before starting to play samples. Calling this on a live sound will cause glitches.
		void setDelaySamples(handle aVoiceHandle, unsigned int aSamples);

		// Set up volume fader
		void fadeVolume(handle aVoiceHandle, float aTo, time aTime);
		// Set up panning fader
		void fadePan(handle aVoiceHandle, float aTo, time aTime);
		// Set up relative play speed fader
		void fadeRelativePlaySpeed(handle aVoiceHandle, float aTo, time aTime);
		// Set up global volume fader
		void fadeGlobalVolume(float aTo, time aTime);
		// Schedule a stream to pause
		void schedulePause(handle aVoiceHandle, time aTime);
		// Schedule a stream to stop
		void scheduleStop(handle aVoiceHandle, time aTime);

		// Set up volume oscillator
		void oscillateVolume(handle aVoiceHandle, float aFrom, float aTo, time aTime);
		// Set up panning oscillator
		void oscillatePan(handle aVoiceHandle, float aFrom, float aTo, time aTime);
		// Set up relative play speed oscillator
		void oscillateRelativePlaySpeed(handle aVoiceHandle, float aFrom, float aTo, time aTime);
		// Set up global volume oscillator
		void oscillateGlobalVolume(float aFrom, float aTo, time aTime);

		// Set global filters. Set to NULL to clear the filter.
		void setGlobalFilter(unsigned int aFilterId, Filter *aFilter);

		// Enable or disable visualization data gathering
		void setVisualizationEnable(bool aEnable);

		// Calculate and get 256 floats of FFT data for visualization. Visualization has to be enabled before use.
		float *calcFFT();

		// Get 256 floats of wave data for visualization. Visualization has to be enabled before use.
		float *getWave();

		// Get approximate output volume for a channel for visualization. Visualization has to be enabled before use.
		float getApproximateVolume(unsigned int aChannel);

		// Get current loop count. Returns 0 if handle is not valid. (All audio sources may not update loop count)
		unsigned int getLoopCount(handle aVoiceHandle);

		// Get audiosource-specific information from a voice.
		float getInfo(handle aVoiceHandle, unsigned int aInfoKey);

		// Create a voice group. Returns 0 if unable (out of voice groups / out of memory)
		handle createVoiceGroup();
		// Destroy a voice group.
		result destroyVoiceGroup(handle aVoiceGroupHandle);
		// Add a voice handle to a voice group
		result addVoiceToGroup(handle aVoiceGroupHandle, handle aVoiceHandle);
		// Is this handle a valid voice group?
		bool isVoiceGroup(handle aVoiceGroupHandle);
		// Is this voice group empty?
		bool isVoiceGroupEmpty(handle aVoiceGroupHandle);

		// Perform 3d audio parameter update
		void update3dAudio();

		// Set the speed of sound constant for doppler
		result set3dSoundSpeed(float aSpeed);
		// Get the current speed of sound constant for doppler
		float get3dSoundSpeed();
		// Set 3d listener parameters
		void set3dListenerParameters(float aPosX, float aPosY, float aPosZ, float aAtX, float aAtY, float aAtZ, float aUpX, float aUpY, float aUpZ, float aVelocityX = 0.0f, float aVelocityY = 0.0f, float aVelocityZ = 0.0f);
		// Set 3d listener position
		void set3dListenerPosition(float aPosX, float aPosY, float aPosZ);
		// Set 3d listener "at" vector
		void set3dListenerAt(float aAtX, float aAtY, float aAtZ);
		// set 3d listener "up" vector
		void set3dListenerUp(float aUpX, float aUpY, float aUpZ);
		// Set 3d listener velocity
		void set3dListenerVelocity(float aVelocityX, float aVelocityY, float aVelocityZ);

		// Set 3d audio source parameters
		void set3dSourceParameters(handle aVoiceHandle, float aPosX, float aPosY, float aPosZ, float aVelocityX = 0.0f, float aVelocityY = 0.0f, float aVelocityZ = 0.0f);
		// Set 3d audio source position
		void set3dSourcePosition(handle aVoiceHandle, float aPosX, float aPosY, float aPosZ);
		// Set 3d audio source velocity
		void set3dSourceVelocity(handle aVoiceHandle, float aVelocityX, float aVelocityY, float aVelocityZ);
		// Set 3d audio source min/max distance (distance < min means max volume)
		void set3dSourceMinMaxDistance(handle aVoiceHandle, float aMinDistance, float aMaxDistance);
		// Set 3d audio source attenuation parameters
		void set3dSourceAttenuation(handle aVoiceHandle, unsigned int aAttenuationModel, float aAttenuationRolloffFactor);
		// Set 3d audio source doppler factor to reduce or enhance doppler effect. Default = 1.0
		void set3dSourceDopplerFactor(handle aVoiceHandle, float aDopplerFactor);

		// Rest of the stuff is used internally.

		// Returns mixed float samples in buffer. Called by the back-end, or user with null driver.
		void mix(float *aBuffer, unsigned int aSamples);
		// Returns mixed 16-bit signed integer samples in buffer. Called by the back-end, or user with null driver.
		void mixSigned16(short *aBuffer, unsigned int aSamples);
	public:
		// Mix N samples * M channels. Called by other mix_ functions.
		void mix_internal(unsigned int aSamples);

		// Handle rest of initialization (called from backend)
		void postinit_internal(unsigned int aSamplerate, unsigned int aBufferSize, unsigned int aFlags, unsigned int aChannels);

		// Update list of active voices
		void calcActiveVoices_internal();
		// Map resample buffers to active voices
		void mapResampleBuffers_internal();
		// Perform mixing for a specific bus
		void mixBus_internal(float *aBuffer, unsigned int aSamplesToRead, unsigned int aBufferSize, float *aScratch, unsigned int aBus, float aSamplerate, unsigned int aChannels);
		// Find a free voice, stopping the oldest if no free voice is found.
		int findFreeVoice_internal();
		// Converts handle to voice, if the handle is valid. Returns -1 if not.
		int getVoiceFromHandle_internal(handle aVoiceHandle) const;
		// Converts voice + playindex into handle
		handle getHandleFromVoice_internal(unsigned int aVoice) const;
		// Stop voice (not handle).
		void stopVoice_internal(unsigned int aVoice);
		// Set voice (not handle) pan.
		void setVoicePan_internal(unsigned int aVoice, float aPan);
		// Set voice (not handle) relative play speed.
		result setVoiceRelativePlaySpeed_internal(unsigned int aVoice, float aSpeed);
		// Set voice (not handle) volume.
		void setVoiceVolume_internal(unsigned int aVoice, float aVolume);
		// Set voice (not handle) pause state.
		void setVoicePause_internal(unsigned int aVoice, int aPause);
		// Update overall volume from set and 3d volumes
		void updateVoiceVolume_internal(unsigned int aVoice);
		// Update overall relative play speed from set and 3d speeds
		void updateVoiceRelativePlaySpeed_internal(unsigned int aVoice);
		// Perform 3d audio calculation for array of voices
		void update3dVoices_internal(unsigned int *aVoiceList, unsigned int aVoiceCount);
		// Clip the samples in the buffer
		void clip_internal(AlignedFloatBuffer &aBuffer, AlignedFloatBuffer &aDestBuffer, unsigned int aSamples, float aVolume0, float aVolume1);
		// Remove all non-active voices from group
		void trimVoiceGroup_internal(handle aVoiceGroupHandle);
		// Get pointer to the zero-terminated array of voice handles in a voice group
		handle * voiceGroupHandleToArray_internal(handle aVoiceGroupHandle) const;

		// Lock audio thread mutex.
		void lockAudioMutex_internal();
		// Unlock audio thread mutex.
		void unlockAudioMutex_internal();

		// Max. number of active voices. Busses and tickable inaudibles also count against this.
		unsigned int mMaxActiveVoices;
		// Highest voice in use so far
		unsigned int mHighestVoice;
		// Scratch buffer, used for resampling.
		AlignedFloatBuffer mScratch;
		// Current size of the scratch, in samples.
		unsigned int mScratchSize;
		// Amount of scratch needed.
		unsigned int mScratchNeeded;
		// Output scratch buffer, used in mix_().
		AlignedFloatBuffer mOutputScratch;
		// Resampler buffers, two per active voice.
		AlignedFloatBuffer *mResampleData;
		// Owners of the resample data
		AudioSourceInstance **mResampleDataOwner;
		// Audio voices.
		AudioSourceInstance *mVoice[VOICE_COUNT];
		// Output sample rate (not float)
		unsigned int mSamplerate;
		// Output channel count
		unsigned int mChannels;
		// Current backend ID
		unsigned int mBackendID;
		// Current backend string
		const char * mBackendString;
		// Maximum size of output buffer; used to calculate needed scratch.
		unsigned int mBufferSize;
		// Flags; see Soloud::FLAGS
		unsigned int mFlags;
		// Global volume. Applied before clipping.
		float mGlobalVolume;
		// Post-clip scaler. Applied after clipping.
		float mPostClipScaler;
		// Current play index. Used to create audio handles.
		unsigned int mPlayIndex;
		// Current sound source index. Used to create sound source IDs.
		unsigned int mAudioSourceID;
		// Fader for the global volume.
		Fader mGlobalVolumeFader;
		// Global stream time, for the global volume fader.
		time mStreamTime;
		// Last time seen by the playClocked call
		time mLastClockedTime;
		// Global filter
		Filter *mFilter[FILTERS_PER_STREAM];
		// Global filter instance
		FilterInstance *mFilterInstance[FILTERS_PER_STREAM];

		// Approximate volume for channels.
		float mVisualizationChannelVolume[MAX_CHANNELS];
		// Mono-mixed wave data for visualization and for visualization FFT input
		float mVisualizationWaveData[256];
		// FFT output data
		float mFFTData[256];
		// Snapshot of wave data for visualization
		float mWaveData[256];

		// 3d listener position
		float m3dPosition[3];
		// 3d listener look-at
		float m3dAt[3];
		// 3d listener up
		float m3dUp[3];
		// 3d listener velocity
		float m3dVelocity[3];
		// 3d speed of sound (for doppler)
		float m3dSoundSpeed;

		// 3d position of speakers
		float m3dSpeakerPosition[3 * MAX_CHANNELS];

		// Data related to 3d processing, separate from AudioSource so we can do 3d calculations without audio mutex.
		AudioSourceInstance3dData m3dData[VOICE_COUNT];

		// For each voice group, first int is number of ints alocated.
		unsigned int **mVoiceGroup;
		unsigned int mVoiceGroupCount;

		// List of currently active voices
		unsigned int mActiveVoice[VOICE_COUNT];
		// Number of currently active voices
		unsigned int mActiveVoiceCount;
		// Active voices list needs to be recalculated
		bool mActiveVoiceDirty;
	};
};

#endif
