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

#include <string.h>
#include <stdlib.h>
#include <math.h> // sin
#include <float.h> // _controlfp
#include "soloud_internal.h"
#include "soloud_thread.h"
#include "soloud_fft.h"


#ifdef SOLOUD_SSE_INTRINSICS
#include <xmmintrin.h>
#ifdef _M_IX86
#include <emmintrin.h>
#endif
#endif

//#define FLOATING_POINT_DEBUG


#if !defined(WITH_MUAUDIO) && !defined(WITH_SDL2) && !defined(WITH_SDL1) && !defined(WITH_PORTAUDIO) && \
   !defined(WITH_OPENAL) && !defined(WITH_XAUDIO2) && !defined(WITH_WINMM) && \
   !defined(WITH_WASAPI) && !defined(WITH_OSS) && !defined(WITH_SDL1_STATIC) && \
   !defined(WITH_SDL2_STATIC) && !defined(WITH_ALSA) && !defined(WITH_OPENSLES) && \
   !defined(WITH_NULL) && !defined(WITH_COREAUDIO) && !defined(WITH_VITA_HOMEBREW) &&\
   !defined(WITH_JACK) && !defined(WITH_NOSOUND) && !defined(WITH_MINIAUDIO)
#error It appears you haven't enabled any of the back-ends. Please #define one or more of the WITH_ defines (or use premake) '
#endif


namespace SoLoud
{
    AlignedFloatBuffer::AlignedFloatBuffer()
    {
        mBasePtr = 0;
        mData = 0;
        mFloats = 0;
    }

    result AlignedFloatBuffer::init(unsigned int aFloats)
    {
        delete[] mBasePtr;
        mBasePtr = 0;
        mData = 0;
        mFloats = aFloats;
#ifndef SOLOUD_SSE_INTRINSICS
        mBasePtr = new unsigned char[aFloats * sizeof(float)];
        if (mBasePtr == NULL)
            return OUT_OF_MEMORY;
        mData = (float*)mBasePtr;
#else
        mBasePtr = new unsigned char[aFloats * sizeof(float) + 16];
        if (mBasePtr == NULL)
            return OUT_OF_MEMORY;
        mData = (float *)(((size_t)mBasePtr + 15)&~15);
#endif
        return SO_NO_ERROR;
    }

    void AlignedFloatBuffer::clear()
    {
        memset(mData, 0, sizeof(float) * mFloats);
    }

    AlignedFloatBuffer::~AlignedFloatBuffer()
    {
        delete[] mBasePtr;
    }

    TinyAlignedFloatBuffer::TinyAlignedFloatBuffer()
    {
        unsigned char * basePtr = &mActualData[0];
        mData = (float *)(((size_t)basePtr + 15)&~15);
    }

    Soloud::Soloud()
    {
#ifdef FLOATING_POINT_DEBUG
        unsigned int u;
        u = _controlfp(0, 0);
        u = u & ~(_EM_INVALID | /*_EM_DENORMAL |*/ _EM_ZERODIVIDE | _EM_OVERFLOW /*| _EM_UNDERFLOW  | _EM_INEXACT*/);
        _controlfp(u, _MCW_EM);
#endif
        mInsideAudioThreadMutex = false;
        mScratchSize = 0;
        mScratchNeeded = 0;
        mSamplerate = 0;
        mBufferSize = 0;
        mFlags = 0;
        mGlobalVolume = 0;
        mPlayIndex = 0;
        mBackendData = NULL;
        mAudioThreadMutex = NULL;
        mPostClipScaler = 0;
        mBackendCleanupFunc = NULL;
        mChannels = 2;
        mStreamTime = 0;
        mLastClockedTime = 0;
        mAudioSourceID = 1;
        mBackendString = 0;
        mBackendID = 0;
        mActiveVoiceDirty = true;
        mActiveVoiceCount = 0;
        int i;
        for (i = 0; i < VOICE_COUNT; i++)
            mActiveVoice[i] = 0;
        for (i = 0; i < FILTERS_PER_STREAM; i++)
        {
            mFilter[i] = NULL;
            mFilterInstance[i] = NULL;
        }
        for (i = 0; i < 256; i++)
        {
            mFFTData[i] = 0;
            mVisualizationWaveData[i] = 0;
            mWaveData[i] = 0;
        }
        for (i = 0; i < MAX_CHANNELS; i++)
        {
            mVisualizationChannelVolume[i] = 0;
        }
        for (i = 0; i < VOICE_COUNT; i++)
        {
            mVoice[i] = 0;
        }
        mVoiceGroup = 0;
        mVoiceGroupCount = 0;

        m3dPosition[0] = 0;
        m3dPosition[1] = 0;
        m3dPosition[2] = 0;
        m3dAt[0] = 0;
        m3dAt[1] = 0;
        m3dAt[2] = -1;
        m3dUp[0] = 0;
        m3dUp[1] = 1;
        m3dUp[2] = 0;
        m3dVelocity[0] = 0;
        m3dVelocity[1] = 0;
        m3dVelocity[2] = 0;
        m3dSoundSpeed = 343.3f;
        mMaxActiveVoices = 16;
        mHighestVoice = 0;
        mResampleData = NULL;
        mResampleDataOwner = NULL;
        for (i = 0; i < 3 * MAX_CHANNELS; i++)
            m3dSpeakerPosition[i] = 0;
    }

    Soloud::~Soloud()
    {
        // let's stop all sounds before deinit, so we don't mess up our mutexes
        stopAll();
        deinit();
        unsigned int i;
        for (i = 0; i < FILTERS_PER_STREAM; i++)
        {
            delete mFilterInstance[i];
        }
        for (i = 0; i < mVoiceGroupCount; i++)
            delete[] mVoiceGroup[i];
        delete[] mVoiceGroup;
        delete[] mResampleData;
        delete[] mResampleDataOwner;
    }

    void Soloud::deinit()
    {
        SOLOUD_ASSERT(!mInsideAudioThreadMutex);
        stopAll();
        if (mBackendCleanupFunc)
            mBackendCleanupFunc(this);
        mBackendCleanupFunc = 0;
        if (mAudioThreadMutex)
            Thread::destroyMutex(mAudioThreadMutex);
        mAudioThreadMutex = NULL;
    }

