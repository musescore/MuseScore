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

#include "soloud.h"

namespace SoLoud
{

	int Filter::getParamCount()
	{
		return 1; // there's always WET
	}

	const char* Filter::getParamName(unsigned int aParamIndex)
	{
        (void)aParamIndex;
		return "Wet";
	}

	unsigned int Filter::getParamType(unsigned int aParamIndex)
	{
        (void)aParamIndex;
		return Filter::FLOAT_PARAM;
	}
	
	float Filter::getParamMax(unsigned int aParamIndex)
	{
        (void)aParamIndex;
		return 1;
	}

	float Filter::getParamMin(unsigned int aParamIndex)
	{
        (void)aParamIndex;
		return 0;
	}

	Filter::Filter()
	{
	}

	Filter::~Filter()
	{
	}
	
	FilterInstance::FilterInstance()
	{
		mNumParams = 0;
		mParamChanged = 0;
		mParam = 0;
		mParamFader = 0;
	}

	result FilterInstance::initParams(int aNumParams)
	{		
		mNumParams = aNumParams;
		delete[] mParam;
		delete[] mParamFader;
		mParam = new float[mNumParams];
		mParamFader = new Fader[mNumParams];

		if (mParam == NULL || mParamFader == NULL)
		{
			delete[] mParam;
			delete[] mParamFader;
			mParam = NULL;
			mParamFader = NULL;
			mNumParams = 0;
			return OUT_OF_MEMORY;
		}

		unsigned int i;
		for (i = 0; i < mNumParams; i++)
		{
			mParam[i] = 0;
			mParamFader[i].mActive = 0;
		}
		mParam[0] = 1; // set 'wet' to 1

		return 0;
	}

	void FilterInstance::updateParams(double aTime)
	{
		unsigned int i;
		for (i = 0; i < mNumParams; i++)
		{
			if (mParamFader[i].mActive > 0)
			{
				mParamChanged |= 1 << i;
				mParam[i] = mParamFader[i].get(aTime);
			}
		}
	}

	FilterInstance::~FilterInstance()
	{
		delete[] mParam;
		delete[] mParamFader;
	}

	void FilterInstance::setFilterParameter(unsigned int aAttributeId, float aValue)
	{
		if (aAttributeId >= mNumParams)
			return;

		mParamFader[aAttributeId].mActive = 0;
		mParam[aAttributeId] = aValue;
		mParamChanged |= 1 << aAttributeId;
	}

	void FilterInstance::fadeFilterParameter(unsigned int aAttributeId, float aTo, double aTime, double aStartTime)
	{
		if (aAttributeId >= mNumParams || aTime <= 0 || aTo == mParam[aAttributeId])
			return;

		mParamFader[aAttributeId].set(mParam[aAttributeId], aTo, aTime, aStartTime);
	}

	void FilterInstance::oscillateFilterParameter(unsigned int aAttributeId, float aFrom, float aTo, double aTime, double aStartTime)
	{
		if (aAttributeId >= mNumParams || aTime <= 0 || aFrom == aTo)
			return;

		mParamFader[aAttributeId].setLFO(aFrom, aTo, aTime, aStartTime);
	}

	float FilterInstance::getFilterParameter(unsigned int aAttributeId)
	{
		if (aAttributeId >= mNumParams)
			return 0;

		return mParam[aAttributeId];
	}

	void FilterInstance::filter(float *aBuffer, unsigned int aSamples, unsigned int aChannels, float aSamplerate, double aTime)
	{
		unsigned int i;
		for (i = 0; i < aChannels; i++)
		{
			filterChannel(aBuffer + i * aSamples, aSamples, aSamplerate, aTime, i, aChannels);
		}
	}

	void FilterInstance::filterChannel(float * /*aBuffer*/, unsigned int /*aSamples*/, float /*aSamplerate*/, double /*aTime*/, unsigned int /*aChannel*/, unsigned int /*aChannels*/)
	{
	}

};

