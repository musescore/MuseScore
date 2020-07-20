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
#include "soloud_freeverbfilter.h"


namespace SoLoud
{
	namespace FreeverbImpl
	{
		// Based on code written by Jezar at Dreampoint, June 2000 http://www.dreampoint.co.uk, 
		// which was placed in public domain. The code was massaged quite a bit by 
		// Jari Komppa, result in the license listed at top of this file.

		class Comb
		{
		public:
			Comb();
			void	setbuffer(float* aBuf, int aSize);
			float	process(float aInp);
			void	mute();
			void	setdamp(float val);
			void	setfeedback(float val);
			float	mFeedback;
			float	mFilterstore;
			float	mDamp1;
			float	mDamp2;
			float*  mBuffer;
			int		mBufsize;
			int		mBufidx;
		};

		class Allpass
		{
		public:
			Allpass();
			void	setbuffer(float* aBuf, int aSize);
			float   process(float aInp);
			void	mute();
			void	setfeedback(float aVal);
			float	mFeedback;
			float*  mBuffer;
			int		mBufsize;
			int		mBufidx;
		};

		const int	gNumcombs = 8;
		const int	gNumallpasses = 4;
		const float	gMuted = 0;
		const float	gFixedgain = 0.015f;
		const float gScalewet = 3;
		const float gScaledry = 2;
		const float gScaledamp = 0.4f;
		const float gScaleroom = 0.28f;
		const float gOffsetroom = 0.7f;
		const float gInitialroom = 0.5f;
		const float gInitialdamp = 0.5f;
		const float gInitialwet = 1 / gScalewet;
		const float gInitialdry = 0;
		const float gInitialwidth = 1;
		const float gInitialmode = 0;
		const float gFreezemode = 0.5f;
		const int	gStereospread = 23;

		// These values assume 44.1KHz sample rate
		// they will probably be OK for 48KHz sample rate
		// but would need scaling for 96KHz (or other) sample rates.
		// The values were obtained by listening tests.
		const int gCombtuningL1 = 1116;
		const int gCombtuningR1 = 1116 + gStereospread;
		const int gCombtuningL2 = 1188;
		const int gCombtuningR2 = 1188 + gStereospread;
		const int gCombtuningL3 = 1277;
		const int gCombtuningR3 = 1277 + gStereospread;
		const int gCombtuningL4 = 1356;
		const int gCombtuningR4 = 1356 + gStereospread;
		const int gCombtuningL5 = 1422;
		const int gCombtuningR5 = 1422 + gStereospread;
		const int gCombtuningL6 = 1491;
		const int gCombtuningR6 = 1491 + gStereospread;
		const int gCombtuningL7 = 1557;
		const int gCombtuningR7 = 1557 + gStereospread;
		const int gCombtuningL8 = 1617;
		const int gCombtuningR8 = 1617 + gStereospread;
		const int gAllpasstuningL1 = 556;
		const int gAllpasstuningR1 = 556 + gStereospread;
		const int gAllpasstuningL2 = 441;
		const int gAllpasstuningR2 = 441 + gStereospread;
		const int gAllpasstuningL3 = 341;
		const int gAllpasstuningR3 = 341 + gStereospread;
		const int gAllpasstuningL4 = 225;
		const int gAllpasstuningR4 = 225 + gStereospread;


		class Revmodel
		{
		public:
			Revmodel();
			void	mute();
			void	process(float* aSampleData, long aNumSamples);
			void	setroomsize(float aValue);
			void	setdamp(float aValue);
			void	setwet(float aValue);
			void	setdry(float aValue);
			void	setwidth(float aValue);
			void	setmode(float aValue);
			void	update();

			float	mGain;
			float	mRoomsize, mRoomsize1;
			float	mDamp, mDamp1;
			float	mWet, mWet1, mWet2;
			float	mDry;
			float	mWidth;
			float	mMode;

			int		mDirty;

			// The following are all declared inline 
			// to remove the need for dynamic allocation
			// with its subsequent error-checking messiness

			// Comb filters
			Comb	mCombL[gNumcombs];
			Comb	mCombR[gNumcombs];

			// Allpass filters
			Allpass	mAllpassL[gNumallpasses];
			Allpass	mAllpassR[gNumallpasses];