    result Soloud::init(unsigned int aFlags, unsigned int aBackend, unsigned int aSamplerate, unsigned int aBufferSize, unsigned int aChannels)
    {
        if (aBackend >= BACKEND_MAX || aChannels == 3 || aChannels == 5 || aChannels == 7 || aChannels > MAX_CHANNELS)
            return INVALID_PARAMETER;

        deinit();

        mAudioThreadMutex = Thread::createMutex();

        mBackendID = 0;
        mBackendString = 0;

        int samplerate = 44100;
        int buffersize = 2048;
        int inited = 0;

        if (aSamplerate != Soloud::AUTO) samplerate = aSamplerate;
        if (aBufferSize != Soloud::AUTO) buffersize = aBufferSize;

#if defined(WITH_MUAUDIO)

        if (!inited &&
            (aBackend == Soloud::MUAUDIO ||
             aBackend == Soloud::AUTO))
        {
            if (aBufferSize == Soloud::AUTO) buffersize = 2048;

            int ret = muaudio_init(this, aFlags, samplerate, buffersize, aChannels);
            if (ret == 0)
            {
                inited = 1;
                mBackendID = Soloud::MUAUDIO;
            }

            if (ret != 0 && aBackend != Soloud::AUTO)
                return ret;
        }

#endif

#if defined(WITH_SDL1_STATIC)
        if (!inited &&
            (aBackend == Soloud::SDL1 ||
            aBackend == Soloud::AUTO))
        {
            if (aBufferSize == Soloud::AUTO) buffersize = 2048;

            int ret = sdl1static_init(this, aFlags, samplerate, buffersize, aChannels);
            if (ret == 0)
            {
                inited = 1;
                mBackendID = Soloud::SDL1;
            }

            if (ret != 0 && aBackend != Soloud::AUTO)
                return ret;
        }
#endif

#if defined(WITH_SDL2_STATIC)
        if (!inited &&
            (aBackend == Soloud::SDL2 ||
            aBackend == Soloud::AUTO))
        {
            if (aBufferSize == Soloud::AUTO) buffersize = 2048;

            int ret = sdl2static_init(this, aFlags, samplerate, buffersize, aChannels);
            if (ret == 0)
            {
                inited = 1;
                mBackendID = Soloud::SDL2;
            }

            if (ret != 0 && aBackend != Soloud::AUTO)
                return ret;
        }
#endif

#if defined(WITH_SDL2)
        if (!inited &&
            (aBackend == Soloud::SDL2 ||
            aBackend == Soloud::AUTO))
        {
            if (aBufferSize == Soloud::AUTO) buffersize = 2048;

            int ret = sdl2_init(this, aFlags, samplerate, buffersize, aChannels);
            if (ret == 0)
            {
                inited = 1;
                mBackendID = Soloud::SDL2;
            }

            if (ret != 0 && aBackend != Soloud::AUTO)
                return ret;
        }
#endif

#if defined(WITH_SDL1)
        if (!inited &&
            (aBackend == Soloud::SDL1 ||
            aBackend == Soloud::AUTO))
        {
            if (aBufferSize == Soloud::AUTO) buffersize = 2048;

            int ret = sdl1_init(this, aFlags, samplerate, buffersize, aChannels);
            if (ret == 0)
            {
                inited = 1;
                mBackendID = Soloud::SDL1;
            }

            if (ret != 0 && aBackend != Soloud::AUTO)
                return ret;
        }
#endif

#if defined(WITH_MINIAUDIO)
        if (!inited &&
            (aBackend == Soloud::MINIAUDIO ||
                aBackend == Soloud::AUTO))
        {
            if (aBufferSize == Soloud::AUTO) buffersize = 2048;

            int ret = miniaudio_init(this, aFlags, samplerate, buffersize, aChannels);
            if (ret == 0)
            {
                inited = 1;
                mBackendID = Soloud::MINIAUDIO;
            }

            if (ret != 0 && aBackend != Soloud::AUTO)
                return ret;
        }
#endif

#if defined(WITH_PORTAUDIO)
        if (!inited &&
            (aBackend == Soloud::PORTAUDIO ||
            aBackend == Soloud::AUTO))
        {
            if (aBufferSize == Soloud::AUTO) buffersize = 2048;

            int ret = portaudio_init(this, aFlags, samplerate, buffersize, aChannels);
            if (ret == 0)
            {
                inited = 1;
                mBackendID = Soloud::PORTAUDIO;
            }

            if (ret != 0 && aBackend != Soloud::AUTO)
                return ret;
        }
#endif

#if defined(WITH_XAUDIO2)
        if (!inited &&
            (aBackend == Soloud::XAUDIO2 ||
            aBackend == Soloud::AUTO))
        {
            if (aBufferSize == Soloud::AUTO) buffersize = 4096;

            int ret = xaudio2_init(this, aFlags, samplerate, buffersize, aChannels);
            if (ret == 0)
            {
                inited = 1;
                mBackendID = Soloud::XAUDIO2;
            }

            if (ret != 0 && aBackend != Soloud::AUTO)
                return ret;
        }
#endif

#if defined(WITH_WINMM)
        if (!inited &&
            (aBackend == Soloud::WINMM ||
            aBackend == Soloud::AUTO))
        {
            if (aBufferSize == Soloud::AUTO) buffersize = 4096;

            int ret = winmm_init(this, aFlags, samplerate, buffersize, aChannels);
            if (ret == 0)
            {
                inited = 1;
                mBackendID = Soloud::WINMM;
            }

            if (ret != 0 && aBackend != Soloud::AUTO)
                return ret;
        }
#endif

#if defined(WITH_WASAPI)
        if (!inited &&
            (aBackend == Soloud::WASAPI ||
            aBackend == Soloud::AUTO))
        {
            if (aBufferSize == Soloud::AUTO) buffersize = 4096;
            if (aSamplerate == Soloud::AUTO) samplerate = 48000;

            int ret = wasapi_init(this, aFlags, samplerate, buffersize, aChannels);
            if (ret == 0)
            {
                inited = 1;
                mBackendID = Soloud::WASAPI;
            }

            if (ret != 0 && aBackend != Soloud::AUTO)
                return ret;
        }
#endif

#if defined(WITH_ALSA)
        if (!inited &&
            (aBackend == Soloud::ALSA ||
            aBackend == Soloud::AUTO))
        {
            if (aBufferSize == Soloud::AUTO) buffersize = 2048;

            int ret = alsa_init(this, aFlags, samplerate, buffersize, aChannels);
            if (ret == 0)
            {
                inited = 1;
                mBackendID = Soloud::ALSA;
            }

            if (ret != 0 && aBackend != Soloud::AUTO)
                return ret;
        }
#endif

#if defined(WITH_JACK)
        if (!inited &&
            (aBackend == Soloud::JACK ||
            aBackend == Soloud::AUTO))
        {
            if (aBufferSize == Soloud::AUTO) buffersize = 2048;

            int ret = jack_init(this, aFlags, samplerate, buffersize, aChannels);
            if (ret == 0)
            {
                inited = 1;
                mBackendID = Soloud::JACK;
            }

            if (ret != 0 && aBackend != Soloud::AUTO)
                return ret;
        }
#endif

#if defined(WITH_OSS)
        if (!inited &&
            (aBackend == Soloud::OSS ||
            aBackend == Soloud::AUTO))
        {
            if (aBufferSize == Soloud::AUTO) buffersize = 2048;

            int ret = oss_init(this, aFlags, samplerate, buffersize, aChannels);
            if (ret == 0)
            {
                inited = 1;
                mBackendID = Soloud::OSS;
            }

            if (ret != 0 && aBackend != Soloud::AUTO)
                return ret;
        }
#endif

#if defined(WITH_OPENAL)
        if (!inited &&
            (aBackend == Soloud::OPENAL ||
            aBackend == Soloud::AUTO))
        {
            if (aBufferSize == Soloud::AUTO) buffersize = 4096;

            int ret = openal_init(this, aFlags, samplerate, buffersize, aChannels);
            if (ret == 0)
            {
                inited = 1;
                mBackendID = Soloud::OPENAL;
            }

            if (ret != 0 && aBackend != Soloud::AUTO)
                return ret;
        }
#endif

#if defined(WITH_COREAUDIO)
        if (!inited &&
            (aBackend == Soloud::COREAUDIO ||
            aBackend == Soloud::AUTO))
        {
            if (aBufferSize == Soloud::AUTO) buffersize = 2048;

            int ret = coreaudio_init(this, aFlags, samplerate, buffersize, aChannels);
            if (ret == 0)
            {
                inited = 1;
                mBackendID = Soloud::COREAUDIO;
            }

            if (ret != 0 && aBackend != Soloud::AUTO)
                return ret;
        }
#endif

#if defined(WITH_OPENSLES)
        if (!inited &&
            (aBackend == Soloud::OPENSLES ||
            aBackend == Soloud::AUTO))
        {
            if (aBufferSize == Soloud::AUTO) buffersize = 4096;

            int ret = opensles_init(this, aFlags, samplerate, buffersize, aChannels);
            if (ret == 0)
            {
                inited = 1;
                mBackendID = Soloud::OPENSLES;
            }

            if (ret != 0 && aBackend != Soloud::AUTO)
                return ret;
        }
#endif

#if defined(WITH_VITA_HOMEBREW)
        if (!inited &&
            (aBackend == Soloud::VITA_HOMEBREW ||
            aBackend == Soloud::AUTO))
        {
            int ret = vita_homebrew_init(this, aFlags, samplerate, buffersize, aChannels);
            if (ret == 0)
            {
                inited = 1;
                mBackendID = Soloud::VITA_HOMEBREW;
            }

            if (ret != 0 && aBackend != Soloud::AUTO)
                return ret;
        }
#endif

#if defined(WITH_NOSOUND)
        if (!inited &&
            (aBackend == Soloud::NOSOUND ||
                aBackend == Soloud::AUTO))
        {
            if (aBufferSize == Soloud::AUTO) buffersize = 2048;

            int ret = nosound_init(this, aFlags, samplerate, buffersize, aChannels);
            if (ret == 0)
            {
                inited = 1;
                mBackendID = Soloud::NOSOUND;
            }

            if (ret != 0 && aBackend != Soloud::AUTO)
                return ret;
        }
#endif


#if defined(WITH_NULL)
        if (!inited &&
            (aBackend == Soloud::NULLDRIVER))
        {
            if (aBufferSize == Soloud::AUTO) buffersize = 2048;

            int ret = null_init(this, aFlags, samplerate, buffersize, aChannels);
            if (ret == 0)
            {
                inited = 1;
                mBackendID = Soloud::NULLDRIVER;
            }

            if (ret != 0)
                return ret;
        }
#endif

        if (!inited && aBackend != Soloud::AUTO)
            return NOT_IMPLEMENTED;
        if (!inited)
            return UNKNOWN_ERROR;
        return 0;
    }

    void Soloud::postinit_internal(unsigned int aSamplerate, unsigned int aBufferSize, unsigned int aFlags, unsigned int aChannels)
    {
        mGlobalVolume = 1;
        mChannels = aChannels;
        mSamplerate = aSamplerate;
        mBufferSize = aBufferSize;
        mScratchSize = aBufferSize;
        if (mScratchSize < SAMPLE_GRANULARITY * 2) mScratchSize = SAMPLE_GRANULARITY * 2;
        if (mScratchSize < 4096) mScratchSize = 4096;
        mScratchNeeded = mScratchSize;
        mScratch.init(mScratchSize * MAX_CHANNELS);
        mOutputScratch.init(mScratchSize * MAX_CHANNELS);
        mResampleData = new AlignedFloatBuffer[mMaxActiveVoices * 2];
        mResampleDataOwner = new AudioSourceInstance*[mMaxActiveVoices];
        unsigned int i;
        for (i = 0; i < mMaxActiveVoices * 2; i++)
            mResampleData[i].init(SAMPLE_GRANULARITY * MAX_CHANNELS);
        for (i = 0; i < mMaxActiveVoices; i++)
            mResampleDataOwner[i] = NULL;
        mFlags = aFlags;
        mPostClipScaler = 0.95f;
        switch (mChannels)
        {
        case 1:
            m3dSpeakerPosition[0 * 3 + 0] = 0;
            m3dSpeakerPosition[0 * 3 + 1] = 0;
            m3dSpeakerPosition[0 * 3 + 2] = 1;
            break;
        case 2:
            m3dSpeakerPosition[0 * 3 + 0] = 2;
            m3dSpeakerPosition[0 * 3 + 1] = 0;
            m3dSpeakerPosition[0 * 3 + 2] = 1;
            m3dSpeakerPosition[1 * 3 + 0] = -2;
            m3dSpeakerPosition[1 * 3 + 1] = 0;
            m3dSpeakerPosition[1 * 3 + 2] = 1;
            break;
        case 4:
            m3dSpeakerPosition[0 * 3 + 0] = 2;
            m3dSpeakerPosition[0 * 3 + 1] = 0;
            m3dSpeakerPosition[0 * 3 + 2] = 1;
            m3dSpeakerPosition[1 * 3 + 0] = -2;
            m3dSpeakerPosition[1 * 3 + 1] = 0;
            m3dSpeakerPosition[1 * 3 + 2] = 1;
            // I suppose technically the second pair should be straight left & right,
            // but I prefer moving them a bit back to mirror the front speakers.
            m3dSpeakerPosition[2 * 3 + 0] = 2;
            m3dSpeakerPosition[2 * 3 + 1] = 0;
            m3dSpeakerPosition[2 * 3 + 2] = -1;
            m3dSpeakerPosition[3 * 3 + 0] = -2;
            m3dSpeakerPosition[3 * 3 + 1] = 0;
            m3dSpeakerPosition[3 * 3 + 2] = -1;
            break;
        case 6:
            m3dSpeakerPosition[0 * 3 + 0] = 2;
            m3dSpeakerPosition[0 * 3 + 1] = 0;
            m3dSpeakerPosition[0 * 3 + 2] = 1;
            m3dSpeakerPosition[1 * 3 + 0] = -2;
            m3dSpeakerPosition[1 * 3 + 1] = 0;
            m3dSpeakerPosition[1 * 3 + 2] = 1;

            // center and subwoofer.
            m3dSpeakerPosition[2 * 3 + 0] = 0;
            m3dSpeakerPosition[2 * 3 + 1] = 0;
            m3dSpeakerPosition[2 * 3 + 2] = 1;
            // Sub should be "mix of everything". We'll handle it as a special case and make it a null vector.
            m3dSpeakerPosition[3 * 3 + 0] = 0;
            m3dSpeakerPosition[3 * 3 + 1] = 0;
            m3dSpeakerPosition[3 * 3 + 2] = 0;

            // I suppose technically the second pair should be straight left & right,
            // but I prefer moving them a bit back to mirror the front speakers.
            m3dSpeakerPosition[4 * 3 + 0] = 2;
            m3dSpeakerPosition[4 * 3 + 1] = 0;
            m3dSpeakerPosition[4 * 3 + 2] = -1;
            m3dSpeakerPosition[5 * 3 + 0] = -2;
            m3dSpeakerPosition[5 * 3 + 1] = 0;
            m3dSpeakerPosition[5 * 3 + 2] = -1;
            break;
        case 8:
            m3dSpeakerPosition[0 * 3 + 0] = 2;
            m3dSpeakerPosition[0 * 3 + 1] = 0;
            m3dSpeakerPosition[0 * 3 + 2] = 1;
            m3dSpeakerPosition[1 * 3 + 0] = -2;
            m3dSpeakerPosition[1 * 3 + 1] = 0;
            m3dSpeakerPosition[1 * 3 + 2] = 1;

            // center and subwoofer.
            m3dSpeakerPosition[2 * 3 + 0] = 0;
            m3dSpeakerPosition[2 * 3 + 1] = 0;
            m3dSpeakerPosition[2 * 3 + 2] = 1;
            // Sub should be "mix of everything". We'll handle it as a special case and make it a null vector.
            m3dSpeakerPosition[3 * 3 + 0] = 0;
            m3dSpeakerPosition[3 * 3 + 1] = 0;
            m3dSpeakerPosition[3 * 3 + 2] = 0;

            // side
            m3dSpeakerPosition[4 * 3 + 0] = 2;
            m3dSpeakerPosition[4 * 3 + 1] = 0;
            m3dSpeakerPosition[4 * 3 + 2] = 0;
            m3dSpeakerPosition[5 * 3 + 0] = -2;
            m3dSpeakerPosition[5 * 3 + 1] = 0;
            m3dSpeakerPosition[5 * 3 + 2] = 0;

            // back
            m3dSpeakerPosition[6 * 3 + 0] = 2;
            m3dSpeakerPosition[6 * 3 + 1] = 0;
            m3dSpeakerPosition[6 * 3 + 2] = -1;
            m3dSpeakerPosition[7 * 3 + 0] = -2;
            m3dSpeakerPosition[7 * 3 + 1] = 0;
            m3dSpeakerPosition[7 * 3 + 2] = -1;
            break;
        }
    }

