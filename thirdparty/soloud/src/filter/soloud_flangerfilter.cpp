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
#include <string.h>
#include "soloud.h"
#include "soloud_flangerfilter.h"

namespace SoLoud
{
	FlangerFilterInstance::FlangerFilterInstance(FlangerFilter *aParent)
	{
		mParent = aParent;
		mBuffer = 0;
		mBufferLength = 0;
		mOffset = 0;
		mIndex = 0;
		initParams(3);
		mParam[FlangerFilter::WET] = 1;
		mParam[FlangerFilter::FREQ] = mParent->mFreq;
		mParam[FlangerFilter::DELAY] = mParent->mDelay;
	}

	void FlangerFilterInstance::filter(float *aBuffer, unsigned int aSamples, unsigned int aChannels, float aSamplerate, double aTime)
	{
		updateParams(aTime);

		if (mBufferLength < mParam[FlangerFilter::DELAY] * aSamplerate)
		{
			delete[] mBuffer;
			mBufferLength = (int)ceil(mParam[FlangerFilter::DELAY] * aSamplerate);
			mBuffer = new float[mBufferLength * aChannels];
			if (mBuffer == NULL)
			{
				mBufferLength = 0;
				return;
			}
			memset(mBuffer, 0, sizeof(float) * mBufferLength * aChannels);
		}

		unsigned int i, j;
		int maxsamples = (int)ceil(mParam[FlangerFilter::DELAY] * aSamplerate);
		double inc = mParam[FlangerFilter::FREQ] * M_PI * 2 / aSamplerate;
		for (i = 0; i < aChannels; i++)
		{
			int mbofs = i * mBufferLength;
			int abofs = i * aSamples;
			for (j = 0; j < aSamples; j++, abofs++)
			{
				int delay = (int)floor(maxsamples * (1 + cos(mIndex))) / 2;
				mIndex += inc;
				mBuffer[mbofs + mOffset % mBufferLength] = aBuffer[abofs];
				float n = 0.5f * (aBuffer[abofs] + mBuffer[mbofs + (mBufferLength - delay + mOffset) % mBufferLength]);
				mOffset++;
				aBuffer[abofs] += (n - aBuffer[abofs]) * mParam[FlangerFilter::WET];
			}
			mOffset -= aSamples;
		}
		mOffset += aSamples;
		mOffset %= mBufferLength;
	}

	FlangerFilterInstance::~FlangerFilterInstance()
	{
		delete[] mBuffer;
	}

	FlangerFilter::FlangerFilter()
	{
		mDelay = 0.005f;
		mFreq = 10;
	}

	result FlangerFilter::setParams(float aDelay, float aFreq)
	{
		if (aDelay <= 0 || aFreq <= 0)
			return INVALID_PARAMETER;

		mDelay = aDelay;
		mFreq = aFreq;
		
		return 0;
	}

	int FlangerFilter::getParamCount()
	{
		return 3;
	}

	const char* FlangerFilter::getParamName(unsigned int aParamIndex)
	{
		if (aParamIndex > 2)
			return 0;
		const char *names[3] = {
			"Wet",
			"Delay",
			"Freq"
		};
		return names[aParamIndex];
	}

	unsigned int FlangerFilter::getParamType(unsigned int aParamIndex)
	{
		return FLOAT_PARAM;
	}

	float FlangerFilter::getParamMax(unsigned int aParamIndex)
	{
		switch (aParamIndex)
		{
		case DELAY: return 0.1f;
		case FREQ: return 100;
		}
		return 1;
	}

	float FlangerFilter::getParamMin(unsigned int aParamIndex)
	{
		if (aParamIndex == WET)
			return 0;
		return 0.001f;
	}

	FilterInstance *FlangerFilter::createInstance()
	{
		return new FlangerFilterInstance(this);
	}
}
