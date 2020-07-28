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

#include "soloud.h"
#include "soloud_echofilter.h"

namespace SoLoud
{
	EchoFilterInstance::EchoFilterInstance(EchoFilter *aParent)
	{
		mBuffer = 0;
		mBufferLength = 0;
		mBufferMaxLength = 0;
		mOffset = 0;
		initParams(4);
		mParam[EchoFilter::DELAY] = aParent->mDelay;
		mParam[EchoFilter::DECAY] = aParent->mDecay;
		mParam[EchoFilter::FILTER] = aParent->mFilter;
	}

	void EchoFilterInstance::filter(float *aBuffer, unsigned int aSamples, unsigned int aChannels, float aSamplerate, double aTime)
	{
		updateParams(aTime);
		if (mBuffer == 0)
		{
			// We only know channels and sample rate at this point.. not really optimal
			mBufferMaxLength = (int)ceil(mParam[EchoFilter::DELAY] * aSamplerate);
			mBuffer = new float[mBufferMaxLength * aChannels];
			unsigned int i;
			for (i = 0; i < mBufferMaxLength * aChannels; i++)
			{
				mBuffer[i] = 0;
			}
		}

		mBufferLength = (int)ceil(mParam[EchoFilter::DELAY] * aSamplerate);
		if (mBufferLength > mBufferMaxLength)
			mBufferLength = mBufferMaxLength;

		unsigned int i, j;
		int prevofs = (mOffset + mBufferLength - 1) % mBufferLength;
		for (i = 0; i < aSamples; i++)
		{
			for (j = 0; j < aChannels; j++)
			{
				int chofs = j * mBufferLength;
				int bchofs = j * aSamples;
				
				mBuffer[mOffset + chofs] = mParam[EchoFilter::FILTER] * mBuffer[prevofs + chofs] + (1 - mParam[EchoFilter::FILTER]) * mBuffer[mOffset + chofs];
				
				float n = aBuffer[i + bchofs] + mBuffer[mOffset + chofs] * mParam[EchoFilter::DECAY];
				mBuffer[mOffset + chofs] = n;

				aBuffer[i + bchofs] += (n - aBuffer[i + bchofs]) * mParam[EchoFilter::WET];
			}
			prevofs = mOffset;
			mOffset = (mOffset + 1) % mBufferLength;
		}
	}

	EchoFilterInstance::~EchoFilterInstance()
	{
		delete[] mBuffer;
	}

	EchoFilter::EchoFilter()
	{
		mDelay = 0.3f;
		mDecay = 0.7f;
		mFilter = 0.0f;
	}

	result EchoFilter::setParams(float aDelay, float aDecay, float aFilter)
	{
		if (aDelay <= 0 || aDecay <= 0 || aFilter < 0 || aFilter >= 1.0f)
			return INVALID_PARAMETER;

		mDecay = aDecay;
		mDelay = aDelay;
		mFilter = aFilter;
		
		return 0;
	}

	int EchoFilter::getParamCount()
	{
		return 4;
	}

	const char* EchoFilter::getParamName(unsigned int aParamIndex)
	{
		if (aParamIndex > 3)
			return 0;
		const char *names[4] = {
			"Wet",
			"Delay",
			"Decay",
			"Filter"
		};
		return names[aParamIndex];
	}

	unsigned int EchoFilter::getParamType(unsigned int aParamIndex)
	{
		return FLOAT_PARAM;
	}

	float EchoFilter::getParamMax(unsigned int aParamIndex)
	{
		switch (aParamIndex)
		{
		case DELAY: return mDelay;
		}
		return 1;
	}

	float EchoFilter::getParamMin(unsigned int aParamIndex)
	{
		return 0;
	}

	FilterInstance *EchoFilter::createInstance()
	{
		return new EchoFilterInstance(this);
	}
}