    const char * Soloud::getErrorString(result aErrorCode) const
    {
        switch (aErrorCode)
        {
        case SO_NO_ERROR: return "No error";
        case INVALID_PARAMETER: return "Some parameter is invalid";
        case FILE_NOT_FOUND: return "File not found";
        case FILE_LOAD_FAILED: return "File found, but could not be loaded";
        case DLL_NOT_FOUND: return "DLL not found, or wrong DLL";
        case OUT_OF_MEMORY: return "Out of memory";
        case NOT_IMPLEMENTED: return "Feature not implemented";
        /*case UNKNOWN_ERROR: return "Other error";*/
        }
        return "Other error";
    }


    float * Soloud::getWave()
    {
        int i;
        lockAudioMutex_internal();
        for (i = 0; i < 256; i++)
            mWaveData[i] = mVisualizationWaveData[i];
        unlockAudioMutex_internal();
        return mWaveData;
    }

    float Soloud::getApproximateVolume(unsigned int aChannel)
    {
        if (aChannel > mChannels)
            return 0;
        float vol = 0;
        lockAudioMutex_internal();
        vol = mVisualizationChannelVolume[aChannel];
        unlockAudioMutex_internal();
        return vol;
    }


    float * Soloud::calcFFT()
    {
        lockAudioMutex_internal();
        float temp[1024];
        int i;
        for (i = 0; i < 256; i++)
        {
            temp[i*2] = mVisualizationWaveData[i];
            temp[i*2+1] = 0;
            temp[i+512] = 0;
            temp[i+768] = 0;
        }
        unlockAudioMutex_internal();

        SoLoud::FFT::fft1024(temp);

        for (i = 0; i < 256; i++)
        {
            float real = temp[i * 2];
            float imag = temp[i * 2 + 1];
            mFFTData[i] = (float)sqrt(real*real+imag*imag);
        }

        return mFFTData;
    }

#if defined(SOLOUD_SSE_INTRINSICS)
    void Soloud::clip_internal(AlignedFloatBuffer &aBuffer, AlignedFloatBuffer &aDestBuffer, unsigned int aSamples, float aVolume0, float aVolume1)
    {
        float vd = (aVolume1 - aVolume0) / aSamples;
        float v = aVolume0;
        unsigned int i, j, c, d;
        unsigned int samplequads = (aSamples + 3) / 4; // rounded up

        // Clip
        if (mFlags & CLIP_ROUNDOFF)
        {
            float nb = -1.65f;		__m128 negbound = _mm_load_ps1(&nb);
            float pb = 1.65f;		__m128 posbound = _mm_load_ps1(&pb);
            float ls = 0.87f;		__m128 linearscale = _mm_load_ps1(&ls);
            float cs = -0.1f;		__m128 cubicscale = _mm_load_ps1(&cs);
            float nw = -0.9862875f;	__m128 negwall = _mm_load_ps1(&nw);
            float pw = 0.9862875f;	__m128 poswall = _mm_load_ps1(&pw);
            __m128 postscale = _mm_load_ps1(&mPostClipScaler);
            TinyAlignedFloatBuffer volumes;
            volumes.mData[0] = v;
            volumes.mData[1] = v + vd;
            volumes.mData[2] = v + vd + vd;
            volumes.mData[3] = v + vd + vd + vd;
            vd *= 4;
            __m128 vdelta = _mm_load_ps1(&vd);
            c = 0;
            d = 0;
            for (j = 0; j < mChannels; j++)
            {
                __m128 vol = _mm_load_ps(volumes.mData);

                for (i = 0; i < samplequads; i++)
                {
                    //float f1 = origdata[c] * v;	c++; v += vd;
                    __m128 f = _mm_load_ps(&aBuffer.mData[c]);
                    c += 4;
                    f = _mm_mul_ps(f, vol);
                    vol = _mm_add_ps(vol, vdelta);

                    //float u1 = (f1 > -1.65f);
                    __m128 u = _mm_cmpgt_ps(f, negbound);

                    //float o1 = (f1 < 1.65f);
                    __m128 o = _mm_cmplt_ps(f, posbound);

                    //f1 = (0.87f * f1 - 0.1f * f1 * f1 * f1) * u1 * o1;
                    __m128 lin = _mm_mul_ps(f, linearscale);
                    __m128 cubic = _mm_mul_ps(f, f);
                    cubic = _mm_mul_ps(cubic, f);
                    cubic = _mm_mul_ps(cubic, cubicscale);
                    f = _mm_add_ps(cubic, lin);

                    //f1 = f1 * u1 + !u1 * -0.9862875f;
                    __m128 lowmask = _mm_andnot_ps(u, negwall);
                    __m128 ilowmask = _mm_and_ps(u, f);
                    f = _mm_add_ps(lowmask, ilowmask);

                    //f1 = f1 * o1 + !o1 * 0.9862875f;
                    __m128 himask = _mm_andnot_ps(o, poswall);
                    __m128 ihimask = _mm_and_ps(o, f);
                    f = _mm_add_ps(himask, ihimask);

                    // outdata[d] = f1 * postclip; d++;
                    f = _mm_mul_ps(f, postscale);
                    _mm_store_ps(&aDestBuffer.mData[d], f);
                    d += 4;
                }
            }
        }
        else
        {
            float nb = -1.0f;	__m128 negbound = _mm_load_ps1(&nb);
            float pb = 1.0f;	__m128 posbound = _mm_load_ps1(&pb);
            __m128 postscale = _mm_load_ps1(&mPostClipScaler);
            TinyAlignedFloatBuffer volumes;
            volumes.mData[0] = v;
            volumes.mData[1] = v + vd;
            volumes.mData[2] = v + vd + vd;
            volumes.mData[3] = v + vd + vd + vd;
            vd *= 4;
            __m128 vdelta = _mm_load_ps1(&vd);
            c = 0;
            d = 0;
            for (j = 0; j < mChannels; j++)
            {
                __m128 vol = _mm_load_ps(volumes.mData);
                for (i = 0; i < samplequads; i++)
                {
                    //float f1 = aBuffer.mData[c] * v; c++; v += vd;
                    __m128 f = _mm_load_ps(&aBuffer.mData[c]);
                    c += 4;
                    f = _mm_mul_ps(f, vol);
                    vol = _mm_add_ps(vol, vdelta);

                    //f1 = (f1 <= -1) ? -1 : (f1 >= 1) ? 1 : f1;
                    f = _mm_max_ps(f, negbound);
                    f = _mm_min_ps(f, posbound);

                    //aDestBuffer.mData[d] = f1 * mPostClipScaler; d++;
                    f = _mm_mul_ps(f, postscale);
                    _mm_store_ps(&aDestBuffer.mData[d], f);
                    d += 4;
                }
            }
        }
    }
#else // fallback code
    void Soloud::clip_internal(AlignedFloatBuffer &aBuffer, AlignedFloatBuffer &aDestBuffer, unsigned int aSamples, float aVolume0, float aVolume1)
    {
        float vd = (aVolume1 - aVolume0) / aSamples;
        float v = aVolume0;
        unsigned int i, j, c, d;
        unsigned int samplequads = (aSamples + 3) / 4; // rounded up
        // Clip
        if (mFlags & CLIP_ROUNDOFF)
        {
            c = 0;
            d = 0;
            for (j = 0; j < mChannels; j++)
            {
                v = aVolume0;
                for (i = 0; i < samplequads; i++)
                {
                    float f1 = aBuffer.mData[c] * v; c++; v += vd;
                    float f2 = aBuffer.mData[c] * v; c++; v += vd;
                    float f3 = aBuffer.mData[c] * v; c++; v += vd;
                    float f4 = aBuffer.mData[c] * v; c++; v += vd;

                    f1 = (f1 <= -1.65f) ? -0.9862875f : (f1 >= 1.65f) ? 0.9862875f : (0.87f * f1 - 0.1f * f1 * f1 * f1);
                    f2 = (f2 <= -1.65f) ? -0.9862875f : (f2 >= 1.65f) ? 0.9862875f : (0.87f * f2 - 0.1f * f2 * f2 * f2);
                    f3 = (f3 <= -1.65f) ? -0.9862875f : (f3 >= 1.65f) ? 0.9862875f : (0.87f * f3 - 0.1f * f3 * f3 * f3);
                    f4 = (f4 <= -1.65f) ? -0.9862875f : (f4 >= 1.65f) ? 0.9862875f : (0.87f * f4 - 0.1f * f4 * f4 * f4);

                    aDestBuffer.mData[d] = f1 * mPostClipScaler; d++;
                    aDestBuffer.mData[d] = f2 * mPostClipScaler; d++;
                    aDestBuffer.mData[d] = f3 * mPostClipScaler; d++;
                    aDestBuffer.mData[d] = f4 * mPostClipScaler; d++;
                }
            }
        }
        else
        {
            c = 0;
            d = 0;
            for (j = 0; j < mChannels; j++)
            {
                v = aVolume0;
                for (i = 0; i < samplequads; i++)
                {
                    float f1 = aBuffer.mData[c] * v; c++; v += vd;
                    float f2 = aBuffer.mData[c] * v; c++; v += vd;
                    float f3 = aBuffer.mData[c] * v; c++; v += vd;
                    float f4 = aBuffer.mData[c] * v; c++; v += vd;

                    f1 = (f1 <= -1) ? -1 : (f1 >= 1) ? 1 : f1;
                    f2 = (f2 <= -1) ? -1 : (f2 >= 1) ? 1 : f2;
                    f3 = (f3 <= -1) ? -1 : (f3 >= 1) ? 1 : f3;
                    f4 = (f4 <= -1) ? -1 : (f4 >= 1) ? 1 : f4;

                    aDestBuffer.mData[d] = f1 * mPostClipScaler; d++;
                    aDestBuffer.mData[d] = f2 * mPostClipScaler; d++;
                    aDestBuffer.mData[d] = f3 * mPostClipScaler; d++;
                    aDestBuffer.mData[d] = f4 * mPostClipScaler; d++;
            }
        }
    }
}
#endif

#define FIXPOINT_FRAC_BITS 20
#define FIXPOINT_FRAC_MUL (1 << FIXPOINT_FRAC_BITS)
#define FIXPOINT_FRAC_MASK ((1 << FIXPOINT_FRAC_BITS) - 1)

