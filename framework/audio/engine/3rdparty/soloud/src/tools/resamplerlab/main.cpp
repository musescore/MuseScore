/*
SoLoud audio engine - tool to develop resamplers with
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
#include <stdlib.h>
#include <stdio.h>
#include <stddef.h>
#include <math.h>
#include "stb_image_write.h"

#define MAX_FUNC 3
#define MAX_RESAMPLER 6

#ifndef TAU
#define TAU 6.283185307179586476925286766559f
#endif

#ifndef M_PI
#define M_PI 3.14159265359f
#endif


unsigned char TFX_AsciiFontdata[12*256] = {
  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,// ' '
  0, 12, 30, 30, 30, 12, 12,  0, 12, 12,  0,  0,// '!'
  0,102,102,102, 36,  0,  0,  0,  0,  0,  0,  0,// '"'
  0, 54, 54,127, 54, 54, 54,127, 54, 54,  0,  0,// '#'
 12, 12, 62,  3,  3, 30, 48, 48, 31, 12, 12,  0,// '$'
  0,  0,  0, 35, 51, 24, 12,  6, 51, 49,  0,  0,// '%'
  0, 14, 27, 27, 14, 95,123, 51, 59,110,  0,  0,// '&'
  0, 12, 12, 12,  6,  0,  0,  0,  0,  0,  0,  0,// '''
  0, 48, 24, 12,  6,  6,  6, 12, 24, 48,  0,  0,// '('
  0,  6, 12, 24, 48, 48, 48, 24, 12,  6,  0,  0,// ')'
  0,  0,  0,102, 60,255, 60,102,  0,  0,  0,  0,// '*'
  0,  0,  0, 24, 24,126, 24, 24,  0,  0,  0,  0,// '+'
  0,  0,  0,  0,  0,  0,  0,  0, 28, 28,  6,  0,// ','
  0,  0,  0,  0,  0,127,  0,  0,  0,  0,  0,  0,// '-'
  0,  0,  0,  0,  0,  0,  0,  0, 28, 28,  0,  0,// '.'
  0,  0, 64, 96, 48, 24, 12,  6,  3,  1,  0,  0,// '/'
  0, 62, 99,115,123,107,111,103, 99, 62,  0,  0,// '0'
  0,  8, 12, 15, 12, 12, 12, 12, 12, 63,  0,  0,// '1'
  0, 30, 51, 51, 48, 24, 12,  6, 51, 63,  0,  0,// '2'
  0, 30, 51, 48, 48, 28, 48, 48, 51, 30,  0,  0,// '3'
  0, 48, 56, 60, 54, 51,127, 48, 48,120,  0,  0,// '4'
  0, 63,  3,  3,  3, 31, 48, 48, 51, 30,  0,  0,// '5'
  0, 28,  6,  3,  3, 31, 51, 51, 51, 30,  0,  0,// '6'
  0,127, 99, 99, 96, 48, 24, 12, 12, 12,  0,  0,// '7'
  0, 30, 51, 51, 55, 30, 59, 51, 51, 30,  0,  0,// '8'
  0, 30, 51, 51, 51, 62, 24, 24, 12, 14,  0,  0,// '9'
  0,  0,  0, 28, 28,  0,  0, 28, 28,  0,  0,  0,// ':'
  0,  0,  0, 28, 28,  0,  0, 28, 28, 24, 12,  0,// ';'
  0, 48, 24, 12,  6,  3,  6, 12, 24, 48,  0,  0,// '<'
  0,  0,  0,  0,126,  0,126,  0,  0,  0,  0,  0,// '='
  0,  6, 12, 24, 48, 96, 48, 24, 12,  6,  0,  0,// '>'
  0, 30, 51, 48, 24, 12, 12,  0, 12, 12,  0,  0,// '?'
  0, 62, 99, 99,123,123,123,  3,  3, 62,  0,  0,// '@'
  0, 12, 30, 51, 51, 51, 63, 51, 51, 51,  0,  0,// 'A'
  0, 63,102,102,102, 62,102,102,102, 63,  0,  0,// 'B'
  0, 60,102, 99,  3,  3,  3, 99,102, 60,  0,  0,// 'C'
  0, 31, 54,102,102,102,102,102, 54, 31,  0,  0,// 'D'
  0,127, 70,  6, 38, 62, 38,  6, 70,127,  0,  0,// 'E'
  0,127,102, 70, 38, 62, 38,  6,  6, 15,  0,  0,// 'F'
  0, 60,102, 99,  3,  3,115, 99,102,124,  0,  0,// 'G'
  0, 51, 51, 51, 51, 63, 51, 51, 51, 51,  0,  0,// 'H'
  0, 30, 12, 12, 12, 12, 12, 12, 12, 30,  0,  0,// 'I'
  0,120, 48, 48, 48, 48, 51, 51, 51, 30,  0,  0,// 'J'
  0,103,102, 54, 54, 30, 54, 54,102,103,  0,  0,// 'K'
  0, 15,  6,  6,  6,  6, 70,102,102,127,  0,  0,// 'L'
  0, 99,119,127,127,107, 99, 99, 99, 99,  0,  0,// 'M'
  0, 99, 99,103,111,127,123,115, 99, 99,  0,  0,// 'N'
  0, 28, 54, 99, 99, 99, 99, 99, 54, 28,  0,  0,// 'O'
  0, 63,102,102,102, 62,  6,  6,  6, 15,  0,  0,// 'P'
  0, 28, 54, 99, 99, 99,115,123, 62, 48,120,  0,// 'Q'
  0, 63,102,102,102, 62, 54,102,102,103,  0,  0,// 'R'
  0, 30, 51, 51,  3, 14, 24, 51, 51, 30,  0,  0,// 'S'
  0, 63, 45, 12, 12, 12, 12, 12, 12, 30,  0,  0,// 'T'
  0, 51, 51, 51, 51, 51, 51, 51, 51, 30,  0,  0,// 'U'
  0, 51, 51, 51, 51, 51, 51, 51, 30, 12,  0,  0,// 'V'
  0, 99, 99, 99, 99,107,107, 54, 54, 54,  0,  0,// 'W'
  0, 51, 51, 51, 30, 12, 30, 51, 51, 51,  0,  0,// 'X'
  0, 51, 51, 51, 51, 30, 12, 12, 12, 30,  0,  0,// 'Y'
  0,127,115, 25, 24, 12,  6, 70, 99,127,  0,  0,// 'Z'
  0, 60, 12, 12, 12, 12, 12, 12, 12, 60,  0,  0,// '['
  0,  0,  1,  3,  6, 12, 24, 48, 96, 64,  0,  0,// '\'
  0, 60, 48, 48, 48, 48, 48, 48, 48, 60,  0,  0,// ']'
  8, 28, 54, 99,  0,  0,  0,  0,  0,  0,  0,  0,// '^'
  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,255,  0,// '_'
 12, 12, 24,  0,  0,  0,  0,  0,  0,  0,  0,  0,// '`'
  0,  0,  0,  0, 30, 48, 62, 51, 51,110,  0,  0,// 'a'
  0,  7,  6,  6, 62,102,102,102,102, 59,  0,  0,// 'b'
  0,  0,  0,  0, 30, 51,  3,  3, 51, 30,  0,  0,// 'c'
  0, 56, 48, 48, 62, 51, 51, 51, 51,110,  0,  0,// 'd'
  0,  0,  0,  0, 30, 51, 63,  3, 51, 30,  0,  0,// 'e'
  0, 28, 54,  6,  6, 31,  6,  6,  6, 15,  0,  0,// 'f'
  0,  0,  0,  0,110, 51, 51, 51, 62, 48, 51, 30,// 'g'
  0,  7,  6,  6, 54,110,102,102,102,103,  0,  0,// 'h'
  0, 24, 24,  0, 30, 24, 24, 24, 24,126,  0,  0,// 'i'
  0, 48, 48,  0, 60, 48, 48, 48, 48, 51, 51, 30,// 'j'
  0,  7,  6,  6,102, 54, 30, 54,102,103,  0,  0,// 'k'
  0, 30, 24, 24, 24, 24, 24, 24, 24,126,  0,  0,// 'l'
  0,  0,  0,  0, 63,107,107,107,107, 99,  0,  0,// 'm'
  0,  0,  0,  0, 31, 51, 51, 51, 51, 51,  0,  0,// 'n'
  0,  0,  0,  0, 30, 51, 51, 51, 51, 30,  0,  0,// 'o'
  0,  0,  0,  0, 59,102,102,102,102, 62,  6, 15,// 'p'
  0,  0,  0,  0,110, 51, 51, 51, 51, 62, 48,120,// 'q'
  0,  0,  0,  0, 55,118,110,  6,  6, 15,  0,  0,// 'r'
  0,  0,  0,  0, 30, 51,  6, 24, 51, 30,  0,  0,// 's'
  0,  0,  4,  6, 63,  6,  6,  6, 54, 28,  0,  0,// 't'
  0,  0,  0,  0, 51, 51, 51, 51, 51,110,  0,  0,// 'u'
  0,  0,  0,  0, 51, 51, 51, 51, 30, 12,  0,  0,// 'v'
  0,  0,  0,  0, 99, 99,107,107, 54, 54,  0,  0,// 'w'
  0,  0,  0,  0, 99, 54, 28, 28, 54, 99,  0,  0,// 'x'
  0,  0,  0,  0,102,102,102,102, 60, 48, 24, 15,// 'y'
  0,  0,  0,  0, 63, 49, 24,  6, 35, 63,  0,  0,// 'z'
  0, 56, 12, 12,  6,  3,  6, 12, 12, 56,  0,  0,// '{'
  0, 24, 24, 24, 24,  0, 24, 24, 24, 24,  0,  0,// '|'
  0,  7, 12, 12, 24, 48, 24, 12, 12,  7,  0,  0,// '}'
  0,206, 91,115,  0,  0,  0,  0,  0,  0,  0,  0,// '~'
};

// The pixel data is 8x12 bits, so each glyph is 12 bytes.
int ispixel(char ch, int x, int y)
{
  return (TFX_AsciiFontdata[(ch - 32) * 12 + y] & (1 << x)) != 0;
}

void drawchar(int aChar, int aX, int aY, int *aBitmap, int aBitmapWidth, int aColor)
{
	int i, j;
	for (i = 0; i < 12; i++)
	{
		for (j = 0; j < 8; j++)
		{
			if (ispixel(aChar, j, i))
			{
				aBitmap[(aY+i)*aBitmapWidth+aX+j] = aColor;
			}
		}
	}
}

void drawstring(const char *aString, int aX, int aY, int *aBitmap, int aBitmapWidth, int aColor)
{
	while (*aString)
	{
		drawchar(*aString, aX, aY, aBitmap, aBitmapWidth, aColor);
		aX += 8;
		if (aX+8 > aBitmapWidth)
			return;
		aString++;
	}
}

#define SAMPLE_GRANULARITY 512
#define FIXPOINT_FRAC_BITS 20
#define FIXPOINT_FRAC_MUL (1 << FIXPOINT_FRAC_BITS)
#define FIXPOINT_FRAC_MASK ((1 << FIXPOINT_FRAC_BITS) - 1)


void resample_experiment(float *aSrc,
                          float *aSrc1,
                          float *aDst,
                          int aSrcOffset,
                          int aDstSampleCount,
                          float aSrcSamplerate,
                          float aDstSamplerate,
                          int aStepFixed)
{
	int i;
	int pos = aSrcOffset;
	float freq = aSrcSamplerate / 2;
	float omega = (float)((2.0f * M_PI * freq / aDstSamplerate));
	float sin_omega = (float)sin(omega);
	float cos_omega = (float)cos(omega);
	float resonance = 2.0f;
	float alpha = sin_omega / (2.0f * resonance);
	float scalar = 1.0f / (1.0f + alpha);

	float A0 = 0.5f * (1.0f - cos_omega) * scalar;
	float A1 = (1.0f - cos_omega) * scalar;
	float A2 = A0;
	float B1 = -2.0f * cos_omega * scalar;
	float B2 = (1.0f - alpha) * scalar;

	 float X1 = 0, Y1 = 0, X2 = 0, Y2 = 0, x = 0;
	 int iter = 0;

	for (i = 0; i < aDstSampleCount; i++, pos += aStepFixed, iter++)
	{
		int p = pos >> FIXPOINT_FRAC_BITS;

		if ((iter & 1) == 0)
		{
			// Generate outputs by filtering inputs.
			x = aSrc[p];
			Y2 = (A0 * x) + (A1 * X1) + (A2 * X2) - (B1 * Y1) - (B2 * Y2);
			aDst[i] = Y2;
		}
		else
		{
			// Permute filter operations to reduce data movement.
			// Just substitute variables instead of doing mX1=x, etc.
			X2 = aSrc[p];
			Y1 = (A0 * X2) + (A1 * x) + (A2 * X1) - (B1 * Y2) - (B2 * Y1);
			aDst[i] = Y1;

			// Only move a little data.
			X1 = X2;
			X2 = x;
		}
	}
}

float catmullrom(float t, float p0, float p1, float p2, float p3)
{
return 0.5f * (
              (2 * p1) +
              (-p0 + p2) * t +
              (2 * p0 - 5 * p1 + 4 * p2 - p3) * t * t +
              (-p0 + 3 * p1 - 3 * p2 + p3) * t * t * t
              );
}

void resample_catmullrom(float *aSrc,
                          float *aSrc1,
                          float *aDst,
                          int aSrcOffset,
                          int aDstSampleCount,
                          float aSrcSamplerate,
                          float aDstSamplerate,
                          int aStepFixed)
{
  int i;
  int pos = aSrcOffset;

  for (i = 0; i < aDstSampleCount; i++, pos += aStepFixed)
  {
    int p = pos >> FIXPOINT_FRAC_BITS;
    int f = pos & FIXPOINT_FRAC_MASK;

	float s0, s1, s2, s3;

	if (p < 3)
	{
		s3 = aSrc1[512+p-3];
	}
	else
	{
		s3 = aSrc[p-3];
	}

	if (p < 2)
	{
		s2 = aSrc1[512+p-2];
	}
	else
	{
		s2 = aSrc[p-2];
	}

	if (p < 1)
	{
		s1 = aSrc1[512+p-1];
	}
	else
	{
		s1 = aSrc[p-1];
	}

	s0 = aSrc[p];

	aDst[i] = catmullrom(f/(float)FIXPOINT_FRAC_MUL,s3,s2,s1,s0);
  }
}

float sincpi(float x)
{
	if (x == 0)
		return 1;
	return (float)(sin(M_PI * x) / (M_PI * x));
}

void resample_sinc6(float *aSrc,
                     float *aSrc1,
                     float *aDst,
                     int aSrcOffset,
                     int aDstSampleCount,
                     float aSrcSamplerate,
                     float aDstSamplerate,
                     int aStepFixed)
{
  int i;
  int pos = aSrcOffset;

  for (i = 0; i < aDstSampleCount; i++, pos += aStepFixed)
  {
    int p = pos >> FIXPOINT_FRAC_BITS;
    int f = pos & FIXPOINT_FRAC_MASK;

	float s0 = 0, s1 = 0, s2 = 0, s3 = 0, s4 = 0, s5 = 0;

	if (p < 5)
	{
		s5 = aSrc1[512+p-5];
	}
	else
	{
		s5 = aSrc[p-5];
	}

	if (p < 4)
	{
		s4 = aSrc1[512+p-4];
	}
	else
	{
		s4 = aSrc[p-4];
	}

	if (p < 3)
	{
		s3 = aSrc1[512+p-3];
	}
	else
	{
		s3 = aSrc[p-3];
	}

	if (p < 2)
	{
		s2 = aSrc1[512+p-2];
	}
	else
	{
		s2 = aSrc[p-2];
	}

	if (p < 1)
	{
		s1 = aSrc1[512+p-1];
	}
	else
	{
		s1 = aSrc[p-1];
	}

	s0 = aSrc[p];

	float a = 1 - (f / (float)FIXPOINT_FRAC_MUL);

	aDst[i] = 
		 (
			s0 * sincpi(2 + a) +
			s1 * sincpi(1 + a) +
			s2 * sincpi(a) +
			s3 * sincpi(1 - a) +
			s4 * sincpi(2 - a) + 
			s5 * sincpi(3 - a)
			);
  }
}

void resample_gauss5(float *aSrc,
                     float *aSrc1,
                     float *aDst,
                     int aSrcOffset,
                     int aDstSampleCount,
                     float aSrcSamplerate,
                     float aDstSamplerate,
                     int aStepFixed)
{
  int i;
  int pos = aSrcOffset;

  for (i = 0; i < aDstSampleCount; i++, pos += aStepFixed)
  {
    int p = pos >> FIXPOINT_FRAC_BITS;
    int f = pos & FIXPOINT_FRAC_MASK;

	float s0, s1, s2, s3, s4;

	if (p < 4)
	{
		s4 = aSrc1[512+p-4];
	}
	else
	{
		s4 = aSrc[p-4];
	}

	if (p < 3)
	{
		s3 = aSrc1[512+p-3];
	}
	else
	{
		s3 = aSrc[p-3];
	}

	if (p < 2)
	{
		s2 = aSrc1[512+p-2];
	}
	else
	{
		s2 = aSrc[p-2];
	}

	if (p < 1)
	{
		s1 = aSrc1[512+p-1];
	}
	else
	{
		s1 = aSrc[p-1];
	}

	s0 = aSrc[p];

	aDst[i] = (float)
		 (
			s0 * 0.05 +
			s1 * 0.25 +
			s2 * 0.4 +
			s3 * 0.25 +
			s4 * 0.05
			);
  }
}

void resample_pointsample(float *aSrc,
                          float *aSrc1,
                          float *aDst,
                          int aSrcOffset,
                          int aDstSampleCount,
                          float aSrcSamplerate,
                          float aDstSamplerate,
                          int aStepFixed)
{
  int i;
  int pos = aSrcOffset;

  for (i = 0; i < aDstSampleCount; i++, pos += aStepFixed)
  {
    int p = pos >> FIXPOINT_FRAC_BITS;
    aDst[i] = aSrc[p];
  }
}

void resample_linear(float *aSrc,
                     float *aSrc1,
                     float *aDst,
                     int aSrcOffset,
                     int aDstSampleCount,
                     float aSrcSamplerate,
                     float aDstSamplerate,
                     int aStepFixed)
{
  int i;
  int pos = aSrcOffset;

  for (i = 0; i < aDstSampleCount; i++, pos += aStepFixed)
  {
    int p = pos >> FIXPOINT_FRAC_BITS;
    int f = pos & FIXPOINT_FRAC_MASK;
#ifdef _DEBUG

    if (p >= SAMPLE_GRANULARITY || p < 0)
    {
      // This should never actually happen
      p = SAMPLE_GRANULARITY - 1;
    }

#endif
    float s1 = aSrc1[SAMPLE_GRANULARITY - 1];

    float s2 = aSrc[p];

    if (p != 0)
    {
      s1 = aSrc[p-1];
    }

    aDst[i] = s1 + (s2 - s1) * f * (1 / (float)FIXPOINT_FRAC_MUL);
  }
}

static void smbFft(float *fftBuffer, int fftFrameSizeLog, int sign)
/* 
	* COPYRIGHT 1996 Stephan M. Bernsee <smb [AT] dspdimension [DOT] com>
	*
	* 						The Wide Open License (WOL)
	*
	* Permission to use, copy, modify, distribute and sell this software and its
	* documentation for any purpose is hereby granted without fee, provided that
	* the above copyright notice and this license appear in all source copies. 
	* THIS SOFTWARE IS PROVIDED "AS IS" WITHOUT EXPRESS OR IMPLIED WARRANTY OF
	* ANY KIND. See http://www.dspguru.com/wol.htm for more information.
	*
	* Sign = -1 is FFT, 1 is iFFT (inverse)
	* Fills fftBuffer[0...2*fftFrameSize-1] with the Fourier transform of the
	* time domain data in fftBuffer[0...2*fftFrameSize-1]. 
	* The FFT array takes and returns the cosine and sine parts in an interleaved 
	* manner, ie.	fftBuffer[0] = cosPart[0], fftBuffer[1] = sinPart[0], asf. 
	* fftFrameSize	must be a power of 2. 
	* It expects a complex input signal (see footnote 2), ie. when working with 
	* 'common' audio signals our input signal has to be passed as 
	* {in[0],0.,in[1],0.,in[2],0.,...} asf. 
	* In that case, the transform of the frequencies of interest is in 
	* fftBuffer[0...fftFrameSize].
*/
{
	float wr, wi, arg, *p1, *p2, temp;
	float tr, ti, ur, ui, *p1r, *p1i, *p2r, *p2i;
	int i, bitm, j, le, le2, k;
	int fftFrameSize = 1 << fftFrameSizeLog;

	for (i = 2; i < 2 * fftFrameSize - 2; i += 2) 
	{
		for (bitm = 2, j = 0; bitm < 2 * fftFrameSize; bitm <<= 1) 
		{
			if (i & bitm) j++;
			j <<= 1;
		}

		if (i < j) 
		{
			p1 = fftBuffer+i; 
			p2 = fftBuffer+j;
			temp = *p1; 
			*(p1++) = *p2;
			*(p2++) = temp; 
			temp = *p1;
			*p1 = *p2; 
			*p2 = temp;
		}
	}
	for (k = 0, le = 2; k < fftFrameSizeLog; k++) 
	{
		le <<= 1;
		le2 = le >> 1;
		ur = 1.0;
		ui = 0.0;
		arg = (float)(M_PI / (le2 >> 1));
		wr = (float)cos(arg);
		wi = sign * (float)sin(arg);
		for (j = 0; j < le2; j += 2) 
		{
			p1r = fftBuffer + j; 
			p1i = p1r + 1;
			p2r = p1r + le2; 
			p2i = p2r + 1;
			for (i = j; i < 2 * fftFrameSize; i += le) 
			{
				tr = *p2r * ur - *p2i * ui;
				ti = *p2r * ui + *p2i * ur;
				*p2r = *p1r - tr; 
				*p2i = *p1i - ti;
				*p1r += tr; 
				*p1i += ti;
				p1r += le; 
				p1i += le;
				p2r += le; 
				p2i += le;
			}
			tr = ur * wr - ui * wi;
			ui = ur * wi + ui * wr;
			ur = tr;
		}
	}
}