			// Buffers for the combs
			float	mBufcombL1[gCombtuningL1];
			float	mBufcombR1[gCombtuningR1];
			float	mBufcombL2[gCombtuningL2];
			float	mBufcombR2[gCombtuningR2];
			float	mBufcombL3[gCombtuningL3];
			float	mBufcombR3[gCombtuningR3];
			float	mBufcombL4[gCombtuningL4];
			float	mBufcombR4[gCombtuningR4];
			float	mBufcombL5[gCombtuningL5];
			float	mBufcombR5[gCombtuningR5];
			float	mBufcombL6[gCombtuningL6];
			float	mBufcombR6[gCombtuningR6];
			float	mBufcombL7[gCombtuningL7];
			float	mBufcombR7[gCombtuningR7];
			float	mBufcombL8[gCombtuningL8];
			float	mBufcombR8[gCombtuningR8];

			// Buffers for the allpasses
			float	mBufallpassL1[gAllpasstuningL1];
			float	mBufallpassR1[gAllpasstuningR1];
			float	mBufallpassL2[gAllpasstuningL2];
			float	mBufallpassR2[gAllpasstuningR2];
			float	mBufallpassL3[gAllpasstuningL3];
			float	mBufallpassR3[gAllpasstuningR3];
			float	mBufallpassL4[gAllpasstuningL4];
			float	mBufallpassR4[gAllpasstuningR4];
		};

		Allpass::Allpass()
		{
			mBufidx = 0;
			mFeedback = 0;
			mBuffer = 0;
			mBufsize = 0;
		}

		float Allpass::process(float aInput)
		{
			float output;
			float bufout;

			bufout = mBuffer[mBufidx];

			output = -aInput + bufout;
			mBuffer[mBufidx] = aInput + (bufout * mFeedback);

			if (++mBufidx >= mBufsize) mBufidx = 0;

			return output;
		}

		void Allpass::setbuffer(float* aBuf, int aSize)
		{
			mBuffer = aBuf;
			mBufsize = aSize;
		}

		void Allpass::mute()
		{
			for (int i = 0; i < mBufsize; i++)
				mBuffer[i] = 0;
		}

		void Allpass::setfeedback(float aVal)
		{
			mFeedback = aVal;
		}

		Comb::Comb()
		{
			mFilterstore = 0;
			mBufidx = 0;
			mFeedback = 0;
			mDamp1 = 0;
			mDamp2 = 0;
			mBuffer = 0;
			mBufsize = 0;
		}

		float Comb::process(float aInput)
		{
			float output;

			output = mBuffer[mBufidx];

			mFilterstore = (output * mDamp2) + (mFilterstore * mDamp1);

			mBuffer[mBufidx] = aInput + (mFilterstore * mFeedback);

			if (++mBufidx >= mBufsize) mBufidx = 0;

			return output;
		}

		void Comb::setbuffer(float* aBuf, int aSize)
		{
			mBuffer = aBuf;
			mBufsize = aSize;
		}

		void Comb::mute()
		{
			for (int i = 0; i < mBufsize; i++)
				mBuffer[i] = 0;
		}

		void Comb::setdamp(float aVal)
		{
			mDamp1 = aVal;
			mDamp2 = 1 - aVal;
		}

		void Comb::setfeedback(float aVal)
		{
			mFeedback = aVal;
		}
	