    void resample(float *aSrc,
                  float *aSrc1,
                  float *aDst,
                  int aSrcOffset,
                  int aDstSampleCount,
                  float /*aSrcSamplerate*/,
                  float /*aDstSamplerate*/,
                  int aStepFixed)
    {
#if 0

#elif defined(RESAMPLER_LINEAR)
        int i;
        int pos = aSrcOffset;

        for (i = 0; i < aDstSampleCount; i++, pos += aStepFixed)
        {
            int p = pos >> FIXPOINT_FRAC_BITS;
            int f = pos & FIXPOINT_FRAC_MASK;
#ifdef _DEBUG
            if (p >= SAMPLE_GRANULARITY || p < 0)
            {
                // This should never actually happen
                p = SAMPLE_GRANULARITY - 1;
            }
#endif
            float s1 = aSrc1[SAMPLE_GRANULARITY - 1];
            float s2 = aSrc[p];
            if (p != 0)
            {
                s1 = aSrc[p-1];
            }
            aDst[i] = s1 + (s2 - s1) * f * (1 / (float)FIXPOINT_FRAC_MUL);
        }
#else // Point sample
        int i;
        int pos = aSrcOffset;

        for (i = 0; i < aDstSampleCount; i++, pos += aStepFixed)
        {
            int p = pos >> FIXPOINT_FRAC_BITS;
            aDst[i] = aSrc[p];
        }
#endif
    }