void plot_diff(const char *aFilename, int aSampleCount, int aHeight, float *aSrc1, float *aSrc2, int aColor1, int aColor2, int aBgColor, int aGridColor)
{
	int width = aSampleCount + 512;
	int *bitmap = new int[width * aHeight];
	int i;
	for (i = 0; i < width * aHeight; i++)
	{
		bitmap[i] = aBgColor;
	}

	for (i = 0; i < aSampleCount; i++)
	{
		bitmap[(aHeight / 2) * width + i] = aGridColor;

		if (i & 1)
		{
			bitmap[((aHeight * 1) / 4) * width + i] = aGridColor;
			bitmap[((aHeight * 3) / 4) * width + i] = aGridColor;
		}

		if ((i & 3) == 0)
		{
			bitmap[((aHeight * 1) / 8) * width + i] = aGridColor;
			bitmap[((aHeight * 3) / 8) * width + i] = aGridColor;
			bitmap[((aHeight * 5) / 8) * width + i] = aGridColor;
			bitmap[((aHeight * 7) / 8) * width + i] = aGridColor;
		}

		if ((i & 7) == 0)
		{
			bitmap[((aHeight * 1) / 16) * width + i] = aGridColor;
			bitmap[((aHeight * 3) / 16) * width + i] = aGridColor;
			bitmap[((aHeight * 5) / 16) * width + i] = aGridColor;
			bitmap[((aHeight * 7) / 16) * width + i] = aGridColor;
			bitmap[((aHeight * 9) / 16) * width + i] = aGridColor;
			bitmap[((aHeight * 11) / 16) * width + i] = aGridColor;
			bitmap[((aHeight * 13) / 16) * width + i] = aGridColor;
			bitmap[((aHeight * 15) / 16) * width + i] = aGridColor;
		}
	}

	for (i = 0; i < aHeight; i++)
	{
		bitmap[i * width + aSampleCount / 2] = aGridColor;

		if (i & 1)
		{
			bitmap[i * width + (aSampleCount * 1) / 4] = aGridColor;
			bitmap[i * width + (aSampleCount * 3) / 4] = aGridColor;
		}

		if ((i & 3) == 0)
		{
			bitmap[i * width + (aSampleCount * 1) / 8] = aGridColor;
			bitmap[i * width + (aSampleCount * 3) / 8] = aGridColor;
			bitmap[i * width + (aSampleCount * 5) / 8] = aGridColor;
			bitmap[i * width + (aSampleCount * 7) / 8] = aGridColor;
		}

		if ((i & 7) == 0)
		{
			bitmap[i * width + (aSampleCount * 1) / 16] = aGridColor;
			bitmap[i * width + (aSampleCount * 3) / 16] = aGridColor;
			bitmap[i * width + (aSampleCount * 5) / 16] = aGridColor;
			bitmap[i * width + (aSampleCount * 7) / 16] = aGridColor;
			bitmap[i * width + (aSampleCount * 9) / 16] = aGridColor;
			bitmap[i * width + (aSampleCount * 11) / 16] = aGridColor;
			bitmap[i * width + (aSampleCount * 13) / 16] = aGridColor;
			bitmap[i * width + (aSampleCount * 15) / 16] = aGridColor;
		}
	}

	for (i = 0; i < aSampleCount-1; i++)
	{
		if (aSrc1[i] > -2 && aSrc1[i] < 2 && aSrc1[i+1] > -2 && aSrc1[i+1] < 2)
		{
			float v1 = 0.5f - (aSrc1[i] + 1) / 4 + 0.25f;
			float v2 = 0.5f - (aSrc1[i+1] + 1) / 4 + 0.25f;
			v1 *= aHeight;
			v2 *= aHeight;
			if (v1 > v2)
			{
				float t = v1;
				v1 = v2;
				v2 = t;
			}
			float j;
			for (j = v1; j <= v2; j++)
			{
				bitmap[(int)floor(j) * width + i] = aColor1;
			}
		}

		if (aSrc2[i] > -2 && aSrc2[i] < 2 && aSrc2[i+1] > -2 && aSrc2[i+1] < 2)
		{
			float v1 = 0.5f - (aSrc2[i] + 1) / 4 + 0.25f;
			float v2 = 0.5f - (aSrc2[i+1] + 1) / 4 + 0.25f;
			v1 *= aHeight;
			v2 *= aHeight;
			if (v1 > v2)
			{
				float t = v1;
				v1 = v2;
				v2 = t;
			}
			float j;
			for (j = v1; j <= v2; j++)
			{
				bitmap[(int)floor(j) * width + i] = aColor2;
			}
		}
	}

	drawstring("SoLoud Resampler Lab - http://soloud-audio.com", 0, 0, bitmap, width, 0xff000000);
	drawstring(aFilename, 0, 12, bitmap, width, 0xff000000);
	char tempstr[1024];

	/*
	float maxdiff = 0;
	float diffsum = 0;
	for (i = 0; i < aSampleCount/2; i++)
	{
		float diff = fabs(aSrc1[i+aSampleCount/4]-aSrc2[i+aSampleCount/4]);
		if (diff > maxdiff) maxdiff = diff;
		diffsum += diff;
	}
	sprintf(tempstr, "Avg diff:%3.7f", diffsum / aSampleCount);
	drawstring(tempstr, 0, aHeight-12*2, bitmap, width, 0xff000000);
	sprintf(tempstr, "Max diff:%3.7f", maxdiff);
	drawstring(tempstr, 0, aHeight-12*1, bitmap, width, 0xff000000);
	*/

	float maxdiff_d = 0;
	float diffsum_d = 0;
	for (i = 0; i < aSampleCount/2; i++)
	{
		float d1 = aSrc1[i+aSampleCount/4 + 1] - aSrc1[i+aSampleCount/4];
		float d2 = aSrc2[i+aSampleCount/4 + 1] - aSrc2[i+aSampleCount/4];
		float diff = (float)fabs(d1 - d2);
		if (diff > maxdiff_d) maxdiff_d = diff;
		diffsum_d += diff;
	}
	
	sprintf(tempstr, "Avg d diff:%3.7f", diffsum_d / aSampleCount);
	drawstring(tempstr, aSampleCount/2, aHeight-12*2, bitmap, width, 0xff000000);
	sprintf(tempstr, "Max d diff:%3.7f", maxdiff_d);
	drawstring(tempstr, aSampleCount/2, aHeight-12*1, bitmap, width, 0xff000000);

	float temp[2048];
	float fftdata[512];
	for (i = 0; i < 256; i++)
	{
		temp[i*2] = aSrc1[i];
		temp[i*2+1] = 0;
		temp[i+512] = 0;
		temp[i+768] = 0;
		temp[i+1024] = 0;
		temp[i+1280] = 0;
		temp[i+1536] = 0;
		temp[i+1792] = 0;
	}

	smbFft(temp, 10, -1);

	for (i = 0; i < 512; i++)
	{
		float real = temp[i*2];
		float imag = temp[i*2+1];
		fftdata[i] = (float)sqrt(real*real+imag*imag);
	}
	
	for (i = 0; i < 512; i++)
	{			
		int v = (aHeight/2)-(int)floor(fftdata[i] * (aHeight/50));
		if (v < 0) v = 0;
		if (v > aHeight/2) v = aHeight/2;
		int j;
		for (j = v; j < aHeight/2; j++)
		{
			bitmap[aSampleCount + i + j * width] = aColor1;
		}
	}
	
	for (i = 0; i < 256; i++)
	{
		temp[i*2] = aSrc2[i];
		temp[i*2+1] = 0;
		temp[i+512] = 0;
		temp[i+768] = 0;
		temp[i+1024] = 0;
		temp[i+1280] = 0;
		temp[i+1536] = 0;
		temp[i+1792] = 0;
	}

	smbFft(temp, 10, -1);

	for (i = 0; i < 512; i++)
	{
		float real = temp[i*2];
		float imag = temp[i*2+1];
		fftdata[i] = (float)sqrt(real*real+imag*imag);
	}
	
	for (i = 0; i < 512; i++)
	{			
		int v = aHeight-(int)floor(fftdata[i] * (aHeight/50));
		if (v < aHeight/2) v = aHeight/2;
		if (v > aHeight) v = aHeight;
		int j;
		for (j = v; j < aHeight; j++)
		{
			bitmap[aSampleCount + i + j * width] = aColor2;
		}
	}
	

	stbi_write_png(aFilename, width, aHeight, 4, bitmap, width * 4);
	delete[] bitmap;
}