		Revmodel::Revmodel()
		{
			mGain = 0;
			mRoomsize = 0;
			mRoomsize1 = 0;
			mDamp = 0;
			mDamp1 = 0;
			mWet = 0;
			mWet1 = 0;
			mWet2 = 0;
			mDry = 0;
			mWidth = 0;
			mMode = 0;

			mDirty = 1;

			// Tie the components to their buffers
			mCombL[0].setbuffer(mBufcombL1, gCombtuningL1);
			mCombR[0].setbuffer(mBufcombR1, gCombtuningR1);
			mCombL[1].setbuffer(mBufcombL2, gCombtuningL2);
			mCombR[1].setbuffer(mBufcombR2, gCombtuningR2);
			mCombL[2].setbuffer(mBufcombL3, gCombtuningL3);
			mCombR[2].setbuffer(mBufcombR3, gCombtuningR3);
			mCombL[3].setbuffer(mBufcombL4, gCombtuningL4);
			mCombR[3].setbuffer(mBufcombR4, gCombtuningR4);
			mCombL[4].setbuffer(mBufcombL5, gCombtuningL5);
			mCombR[4].setbuffer(mBufcombR5, gCombtuningR5);
			mCombL[5].setbuffer(mBufcombL6, gCombtuningL6);
			mCombR[5].setbuffer(mBufcombR6, gCombtuningR6);
			mCombL[6].setbuffer(mBufcombL7, gCombtuningL7);
			mCombR[6].setbuffer(mBufcombR7, gCombtuningR7);
			mCombL[7].setbuffer(mBufcombL8, gCombtuningL8);
			mCombR[7].setbuffer(mBufcombR8, gCombtuningR8);
			mAllpassL[0].setbuffer(mBufallpassL1, gAllpasstuningL1);
			mAllpassR[0].setbuffer(mBufallpassR1, gAllpasstuningR1);
			mAllpassL[1].setbuffer(mBufallpassL2, gAllpasstuningL2);
			mAllpassR[1].setbuffer(mBufallpassR2, gAllpasstuningR2);
			mAllpassL[2].setbuffer(mBufallpassL3, gAllpasstuningL3);
			mAllpassR[2].setbuffer(mBufallpassR3, gAllpasstuningR3);
			mAllpassL[3].setbuffer(mBufallpassL4, gAllpasstuningL4);
			mAllpassR[3].setbuffer(mBufallpassR4, gAllpasstuningR4);

			// Set default values
			mAllpassL[0].setfeedback(0.5f);
			mAllpassR[0].setfeedback(0.5f);
			mAllpassL[1].setfeedback(0.5f);
			mAllpassR[1].setfeedback(0.5f);
			mAllpassL[2].setfeedback(0.5f);
			mAllpassR[2].setfeedback(0.5f);
			mAllpassL[3].setfeedback(0.5f);
			mAllpassR[3].setfeedback(0.5f);
			setwet(gInitialwet);
			setroomsize(gInitialroom);
			setdry(gInitialdry);
			setdamp(gInitialdamp);
			setwidth(gInitialwidth);
			setmode(gInitialmode);			

			// Buffer will be full of rubbish - so we MUST mute them
			mute();
		}

		void Revmodel::mute()
		{			
			if (mMode >= gFreezemode)
				return;

			for (int i = 0; i < gNumcombs; i++)
			{
				mCombL[i].mute();
				mCombR[i].mute();
			}
			for (int i = 0; i < gNumallpasses; i++)
			{
				mAllpassL[i].mute();
				mAllpassR[i].mute();
			}
		}

		void Revmodel::process(float* aSampleData, long aNumSamples)
		{
			float* inputL, * inputR;
			inputL = aSampleData;
			inputR = aSampleData + aNumSamples;

			if (mDirty)
				update();
			mDirty = 0;

			while (aNumSamples-- > 0)
			{
				float outL, outR, input;
				outL = outR = 0;
				input = (*inputL + *inputR) * mGain;

				// Accumulate comb filters in parallel
				for (int i = 0; i < gNumcombs; i++)
				{
					outL += mCombL[i].process(input);
					outR += mCombR[i].process(input);
				}

				// Feed through allpasses in series
				for (int i = 0; i < gNumallpasses; i++)
				{
					outL = mAllpassL[i].process(outL);
					outR = mAllpassR[i].process(outR);
				}

				// Calculate output REPLACING anything already there
				*inputL = outL * mWet1 + outR * mWet2 + *inputL * mDry;
				*inputR = outR * mWet1 + outL * mWet2 + *inputR * mDry;

				// Increment sample pointers, allowing for interleave (if any)
				inputL++;
				inputR++;
			}
		}