    void panAndExpand(AudioSourceInstance *aVoice, float *aBuffer, unsigned int aSamplesToRead, unsigned int aBufferSize, float *aScratch, unsigned int aChannels)
    {
        float pan[MAX_CHANNELS]; // current speaker volume
        float pand[MAX_CHANNELS]; // destination speaker volume
        float pani[MAX_CHANNELS]; // speaker volume increment per sample
        unsigned int j, k;
        for (k = 0; k < aChannels; k++)
        {
            pan[k] = aVoice->mCurrentChannelVolume[k];
            pand[k] = aVoice->mChannelVolume[k] * aVoice->mOverallVolume;
            pani[k] = (pand[k] - pan[k]) / aSamplesToRead; // TODO: this is a bit inconsistent.. but it's a hack to begin with
        }

        int ofs = 0;
        switch (aChannels)
        {
        case 1: // Target is mono. Sum everything. (1->1, 2->1, 4->1, 6->1, 8->1)
            for (j = 0, ofs = 0; j < aVoice->mChannels; j++, ofs += aBufferSize)
            {
                pan[0] = aVoice->mCurrentChannelVolume[0];
                for (k = 0; k < aSamplesToRead; k++)
                {
                    pan[0] += pani[0];
                    aBuffer[k] += aScratch[ofs + k] * pan[0];
                }
            }
            break;
        case 2:
            switch (aVoice->mChannels)
            {
            case 8: // 8->2, just sum lefties and righties, add a bit of center and sub?
                for (j = 0; j < aSamplesToRead; j++)
                {
                    pan[0] += pani[0];
                    pan[1] += pani[1];
                    float s1 = aScratch[j];
                    float s2 = aScratch[aBufferSize + j];
                    float s3 = aScratch[aBufferSize * 2 + j];
                    float s4 = aScratch[aBufferSize * 3 + j];
                    float s5 = aScratch[aBufferSize * 4 + j];
                    float s6 = aScratch[aBufferSize * 5 + j];
                    float s7 = aScratch[aBufferSize * 6 + j];
                    float s8 = aScratch[aBufferSize * 7 + j];
                    aBuffer[j + 0]           += 0.2f * (s1 + s3 + s4 + s5 + s7) * pan[0];
                    aBuffer[j + aBufferSize] += 0.2f * (s2 + s3 + s4 + s6 + s8) * pan[1];
                }
                break;
            case 6: // 6->2, just sum lefties and righties, add a bit of center and sub?
                for (j = 0; j < aSamplesToRead; j++)
                {
                    pan[0] += pani[0];
                    pan[1] += pani[1];
                    float s1 = aScratch[j];
                    float s2 = aScratch[aBufferSize + j];
                    float s3 = aScratch[aBufferSize * 2 + j];
                    float s4 = aScratch[aBufferSize * 3 + j];
                    float s5 = aScratch[aBufferSize * 4 + j];
                    float s6 = aScratch[aBufferSize * 5 + j];
                    aBuffer[j + 0] += 0.3f * (s1 + s3 + s4 + s5) * pan[0];
                    aBuffer[j + aBufferSize] += 0.3f * (s2 + s3 + s4 + s6) * pan[1];
                }
                break;
            case 4: // 4->2, just sum lefties and righties
                for (j = 0; j < aSamplesToRead; j++)
                {
                    pan[0] += pani[0];
                    pan[1] += pani[1];
                    float s1 = aScratch[j];
                    float s2 = aScratch[aBufferSize + j];
                    float s3 = aScratch[aBufferSize * 2 + j];
                    float s4 = aScratch[aBufferSize * 3 + j];
                    aBuffer[j + 0] += 0.5f * (s1 + s3) * pan[0];
                    aBuffer[j + aBufferSize] += 0.5f * (s2 + s4) * pan[1];
                }
                break;
            case 2: // 2->2
                for (j = 0; j < aSamplesToRead; j++)
                {
                    pan[0] += pani[0];
                    pan[1] += pani[1];
                    float s1 = aScratch[j];
                    float s2 = aScratch[aBufferSize + j];
                    aBuffer[j + 0] += s1 * pan[0];
                    aBuffer[j + aBufferSize] += s2 * pan[1];
                }
                break;
            case 1: // 1->2
                for (j = 0; j < aSamplesToRead; j++)
                {
                    pan[0] += pani[0];
                    pan[1] += pani[1];
                    float s = aScratch[j];
                    aBuffer[j + 0] += s * pan[0];
                    aBuffer[j + aBufferSize] += s * pan[1];
                }
                break;
            }
            break;
        case 4:
            switch (aVoice->mChannels)
            {
            case 8: // 8->4, add a bit of center, sub?
                for (j = 0; j < aSamplesToRead; j++)
                {
                    pan[0] += pani[0];
                    pan[1] += pani[1];
                    pan[2] += pani[2];
                    pan[3] += pani[3];
                    float s1 = aScratch[j];
                    float s2 = aScratch[aBufferSize + j];
                    float s3 = aScratch[aBufferSize * 2 + j];
                    float s4 = aScratch[aBufferSize * 3 + j];
                    float s5 = aScratch[aBufferSize * 4 + j];
                    float s6 = aScratch[aBufferSize * 5 + j];
                    float s7 = aScratch[aBufferSize * 6 + j];
                    float s8 = aScratch[aBufferSize * 7 + j];
                    float c = (s3 + s4) * 0.7f;
                    aBuffer[j + 0]               += s1 * pan[0] + c;
                    aBuffer[j + aBufferSize]     += s2 * pan[1] + c;
                    aBuffer[j + aBufferSize * 2] += 0.5f * (s5 + s7) * pan[2];
                    aBuffer[j + aBufferSize * 3] += 0.5f * (s6 + s8) * pan[3];
                }
                break;
            case 6: // 6->4, add a bit of center, sub?
                for (j = 0; j < aSamplesToRead; j++)
                {
                    pan[0] += pani[0];
                    pan[1] += pani[1];
                    pan[2] += pani[2];
                    pan[3] += pani[3];
                    float s1 = aScratch[j];
                    float s2 = aScratch[aBufferSize + j];
                    float s3 = aScratch[aBufferSize * 2 + j];
                    float s4 = aScratch[aBufferSize * 3 + j];
                    float s5 = aScratch[aBufferSize * 4 + j];
                    float s6 = aScratch[aBufferSize * 5 + j];
                    float c = (s3 + s4) * 0.7f;
                    aBuffer[j + 0] += s1 * pan[0] + c;
                    aBuffer[j + aBufferSize] += s2 * pan[1] + c;
                    aBuffer[j + aBufferSize * 2] += s5 * pan[2];
                    aBuffer[j + aBufferSize * 3] += s6 * pan[3];
                }
                break;
            case 4: // 4->4
                for (j = 0; j < aSamplesToRead; j++)
                {
                    pan[0] += pani[0];
                    pan[1] += pani[1];
                    pan[2] += pani[2];
                    pan[3] += pani[3];
                    float s1 = aScratch[j];
                    float s2 = aScratch[aBufferSize + j];
                    float s3 = aScratch[aBufferSize * 2 + j];
                    float s4 = aScratch[aBufferSize * 3 + j];
                    aBuffer[j + 0] += s1 * pan[0];
                    aBuffer[j + aBufferSize] += s2 * pan[1];
                    aBuffer[j + aBufferSize * 2] += s3 * pan[2];
                    aBuffer[j + aBufferSize * 3] += s4 * pan[3];
                }
                break;
            case 2: // 2->4
                for (j = 0; j < aSamplesToRead; j++)
                {
                    pan[0] += pani[0];
                    pan[1] += pani[1];
                    pan[2] += pani[2];
                    pan[3] += pani[3];
                    float s1 = aScratch[j];
                    float s2 = aScratch[aBufferSize + j];
                    aBuffer[j + 0] += s1 * pan[0];
                    aBuffer[j + aBufferSize] += s2 * pan[1];
                    aBuffer[j + aBufferSize * 2] += s1 * pan[2];
                    aBuffer[j + aBufferSize * 3] += s2 * pan[3];
                }
                break;
            case 1: // 1->4
                for (j = 0; j < aSamplesToRead; j++)
                {
                    pan[0] += pani[0];
                    pan[1] += pani[1];
                    pan[2] += pani[2];
                    pan[3] += pani[3];
                    float s = aScratch[j];
                    aBuffer[j + 0] += s * pan[0];
                    aBuffer[j + aBufferSize] += s * pan[1];
                    aBuffer[j + aBufferSize * 2] += s * pan[2];
                    aBuffer[j + aBufferSize * 3] += s * pan[3];
                }
                break;
            }
            break;
        case 6:
            switch (aVoice->mChannels)
            {
            case 8: // 8->6
                for (j = 0; j < aSamplesToRead; j++)
                {
                    pan[0] += pani[0];
                    pan[1] += pani[1];
                    pan[2] += pani[2];
                    pan[3] += pani[3];
                    pan[4] += pani[4];
                    pan[5] += pani[5];
                    float s1 = aScratch[j];
                    float s2 = aScratch[aBufferSize + j];
                    float s3 = aScratch[aBufferSize * 2 + j];
                    float s4 = aScratch[aBufferSize * 3 + j];
                    float s5 = aScratch[aBufferSize * 4 + j];
                    float s6 = aScratch[aBufferSize * 5 + j];
                    float s7 = aScratch[aBufferSize * 6 + j];
                    float s8 = aScratch[aBufferSize * 7 + j];
                    aBuffer[j + 0] += s1 * pan[0];
                    aBuffer[j + aBufferSize] += s2 * pan[1];
                    aBuffer[j + aBufferSize * 2] += s3 * pan[2];
                    aBuffer[j + aBufferSize * 3] += s4 * pan[3];
                    aBuffer[j + aBufferSize * 4] += 0.5f * (s5 + s7) * pan[4];
                    aBuffer[j + aBufferSize * 5] += 0.5f * (s6 + s8) * pan[5];
                }
                break;
            case 6: // 6->6
                for (j = 0; j < aSamplesToRead; j++)
                {
                    pan[0] += pani[0];
                    pan[1] += pani[1];
                    pan[2] += pani[2];
                    pan[3] += pani[3];
                    pan[4] += pani[4];
                    pan[5] += pani[5];
                    float s1 = aScratch[j];
                    float s2 = aScratch[aBufferSize + j];
                    float s3 = aScratch[aBufferSize * 2 + j];
                    float s4 = aScratch[aBufferSize * 3 + j];
                    float s5 = aScratch[aBufferSize * 4 + j];
                    float s6 = aScratch[aBufferSize * 5 + j];
                    aBuffer[j + 0] += s1 * pan[0];
                    aBuffer[j + aBufferSize] += s2 * pan[1];
                    aBuffer[j + aBufferSize * 2] += s3 * pan[2];
                    aBuffer[j + aBufferSize * 3] += s4 * pan[3];
                    aBuffer[j + aBufferSize * 4] += s5 * pan[4];
                    aBuffer[j + aBufferSize * 5] += s6 * pan[5];
                }
                break;
            case 4: // 4->6
                for (j = 0; j < aSamplesToRead; j++)
                {
                    pan[0] += pani[0];
                    pan[1] += pani[1];
                    pan[2] += pani[2];
                    pan[3] += pani[3];
                    pan[4] += pani[4];
                    pan[5] += pani[5];
                    float s1 = aScratch[j];
                    float s2 = aScratch[aBufferSize + j];
                    float s3 = aScratch[aBufferSize * 2 + j];
                    float s4 = aScratch[aBufferSize * 3 + j];
                    aBuffer[j + 0] += s1 * pan[0];
                    aBuffer[j + aBufferSize] += s2 * pan[1];
                    aBuffer[j + aBufferSize * 2] += 0.5f * (s1 + s2) * pan[2];
                    aBuffer[j + aBufferSize * 3] += 0.25f * (s1 + s2 + s3 + s4) * pan[3];
                    aBuffer[j + aBufferSize * 4] += s3 * pan[4];
                    aBuffer[j + aBufferSize * 5] += s4 * pan[5];
                }
                break;
            case 2: // 2->6
                for (j = 0; j < aSamplesToRead; j++)
                {
                    pan[0] += pani[0];
                    pan[1] += pani[1];
                    pan[2] += pani[2];
                    pan[3] += pani[3];
                    pan[4] += pani[4];
                    pan[5] += pani[5];
                    float s1 = aScratch[j];
                    float s2 = aScratch[aBufferSize + j];
                    aBuffer[j + 0] += s1 * pan[0];
                    aBuffer[j + aBufferSize] += s2 * pan[1];
                    aBuffer[j + aBufferSize * 2] += 0.5f * (s1 + s2) * pan[2];
                    aBuffer[j + aBufferSize * 3] += 0.5f * (s1 + s2) * pan[3];
                    aBuffer[j + aBufferSize * 4] += s1 * pan[4];
                    aBuffer[j + aBufferSize * 5] += s2 * pan[5];
                }
                break;
            case 1: // 1->6
                for (j = 0; j < aSamplesToRead; j++)
                {
                    pan[0] += pani[0];
                    pan[1] += pani[1];
                    pan[2] += pani[2];
                    pan[3] += pani[3];
                    pan[4] += pani[4];
                    pan[5] += pani[5];
                    float s = aScratch[j];
                    aBuffer[j + 0] += s * pan[0];
                    aBuffer[j + aBufferSize] += s * pan[1];
                    aBuffer[j + aBufferSize * 2] += s * pan[2];
                    aBuffer[j + aBufferSize * 3] += s * pan[3];
                    aBuffer[j + aBufferSize * 4] += s * pan[4];
                    aBuffer[j + aBufferSize * 5] += s * pan[5];
                }
                break;
            }
            break;
        case 8:
            switch (aVoice->mChannels)
            {
            case 8: // 8->8
                for (j = 0; j < aSamplesToRead; j++)
                {
                    pan[0] += pani[0];
                    pan[1] += pani[1];
                    pan[2] += pani[2];
                    pan[3] += pani[3];
                    pan[4] += pani[4];
                    pan[5] += pani[5];
                    pan[6] += pani[6];
                    pan[7] += pani[7];
                    float s1 = aScratch[j];
                    float s2 = aScratch[aBufferSize + j];
                    float s3 = aScratch[aBufferSize * 2 + j];
                    float s4 = aScratch[aBufferSize * 3 + j];
                    float s5 = aScratch[aBufferSize * 4 + j];
                    float s6 = aScratch[aBufferSize * 5 + j];
                    float s7 = aScratch[aBufferSize * 6 + j];
                    float s8 = aScratch[aBufferSize * 7 + j];
                    aBuffer[j + 0] += s1 * pan[0];
                    aBuffer[j + aBufferSize] += s2 * pan[1];
                    aBuffer[j + aBufferSize * 2] += s3 * pan[2];
                    aBuffer[j + aBufferSize * 3] += s4 * pan[3];
                    aBuffer[j + aBufferSize * 4] += s5 * pan[4];
                    aBuffer[j + aBufferSize * 5] += s6 * pan[5];
                    aBuffer[j + aBufferSize * 6] += s7 * pan[6];
                    aBuffer[j + aBufferSize * 7] += s8 * pan[7];
                }
                break;
            case 6: // 6->8
                for (j = 0; j < aSamplesToRead; j++)
                {
                    pan[0] += pani[0];
                    pan[1] += pani[1];
                    pan[2] += pani[2];
                    pan[3] += pani[3];
                    pan[4] += pani[4];
                    pan[5] += pani[5];
                    pan[6] += pani[6];
                    pan[7] += pani[7];
                    float s1 = aScratch[j];
                    float s2 = aScratch[aBufferSize + j];
                    float s3 = aScratch[aBufferSize * 2 + j];
                    float s4 = aScratch[aBufferSize * 3 + j];
                    float s5 = aScratch[aBufferSize * 4 + j];
                    float s6 = aScratch[aBufferSize * 5 + j];
                    aBuffer[j + 0] += s1 * pan[0];
                    aBuffer[j + aBufferSize] += s2 * pan[1];
                    aBuffer[j + aBufferSize * 2] += s3 * pan[2];
                    aBuffer[j + aBufferSize * 3] += s4 * pan[3];
                    aBuffer[j + aBufferSize * 4] += 0.5f * (s5 + s1) * pan[4];
                    aBuffer[j + aBufferSize * 5] += 0.5f * (s6 + s2) * pan[5];
                    aBuffer[j + aBufferSize * 6] += s5 * pan[6];
                    aBuffer[j + aBufferSize * 7] += s6 * pan[7];
                }
                break;
            case 4: // 4->8
                for (j = 0; j < aSamplesToRead; j++)
                {
                    pan[0] += pani[0];
                    pan[1] += pani[1];
                    pan[2] += pani[2];
                    pan[3] += pani[3];
                    pan[4] += pani[4];
                    pan[5] += pani[5];
                    pan[6] += pani[6];
                    pan[7] += pani[7];
                    float s1 = aScratch[j];
                    float s2 = aScratch[aBufferSize + j];
                    float s3 = aScratch[aBufferSize * 2 + j];
                    float s4 = aScratch[aBufferSize * 3 + j];
                    aBuffer[j + 0] += s1 * pan[0];
                    aBuffer[j + aBufferSize] += s2 * pan[1];
                    aBuffer[j + aBufferSize * 2] += 0.5f * (s1 + s2) * pan[2];
                    aBuffer[j + aBufferSize * 3] += 0.25f * (s1 + s2 + s3 + s4) * pan[3];
                    aBuffer[j + aBufferSize * 4] += 0.5f * (s1 + s3) * pan[4];
                    aBuffer[j + aBufferSize * 5] += 0.5f * (s2 + s4) * pan[5];
                    aBuffer[j + aBufferSize * 6] += s3 * pan[4];
                    aBuffer[j + aBufferSize * 7] += s4 * pan[5];
                }
                break;
            case 2: // 2->8
                for (j = 0; j < aSamplesToRead; j++)
                {
                    pan[0] += pani[0];
                    pan[1] += pani[1];
                    pan[2] += pani[2];
                    pan[3] += pani[3];
                    pan[4] += pani[4];
                    pan[5] += pani[5];
                    pan[6] += pani[6];
                    pan[7] += pani[7];
                    float s1 = aScratch[j];
                    float s2 = aScratch[aBufferSize + j];
                    aBuffer[j + 0] += s1 * pan[0];
                    aBuffer[j + aBufferSize] += s2 * pan[1];
                    aBuffer[j + aBufferSize * 2] += 0.5f * (s1 + s2) * pan[2];
                    aBuffer[j + aBufferSize * 3] += 0.5f * (s1 + s2) * pan[3];
                    aBuffer[j + aBufferSize * 4] += s1 * pan[4];
                    aBuffer[j + aBufferSize * 5] += s2 * pan[5];
                    aBuffer[j + aBufferSize * 6] += s1 * pan[6];
                    aBuffer[j + aBufferSize * 7] += s2 * pan[7];
                }
                break;
            case 1: // 1->8
                for (j = 0; j < aSamplesToRead; j++)
                {
                    pan[0] += pani[0];
                    pan[1] += pani[1];
                    pan[2] += pani[2];
                    pan[3] += pani[3];
                    pan[4] += pani[4];
                    pan[5] += pani[5];
                    pan[6] += pani[6];
                    pan[7] += pani[7];
                    float s = aScratch[j];
                    aBuffer[j + 0] += s * pan[0];
                    aBuffer[j + aBufferSize] += s * pan[1];
                    aBuffer[j + aBufferSize * 2] += s * pan[2];
                    aBuffer[j + aBufferSize * 3] += s * pan[3];
                    aBuffer[j + aBufferSize * 4] += s * pan[4];
                    aBuffer[j + aBufferSize * 5] += s * pan[5];
                    aBuffer[j + aBufferSize * 6] += s * pan[6];
                    aBuffer[j + aBufferSize * 7] += s * pan[7];
                }
                break;
            }
            break;
        }

        for (k = 0; k < aChannels; k++)
            aVoice->mCurrentChannelVolume[k] = pand[k];
    }

