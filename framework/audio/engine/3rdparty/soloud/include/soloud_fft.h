/*
SoLoud audio engine
Copyright (c) 2013-2015 Jari Komppa

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

#ifndef SOLOUD_FFT_H
#define SOLOUD_FFT_H

#include "soloud.h"

namespace SoLoud
{
	namespace FFT
	{
		// Perform 1024 unit FFT. Buffer must have 1024 floats, and will be overwritten
		void fft1024(float *aBuffer);

		// Perform 256 unit FFT. Buffer must have 256 floats, and will be overwritten
		void fft256(float *aBuffer);
		
		// Perform 256 unit IFFT. Buffer must have 256 floats, and will be overwritten
		void ifft256(float *aBuffer);

		// Generic (slower) power of two FFT. Buffer is overwritten.
		void fft(float *aBuffer, unsigned int aBufferLength);

		// Generic (slower) power of two IFFT. Buffer is overwritten.
		void ifft(float *aBuffer, unsigned int aBufferLength);
	};
};

#endif