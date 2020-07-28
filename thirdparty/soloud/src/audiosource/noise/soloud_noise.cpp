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

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include "soloud_noise.h"

namespace SoLoud
{

	NoiseInstance::NoiseInstance(Noise *aParent)
	{
		for (int i = 0; i < 10; i++)
			mOctaveScale[i] = aParent->mOctaveScale[i];
		mPrg.srand(0xfade);
	}

	NoiseInstance::~NoiseInstance()
	{
	    
	}

	unsigned int NoiseInstance::getAudio(float* aBuffer, unsigned int aSamplesToRead, unsigned int /*aBufferSize*/)
	{
		int octavestep[10];
		float octavevalue[10];
		float totalscale = 0;
		for (int j = 0; j < 10; j++)
		{		
			octavevalue[j] = 0;
			octavestep[j] = 1 << j;
			totalscale += mOctaveScale[j];
		}

		for (unsigned int i = 0; i < aSamplesToRead; i++)
		{
			aBuffer[i] = mPrg.rand_float() - 0.5f;
			for (int j = 0; j < 10; j++)
			{
				octavestep[j]++;
				if (octavestep[j] > (1 << (j + 1)))
				{
					octavestep[j] = 0;
					octavevalue[j] = mPrg.rand_float() - 0.5f;
				}
				aBuffer[i] += octavevalue[j] * mOctaveScale[j];
			}
			aBuffer[i] *= 1.0f/totalscale;
		}

		return aSamplesToRead;
	}

	bool NoiseInstance::hasEnded()
	{
		return false;
	}

	void Noise::setOctaveScale(float aOct0, float aOct1, float aOct2, float aOct3, float aOct4, float aOct5, float aOct6, float aOct7, float aOct8, float aOct9)
	{
		mOctaveScale[0] = aOct0;
		mOctaveScale[1] = aOct1;
		mOctaveScale[2] = aOct2;
		mOctaveScale[3] = aOct3;
		mOctaveScale[4] = aOct4;
		mOctaveScale[5] = aOct5;
		mOctaveScale[6] = aOct6;
		mOctaveScale[7] = aOct7;
		mOctaveScale[8] = aOct8;
		mOctaveScale[9] = aOct9;
	}

	void Noise::setType(int aType)
	{
		switch (aType)
		{
		default:
		case WHITE:
			setOctaveScale(1, 0, 0, 0, 0, 0, 0, 0, 0, 0);
			break;
		case PINK:
			setOctaveScale(1, 1, 1, 1, 1, 1, 1, 1, 1, 1);
			break;
		case BROWNISH:
			setOctaveScale(1, 2, 3, 4, 5, 6, 7, 8, 9, 10);
			break;
		case BLUEISH:
			setOctaveScale(10, 9, 8, 7, 6, 5, 4, 3, 2, 1);
			break;
		}
	}

	Noise::Noise()
	{
		mBaseSamplerate = 44100;
		setType(WHITE);
	}

	Noise::~Noise()
	{
		stop();
	}


	AudioSourceInstance * Noise::createInstance() 
	{
		return new NoiseInstance(this);
	}

};