    void Soloud::mixBus_internal(float *aBuffer, unsigned int aSamplesToRead, unsigned int aBufferSize, float *aScratch, unsigned int aBus, float aSamplerate, unsigned int aChannels)
    {
        unsigned int i, j;
        // Clear accumulation buffer
        for (i = 0; i < aSamplesToRead; i++)
        {
            for (j = 0; j < aChannels; j++)
            {
                aBuffer[i + j * aBufferSize] = 0;
            }
        }

        // Accumulate sound sources
        for (i = 0; i < mActiveVoiceCount; i++)
        {
            AudioSourceInstance *voice = mVoice[mActiveVoice[i]];
            if (voice &&
                voice->mBusHandle == aBus &&
                !(voice->mFlags & AudioSourceInstance::PAUSED) &&
                !(voice->mFlags & AudioSourceInstance::INAUDIBLE))
            {
                float step = voice->mSamplerate / aSamplerate;
                // avoid step overflow
                if (step > (1 << (32 - FIXPOINT_FRAC_BITS)))
                    step = 0;
                unsigned int step_fixed = (int)floor(step * FIXPOINT_FRAC_MUL);
                unsigned int outofs = 0;

                if (voice->mDelaySamples)
                {
                    if (voice->mDelaySamples > aSamplesToRead)
                    {
                        outofs = aSamplesToRead;
                        voice->mDelaySamples -= aSamplesToRead;
                    }
                    else
                    {
                        outofs = voice->mDelaySamples;
                        voice->mDelaySamples = 0;
                    }

                    // Clear scratch where we're skipping
                    unsigned int k;
                    for (k = 0; k < voice->mChannels; k++)
                    {
                        memset(aScratch + k * aBufferSize, 0, sizeof(float) * outofs);
                    }
                }

                while (step_fixed != 0 && outofs < aSamplesToRead)
                {
                    if (voice->mLeftoverSamples == 0)
                    {
                        // Swap resample buffers (ping-pong)
                        AlignedFloatBuffer * t = voice->mResampleData[0];
                        voice->mResampleData[0] = voice->mResampleData[1];
                        voice->mResampleData[1] = t;

                        // Get a block of source data

                        int readcount = 0;
                        if (!voice->hasEnded() || voice->mFlags & AudioSourceInstance::LOOPING)
                        {
                            readcount = voice->getAudio(voice->mResampleData[0]->mData, SAMPLE_GRANULARITY, SAMPLE_GRANULARITY);
                            if (readcount < SAMPLE_GRANULARITY)
                            {
                                if (voice->mFlags & AudioSourceInstance::LOOPING)
                                {
                                    while (readcount < SAMPLE_GRANULARITY && voice->seek(voice->mLoopPoint, mScratch.mData, mScratchSize) == SO_NO_ERROR)
                                    {
                                        voice->mLoopCount++;
                                        int inc = voice->getAudio(voice->mResampleData[0]->mData + readcount, SAMPLE_GRANULARITY - readcount, SAMPLE_GRANULARITY);
                                        readcount += inc;
                                        if (inc == 0) break;
                                    }
                                }
                            }
                        }

                        // Clear remaining of the resample data if the full scratch wasn't used
                        if (readcount < SAMPLE_GRANULARITY)
                        {
                            unsigned int k;
                            for (k = 0; k < voice->mChannels; k++)
                                memset(voice->mResampleData[0]->mData + readcount + SAMPLE_GRANULARITY * k, 0, sizeof(float) * (SAMPLE_GRANULARITY - readcount));
                        }

                        // If we go past zero, crop to zero (a bit of a kludge)
                        if (voice->mSrcOffset < SAMPLE_GRANULARITY * FIXPOINT_FRAC_MUL)
                        {
                            voice->mSrcOffset = 0;
                        }
                        else
                        {
                            // We have new block of data, move pointer backwards
                            voice->mSrcOffset -= SAMPLE_GRANULARITY * FIXPOINT_FRAC_MUL;
                        }


                        // Run the per-stream filters to get our source data

                        for (j = 0; j < FILTERS_PER_STREAM; j++)
                        {
                            if (voice->mFilter[j])
                            {
                                voice->mFilter[j]->filter(
                                    voice->mResampleData[0]->mData,
                                    SAMPLE_GRANULARITY,
                                    voice->mChannels,
                                    voice->mSamplerate,
                                    mStreamTime);
                            }
                        }
                    }
                    else
                    {
                        voice->mLeftoverSamples = 0;
                    }

                    // Figure out how many samples we can generate from this source data.
                    // The value may be zero.

                    unsigned int writesamples = 0;

                    if (voice->mSrcOffset < SAMPLE_GRANULARITY * FIXPOINT_FRAC_MUL)
                    {
                        writesamples = ((SAMPLE_GRANULARITY * FIXPOINT_FRAC_MUL) - voice->mSrcOffset) / step_fixed + 1;

                        // avoid reading past the current buffer..
                        if (((writesamples * step_fixed + voice->mSrcOffset) >> FIXPOINT_FRAC_BITS) >= SAMPLE_GRANULARITY)
                            writesamples--;
                    }


                    // If this is too much for our output buffer, don't write that many:
                    if (writesamples + outofs > aSamplesToRead)
                    {
                        voice->mLeftoverSamples = (writesamples + outofs) - aSamplesToRead;
                        writesamples = aSamplesToRead - outofs;
                    }

                    // Call resampler to generate the samples, once per channel
                    if (writesamples)
                    {
                        for (j = 0; j < voice->mChannels; j++)
                        {
                            resample(voice->mResampleData[0]->mData + SAMPLE_GRANULARITY * j,
                                voice->mResampleData[1]->mData + SAMPLE_GRANULARITY * j,
                                     aScratch + aBufferSize * j + outofs,
                                     voice->mSrcOffset,
                                     writesamples,
                                     voice->mSamplerate,
                                     aSamplerate,
                                     step_fixed);
                        }
                    }

                    // Keep track of how many samples we've written so far
                    outofs += writesamples;

                    // Move source pointer onwards (writesamples may be zero)
                    voice->mSrcOffset += writesamples * step_fixed;
                }

                // Handle panning and channel expansion (and/or shrinking)
                panAndExpand(voice, aBuffer, aSamplesToRead, aBufferSize, aScratch, aChannels);

                // clear voice if the sound is over
                if (!(voice->mFlags & AudioSourceInstance::LOOPING) && voice->hasEnded())
                {
                    stopVoice_internal(mActiveVoice[i]);
                }
            }
            else
                if (voice &&
                    voice->mBusHandle == aBus &&
                    !(voice->mFlags & AudioSourceInstance::PAUSED) &&
                    (voice->mFlags & AudioSourceInstance::INAUDIBLE) &&
                    (voice->mFlags & AudioSourceInstance::INAUDIBLE_TICK))
            {
                // Inaudible but needs ticking. Do minimal work (keep counters up to date and ask audiosource for data)
                float step = voice->mSamplerate / aSamplerate;
                int step_fixed = (int)floor(step * FIXPOINT_FRAC_MUL);
                unsigned int outofs = 0;

                if (voice->mDelaySamples)
                {
                    if (voice->mDelaySamples > aSamplesToRead)
                    {
                        outofs = aSamplesToRead;
                        voice->mDelaySamples -= aSamplesToRead;
                    }
                    else
                    {
                        outofs = voice->mDelaySamples;
                        voice->mDelaySamples = 0;
                    }
                }

                while (step_fixed != 0 && outofs < aSamplesToRead)
                {
                    if (voice->mLeftoverSamples == 0)
                    {
                        // Swap resample buffers (ping-pong)
                        AlignedFloatBuffer * t = voice->mResampleData[0];
                        voice->mResampleData[0] = voice->mResampleData[1];
                        voice->mResampleData[1] = t;

                        // Get a block of source data

                        int readcount = 0;
                        if (!voice->hasEnded() || voice->mFlags & AudioSourceInstance::LOOPING)
                        {
                            readcount = voice->getAudio(voice->mResampleData[0]->mData, SAMPLE_GRANULARITY, SAMPLE_GRANULARITY);
                            if (readcount < SAMPLE_GRANULARITY)
                            {
                                if (voice->mFlags & AudioSourceInstance::LOOPING)
                                {
                                    while (readcount < SAMPLE_GRANULARITY && voice->seek(voice->mLoopPoint, mScratch.mData, mScratchSize) == SO_NO_ERROR)
                                    {
                                        voice->mLoopCount++;
                                        readcount += voice->getAudio(voice->mResampleData[0]->mData + readcount, SAMPLE_GRANULARITY - readcount, SAMPLE_GRANULARITY);
                                    }
                                }
                            }
                        }

                        // If we go past zero, crop to zero (a bit of a kludge)
                        if (voice->mSrcOffset < SAMPLE_GRANULARITY * FIXPOINT_FRAC_MUL)
                        {
                            voice->mSrcOffset = 0;
                        }
                        else
                        {
                            // We have new block of data, move pointer backwards
                            voice->mSrcOffset -= SAMPLE_GRANULARITY * FIXPOINT_FRAC_MUL;
                        }

                        // Skip filters
                    }
                    else
                    {
                        voice->mLeftoverSamples = 0;
                    }

                    // Figure out how many samples we can generate from this source data.
                    // The value may be zero.

                    unsigned int writesamples = 0;

                    if (voice->mSrcOffset < SAMPLE_GRANULARITY * FIXPOINT_FRAC_MUL)
                    {
                        writesamples = ((SAMPLE_GRANULARITY * FIXPOINT_FRAC_MUL) - voice->mSrcOffset) / step_fixed + 1;

                        // avoid reading past the current buffer..
                        if (((writesamples * step_fixed + voice->mSrcOffset) >> FIXPOINT_FRAC_BITS) >= SAMPLE_GRANULARITY)
                            writesamples--;
                    }


                    // If this is too much for our output buffer, don't write that many:
                    if (writesamples + outofs > aSamplesToRead)
                    {
                        voice->mLeftoverSamples = (writesamples + outofs) - aSamplesToRead;
                        writesamples = aSamplesToRead - outofs;
                    }

                    // Skip resampler

                    // Keep track of how many samples we've written so far
                    outofs += writesamples;

                    // Move source pointer onwards (writesamples may be zero)
                    voice->mSrcOffset += writesamples * step_fixed;
                }

                // clear voice if the sound is over
                if (!(voice->mFlags & AudioSourceInstance::LOOPING) && voice->hasEnded())
                {
                    stopVoice_internal(mActiveVoice[i]);
                }
            }
        }
    }