float saw(float v)
{
	float t = v / TAU;
	t = t - (float)floor(t);
	return (float)(t-0.5)*2;
}

float square(float v)
{
	float t = v / TAU;
	t = t - (float)floor(t);
	if (t > 0.5) return 1;
	return -1;
}

char *resamplername[MAX_RESAMPLER] =
{
	"point",
	"linear",
	"catmull-rom",
	"sinc6",
	"gauss5",
	"experiment"
};

void upsampletest(int aResampler, int aFunction, float aMultiplier, FILE *aIndexf, FILE *aIndexfs)
{
	float *a, *b, *temp;
	int i;
	int src_samples = (int)floor(512/aMultiplier)+1;

	a = new float[512];
	b = new float[512];
	temp = new float[src_samples];
	
	char *func = "";
	char *samp = resamplername[aResampler];

	switch (aFunction)
	{
	//case 0:
	default:
		func = "sin";
		for (i = 0; i < 512; i++)
		{
			a[i] = (float)sin(4 * i / 512.0f * TAU);
		}
		for (i = 0; i < src_samples; i++)
		{
			temp[i] = (float)sin(4 * i / 512.0f * TAU * aMultiplier);
		}
		break;
	case 1:
		func = "saw";
		for (i = 0; i < 512; i++)
		{
			a[i] = saw(4 * i / 512.0f * TAU);
		}
		for (i = 0; i < src_samples; i++)
		{
			temp[i] = saw(4 * i / 512.0f * TAU * aMultiplier);
		}
		break;
	case 2:
		func = "sqr";
		for (i = 0; i < 512; i++)
		{
			a[i] = square(4 * i / 512.0f * TAU);
		}
		for (i = 0; i < src_samples; i++)
		{
			temp[i] = square(4 * i / 512.0f * TAU * aMultiplier);
		}
		break;
	}

	int step_fixed = (int)floor(FIXPOINT_FRAC_MUL / aMultiplier);

	int curr = 0;
	int prev = 0;
	int samples_out = 0;
	int mSrcOffset = 0;
	

	while (samples_out < 512)
	{
		int writesamples;

		writesamples = ((512 * FIXPOINT_FRAC_MUL) - mSrcOffset) / step_fixed + 1;

		// avoid reading past the current buffer..
		if (((writesamples * step_fixed + mSrcOffset) >> FIXPOINT_FRAC_BITS) >= SAMPLE_GRANULARITY + 1)
			writesamples--;

		if (writesamples > (512 - samples_out)) writesamples = 512 - samples_out;

		switch (aResampler)
		{
		//case 0:
		default:
			resample_pointsample(temp + curr, temp + prev, b + samples_out, mSrcOffset, writesamples, 44100 / aMultiplier, 44100, step_fixed);
			break;
		case 1:
			resample_linear(temp + curr, temp + prev, b + samples_out, mSrcOffset, writesamples, 44100 / aMultiplier, 44100, step_fixed);
			break;
		case 2:
			resample_catmullrom(temp + curr, temp + prev, b + samples_out, mSrcOffset, writesamples, 44100 / aMultiplier, 44100, step_fixed);
			break;
		case 3:
			resample_sinc6(temp + curr, temp + prev, b + samples_out, mSrcOffset, writesamples, 44100 / aMultiplier, 44100, step_fixed);
			break;
		case 4:
			resample_gauss5(temp + curr, temp + prev, b + samples_out, mSrcOffset, writesamples, 44100 / aMultiplier, 44100, step_fixed);
			break;
		case 5:
			resample_experiment(temp + curr, temp + prev, b + samples_out, mSrcOffset, writesamples, 44100 / aMultiplier, 44100, step_fixed);
			break;
		}
		samples_out += writesamples;
		mSrcOffset += writesamples * step_fixed;
		prev = curr;
		curr += 512;
		mSrcOffset = mSrcOffset & FIXPOINT_FRAC_MASK;
		//if (mSrcOffset < 0) mSrcOffset = 0;
		//while (mSrcOffset < 0) mSrcOffset += step_fixed;
	}

	char tempstr[1024];
	if (aMultiplier >= 1)
	{
		sprintf(tempstr, "%s_%s_%dx.png", samp, func, (int)aMultiplier);
	}
	else
	{
		sprintf(tempstr, "%s_%s_0_%04dx.png", samp, func, (int)(aMultiplier*10000));
	}

	plot_diff(tempstr, 512, 256, a, b, 0xff0000ff, 0xffff0000, 0xffffffff, 0xffcccccc);

	fprintf(aIndexf, "<img src=\"%s\">&nbsp;", tempstr);
	fprintf(aIndexfs, "<img src=\"%s\">&nbsp;", tempstr);

	delete[] a;
	delete[] b;
	delete[] temp;
	printf(".");
}


