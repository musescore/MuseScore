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

// Core operations related to faders (not including filters)

namespace SoLoud
{
	void Soloud::schedulePause(handle aVoiceHandle, time aTime)
	{
		if (aTime <= 0)
		{
			setPause(aVoiceHandle, 1);
			return;
		}
		FOR_ALL_VOICES_PRE
		mVoice[ch]->mPauseScheduler.set(1, 0, aTime, mVoice[ch]->mStreamTime);
		FOR_ALL_VOICES_POST
	}

	void Soloud::scheduleStop(handle aVoiceHandle, time aTime)
	{
		if (aTime <= 0)
		{
			stop(aVoiceHandle);
			return;
		}
		FOR_ALL_VOICES_PRE
		mVoice[ch]->mStopScheduler.set(1, 0, aTime, mVoice[ch]->mStreamTime);
		FOR_ALL_VOICES_POST
	}

	void Soloud::fadeVolume(handle aVoiceHandle, float aTo, time aTime)
	{
		float from = getVolume(aVoiceHandle);
		if (aTime <= 0 || aTo == from)
		{
			setVolume(aVoiceHandle, aTo);
			return;
		}

		FOR_ALL_VOICES_PRE
		mVoice[ch]->mVolumeFader.set(from, aTo, aTime, mVoice[ch]->mStreamTime);
		FOR_ALL_VOICES_POST
	}

	void Soloud::fadePan(handle aVoiceHandle, float aTo, time aTime)
	{
		float from = getPan(aVoiceHandle);
		if (aTime <= 0 || aTo == from)
		{
			setPan(aVoiceHandle, aTo);
			return;
		}

		FOR_ALL_VOICES_PRE
		mVoice[ch]->mPanFader.set(from, aTo, aTime, mVoice[ch]->mStreamTime);
		FOR_ALL_VOICES_POST
	}

	void Soloud::fadeRelativePlaySpeed(handle aVoiceHandle, float aTo, time aTime)
	{
		float from = getRelativePlaySpeed(aVoiceHandle);
		if (aTime <= 0 || aTo == from)
		{
			setRelativePlaySpeed(aVoiceHandle, aTo);
			return;
		}
		FOR_ALL_VOICES_PRE
		mVoice[ch]->mRelativePlaySpeedFader.set(from, aTo, aTime, mVoice[ch]->mStreamTime);
		FOR_ALL_VOICES_POST
	}

	void Soloud::fadeGlobalVolume(float aTo, time aTime)
	{
		float from = getGlobalVolume();
		if (aTime <= 0 || aTo == from)
		{
			setGlobalVolume(aTo);
			return;
		}
		mGlobalVolumeFader.set(from, aTo, aTime, mStreamTime);
	}


	void Soloud::oscillateVolume(handle aVoiceHandle, float aFrom, float aTo, time aTime)
	{
		if (aTime <= 0 || aTo == aFrom)
		{
			setVolume(aVoiceHandle, aTo);
			return;
		}

		FOR_ALL_VOICES_PRE
		mVoice[ch]->mVolumeFader.setLFO(aFrom, aTo, aTime, mVoice[ch]->mStreamTime);
		FOR_ALL_VOICES_POST
	}

	void Soloud::oscillatePan(handle aVoiceHandle, float aFrom, float aTo, time aTime)
	{
		if (aTime <= 0 || aTo == aFrom)
		{
			setPan(aVoiceHandle, aTo);
			return;
		}

		FOR_ALL_VOICES_PRE
		mVoice[ch]->mPanFader.setLFO(aFrom, aTo, aTime, mVoice[ch]->mStreamTime);
		FOR_ALL_VOICES_POST
	}

	void Soloud::oscillateRelativePlaySpeed(handle aVoiceHandle, float aFrom, float aTo, time aTime)
	{
		if (aTime <= 0 || aTo == aFrom)
		{
			setRelativePlaySpeed(aVoiceHandle, aTo);
			return;
		}
		
		FOR_ALL_VOICES_PRE
		mVoice[ch]->mRelativePlaySpeedFader.setLFO(aFrom, aTo, aTime, mVoice[ch]->mStreamTime);
		FOR_ALL_VOICES_POST
	}

	void Soloud::oscillateGlobalVolume(float aFrom, float aTo, time aTime)
	{
		if (aTime <= 0 || aTo == aFrom)
		{
			setGlobalVolume(aTo);
			return;
		}
		mGlobalVolumeFader.setLFO(aFrom, aTo, aTime, mStreamTime);
	}
}