    void Soloud::mapResampleBuffers_internal()
    {
        SOLOUD_ASSERT(mMaxActiveVoices < 256);
        char live[256];
        memset(live, 0, mMaxActiveVoices);
        unsigned int i, j;
        for (i = 0; i < mMaxActiveVoices; i++)
        {
            for (j = 0; j < mMaxActiveVoices; j++)
            {
                if (mResampleDataOwner[i] && mResampleDataOwner[i] == mVoice[mActiveVoice[j]])
                {
                    live[i] |= 1; // Live channel
                    live[j] |= 2; // Live voice
                }
            }
        }

        for (i = 0; i < mMaxActiveVoices; i++)
        {
            if (!(live[i] & 1) && mResampleDataOwner[i]) // For all dead channels with owners..
            {
                mResampleDataOwner[i]->mResampleData[0] = 0;
                mResampleDataOwner[i]->mResampleData[1] = 0;
                mResampleDataOwner[i] = 0;
            }
        }

        int latestfree = 0;
        for (i = 0; i < mActiveVoiceCount; i++)
        {
            if (!(live[i] & 2) && mVoice[mActiveVoice[i]]) // For all live voices with no channel..
            {
                int found = -1;
                for (j = latestfree; found == -1 && j < mMaxActiveVoices; j++)
                {
                    if (mResampleDataOwner[j] == 0)
                    {
                        found = j;
                    }
                }
                SOLOUD_ASSERT(found != -1);
                mResampleDataOwner[found] = mVoice[mActiveVoice[i]];
                mResampleDataOwner[found]->mResampleData[0] = &mResampleData[found * 2 + 0];
                mResampleDataOwner[found]->mResampleData[1] = &mResampleData[found * 2 + 1];
                mResampleDataOwner[found]->mResampleData[0]->clear();
                mResampleDataOwner[found]->mResampleData[1]->clear();
                latestfree = found + 1;
            }
        }
    }

    void Soloud::calcActiveVoices_internal()
    {
        // TODO: consider whether we need to re-evaluate the active voices all the time.
        // It is a must when new voices are started, but otherwise we could get away
        // with postponing it sometimes..

        mActiveVoiceDirty = false;

        // Populate
        unsigned int i, candidates, mustlive;
        candidates = 0;
        mustlive = 0;
        for (i = 0; i < mHighestVoice; i++)
        {
            if (mVoice[i] && (!(mVoice[i]->mFlags & (AudioSourceInstance::INAUDIBLE | AudioSourceInstance::PAUSED)) || (mVoice[i]->mFlags & AudioSourceInstance::INAUDIBLE_TICK)))
            {
                mActiveVoice[candidates] = i;
                candidates++;
                if (mVoice[i]->mFlags & AudioSourceInstance::INAUDIBLE_TICK)
                {
                    mActiveVoice[candidates - 1] = mActiveVoice[mustlive];
                    mActiveVoice[mustlive] = i;
                    mustlive++;
                }
            }
        }

        // Check for early out
        if (candidates <= mMaxActiveVoices)
        {
            // everything is audible, early out
            mActiveVoiceCount = candidates;
            mapResampleBuffers_internal();
            return;
        }

        mActiveVoiceCount = mMaxActiveVoices;

        if (mustlive >= mMaxActiveVoices)
        {
            // Oopsie. Well, nothing to sort, since the "must live" voices already
            // ate all our active voice slots.
            // This is a potentially an error situation, but we have no way to report
            // error from here. And asserting could be bad, too.
            return;
        }

        // If we get this far, there's nothing to it: we'll have to sort the voices to find the most audible.

        // Iterative partial quicksort:
        int left = 0, stack[24], pos = 0, right;
        int len = candidates - mustlive;
        unsigned int *data = mActiveVoice + mustlive;
        int k = mActiveVoiceCount;
        for (;;)
        {
            for (; left + 1 < len; len++)
            {
                if (pos == 24) len = stack[pos = 0];
                int pivot = data[left];
                float pivotvol = mVoice[pivot]->mOverallVolume;
                stack[pos++] = len;
                for (right = left - 1;;)
                {
                    do
                    {
                        right++;
                    }
                    while (mVoice[data[right]]->mOverallVolume > pivotvol);
                    do
                    {
                        len--;
                    }
                    while (pivotvol > mVoice[data[len]]->mOverallVolume);
                    if (right >= len) break;
                    int temp = data[right];
                    data[right] = data[len];
                    data[len] = temp;
                }
            }
            if (pos == 0) break;
            if (left >= k) break;
            left = len;
            len = stack[--pos];
        }
        // TODO: should the rest of the voices be flagged INAUDIBLE?
        mapResampleBuffers_internal();
    }

    void Soloud::mix_internal(unsigned int aSamples)
    {
#ifdef FLOATING_POINT_DEBUG
        // This needs to be done in the audio thread as well..
        static int done = 0;
        if (!done)
        {
            unsigned int u;
            u = _controlfp(0, 0);
            u = u & ~(_EM_INVALID | /*_EM_DENORMAL |*/ _EM_ZERODIVIDE | _EM_OVERFLOW /*| _EM_UNDERFLOW  | _EM_INEXACT*/);
            _controlfp(u, _MCW_EM);
            done = 1;
        }
#endif

#ifdef _MCW_DN
        {
            static bool once = false;
            if (!once)
            {
                once = true;
                if (!(mFlags & NO_FPU_REGISTER_CHANGE))
                {
                    _controlfp(_DN_FLUSH, _MCW_DN);
                }
            }
        }
#endif

#ifdef SOLOUD_SSE_INTRINSICS
        {
            static bool once = false;
            if (!once)
            {
                once = true;
                // Set denorm clear to zero (CTZ) and denorms are zero (DAZ) flags on.
                // This causes all math to consider really tiny values as zero, which
                // helps performance. I'd rather use constants from the sse headers,
                // but for some reason the DAZ value is not defined there(!)
                if (!(mFlags & NO_FPU_REGISTER_CHANGE))
                {
                    _mm_setcsr(_mm_getcsr() | 0x8040);
                }
            }
        }
#endif

        float buffertime = aSamples / (float)mSamplerate;
        float globalVolume[2];
        mStreamTime += buffertime;
        mLastClockedTime = 0;

        globalVolume[0] = mGlobalVolume;
        if (mGlobalVolumeFader.mActive)
        {
            mGlobalVolume = mGlobalVolumeFader.get(mStreamTime);
        }
        globalVolume[1] = mGlobalVolume;

        lockAudioMutex_internal();

        // Process faders. May change scratch size.
        int i;
        for (i = 0; i < (signed)mHighestVoice; i++)
        {
            if (mVoice[i] && !(mVoice[i]->mFlags & AudioSourceInstance::PAUSED))
            {
                float volume[2];
                (void)volume;

                mVoice[i]->mActiveFader = 0;

                if (mGlobalVolumeFader.mActive > 0)
                {
                    mVoice[i]->mActiveFader = 1;
                }

                mVoice[i]->mStreamTime += buffertime;
                mVoice[i]->mStreamPosition += (double)buffertime * (double)mVoice[i]->mOverallRelativePlaySpeed;

                // TODO: this is actually unstable, because mStreamTime depends on the relative
                // play speed.
                if (mVoice[i]->mRelativePlaySpeedFader.mActive > 0)
                {
                    float speed = mVoice[i]->mRelativePlaySpeedFader.get(mVoice[i]->mStreamTime);
                    setVoiceRelativePlaySpeed_internal(i, speed);
                }

                volume[0] = mVoice[i]->mOverallVolume;
                if (mVoice[i]->mVolumeFader.mActive > 0)
                {
                    mVoice[i]->mSetVolume = mVoice[i]->mVolumeFader.get(mVoice[i]->mStreamTime);
                    mVoice[i]->mActiveFader = 1;
                    updateVoiceVolume_internal(i);
                    mActiveVoiceDirty = true;
                }
                volume[1] = mVoice[i]->mOverallVolume;

                if (mVoice[i]->mPanFader.mActive > 0)
                {
                    float pan = mVoice[i]->mPanFader.get(mVoice[i]->mStreamTime);
                    setVoicePan_internal(i, pan);
                    mVoice[i]->mActiveFader = 1;
                }

                if (mVoice[i]->mPauseScheduler.mActive)
                {
                    mVoice[i]->mPauseScheduler.get(mVoice[i]->mStreamTime);
                    if (mVoice[i]->mPauseScheduler.mActive == -1)
                    {
                        mVoice[i]->mPauseScheduler.mActive = 0;
                        setVoicePause_internal(i, 1);
                    }
                }

                if (mVoice[i]->mStopScheduler.mActive)
                {
                    mVoice[i]->mStopScheduler.get(mVoice[i]->mStreamTime);
                    if (mVoice[i]->mStopScheduler.mActive == -1)
                    {
                        mVoice[i]->mStopScheduler.mActive = 0;
                        stopVoice_internal(i);
                    }
                }
            }
        }

        if (mActiveVoiceDirty)
            calcActiveVoices_internal();

        // Resize scratch if needed.
        if (mScratchSize < mScratchNeeded)
        {
            mScratchSize = mScratchNeeded;
            mScratch.init(mScratchSize * MAX_CHANNELS);
        }

        mixBus_internal(mOutputScratch.mData, aSamples, aSamples, mScratch.mData, 0, (float)mSamplerate, mChannels);

        for (i = 0; i < FILTERS_PER_STREAM; i++)
        {
            if (mFilterInstance[i])
            {
                mFilterInstance[i]->filter(mOutputScratch.mData, aSamples, mChannels, (float)mSamplerate, mStreamTime);
            }
        }

        unlockAudioMutex_internal();

        clip_internal(mOutputScratch, mScratch, aSamples, globalVolume[0], globalVolume[1]);

        if (mFlags & ENABLE_VISUALIZATION)
        {
            for (i = 0; i < MAX_CHANNELS; i++)
            {
                mVisualizationChannelVolume[i] = 0;
            }
            if (aSamples > 255)
            {
                for (i = 0; i < 256; i++)
                {
                    int j;
                    mVisualizationWaveData[i] = 0;
                    for (j = 0; j < (signed)mChannels; j++)
                    {
                        float sample = mScratch.mData[i + j * aSamples];
                        float absvol = (float)fabs(sample);
                        if (mVisualizationChannelVolume[j] < absvol)
                            mVisualizationChannelVolume[j] = absvol;
                        mVisualizationWaveData[i] += sample;
                    }
                }
            }
            else
            {
                // Very unlikely failsafe branch
                for (i = 0; i < 256; i++)
                {
                    int j;
                    mVisualizationWaveData[i] = 0;
                    for (j = 0; j < (signed)mChannels; j++)
                    {
                        float sample = mScratch.mData[(i % aSamples) + j * aSamples];
                        float absvol = (float)fabs(sample);
                        if (mVisualizationChannelVolume[j] < absvol)
                            mVisualizationChannelVolume[j] = absvol;
                        mVisualizationWaveData[i] += sample;
                    }
                }
            }
        }
    }

