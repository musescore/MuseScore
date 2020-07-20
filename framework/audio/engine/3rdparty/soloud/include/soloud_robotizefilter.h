/*
SoLoud audio engine
Copyright (c) 2020 Jari Komppa

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

#ifndef SOLOUD_ROBOTIZEFILTER_H
#define SOLOUD_ROBOTIZEFILTER_H

#include "soloud.h"
#include "soloud_filter.h"
#include "soloud_misc.h"

namespace SoLoud
{
	class RobotizeFilter;

	class RobotizeFilterInstance : public FilterInstance
	{
		enum FILTERATTRIBUTE
		{
			WET = 0,
			FREQ,
			WAVE
		};
		RobotizeFilter *mParent;
	public:
		virtual void filterChannel(float *aBuffer, unsigned int aSamples, float aSamplerate, time aTime, unsigned int aChannel, unsigned int aChannels);
		RobotizeFilterInstance(RobotizeFilter *aParent);
	};

	class RobotizeFilter : public Filter
	{
	public:
		enum FILTERATTRIBUTE
		{
			WET = 0,
			FREQ,
			WAVE
		};
		float mFreq;
		int mWave;
		virtual int getParamCount();
		virtual const char* getParamName(unsigned int aParamIndex);
		virtual unsigned int getParamType(unsigned int aParamIndex);
		virtual float getParamMax(unsigned int aParamIndex);
		virtual float getParamMin(unsigned int aParamIndex);
		void setParams(float aFreq, int aWaveform);
		virtual FilterInstance *createInstance();
		RobotizeFilter();
	};
}

#endif