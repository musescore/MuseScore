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
#include "soloud_fftfilter.h"
#include "soloud_fft.h"


namespace SoLoud
{
	FFTFilterInstance::FFTFilterInstance()
	{
		mParent = 0;
		mInputBuffer = 0;
		mMixBuffer = 0;
		mTemp = 0;
		int i;
		for (i = 0; i < MAX_CHANNELS; i++)
			mOffset[i] = 0;
	}

	FFTFilterInstance::FFTFilterInstance(FFTFilter *aParent)
	{
		mParent = aParent;
		mInputBuffer = 0;
		mMixBuffer = 0;
		mTemp = 0;
		int i;
		for (i = 0; i < MAX_CHANNELS; i++)
			mOffset[i] = 0;
		initParams(1);
	}

	void FFTFilterInstance::filterChannel(float *aBuffer, unsigned int aSamples, float aSamplerate, double aTime, unsigned int aChannel, unsigned int aChannels)
	{
		if (aChannel == 0)
		{
			updateParams(aTime);
		}

		if (mInputBuffer == 0)
		{
			mInputBuffer = new float[512 * aChannels];
			mMixBuffer = new float[512 * aChannels];
			mTemp = new float[256];
			memset(mInputBuffer, 0x2f, sizeof(float) * 512 * aChannels);
			memset(mMixBuffer, 0, sizeof(float) * 512 * aChannels);
		}

		float * b = mTemp;

		int i;
		unsigned int ofs = 0;
		unsigned int chofs = 512 * aChannel;
		unsigned int bofs = mOffset[aChannel];
		
		while (ofs < aSamples)
		{
			for (i = 0; i < 128; i++)
			{
				mInputBuffer[chofs + ((bofs + i + 128) & 511)] = aBuffer[ofs + i];
				mMixBuffer[chofs + ((bofs + i + 128) & 511)] = 0;
			}
			
			for (i = 0; i < 256; i++)
			{
				b[i] = mInputBuffer[chofs + ((bofs + i) & 511)];
			}
			FFT::fft256(b);

			// do magic
			fftFilterChannel(b, 128, aSamplerate, aTime, aChannel, aChannels);
			
			FFT::ifft256(b);

			for (i = 0; i < 256; i++)
			{
				mMixBuffer[chofs + ((bofs + i) & 511)] += b[i] * (128 - abs(128 - i)) * (1.0f / 128.0f);
			}			
			
			for (i = 0; i < 128; i++)
			{
				aBuffer[ofs + i] += (mMixBuffer[chofs + ((bofs + i) & 511)] - aBuffer[ofs + i]) * mParam[0];
			}
			ofs += 128;
			bofs += 128;
		}
		mOffset[aChannel] = bofs;
	}

	void FFTFilterInstance::fftFilterChannel(float *aFFTBuffer, unsigned int aSamples, float /*aSamplerate*/, time /*aTime*/, unsigned int /*aChannel*/, unsigned int /*aChannels*/)
	{
		unsigned int i;
		for (i = 4; i < aSamples; i++)
		{
			aFFTBuffer[(i - 4) * 2] = aFFTBuffer[i * 2];
			aFFTBuffer[(i - 4) * 2 + 1] = aFFTBuffer[i * 2 + 1];
		}
		for (i = 0; i < 4; i++)
		{
			aFFTBuffer[(aSamples - 4) * 2 + i * 2] = 0;
			aFFTBuffer[(aSamples - 4) * 2 + i * 2 + 1] = 0;
		}
	}

	FFTFilterInstance::~FFTFilterInstance()
	{
		delete[] mTemp;
		delete[] mInputBuffer;
		delete[] mMixBuffer;
	}

	FFTFilter::FFTFilter()
	{
	}

	FilterInstance *FFTFilter::createInstance()
	{
		return new FFTFilterInstance(this);
	}
}
