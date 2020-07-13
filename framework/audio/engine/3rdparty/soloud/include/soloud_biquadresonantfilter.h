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

#ifndef SOLOUD_BQRFILTER_H
#define SOLOUD_BQRFILTER_H

#include "soloud.h"

namespace SoLoud
{
	class BiquadResonantFilter;

	struct BQRStateData
	{
		float mY1, mY2, mX1, mX2;
	};

	class BiquadResonantFilterInstance : public FilterInstance
	{
		enum FILTERATTRIBUTE
		{
			WET = 0,
			TYPE,
			FREQUENCY,
			RESONANCE
		};

		BQRStateData mState[8];
		float mA0, mA1, mA2, mB1, mB2;
		int mDirty;
		float mSamplerate;

		BiquadResonantFilter *mParent;
		void calcBQRParams();
	public:
		virtual void filterChannel(float *aBuffer, unsigned int aSamples, float aSamplerate, time aTime, unsigned int aChannel, unsigned int aChannels);
		virtual ~BiquadResonantFilterInstance();
		BiquadResonantFilterInstance(BiquadResonantFilter *aParent);
	};

	class BiquadResonantFilter : public Filter
	{
	public:
		enum FILTERTYPE
		{
			LOWPASS = 0,
			HIGHPASS = 1,
			BANDPASS = 2
		};
		enum FILTERATTRIBUTE
		{
			WET = 0,
			TYPE,
			FREQUENCY,
			RESONANCE
		};
		int mFilterType;
		float mFrequency;
		float mResonance;
		virtual int getParamCount();
		virtual const char* getParamName(unsigned int aParamIndex);
		virtual unsigned int getParamType(unsigned int aParamIndex);
		virtual float getParamMax(unsigned int aParamIndex);
		virtual float getParamMin(unsigned int aParamIndex);

		virtual BiquadResonantFilterInstance *createInstance();
		BiquadResonantFilter();
		result setParams(int aType, float aFrequency, float aResonance);
		virtual ~BiquadResonantFilter();
	};
}

#endif