    void Soloud::mix(float *aBuffer, unsigned int aSamples)
    {
        mix_internal(aSamples);
        interlace_samples_float(mScratch.mData, aBuffer, aSamples, mChannels);
    }

    void Soloud::mixSigned16(short *aBuffer, unsigned int aSamples)
    {
        mix_internal(aSamples);
        interlace_samples_s16(mScratch.mData, aBuffer, aSamples, mChannels);
    }

    void deinterlace_samples_float(const float *aSourceBuffer, float *aDestBuffer, unsigned int aSamples, unsigned int aChannels)
    {
        // 121212 -> 111222
        unsigned int i, j, c;
        c = 0;
        for (j = 0; j < aChannels; j++)
        {
            for (i = j; i < aSamples; i += aChannels)
            {
                aDestBuffer[c] = aSourceBuffer[i + j];
                c++;
            }
        }
    }

    void interlace_samples_float(const float *aSourceBuffer, float *aDestBuffer, unsigned int aSamples, unsigned int aChannels)
    {
        // 111222 -> 121212
        unsigned int i, j, c;
        c = 0;
        for (j = 0; j < aChannels; j++)
        {
            for (i = j; i < aSamples * aChannels; i += aChannels)
            {
                aDestBuffer[i] = aSourceBuffer[c];
                c++;
            }
        }
    }

#if 0 // defined(SOLOUD_SSE_INTRINSICS) && defined(_M_IX86)
    /* There are several issues with this code.
       - Doesn't work in x64, because __m64 doesn't exist there.
       - Assumes aDestBuffer is well aligned; this is not guaranteed
       - (didn't call _mm_clear, but that was an easy fix)
       Leaving it here for future reference for now if a more
       working solution is found.
     */
    void interlace_samples_s16_mono(const float *aSourceBuffer, short *aDestBuffer, unsigned int aSamples)
    {
        const __m128 scale = _mm_set1_ps(0x7fff);
        const unsigned int blockSize = 16;
        const unsigned int numBlocks = aSamples / blockSize;
        SOLOUD_ASSERT((aSamples % blockSize) == 0);

        for (unsigned int i = 0; i < numBlocks; ++i)
        {
            // A0 A1 A2 A3 B0 B1 B2 B3 -> A0 A1 A2 A3 B0 B1 B2 B3 (float -> short)
            __m128 s0 = _mm_load_ps(&aSourceBuffer[16 * i + 0]);
            __m128 s1 = _mm_load_ps(&aSourceBuffer[16 * i + 4]);
            __m128 s2 = _mm_load_ps(&aSourceBuffer[16 * i + 8]);
            __m128 s3 = _mm_load_ps(&aSourceBuffer[16 * i + 12]);

            __m64 d0 = _mm_cvtps_pi16(_mm_mul_ps(s0, scale));
            __m64 d1 = _mm_cvtps_pi16(_mm_mul_ps(s1, scale));
            __m64 d2 = _mm_cvtps_pi16(_mm_mul_ps(s2, scale));
            __m64 d3 = _mm_cvtps_pi16(_mm_mul_ps(s3, scale));

            __m128i u = _mm_setr_epi64(d0, d1);
            __m128i v = _mm_setr_epi64(d2, d3);

            _mm_store_si128(reinterpret_cast<__m128i *>(&aDestBuffer[16 * i + 0]), u);
            _mm_store_si128(reinterpret_cast<__m128i *>(&aDestBuffer[16 * i + 8]), v);
        }

        _mm_empty();

        const unsigned int offset = blockSize * numBlocks;
        for (unsigned int i = offset; i < aSamples; ++i)
        {
            aDestBuffer[i] = static_cast<short>(aSourceBuffer[i] * 0x7fff);
        }
    }

    void interlace_samples_s16_stereo(const float *aSourceBuffer, short *aDestBuffer, unsigned int aSamples)
    {
        const __m128 scale = _mm_set1_ps(0x7fff);
        const unsigned int blockSize = 8u;
        const unsigned int numBlocks = aSamples / blockSize;
        SOLOUD_ASSERT((aSamples % blockSize) == 0);

        for (unsigned int i = 0; i < numBlocks; ++i)
        {
            // A0 A1 A2 A3 B0 B1 B2 B3 -> A0 B0 A1 B1 A2 B2 A3 B3
            __m128 a0 = _mm_load_ps(&aSourceBuffer[8 * i + 0]);
            __m128 a1 = _mm_load_ps(&aSourceBuffer[8 * i + 4]);
            __m128 b0 = _mm_load_ps(&aSourceBuffer[aSamples + 8 * i + 0]);
            __m128 b1 = _mm_load_ps(&aSourceBuffer[aSamples + 8 * i + 4]);

            __m128 x = _mm_unpacklo_ps(a0, b0);
            __m128 y = _mm_unpackhi_ps(a0, b0);
            __m128 z = _mm_unpacklo_ps(a1, b1);
            __m128 w = _mm_unpackhi_ps(a1, b1);

            __m64 d0 = _mm_cvtps_pi16(_mm_mul_ps(x, scale));
            __m64 d1 = _mm_cvtps_pi16(_mm_mul_ps(y, scale));
            __m64 d2 = _mm_cvtps_pi16(_mm_mul_ps(z, scale));
            __m64 d3 = _mm_cvtps_pi16(_mm_mul_ps(w, scale));

            __m128i u = _mm_setr_epi64(d0, d1);
            __m128i v = _mm_setr_epi64(d2, d3);

            _mm_store_si128(reinterpret_cast<__m128i *>(&aDestBuffer[16 * i + 0]), u);
            _mm_store_si128(reinterpret_cast<__m128i *>(&aDestBuffer[16 * i + 8]), v);
        }

        _mm_empty();

        const unsigned int offset = blockSize * numBlocks;
        for (unsigned int i = offset; i < aSamples; ++i)
        {
            aDestBuffer[2 * i + 0] = static_cast<short>(aSourceBuffer[i] * 0x7fff);
            aDestBuffer[2 * i + 1] = static_cast<short>(aSourceBuffer[i + aSamples] * 0x7fff);
        }
    }
#endif

    void interlace_samples_s16(const float *aSourceBuffer, short *aDestBuffer, unsigned int aSamples, unsigned int aChannels)
    {
#if 0 // defined(SOLOUD_SSE_INTRINSICS) && defined(_M_IX86)
        switch (aChannels)
        {
        case 1:
            interlace_samples_s16_mono(aSourceBuffer, aDestBuffer, aSamples);
            return;

        case 2:
            interlace_samples_s16_stereo(aSourceBuffer, aDestBuffer, aSamples);
            return;

        default:
            break;
        }
#endif

        // 111222 -> 121212
        unsigned int i, j, c;
        c = 0;
        for (j = 0; j < aChannels; j++)
        {
            for (i = j; i < aSamples * aChannels; i += aChannels)
            {
                aDestBuffer[i] = (short)(aSourceBuffer[c] * 0x7fff);
                c++;
            }
        }
    }

    void Soloud::lockAudioMutex_internal()
    {
        if (mAudioThreadMutex)
        {
            Thread::lockMutex(mAudioThreadMutex);
        }
        SOLOUD_ASSERT(!mInsideAudioThreadMutex);
        mInsideAudioThreadMutex = true;
    }

    void Soloud::unlockAudioMutex_internal()
    {
        SOLOUD_ASSERT(mInsideAudioThreadMutex);
        mInsideAudioThreadMutex = false;
        if (mAudioThreadMutex)
        {
            Thread::unlockMutex(mAudioThreadMutex);
        }
    }

};
