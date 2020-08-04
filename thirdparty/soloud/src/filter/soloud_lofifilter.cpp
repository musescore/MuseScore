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

#include <math.h>
#include <string.h>
#include "soloud.h"
#include "soloud_lofifilter.h"

namespace SoLoud
{

	LofiFilterInstance::LofiFilterInstance(LofiFilter *aParent)
	{
		mParent = aParent;
		initParams(3);
		mParam[SAMPLERATE] = aParent->mSampleRate;
		mParam[BITDEPTH] = aParent->mBitdepth;
		mChannelData[0].mSample = 0;
		mChannelData[0].mSamplesToSkip = 0;
		mChannelData[1].mSample = 0;
		mChannelData[1].mSamplesToSkip = 0;
	}

	void LofiFilterInstance::filterChannel(float *aBuffer, unsigned int aSamples, float aSamplerate, double aTime, unsigned int aChannel, unsigned int /*aChannels*/)
	{
		updateParams(aTime);

		unsigned int i;
		for (i = 0; i < aSamples; i++)
		{
			if (mChannelData[aChannel].mSamplesToSkip <= 0)
			{
				mChannelData[aChannel].mSamplesToSkip += (aSamplerate / mParam[SAMPLERATE]) - 1;
				float q = (float)pow(2, mParam[BITDEPTH]);
				mChannelData[aChannel].mSample = (float)floor(q * aBuffer[i]) / q;
			}
			else
			{
				mChannelData[aChannel].mSamplesToSkip--;
			}
			aBuffer[i] += (mChannelData[aChannel].mSample - aBuffer[i]) * mParam[WET];
		}

	}

	LofiFilterInstance::~LofiFilterInstance()
	{
	}

	LofiFilter::LofiFilter()
	{
		setParams(4000, 3);
	}

	result LofiFilter::setParams(float aSampleRate, float aBitdepth)
	{
		if (aSampleRate <= 0 || aBitdepth <= 0)
			return INVALID_PARAMETER;

		mSampleRate = aSampleRate;
		mBitdepth = aBitdepth;
		return 0;
	}

	LofiFilter::~LofiFilter()
	{
	}

	int LofiFilter::getParamCount()
	{
		return 3;
	}

	const char* LofiFilter::getParamName(unsigned int aParamIndex)
	{
		if (aParamIndex > 2)
			return 0;
		const char *names[3] = {
			"Wet",
			"Samplerate",
			"Bitdepth"
		};
		return names[aParamIndex];
	}

	unsigned int LofiFilter::getParamType(unsigned int aParamIndex)
	{
		return FLOAT_PARAM;
	}

	float LofiFilter::getParamMax(unsigned int aParamIndex)
	{
		switch (aParamIndex)
		{
		case SAMPLERATE: return 22000;
		case BITDEPTH: return 16;
		}
		return 1;
	}

	float LofiFilter::getParamMin(unsigned int aParamIndex)
	{
		switch (aParamIndex)
		{
		case SAMPLERATE: return 100;
		case BITDEPTH: return 0.5;
		}
		return 0;
	}

	LofiFilterInstance *LofiFilter::createInstance()
	{
		return new LofiFilterInstance(this);
	}
}
