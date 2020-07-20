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

#ifndef SOLOUD_FILTER_H
#define SOLOUD_FILTER_H

#include "soloud.h"

namespace SoLoud
{
	class Fader;

	class FilterInstance
	{
	public:
		unsigned int mNumParams;
		unsigned int mParamChanged;
		float *mParam;
		Fader *mParamFader;
		

		FilterInstance();
		virtual result initParams(int aNumParams);
		virtual void updateParams(time aTime);
		virtual void filter(float *aBuffer, unsigned int aSamples, unsigned int aChannels, float aSamplerate, time aTime);
		virtual void filterChannel(float *aBuffer, unsigned int aSamples, float aSamplerate, time aTime, unsigned int aChannel, unsigned int aChannels);
		virtual float getFilterParameter(unsigned int aAttributeId);
		virtual void setFilterParameter(unsigned int aAttributeId, float aValue);
		virtual void fadeFilterParameter(unsigned int aAttributeId, float aTo, time aTime, time aStartTime);
		virtual void oscillateFilterParameter(unsigned int aAttributeId, float aFrom, float aTo, time aTime, time aStartTime);
		virtual ~FilterInstance();
	};

	class Filter
	{
	public:
		enum PARAMTYPE
		{
			FLOAT_PARAM = 0,
			INT_PARAM,
			BOOL_PARAM
		};
		Filter();
		virtual int getParamCount();
		virtual const char* getParamName(unsigned int aParamIndex);
		virtual unsigned int getParamType(unsigned int aParamIndex);
		virtual float getParamMax(unsigned int aParamIndex);
		virtual float getParamMin(unsigned int aParamIndex);

		virtual FilterInstance *createInstance() = 0;
		virtual ~Filter();
	};
};

#endif