int main(int parc, char ** pars)
{
	setbuf(stdout, NULL);
	FILE * indexfs[MAX_RESAMPLER];
	int i;
	for (i = 0; i < MAX_RESAMPLER; i++)
	{
		char temp[200];
		sprintf(temp, "index_%s.html", resamplername[i]);
		indexfs[i] = fopen(temp, "w");
		fprintf(indexfs[i],
			"<html>\n<body>\n");
	}
	FILE * indexf = fopen("index.html", "w");
	fprintf(indexf,
		"<html>\n<body>\n");
	int func, samp, k;
	for (func = 0; func < MAX_FUNC; func++)
	{
		for (k = 0; k < 5; k++)
		{
			for (samp = 0; samp < MAX_RESAMPLER; samp++)
			{
				upsampletest(samp, func, (float)(k * k * 3 + 2), indexf, indexfs[samp]);
			}
			fprintf(indexf, "<br><br>\n");
		}
		for (k = 0; k < 5; k++)
		{
			for (samp = 0; samp < MAX_RESAMPLER; samp++)
			{
				upsampletest(samp, func, (float)(1.0f / (k * k * 3 + 2)), indexf, indexfs[samp]);
			}
			fprintf(indexf, "<br><br>\n");
		}
	}
	fprintf(indexf,"</body>\n</html>\n");
	fclose(indexf);
	for (i = 0; i < MAX_RESAMPLER; i++)
	{
		fprintf(indexfs[i], "</body>\n</html>\n");
		fclose(indexfs[i]);
	}
}