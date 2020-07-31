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

// Core operations related to filters

namespace SoLoud
{
	void Soloud::setGlobalFilter(unsigned int aFilterId, Filter *aFilter)
	{
		if (aFilterId >= FILTERS_PER_STREAM)
			return;

		lockAudioMutex_internal();
		delete mFilterInstance[aFilterId];
		mFilterInstance[aFilterId] = 0;
		
		mFilter[aFilterId] = aFilter;
		if (aFilter)
		{
			mFilterInstance[aFilterId] = mFilter[aFilterId]->createInstance();
		}
		unlockAudioMutex_internal();
	}

	float Soloud::getFilterParameter(handle aVoiceHandle, unsigned int aFilterId, unsigned int aAttributeId)
	{
		float ret = INVALID_PARAMETER;
		if (aFilterId >= FILTERS_PER_STREAM)
			return ret;

		if (aVoiceHandle == 0)
		{
			lockAudioMutex_internal();
			if (mFilterInstance[aFilterId])
			{
				ret = mFilterInstance[aFilterId]->getFilterParameter(aAttributeId);
			}
			unlockAudioMutex_internal();
			return ret;
		}

		int ch = getVoiceFromHandle_internal(aVoiceHandle);
		if (ch == -1) 
		{
			return ret;
		}
		lockAudioMutex_internal();
		if (mVoice[ch] &&
			mVoice[ch]->mFilter[aFilterId])
		{
			ret = mVoice[ch]->mFilter[aFilterId]->getFilterParameter(aAttributeId);
		}
		unlockAudioMutex_internal();
		
		return ret;
	}

	void Soloud::setFilterParameter(handle aVoiceHandle, unsigned int aFilterId, unsigned int aAttributeId, float aValue)
	{
		if (aFilterId >= FILTERS_PER_STREAM)
			return;

		if (aVoiceHandle == 0)
		{
			lockAudioMutex_internal();
			if (mFilterInstance[aFilterId])
			{
				mFilterInstance[aFilterId]->setFilterParameter(aAttributeId, aValue);
			}
			unlockAudioMutex_internal();
			return;
		}

		FOR_ALL_VOICES_PRE
		if (mVoice[ch] &&
			mVoice[ch]->mFilter[aFilterId])
		{
			mVoice[ch]->mFilter[aFilterId]->setFilterParameter(aAttributeId, aValue);
		}
		FOR_ALL_VOICES_POST
	}

	void Soloud::fadeFilterParameter(handle aVoiceHandle, unsigned int aFilterId, unsigned int aAttributeId, float aTo, double aTime)
	{
		if (aFilterId >= FILTERS_PER_STREAM)
			return;

		if (aVoiceHandle == 0)
		{
			lockAudioMutex_internal();
			if (mFilterInstance[aFilterId])
			{
				mFilterInstance[aFilterId]->fadeFilterParameter(aAttributeId, aTo, aTime, mStreamTime);
			}
			unlockAudioMutex_internal();
			return;
		}

		FOR_ALL_VOICES_PRE
		if (mVoice[ch] &&
			mVoice[ch]->mFilter[aFilterId])
		{
			mVoice[ch]->mFilter[aFilterId]->fadeFilterParameter(aAttributeId, aTo, aTime, mStreamTime);
		}
		FOR_ALL_VOICES_POST
	}

	void Soloud::oscillateFilterParameter(handle aVoiceHandle, unsigned int aFilterId, unsigned int aAttributeId, float aFrom, float aTo, double aTime)
	{
		if (aFilterId >= FILTERS_PER_STREAM)
			return;

		if (aVoiceHandle == 0)
		{
			lockAudioMutex_internal();
			if (mFilterInstance[aFilterId])
			{
				mFilterInstance[aFilterId]->oscillateFilterParameter(aAttributeId, aFrom, aTo, aTime, mStreamTime);
			}
			unlockAudioMutex_internal();
			return;
		}

		FOR_ALL_VOICES_PRE
		if (mVoice[ch] &&
			mVoice[ch]->mFilter[aFilterId])
		{
			mVoice[ch]->mFilter[aFilterId]->oscillateFilterParameter(aAttributeId, aFrom, aTo, aTime, mStreamTime);
		}
		FOR_ALL_VOICES_POST
	}

}