		void Revmodel::update()
		{
			// Recalculate internal values after parameter change

			int i;

			mWet1 = mWet * (mWidth / 2 + 0.5f);
			mWet2 = mWet * ((1 - mWidth) / 2);

			if (mMode >= gFreezemode)
			{
				mRoomsize1 = 1;
				mDamp1 = 0;
				mGain = gMuted;
			}
			else
			{
				mRoomsize1 = mRoomsize;
				mDamp1 = mDamp;
				mGain = gFixedgain;
			}

			for (i = 0; i < gNumcombs; i++)
			{
				mCombL[i].setfeedback(mRoomsize1);
				mCombR[i].setfeedback(mRoomsize1);
			}

			for (i = 0; i < gNumcombs; i++)
			{
				mCombL[i].setdamp(mDamp1);
				mCombR[i].setdamp(mDamp1);
			}
		}

		void Revmodel::setroomsize(float aValue)
		{
			mRoomsize = (aValue * gScaleroom) + gOffsetroom;
			mDirty = 1;
		}

		void Revmodel::setdamp(float aValue)
		{
			mDamp = aValue * gScaledamp;
			mDirty = 1;
		}

		void Revmodel::setwet(float aValue)
		{
			mWet = aValue * gScalewet;
			mDirty = 1;
		}

		void Revmodel::setdry(float aValue)
		{
			mDry = aValue * gScaledry;
		}

		void Revmodel::setwidth(float aValue)
		{
			mWidth = aValue;
			mDirty = 1;
		}

		void Revmodel::setmode(float aValue)
		{
			mMode = aValue;
			mDirty = 1;
		}
	}

	FreeverbFilterInstance::FreeverbFilterInstance(FreeverbFilter *aParent)
	{	
		initParams(5);
		
		mParent = aParent;

		mModel = new FreeverbImpl::Revmodel();

		mParam[FREEZE] = aParent->mMode;
		mParam[ROOMSIZE] = aParent->mRoomSize;
		mParam[DAMP] = aParent->mDamp;
		mParam[WIDTH] = aParent->mWidth;
		mParam[WET] = 1;
	}

	void FreeverbFilterInstance::filter(float* aBuffer, unsigned int aSamples, unsigned int aChannels, float aSamplerate, time aTime)
	{
		SOLOUD_ASSERT(aChannels == 2); // Only stereo supported at this time
		if (mParamChanged)
		{
			mModel->setdamp(mParam[DAMP]);
			mModel->setmode(mParam[FREEZE]);
			mModel->setroomsize(mParam[ROOMSIZE]);
			mModel->setwidth(mParam[WIDTH]);
			mModel->setwet(mParam[WET]);
			mModel->setdry(1 - mParam[WET]);
			mParamChanged = 0;
		}
		mModel->process(aBuffer, aSamples);
	}

	FreeverbFilterInstance::~FreeverbFilterInstance()
	{
		delete mModel;
	}

	FreeverbFilter::FreeverbFilter()
	{
		setParams(0, 0.5, 0.5, 1);
	}

	result FreeverbFilter::setParams(float aFreeze, float aRoomSize, float aDamp, float aWidth)
	{
		if (aFreeze < 0 || aFreeze > 1 || aRoomSize <= 0 || aDamp < 0 || aWidth <= 0)
			return INVALID_PARAMETER;

		mMode = aFreeze;
		mRoomSize = aRoomSize;
		mDamp = aDamp;
		mWidth = aWidth;

		return 0;
	}

	int FreeverbFilter::getParamCount()
	{
		return 5;
	}

	const char* FreeverbFilter::getParamName(unsigned int aParamIndex)
	{
		switch (aParamIndex)
		{
		case FREEZE: return "Freeze";
		case ROOMSIZE: return "Room size";
		case DAMP: return "Damp";
		case WIDTH: return "Width";
		}
		return "Wet";
	}

	unsigned int FreeverbFilter::getParamType(unsigned int aParamIndex)
	{
		if (aParamIndex == FREEZE)
			return BOOL_PARAM;
		return FLOAT_PARAM;
	}

	float FreeverbFilter::getParamMax(unsigned int aParamIndex)
	{
		return 1;
	}

	float FreeverbFilter::getParamMin(unsigned int aParamIndex)
	{
		return 0;
	}


	FreeverbFilter::~FreeverbFilter()
	{
	}

	FreeverbFilterInstance *FreeverbFilter::createInstance()
	{
		return new FreeverbFilterInstance(this);
	}
}
