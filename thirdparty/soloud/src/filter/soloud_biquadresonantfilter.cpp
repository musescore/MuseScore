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

--

Based on "Using the Biquad Resonant Filter", 
Phil Burk, Game Programming Gems 3, p. 606
*/

#include <math.h>
#include <string.h>
#include "soloud.h"
#include "soloud_biquadresonantfilter.h"

namespace SoLoud
{
	void BiquadResonantFilterInstance::calcBQRParams()
	{
		mDirty = 0;

		float omega = (float)((2.0f * M_PI * mParam[FREQUENCY]) / mSamplerate);
		float sin_omega = (float)sin(omega);
		float cos_omega = (float)cos(omega);
		float alpha = sin_omega / (2.0f * mParam[RESONANCE]);
		float scalar = 1.0f / (1.0f + alpha);

		switch ((int)(mParam[TYPE]))
		{
		default:
		case BiquadResonantFilter::LOWPASS:
			mA0 = 0.5f * (1.0f - cos_omega) * scalar;
			mA1 = (1.0f - cos_omega) * scalar;
			mA2 = mA0;
			mB1 = -2.0f * cos_omega * scalar;
			mB2 = (1.0f - alpha) * scalar;
			break;
		case BiquadResonantFilter::HIGHPASS:
			mA0 = 0.5f * (1.0f + cos_omega) * scalar;
			mA1 = -(1.0f + cos_omega) * scalar;
			mA2 = mA0;
			mB1 = -2.0f * cos_omega * scalar;
			mB2 = (1.0f - alpha) * scalar;
			break;
		case BiquadResonantFilter::BANDPASS:
			mA0 = alpha * scalar;
			mA1 = 0;
			mA2 = -mA0;
			mB1 = -2.0f * cos_omega * scalar;
			mB2 = (1.0f - alpha) * scalar;
			break;
		}
	}


	BiquadResonantFilterInstance::BiquadResonantFilterInstance(BiquadResonantFilter *aParent)
	{
		int i;
		for (i = 0; i < 8; i++)
		{
			mState[i].mX1 = 0;
			mState[i].mY1 = 0;
			mState[i].mX2 = 0;
			mState[i].mY2 = 0;
		}

		mParent = aParent;

		initParams(4);
		
		mParam[RESONANCE] = aParent->mResonance;
		mParam[FREQUENCY] = aParent->mFrequency;
		mParam[TYPE] = (float)aParent->mFilterType;
		
		mSamplerate = 44100;

		calcBQRParams();
	}

	void BiquadResonantFilterInstance::filterChannel(float *aBuffer, unsigned int aSamples, float aSamplerate, double aTime, unsigned int aChannel, unsigned int /*aChannels*/)
	{
		unsigned int osamples = aSamples;
		if (aChannel == 0)
		{
			updateParams(aTime);

			if (mParamChanged & ((1 << FREQUENCY) | (1 << RESONANCE) | (1 << TYPE)) || aSamplerate != mSamplerate)
			{
				mSamplerate = aSamplerate;
				calcBQRParams();
			}
			mParamChanged = 0;			
		}		
		float x;
		unsigned int i;
		int c = 0;

		BQRStateData &s = mState[aChannel];

		// make sure we access pairs of samples (one sample may be skipped)
		aSamples = aSamples & ~1; 

		for (i = 0; i < aSamples; i +=2, c++)
		{
			// Generate outputs by filtering inputs.
			x = aBuffer[c];
			s.mY2 = (mA0 * x) + (mA1 * s.mX1) + (mA2 * s.mX2) - (mB1 * s.mY1) - (mB2 * s.mY2);
			aBuffer[c] += (s.mY2 - aBuffer[c]) * mParam[WET];

			c++;

			// Permute filter operations to reduce data movement.
			// Just substitute variables instead of doing mX1=x, etc.
			s.mX2 = aBuffer[c];
			s.mY1 = (mA0 * s.mX2) + (mA1 * x) + (mA2 * s.mX1) - (mB1 * s.mY2) - (mB2 * s.mY1);
			aBuffer[c] += (s.mY1 - aBuffer[c]) * mParam[WET];

			// Only move a little data.
			s.mX1 = s.mX2;
			s.mX2 = x;
		}
		// If we skipped a sample earlier, patch it by just copying the previous.
		if (osamples != aSamples)
			aBuffer[c] = aBuffer[c - 1];
	}


	BiquadResonantFilterInstance::~BiquadResonantFilterInstance()
	{
	}

	BiquadResonantFilter::BiquadResonantFilter()
	{
		setParams(LOWPASS, 1000, 2);
	}

	result BiquadResonantFilter::setParams(int aType, float aFrequency, float aResonance)
	{
		if (aType < 0 || aType > 3 || aFrequency <= 0 || aResonance <= 0)
			return INVALID_PARAMETER;

		mFilterType = aType;
		mFrequency = aFrequency;
		mResonance = aResonance;

		return 0;
	}

	int BiquadResonantFilter::getParamCount()
	{
		return 4;
	}

	const char* BiquadResonantFilter::getParamName(unsigned int aParamIndex)
	{
		if (aParamIndex > 3)
			return 0;

		const char* name[4] = {
			"Wet",
			"Type",
			"Frequency",
			"Resonance"
		};
		return name[aParamIndex];
	}
	
	unsigned int BiquadResonantFilter::getParamType(unsigned int aParamIndex)
	{
		if (aParamIndex == TYPE)
			return INT_PARAM;
		return FLOAT_PARAM;
	}

	float BiquadResonantFilter::getParamMax(unsigned int aParamIndex)
	{
		switch (aParamIndex)
		{
		case WET: return 1;
		case TYPE: return 2;
		case FREQUENCY: return 8000;
		case RESONANCE: return 20;
		}
		return 1;
	}
	
	float BiquadResonantFilter::getParamMin(unsigned int aParamIndex)
	{
		switch (aParamIndex)
		{
		case FREQUENCY: return 10;
		case RESONANCE: return 0.1f;
		}
		return 0;
	}


	BiquadResonantFilter::~BiquadResonantFilter()
	{
	}


	BiquadResonantFilterInstance *BiquadResonantFilter::createInstance()
	{
		return new BiquadResonantFilterInstance(this);
	}
}
