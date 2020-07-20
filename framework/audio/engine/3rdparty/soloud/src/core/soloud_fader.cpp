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
	Fader::Fader()
	{
		mCurrent = mFrom = mTo = mDelta = 0;
		mTime = mStartTime = mEndTime = 0;
		mActive = 0;
	}

	void Fader::set(float aFrom, float aTo, double aTime, double aStartTime)
	{
		mCurrent = mFrom;
		mFrom = aFrom;
		mTo = aTo;
		mTime = aTime;
		mStartTime = aStartTime;
		mDelta = aTo - aFrom;
		mEndTime = mStartTime + mTime;
		mActive = 1;
	}

	void Fader::setLFO(float aFrom, float aTo, double aTime, double aStartTime)
	{
		mActive = 2;
		mCurrent = 0;
		mFrom = aFrom;
		mTo = aTo;
		mTime = aTime;
		mDelta = (aTo - aFrom) / 2;
		if (mDelta < 0) mDelta = -mDelta;
		mStartTime = aStartTime;
		mEndTime = (float)M_PI * 2 / mTime;
	}

	float Fader::get(double aCurrentTime)
	{
		if (mActive == 2)
		{
			// LFO mode
			if (mStartTime > aCurrentTime)
			{
				// Time rolled over.
				mStartTime = aCurrentTime;
			}
			double t = aCurrentTime - mStartTime;
			return (float)(sin(t * mEndTime) * mDelta + (mFrom + mDelta));
			
		}
		if (mStartTime > aCurrentTime)
		{
			// Time rolled over.
			// Figure out where we were..
			float p = (mCurrent - mFrom) / mDelta; // 0..1
			mFrom = mCurrent;
			mStartTime = aCurrentTime;
			mTime = mTime * (1 - p); // time left
			mDelta = mTo - mFrom;
			mEndTime = mStartTime + mTime;
		}
		if (aCurrentTime > mEndTime)
		{
			mActive = -1;
			return mTo;
		}
		mCurrent = (float)(mFrom + mDelta * ((aCurrentTime - mStartTime) / mTime));
		return mCurrent;
	}
};
