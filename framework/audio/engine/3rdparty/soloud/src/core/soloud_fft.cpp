/*
SoLoud audio engine
Copyright (c) 2013-2018 Jari Komppa

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

// FFT based on fftreal by Laurent de Soras, under WTFPL

#include "soloud.h"
#include "soloud_fft.h"
#include <string.h>

namespace fftimpl
{
// Based on http://www.kurims.kyoto-u.ac.jp/~ooura/fft.html
// "Copyright Takuya OOURA, 1996-2001 
//  You may use, copy, modify and distribute this code for any purpose 
//  (include commercial use) and without fee. Please refer to this package 
//  when you modify this code."


#ifndef M_PI_2
#define M_PI_2      1.570796326794896619231321691639751442098584699687f
#endif
#ifndef WR5000  /* cos(M_PI_2*0.5000) */
#define WR5000      0.707106781186547524400844362104849039284835937688f
#endif
#ifndef WR2500  /* cos(M_PI_2*0.2500) */
#define WR2500      0.923879532511286756128183189396788286822416625863f
#endif
#ifndef WI2500  /* sin(M_PI_2*0.2500) */
#define WI2500      0.382683432365089771728459984030398866761344562485f
#endif
#ifndef WR1250  /* cos(M_PI_2*0.1250) */
#define WR1250      0.980785280403230449126182236134239036973933730893f
#endif
#ifndef WI1250  /* sin(M_PI_2*0.1250) */
#define WI1250      0.195090322016128267848284868477022240927691617751f
#endif
#ifndef WR3750  /* cos(M_PI_2*0.3750) */
#define WR3750      0.831469612302545237078788377617905756738560811987f
#endif
#ifndef WI3750  /* sin(M_PI_2*0.3750) */
#define WI3750      0.555570233019602224742830813948532874374937190754f
#endif

#ifndef CDFT_LOOP_DIV  /* control of the CDFT's speed & tolerance */
#define CDFT_LOOP_DIV 32
#endif

#ifndef RDFT_LOOP_DIV  /* control of the RDFT's speed & tolerance */
#define RDFT_LOOP_DIV 64
#endif

#ifndef DCST_LOOP_DIV  /* control of the DCT,DST's speed & tolerance */
#define DCST_LOOP_DIV 64
#endif

	void bitrv2(int n, float *a)
	{
		int j0, k0, j1, k1, l, m, i, j, k, nh;
		float xr, xi, yr, yi;

		m = 4;
		for (l = n >> 2; l > 8; l >>= 2) {
			m <<= 1;
		}
		nh = n >> 1;
		if (l == 8) {
			j0 = 0;
			for (k0 = 0; k0 < m; k0 += 4) {
				k = k0;
				for (j = j0; j < j0 + k0; j += 4) {
					xr = a[j];
					xi = a[j + 1];
					yr = a[k];
					yi = a[k + 1];
					a[j] = yr;
					a[j + 1] = yi;
					a[k] = xr;
					a[k + 1] = xi;
					j1 = j + m;
					k1 = k + 2 * m;
					xr = a[j1];
					xi = a[j1 + 1];
					yr = a[k1];
					yi = a[k1 + 1];
					a[j1] = yr;
					a[j1 + 1] = yi;
					a[k1] = xr;
					a[k1 + 1] = xi;
					j1 += m;
					k1 -= m;
					xr = a[j1];
					xi = a[j1 + 1];
					yr = a[k1];
					yi = a[k1 + 1];
					a[j1] = yr;
					a[j1 + 1] = yi;
					a[k1] = xr;
					a[k1 + 1] = xi;
					j1 += m;
					k1 += 2 * m;
					xr = a[j1];
					xi = a[j1 + 1];
					yr = a[k1];
					yi = a[k1 + 1];
					a[j1] = yr;
					a[j1 + 1] = yi;
					a[k1] = xr;
					a[k1 + 1] = xi;
					j1 += nh;
					k1 += 2;
					xr = a[j1];
					xi = a[j1 + 1];
					yr = a[k1];
					yi = a[k1 + 1];
					a[j1] = yr;
					a[j1 + 1] = yi;
					a[k1] = xr;
					a[k1 + 1] = xi;
					j1 -= m;
					k1 -= 2 * m;
					xr = a[j1];
					xi = a[j1 + 1];
					yr = a[k1];
					yi = a[k1 + 1];
					a[j1] = yr;
					a[j1 + 1] = yi;
					a[k1] = xr;
					a[k1 + 1] = xi;
					j1 -= m;
					k1 += m;
					xr = a[j1];
					xi = a[j1 + 1];
					yr = a[k1];
					yi = a[k1 + 1];
					a[j1] = yr;
					a[j1 + 1] = yi;
					a[k1] = xr;
					a[k1 + 1] = xi;
					j1 -= m;
					k1 -= 2 * m;
					xr = a[j1];
					xi = a[j1 + 1];
					yr = a[k1];
					yi = a[k1 + 1];
					a[j1] = yr;
					a[j1 + 1] = yi;
					a[k1] = xr;
					a[k1 + 1] = xi;
					j1 += 2;
					k1 += nh;
					xr = a[j1];
					xi = a[j1 + 1];
					yr = a[k1];
					yi = a[k1 + 1];
					a[j1] = yr;
					a[j1 + 1] = yi;
					a[k1] = xr;
					a[k1 + 1] = xi;
					j1 += m;
					k1 += 2 * m;
					xr = a[j1];
					xi = a[j1 + 1];
					yr = a[k1];
					yi = a[k1 + 1];
					a[j1] = yr;
					a[j1 + 1] = yi;
					a[k1] = xr;
					a[k1 + 1] = xi;
					j1 += m;
					k1 -= m;
					xr = a[j1];
					xi = a[j1 + 1];
					yr = a[k1];
					yi = a[k1 + 1];
					a[j1] = yr;
					a[j1 + 1] = yi;
					a[k1] = xr;
					a[k1 + 1] = xi;
					j1 += m;
					k1 += 2 * m;
					xr = a[j1];
					xi = a[j1 + 1];
					yr = a[k1];
					yi = a[k1 + 1];
					a[j1] = yr;
					a[j1 + 1] = yi;
					a[k1] = xr;
					a[k1 + 1] = xi;
					j1 -= nh;
					k1 -= 2;
					xr = a[j1];
					xi = a[j1 + 1];
					yr = a[k1];
					yi = a[k1 + 1];
					a[j1] = yr;
					a[j1 + 1] = yi;
					a[k1] = xr;
					a[k1 + 1] = xi;
					j1 -= m;
					k1 -= 2 * m;
					xr = a[j1];
					xi = a[j1 + 1];
					yr = a[k1];
					yi = a[k1 + 1];
					a[j1] = yr;
					a[j1 + 1] = yi;
					a[k1] = xr;
					a[k1 + 1] = xi;
					j1 -= m;
					k1 += m;
					xr = a[j1];
					xi = a[j1 + 1];
					yr = a[k1];
					yi = a[k1 + 1];
					a[j1] = yr;
					a[j1 + 1] = yi;
					a[k1] = xr;
					a[k1 + 1] = xi;
					j1 -= m;
					k1 -= 2 * m;
					xr = a[j1];
					xi = a[j1 + 1];
					yr = a[k1];
					yi = a[k1 + 1];
					a[j1] = yr;
					a[j1 + 1] = yi;
					a[k1] = xr;
					a[k1 + 1] = xi;
					for (i = nh >> 1; i > (k ^= i); i >>= 1);
				}
				k1 = j0 + k0;
				j1 = k1 + 2;
				k1 += nh;
				xr = a[j1];
				xi = a[j1 + 1];
				yr = a[k1];
				yi = a[k1 + 1];
				a[j1] = yr;
				a[j1 + 1] = yi;
				a[k1] = xr;
				a[k1 + 1] = xi;
				j1 += m;
				k1 += 2 * m;
				xr = a[j1];
				xi = a[j1 + 1];
				yr = a[k1];
				yi = a[k1 + 1];
				a[j1] = yr;
				a[j1 + 1] = yi;
				a[k1] = xr;
				a[k1 + 1] = xi;
				j1 += m;
				k1 -= m;
				xr = a[j1];
				xi = a[j1 + 1];
				yr = a[k1];
				yi = a[k1 + 1];
				a[j1] = yr;
				a[j1 + 1] = yi;
				a[k1] = xr;
				a[k1 + 1] = xi;
				j1 -= 2;
				k1 -= nh;
				xr = a[j1];
				xi = a[j1 + 1];
				yr = a[k1];
				yi = a[k1 + 1];
				a[j1] = yr;
				a[j1 + 1] = yi;
				a[k1] = xr;
				a[k1 + 1] = xi;
				j1 += nh + 2;
				k1 += nh + 2;
				xr = a[j1];
				xi = a[j1 + 1];
				yr = a[k1];
				yi = a[k1 + 1];
				a[j1] = yr;
				a[j1 + 1] = yi;
				a[k1] = xr;
				a[k1 + 1] = xi;
				j1 -= nh - m;
				k1 += 2 * m - 2;
				xr = a[j1];
				xi = a[j1 + 1];
				yr = a[k1];
				yi = a[k1 + 1];
				a[j1] = yr;
				a[j1 + 1] = yi;
				a[k1] = xr;
				a[k1 + 1] = xi;
				for (i = nh >> 1; i > (j0 ^= i); i >>= 1);
			}
		}
		else {
			j0 = 0;
			for (k0 = 0; k0 < m; k0 += 4) {
				k = k0;
				for (j = j0; j < j0 + k0; j += 4) {
					xr = a[j];
					xi = a[j + 1];
					yr = a[k];
					yi = a[k + 1];
					a[j] = yr;
					a[j + 1] = yi;
					a[k] = xr;
					a[k + 1] = xi;
					j1 = j + m;
					k1 = k + m;
					xr = a[j1];
					xi = a[j1 + 1];
					yr = a[k1];
					yi = a[k1 + 1];
					a[j1] = yr;
					a[j1 + 1] = yi;
					a[k1] = xr;
					a[k1 + 1] = xi;
					j1 += nh;
					k1 += 2;
					xr = a[j1];
					xi = a[j1 + 1];
					yr = a[k1];
					yi = a[k1 + 1];
					a[j1] = yr;
					a[j1 + 1] = yi;
					a[k1] = xr;
					a[k1 + 1] = xi;
					j1 -= m;
					k1 -= m;
					xr = a[j1];
					xi = a[j1 + 1];
					yr = a[k1];
					yi = a[k1 + 1];
					a[j1] = yr;
					a[j1 + 1] = yi;
					a[k1] = xr;
					a[k1 + 1] = xi;
					j1 += 2;
					k1 += nh;
					xr = a[j1];
					xi = a[j1 + 1];
					yr = a[k1];
					yi = a[k1 + 1];
					a[j1] = yr;
					a[j1 + 1] = yi;
					a[k1] = xr;
					a[k1 + 1] = xi;
					j1 += m;
					k1 += m;
					xr = a[j1];
					xi = a[j1 + 1];
					yr = a[k1];
					yi = a[k1 + 1];
					a[j1] = yr;
					a[j1 + 1] = yi;
					a[k1] = xr;
					a[k1 + 1] = xi;
					j1 -= nh;
					k1 -= 2;
					xr = a[j1];
					xi = a[j1 + 1];
					yr = a[k1];
					yi = a[k1 + 1];
					a[j1] = yr;
					a[j1 + 1] = yi;
					a[k1] = xr;
					a[k1 + 1] = xi;
					j1 -= m;
					k1 -= m;
					xr = a[j1];
					xi = a[j1 + 1];
					yr = a[k1];
					yi = a[k1 + 1];
					a[j1] = yr;
					a[j1 + 1] = yi;
					a[k1] = xr;
					a[k1 + 1] = xi;
					for (i = nh >> 1; i > (k ^= i); i >>= 1);
				}
				k1 = j0 + k0;
				j1 = k1 + 2;
				k1 += nh;
				xr = a[j1];
				xi = a[j1 + 1];
				yr = a[k1];
				yi = a[k1 + 1];
				a[j1] = yr;
				a[j1 + 1] = yi;
				a[k1] = xr;
				a[k1 + 1] = xi;
				j1 += m;
				k1 += m;
				xr = a[j1];
				xi = a[j1 + 1];
				yr = a[k1];
				yi = a[k1 + 1];
				a[j1] = yr;
				a[j1 + 1] = yi;
				a[k1] = xr;
				a[k1 + 1] = xi;
				for (i = nh >> 1; i > (j0 ^= i); i >>= 1);
			}
		}
	}


	void bitrv2conj(int n, float *a)
	{
		int j0, k0, j1, k1, l, m, i, j, k, nh;
		float xr, xi, yr, yi;

		m = 4;
		for (l = n >> 2; l > 8; l >>= 2) {
			m <<= 1;
		}
		nh = n >> 1;
		if (l == 8) {
			j0 = 0;
			for (k0 = 0; k0 < m; k0 += 4) {
				k = k0;
				for (j = j0; j < j0 + k0; j += 4) {
					xr = a[j];
					xi = -a[j + 1];
					yr = a[k];
					yi = -a[k + 1];
					a[j] = yr;
					a[j + 1] = yi;
					a[k] = xr;
					a[k + 1] = xi;
					j1 = j + m;
					k1 = k + 2 * m;
					xr = a[j1];
					xi = -a[j1 + 1];
					yr = a[k1];
					yi = -a[k1 + 1];
					a[j1] = yr;
					a[j1 + 1] = yi;
					a[k1] = xr;
					a[k1 + 1] = xi;
					j1 += m;
					k1 -= m;
					xr = a[j1];
					xi = -a[j1 + 1];
					yr = a[k1];
					yi = -a[k1 + 1];
					a[j1] = yr;
					a[j1 + 1] = yi;
					a[k1] = xr;
					a[k1 + 1] = xi;
					j1 += m;
					k1 += 2 * m;
					xr = a[j1];
					xi = -a[j1 + 1];
					yr = a[k1];
					yi = -a[k1 + 1];
					a[j1] = yr;
					a[j1 + 1] = yi;
					a[k1] = xr;
					a[k1 + 1] = xi;
					j1 += nh;
					k1 += 2;
					xr = a[j1];
					xi = -a[j1 + 1];
					yr = a[k1];
					yi = -a[k1 + 1];
					a[j1] = yr;
					a[j1 + 1] = yi;
					a[k1] = xr;
					a[k1 + 1] = xi;
					j1 -= m;
					k1 -= 2 * m;
					xr = a[j1];
					xi = -a[j1 + 1];
					yr = a[k1];
					yi = -a[k1 + 1];
					a[j1] = yr;
					a[j1 + 1] = yi;
					a[k1] = xr;
					a[k1 + 1] = xi;
					j1 -= m;
					k1 += m;
					xr = a[j1];
					xi = -a[j1 + 1];
					yr = a[k1];
					yi = -a[k1 + 1];
					a[j1] = yr;
					a[j1 + 1] = yi;
					a[k1] = xr;
					a[k1 + 1] = xi;
					j1 -= m;
					k1 -= 2 * m;
					xr = a[j1];
					xi = -a[j1 + 1];
					yr = a[k1];
					yi = -a[k1 + 1];
					a[j1] = yr;
					a[j1 + 1] = yi;
					a[k1] = xr;
					a[k1 + 1] = xi;
					j1 += 2;
					k1 += nh;
					xr = a[j1];
					xi = -a[j1 + 1];
					yr = a[k1];
					yi = -a[k1 + 1];
					a[j1] = yr;
					a[j1 + 1] = yi;
					a[k1] = xr;
					a[k1 + 1] = xi;
					j1 += m;
					k1 += 2 * m;
					xr = a[j1];
					xi = -a[j1 + 1];
					yr = a[k1];
					yi = -a[k1 + 1];
					a[j1] = yr;
					a[j1 + 1] = yi;
					a[k1] = xr;
					a[k1 + 1] = xi;
					j1 += m;
					k1 -= m;
					xr = a[j1];
					xi = -a[j1 + 1];
					yr = a[k1];
					yi = -a[k1 + 1];
					a[j1] = yr;
					a[j1 + 1] = yi;
					a[k1] = xr;
					a[k1 + 1] = xi;
					j1 += m;
					k1 += 2 * m;
					xr = a[j1];
					xi = -a[j1 + 1];
					yr = a[k1];
					yi = -a[k1 + 1];
					a[j1] = yr;
					a[j1 + 1] = yi;
					a[k1] = xr;
					a[k1 + 1] = xi;
					j1 -= nh;
					k1 -= 2;
					xr = a[j1];
					xi = -a[j1 + 1];
					yr = a[k1];
					yi = -a[k1 + 1];
					a[j1] = yr;
					a[j1 + 1] = yi;
					a[k1] = xr;
					a[k1 + 1] = xi;
					j1 -= m;
					k1 -= 2 * m;
					xr = a[j1];
					xi = -a[j1 + 1];
					yr = a[k1];
					yi = -a[k1 + 1];
					a[j1] = yr;
					a[j1 + 1] = yi;
					a[k1] = xr;
					a[k1 + 1] = xi;
					j1 -= m;
					k1 += m;
					xr = a[j1];
					xi = -a[j1 + 1];
					yr = a[k1];
					yi = -a[k1 + 1];
					a[j1] = yr;
					a[j1 + 1] = yi;
					a[k1] = xr;
					a[k1 + 1] = xi;
					j1 -= m;
					k1 -= 2 * m;
					xr = a[j1];
					xi = -a[j1 + 1];
					yr = a[k1];
					yi = -a[k1 + 1];
					a[j1] = yr;
					a[j1 + 1] = yi;
					a[k1] = xr;
					a[k1 + 1] = xi;
					for (i = nh >> 1; i > (k ^= i); i >>= 1);
				}
				k1 = j0 + k0;
				j1 = k1 + 2;
				k1 += nh;
				a[j1 - 1] = -a[j1 - 1];
				xr = a[j1];
				xi = -a[j1 + 1];
				yr = a[k1];
				yi = -a[k1 + 1];
				a[j1] = yr;
				a[j1 + 1] = yi;
				a[k1] = xr;
				a[k1 + 1] = xi;
				a[k1 + 3] = -a[k1 + 3];
				j1 += m;
				k1 += 2 * m;
				xr = a[j1];
				xi = -a[j1 + 1];
				yr = a[k1];
				yi = -a[k1 + 1];
				a[j1] = yr;
				a[j1 + 1] = yi;
				a[k1] = xr;
				a[k1 + 1] = xi;
				j1 += m;
				k1 -= m;
				xr = a[j1];
				xi = -a[j1 + 1];
				yr = a[k1];
				yi = -a[k1 + 1];
				a[j1] = yr;
				a[j1 + 1] = yi;
				a[k1] = xr;
				a[k1 + 1] = xi;
				j1 -= 2;
				k1 -= nh;
				xr = a[j1];
				xi = -a[j1 + 1];
				yr = a[k1];
				yi = -a[k1 + 1];
				a[j1] = yr;
				a[j1 + 1] = yi;
				a[k1] = xr;
				a[k1 + 1] = xi;
				j1 += nh + 2;
				k1 += nh + 2;
				xr = a[j1];
				xi = -a[j1 + 1];
				yr = a[k1];
				yi = -a[k1 + 1];
				a[j1] = yr;
				a[j1 + 1] = yi;
				a[k1] = xr;
				a[k1 + 1] = xi;
				j1 -= nh - m;
				k1 += 2 * m - 2;
				a[j1 - 1] = -a[j1 - 1];
				xr = a[j1];
				xi = -a[j1 + 1];
				yr = a[k1];
				yi = -a[k1 + 1];
				a[j1] = yr;
				a[j1 + 1] = yi;
				a[k1] = xr;
				a[k1 + 1] = xi;
				a[k1 + 3] = -a[k1 + 3];
				for (i = nh >> 1; i > (j0 ^= i); i >>= 1);
			}
		}
		else {
			j0 = 0;
			for (k0 = 0; k0 < m; k0 += 4) {
				k = k0;
				for (j = j0; j < j0 + k0; j += 4) {
					xr = a[j];
					xi = -a[j + 1];
					yr = a[k];
					yi = -a[k + 1];
					a[j] = yr;
					a[j + 1] = yi;
					a[k] = xr;
					a[k + 1] = xi;
					j1 = j + m;
					k1 = k + m;
					xr = a[j1];
					xi = -a[j1 + 1];
					yr = a[k1];
					yi = -a[k1 + 1];
					a[j1] = yr;
					a[j1 + 1] = yi;
					a[k1] = xr;
					a[k1 + 1] = xi;
					j1 += nh;
					k1 += 2;
					xr = a[j1];
					xi = -a[j1 + 1];
					yr = a[k1];
					yi = -a[k1 + 1];
					a[j1] = yr;
					a[j1 + 1] = yi;
					a[k1] = xr;
					a[k1 + 1] = xi;
					j1 -= m;
					k1 -= m;
					xr = a[j1];
					xi = -a[j1 + 1];
					yr = a[k1];
					yi = -a[k1 + 1];
					a[j1] = yr;
					a[j1 + 1] = yi;
					a[k1] = xr;
					a[k1 + 1] = xi;
					j1 += 2;
					k1 += nh;
					xr = a[j1];
					xi = -a[j1 + 1];
					yr = a[k1];
					yi = -a[k1 + 1];
					a[j1] = yr;
					a[j1 + 1] = yi;
					a[k1] = xr;
					a[k1 + 1] = xi;
					j1 += m;
					k1 += m;
					xr = a[j1];
					xi = -a[j1 + 1];
					yr = a[k1];
					yi = -a[k1 + 1];
					a[j1] = yr;
					a[j1 + 1] = yi;
					a[k1] = xr;
					a[k1 + 1] = xi;
					j1 -= nh;
					k1 -= 2;
					xr = a[j1];
					xi = -a[j1 + 1];
					yr = a[k1];
					yi = -a[k1 + 1];
					a[j1] = yr;
					a[j1 + 1] = yi;
					a[k1] = xr;
					a[k1 + 1] = xi;
					j1 -= m;
					k1 -= m;
					xr = a[j1];
					xi = -a[j1 + 1];
					yr = a[k1];
					yi = -a[k1 + 1];
					a[j1] = yr;
					a[j1 + 1] = yi;
					a[k1] = xr;
					a[k1 + 1] = xi;
					for (i = nh >> 1; i > (k ^= i); i >>= 1);
				}
				k1 = j0 + k0;
				j1 = k1 + 2;
				k1 += nh;
				a[j1 - 1] = -a[j1 - 1];
				xr = a[j1];
				xi = -a[j1 + 1];
				yr = a[k1];
				yi = -a[k1 + 1];
				a[j1] = yr;
				a[j1 + 1] = yi;
				a[k1] = xr;
				a[k1 + 1] = xi;
				a[k1 + 3] = -a[k1 + 3];
				j1 += m;
				k1 += m;
				a[j1 - 1] = -a[j1 - 1];
				xr = a[j1];
				xi = -a[j1 + 1];
				yr = a[k1];
				yi = -a[k1 + 1];
				a[j1] = yr;
				a[j1 + 1] = yi;
				a[k1] = xr;
				a[k1 + 1] = xi;
				a[k1 + 3] = -a[k1 + 3];
				for (i = nh >> 1; i > (j0 ^= i); i >>= 1);
			}
		}
	}


	void bitrv216(float *a)
	{
		float x1r, x1i, x2r, x2i, x3r, x3i, x4r, x4i,
			x5r, x5i, x7r, x7i, x8r, x8i, x10r, x10i,
			x11r, x11i, x12r, x12i, x13r, x13i, x14r, x14i;

		x1r = a[2];
		x1i = a[3];
		x2r = a[4];
		x2i = a[5];
		x3r = a[6];
		x3i = a[7];
		x4r = a[8];
		x4i = a[9];
		x5r = a[10];
		x5i = a[11];
		x7r = a[14];
		x7i = a[15];
		x8r = a[16];
		x8i = a[17];
		x10r = a[20];
		x10i = a[21];
		x11r = a[22];
		x11i = a[23];
		x12r = a[24];
		x12i = a[25];
		x13r = a[26];
		x13i = a[27];
		x14r = a[28];
		x14i = a[29];
		a[2] = x8r;
		a[3] = x8i;
		a[4] = x4r;
		a[5] = x4i;
		a[6] = x12r;
		a[7] = x12i;
		a[8] = x2r;
		a[9] = x2i;
		a[10] = x10r;
		a[11] = x10i;
		a[14] = x14r;
		a[15] = x14i;
		a[16] = x1r;
		a[17] = x1i;
		a[20] = x5r;
		a[21] = x5i;
		a[22] = x13r;
		a[23] = x13i;
		a[24] = x3r;
		a[25] = x3i;
		a[26] = x11r;
		a[27] = x11i;
		a[28] = x7r;
		a[29] = x7i;
	}


	void bitrv216neg(float *a)
	{
		float x1r, x1i, x2r, x2i, x3r, x3i, x4r, x4i,
			x5r, x5i, x6r, x6i, x7r, x7i, x8r, x8i,
			x9r, x9i, x10r, x10i, x11r, x11i, x12r, x12i,
			x13r, x13i, x14r, x14i, x15r, x15i;

		x1r = a[2];
		x1i = a[3];
		x2r = a[4];
		x2i = a[5];
		x3r = a[6];
		x3i = a[7];
		x4r = a[8];
		x4i = a[9];
		x5r = a[10];
		x5i = a[11];
		x6r = a[12];
		x6i = a[13];
		x7r = a[14];
		x7i = a[15];
		x8r = a[16];
		x8i = a[17];
		x9r = a[18];
		x9i = a[19];
		x10r = a[20];
		x10i = a[21];
		x11r = a[22];
		x11i = a[23];
		x12r = a[24];
		x12i = a[25];
		x13r = a[26];
		x13i = a[27];
		x14r = a[28];
		x14i = a[29];
		x15r = a[30];
		x15i = a[31];
		a[2] = x15r;
		a[3] = x15i;
		a[4] = x7r;
		a[5] = x7i;
		a[6] = x11r;
		a[7] = x11i;
		a[8] = x3r;
		a[9] = x3i;
		a[10] = x13r;
		a[11] = x13i;
		a[12] = x5r;
		a[13] = x5i;
		a[14] = x9r;
		a[15] = x9i;
		a[16] = x1r;
		a[17] = x1i;
		a[18] = x14r;
		a[19] = x14i;
		a[20] = x6r;
		a[21] = x6i;
		a[22] = x10r;
		a[23] = x10i;
		a[24] = x2r;
		a[25] = x2i;
		a[26] = x12r;
		a[27] = x12i;
		a[28] = x4r;
		a[29] = x4i;
		a[30] = x8r;
		a[31] = x8i;
	}


	void bitrv208(float *a)
	{
		float x1r, x1i, x3r, x3i, x4r, x4i, x6r, x6i;

		x1r = a[2];
		x1i = a[3];
		x3r = a[6];
		x3i = a[7];
		x4r = a[8];
		x4i = a[9];
		x6r = a[12];
		x6i = a[13];
		a[2] = x4r;
		a[3] = x4i;
		a[6] = x6r;
		a[7] = x6i;
		a[8] = x1r;
		a[9] = x1i;
		a[12] = x3r;
		a[13] = x3i;
	}


	void bitrv208neg(float *a)
	{
		float x1r, x1i, x2r, x2i, x3r, x3i, x4r, x4i,
			x5r, x5i, x6r, x6i, x7r, x7i;

		x1r = a[2];
		x1i = a[3];
		x2r = a[4];
		x2i = a[5];
		x3r = a[6];
		x3i = a[7];
		x4r = a[8];
		x4i = a[9];
		x5r = a[10];
		x5i = a[11];
		x6r = a[12];
		x6i = a[13];
		x7r = a[14];
		x7i = a[15];
		a[2] = x7r;
		a[3] = x7i;
		a[4] = x3r;
		a[5] = x3i;
		a[6] = x5r;
		a[7] = x5i;
		a[8] = x1r;
		a[9] = x1i;
		a[10] = x6r;
		a[11] = x6i;
		a[12] = x2r;
		a[13] = x2i;
		a[14] = x4r;
		a[15] = x4i;
	}

	void cftb1st(int n, float *a)
	{
		int i, i0, j, j0, j1, j2, j3, m, mh;
		float ew, w1r, w1i, wk1r, wk1i, wk3r, wk3i,
			wd1r, wd1i, wd3r, wd3i, ss1, ss3;
		float x0r, x0i, x1r, x1i, x2r, x2i, x3r, x3i;

		mh = n >> 3;
		m = 2 * mh;
		j1 = m;
		j2 = j1 + m;
		j3 = j2 + m;
		x0r = a[0] + a[j2];
		x0i = -a[1] - a[j2 + 1];
		x1r = a[0] - a[j2];
		x1i = -a[1] + a[j2 + 1];
		x2r = a[j1] + a[j3];
		x2i = a[j1 + 1] + a[j3 + 1];
		x3r = a[j1] - a[j3];
		x3i = a[j1 + 1] - a[j3 + 1];
		a[0] = x0r + x2r;
		a[1] = x0i - x2i;
		a[j1] = x0r - x2r;
		a[j1 + 1] = x0i + x2i;
		a[j2] = x1r + x3i;
		a[j2 + 1] = x1i + x3r;
		a[j3] = x1r - x3i;
		a[j3 + 1] = x1i - x3r;
		wd1r = 1;
		wd1i = 0;
		wd3r = 1;
		wd3i = 0;
		ew = M_PI_2 / m;
		w1r = (float)cos(2 * ew);
		w1i = (float)sin(2 * ew);
		wk1r = w1r;
		wk1i = w1i;
		ss1 = 2 * w1i;
		wk3i = 2 * ss1 * wk1r;
		wk3r = wk1r - wk3i * wk1i;
		wk3i = wk1i - wk3i * wk1r;
		ss3 = 2 * wk3i;
		i = 0;
		for (;;) {
			i0 = i + 4 * CDFT_LOOP_DIV;
			if (i0 > mh - 4) {
				i0 = mh - 4;
			}
			for (j = i + 2; j < i0; j += 4) {
				wd1r -= ss1 * wk1i;
				wd1i += ss1 * wk1r;
				wd3r -= ss3 * wk3i;
				wd3i += ss3 * wk3r;
				j1 = j + m;
				j2 = j1 + m;
				j3 = j2 + m;
				x0r = a[j] + a[j2];
				x0i = -a[j + 1] - a[j2 + 1];
				x1r = a[j] - a[j2];
				x1i = -a[j + 1] + a[j2 + 1];
				x2r = a[j1] + a[j3];
				x2i = a[j1 + 1] + a[j3 + 1];
				x3r = a[j1] - a[j3];
				x3i = a[j1 + 1] - a[j3 + 1];
				a[j] = x0r + x2r;
				a[j + 1] = x0i - x2i;
				a[j1] = x0r - x2r;
				a[j1 + 1] = x0i + x2i;
				x0r = x1r + x3i;
				x0i = x1i + x3r;
				a[j2] = wk1r * x0r - wk1i * x0i;
				a[j2 + 1] = wk1r * x0i + wk1i * x0r;
				x0r = x1r - x3i;
				x0i = x1i - x3r;
				a[j3] = wk3r * x0r + wk3i * x0i;
				a[j3 + 1] = wk3r * x0i - wk3i * x0r;
				x0r = a[j + 2] + a[j2 + 2];
				x0i = -a[j + 3] - a[j2 + 3];
				x1r = a[j + 2] - a[j2 + 2];
				x1i = -a[j + 3] + a[j2 + 3];
				x2r = a[j1 + 2] + a[j3 + 2];
				x2i = a[j1 + 3] + a[j3 + 3];
				x3r = a[j1 + 2] - a[j3 + 2];
				x3i = a[j1 + 3] - a[j3 + 3];
				a[j + 2] = x0r + x2r;
				a[j + 3] = x0i - x2i;
				a[j1 + 2] = x0r - x2r;
				a[j1 + 3] = x0i + x2i;
				x0r = x1r + x3i;
				x0i = x1i + x3r;
				a[j2 + 2] = wd1r * x0r - wd1i * x0i;
				a[j2 + 3] = wd1r * x0i + wd1i * x0r;
				x0r = x1r - x3i;
				x0i = x1i - x3r;
				a[j3 + 2] = wd3r * x0r + wd3i * x0i;
				a[j3 + 3] = wd3r * x0i - wd3i * x0r;
				j0 = m - j;
				j1 = j0 + m;
				j2 = j1 + m;
				j3 = j2 + m;
				x0r = a[j0] + a[j2];
				x0i = -a[j0 + 1] - a[j2 + 1];
				x1r = a[j0] - a[j2];
				x1i = -a[j0 + 1] + a[j2 + 1];
				x2r = a[j1] + a[j3];
				x2i = a[j1 + 1] + a[j3 + 1];
				x3r = a[j1] - a[j3];
				x3i = a[j1 + 1] - a[j3 + 1];
				a[j0] = x0r + x2r;
				a[j0 + 1] = x0i - x2i;
				a[j1] = x0r - x2r;
				a[j1 + 1] = x0i + x2i;
				x0r = x1r + x3i;
				x0i = x1i + x3r;
				a[j2] = wk1i * x0r - wk1r * x0i;
				a[j2 + 1] = wk1i * x0i + wk1r * x0r;
				x0r = x1r - x3i;
				x0i = x1i - x3r;
				a[j3] = wk3i * x0r + wk3r * x0i;
				a[j3 + 1] = wk3i * x0i - wk3r * x0r;
				x0r = a[j0 - 2] + a[j2 - 2];
				x0i = -a[j0 - 1] - a[j2 - 1];
				x1r = a[j0 - 2] - a[j2 - 2];
				x1i = -a[j0 - 1] + a[j2 - 1];
				x2r = a[j1 - 2] + a[j3 - 2];
				x2i = a[j1 - 1] + a[j3 - 1];
				x3r = a[j1 - 2] - a[j3 - 2];
				x3i = a[j1 - 1] - a[j3 - 1];
				a[j0 - 2] = x0r + x2r;
				a[j0 - 1] = x0i - x2i;
				a[j1 - 2] = x0r - x2r;
				a[j1 - 1] = x0i + x2i;
				x0r = x1r + x3i;
				x0i = x1i + x3r;
				a[j2 - 2] = wd1i * x0r - wd1r * x0i;
				a[j2 - 1] = wd1i * x0i + wd1r * x0r;
				x0r = x1r - x3i;
				x0i = x1i - x3r;
				a[j3 - 2] = wd3i * x0r + wd3r * x0i;
				a[j3 - 1] = wd3i * x0i - wd3r * x0r;
				wk1r -= ss1 * wd1i;
				wk1i += ss1 * wd1r;
				wk3r -= ss3 * wd3i;
				wk3i += ss3 * wd3r;
			}
			if (i0 == mh - 4) {
				break;
			}
			wd1r = (float)cos(ew * i0);
			wd1i = (float)sin(ew * i0);
			wd3i = 4 * wd1i * wd1r;
			wd3r = wd1r - wd3i * wd1i;
			wd3i = wd1i - wd3i * wd1r;
			wk1r = w1r * wd1r - w1i * wd1i;
			wk1i = w1r * wd1i + w1i * wd1r;
			wk3i = 4 * wk1i * wk1r;
			wk3r = wk1r - wk3i * wk1i;
			wk3i = wk1i - wk3i * wk1r;
			i = i0;
		}
		wd1r = WR5000;
		j0 = mh;
		j1 = j0 + m;
		j2 = j1 + m;
		j3 = j2 + m;
		x0r = a[j0 - 2] + a[j2 - 2];
		x0i = -a[j0 - 1] - a[j2 - 1];
		x1r = a[j0 - 2] - a[j2 - 2];
		x1i = -a[j0 - 1] + a[j2 - 1];
		x2r = a[j1 - 2] + a[j3 - 2];
		x2i = a[j1 - 1] + a[j3 - 1];
		x3r = a[j1 - 2] - a[j3 - 2];
		x3i = a[j1 - 1] - a[j3 - 1];
		a[j0 - 2] = x0r + x2r;
		a[j0 - 1] = x0i - x2i;
		a[j1 - 2] = x0r - x2r;
		a[j1 - 1] = x0i + x2i;
		x0r = x1r + x3i;
		x0i = x1i + x3r;
		a[j2 - 2] = wk1r * x0r - wk1i * x0i;
		a[j2 - 1] = wk1r * x0i + wk1i * x0r;
		x0r = x1r - x3i;
		x0i = x1i - x3r;
		a[j3 - 2] = wk3r * x0r + wk3i * x0i;
		a[j3 - 1] = wk3r * x0i - wk3i * x0r;
		x0r = a[j0] + a[j2];
		x0i = -a[j0 + 1] - a[j2 + 1];
		x1r = a[j0] - a[j2];
		x1i = -a[j0 + 1] + a[j2 + 1];
		x2r = a[j1] + a[j3];
		x2i = a[j1 + 1] + a[j3 + 1];
		x3r = a[j1] - a[j3];
		x3i = a[j1 + 1] - a[j3 + 1];
		a[j0] = x0r + x2r;
		a[j0 + 1] = x0i - x2i;
		a[j1] = x0r - x2r;
		a[j1 + 1] = x0i + x2i;
		x0r = x1r + x3i;
		x0i = x1i + x3r;
		a[j2] = wd1r * (x0r - x0i);
		a[j2 + 1] = wd1r * (x0i + x0r);
		x0r = x1r - x3i;
		x0i = x1i - x3r;
		a[j3] = -wd1r * (x0r + x0i);
		a[j3 + 1] = -wd1r * (x0i - x0r);
		x0r = a[j0 + 2] + a[j2 + 2];
		x0i = -a[j0 + 3] - a[j2 + 3];
		x1r = a[j0 + 2] - a[j2 + 2];
		x1i = -a[j0 + 3] + a[j2 + 3];
		x2r = a[j1 + 2] + a[j3 + 2];
		x2i = a[j1 + 3] + a[j3 + 3];
		x3r = a[j1 + 2] - a[j3 + 2];
		x3i = a[j1 + 3] - a[j3 + 3];
		a[j0 + 2] = x0r + x2r;
		a[j0 + 3] = x0i - x2i;
		a[j1 + 2] = x0r - x2r;
		a[j1 + 3] = x0i + x2i;
		x0r = x1r + x3i;
		x0i = x1i + x3r;
		a[j2 + 2] = wk1i * x0r - wk1r * x0i;
		a[j2 + 3] = wk1i * x0i + wk1r * x0r;
		x0r = x1r - x3i;
		x0i = x1i - x3r;
		a[j3 + 2] = wk3i * x0r + wk3r * x0i;
		a[j3 + 3] = wk3i * x0i - wk3r * x0r;
	}

	void cftmdl1(int n, float *a)
	{
		int i, i0, j, j0, j1, j2, j3, m, mh;
		float ew, w1r, w1i, wk1r, wk1i, wk3r, wk3i,
			wd1r, wd1i, wd3r, wd3i, ss1, ss3;
		float x0r, x0i, x1r, x1i, x2r, x2i, x3r, x3i;

		mh = n >> 3;
		m = 2 * mh;
		j1 = m;
		j2 = j1 + m;
		j3 = j2 + m;
		x0r = a[0] + a[j2];
		x0i = a[1] + a[j2 + 1];
		x1r = a[0] - a[j2];
		x1i = a[1] - a[j2 + 1];
		x2r = a[j1] + a[j3];
		x2i = a[j1 + 1] + a[j3 + 1];
		x3r = a[j1] - a[j3];
		x3i = a[j1 + 1] - a[j3 + 1];
		a[0] = x0r + x2r;
		a[1] = x0i + x2i;
		a[j1] = x0r - x2r;
		a[j1 + 1] = x0i - x2i;
		a[j2] = x1r - x3i;
		a[j2 + 1] = x1i + x3r;
		a[j3] = x1r + x3i;
		a[j3 + 1] = x1i - x3r;
		wd1r = 1;
		wd1i = 0;
		wd3r = 1;
		wd3i = 0;
		ew = M_PI_2 / m;
		w1r = (float)cos(2 * ew);
		w1i = (float)sin(2 * ew);
		wk1r = w1r;
		wk1i = w1i;
		ss1 = 2 * w1i;
		wk3i = 2 * ss1 * wk1r;
		wk3r = wk1r - wk3i * wk1i;
		wk3i = wk1i - wk3i * wk1r;
		ss3 = 2 * wk3i;
		i = 0;
		for (;;) {
			i0 = i + 4 * CDFT_LOOP_DIV;
			if (i0 > mh - 4) {
				i0 = mh - 4;
			}
			for (j = i + 2; j < i0; j += 4) {
				wd1r -= ss1 * wk1i;
				wd1i += ss1 * wk1r;
				wd3r -= ss3 * wk3i;
				wd3i += ss3 * wk3r;
				j1 = j + m;
				j2 = j1 + m;
				j3 = j2 + m;
				x0r = a[j] + a[j2];
				x0i = a[j + 1] + a[j2 + 1];
				x1r = a[j] - a[j2];
				x1i = a[j + 1] - a[j2 + 1];
				x2r = a[j1] + a[j3];
				x2i = a[j1 + 1] + a[j3 + 1];
				x3r = a[j1] - a[j3];
				x3i = a[j1 + 1] - a[j3 + 1];
				a[j] = x0r + x2r;
				a[j + 1] = x0i + x2i;
				a[j1] = x0r - x2r;
				a[j1 + 1] = x0i - x2i;
				x0r = x1r - x3i;
				x0i = x1i + x3r;
				a[j2] = wk1r * x0r - wk1i * x0i;
				a[j2 + 1] = wk1r * x0i + wk1i * x0r;
				x0r = x1r + x3i;
				x0i = x1i - x3r;
				a[j3] = wk3r * x0r + wk3i * x0i;
				a[j3 + 1] = wk3r * x0i - wk3i * x0r;
				x0r = a[j + 2] + a[j2 + 2];
				x0i = a[j + 3] + a[j2 + 3];
				x1r = a[j + 2] - a[j2 + 2];
				x1i = a[j + 3] - a[j2 + 3];
				x2r = a[j1 + 2] + a[j3 + 2];
				x2i = a[j1 + 3] + a[j3 + 3];
				x3r = a[j1 + 2] - a[j3 + 2];
				x3i = a[j1 + 3] - a[j3 + 3];
				a[j + 2] = x0r + x2r;
				a[j + 3] = x0i + x2i;
				a[j1 + 2] = x0r - x2r;
				a[j1 + 3] = x0i - x2i;
				x0r = x1r - x3i;
				x0i = x1i + x3r;
				a[j2 + 2] = wd1r * x0r - wd1i * x0i;
				a[j2 + 3] = wd1r * x0i + wd1i * x0r;
				x0r = x1r + x3i;
				x0i = x1i - x3r;
				a[j3 + 2] = wd3r * x0r + wd3i * x0i;
				a[j3 + 3] = wd3r * x0i - wd3i * x0r;
				j0 = m - j;
				j1 = j0 + m;
				j2 = j1 + m;
				j3 = j2 + m;
				x0r = a[j0] + a[j2];
				x0i = a[j0 + 1] + a[j2 + 1];
				x1r = a[j0] - a[j2];
				x1i = a[j0 + 1] - a[j2 + 1];
				x2r = a[j1] + a[j3];
				x2i = a[j1 + 1] + a[j3 + 1];
				x3r = a[j1] - a[j3];
				x3i = a[j1 + 1] - a[j3 + 1];
				a[j0] = x0r + x2r;
				a[j0 + 1] = x0i + x2i;
				a[j1] = x0r - x2r;
				a[j1 + 1] = x0i - x2i;
				x0r = x1r - x3i;
				x0i = x1i + x3r;
				a[j2] = wk1i * x0r - wk1r * x0i;
				a[j2 + 1] = wk1i * x0i + wk1r * x0r;
				x0r = x1r + x3i;
				x0i = x1i - x3r;
				a[j3] = wk3i * x0r + wk3r * x0i;
				a[j3 + 1] = wk3i * x0i - wk3r * x0r;
				x0r = a[j0 - 2] + a[j2 - 2];
				x0i = a[j0 - 1] + a[j2 - 1];
				x1r = a[j0 - 2] - a[j2 - 2];
				x1i = a[j0 - 1] - a[j2 - 1];
				x2r = a[j1 - 2] + a[j3 - 2];
				x2i = a[j1 - 1] + a[j3 - 1];
				x3r = a[j1 - 2] - a[j3 - 2];
				x3i = a[j1 - 1] - a[j3 - 1];
				a[j0 - 2] = x0r + x2r;
				a[j0 - 1] = x0i + x2i;
				a[j1 - 2] = x0r - x2r;
				a[j1 - 1] = x0i - x2i;
				x0r = x1r - x3i;
				x0i = x1i + x3r;
				a[j2 - 2] = wd1i * x0r - wd1r * x0i;
				a[j2 - 1] = wd1i * x0i + wd1r * x0r;
				x0r = x1r + x3i;
				x0i = x1i - x3r;
				a[j3 - 2] = wd3i * x0r + wd3r * x0i;
				a[j3 - 1] = wd3i * x0i - wd3r * x0r;
				wk1r -= ss1 * wd1i;
				wk1i += ss1 * wd1r;
				wk3r -= ss3 * wd3i;
				wk3i += ss3 * wd3r;
			}
			if (i0 == mh - 4) {
				break;
			}
			wd1r = (float)cos(ew * i0);
			wd1i = (float)sin(ew * i0);
			wd3i = 4 * wd1i * wd1r;
			wd3r = wd1r - wd3i * wd1i;
			wd3i = wd1i - wd3i * wd1r;
			wk1r = w1r * wd1r - w1i * wd1i;
			wk1i = w1r * wd1i + w1i * wd1r;
			wk3i = 4 * wk1i * wk1r;
			wk3r = wk1r - wk3i * wk1i;
			wk3i = wk1i - wk3i * wk1r;
			i = i0;
		}
		wd1r = WR5000;
		j0 = mh;
		j1 = j0 + m;
		j2 = j1 + m;
		j3 = j2 + m;
		x0r = a[j0 - 2] + a[j2 - 2];
		x0i = a[j0 - 1] + a[j2 - 1];
		x1r = a[j0 - 2] - a[j2 - 2];
		x1i = a[j0 - 1] - a[j2 - 1];
		x2r = a[j1 - 2] + a[j3 - 2];
		x2i = a[j1 - 1] + a[j3 - 1];
		x3r = a[j1 - 2] - a[j3 - 2];
		x3i = a[j1 - 1] - a[j3 - 1];
		a[j0 - 2] = x0r + x2r;
		a[j0 - 1] = x0i + x2i;
		a[j1 - 2] = x0r - x2r;
		a[j1 - 1] = x0i - x2i;
		x0r = x1r - x3i;
		x0i = x1i + x3r;
		a[j2 - 2] = wk1r * x0r - wk1i * x0i;
		a[j2 - 1] = wk1r * x0i + wk1i * x0r;
		x0r = x1r + x3i;
		x0i = x1i - x3r;
		a[j3 - 2] = wk3r * x0r + wk3i * x0i;
		a[j3 - 1] = wk3r * x0i - wk3i * x0r;
		x0r = a[j0] + a[j2];
		x0i = a[j0 + 1] + a[j2 + 1];
		x1r = a[j0] - a[j2];
		x1i = a[j0 + 1] - a[j2 + 1];
		x2r = a[j1] + a[j3];
		x2i = a[j1 + 1] + a[j3 + 1];
		x3r = a[j1] - a[j3];
		x3i = a[j1 + 1] - a[j3 + 1];
		a[j0] = x0r + x2r;
		a[j0 + 1] = x0i + x2i;
		a[j1] = x0r - x2r;
		a[j1 + 1] = x0i - x2i;
		x0r = x1r - x3i;
		x0i = x1i + x3r;
		a[j2] = wd1r * (x0r - x0i);
		a[j2 + 1] = wd1r * (x0i + x0r);
		x0r = x1r + x3i;
		x0i = x1i - x3r;
		a[j3] = -wd1r * (x0r + x0i);
		a[j3 + 1] = -wd1r * (x0i - x0r);
		x0r = a[j0 + 2] + a[j2 + 2];
		x0i = a[j0 + 3] + a[j2 + 3];
		x1r = a[j0 + 2] - a[j2 + 2];
		x1i = a[j0 + 3] - a[j2 + 3];
		x2r = a[j1 + 2] + a[j3 + 2];
		x2i = a[j1 + 3] + a[j3 + 3];
		x3r = a[j1 + 2] - a[j3 + 2];
		x3i = a[j1 + 3] - a[j3 + 3];
		a[j0 + 2] = x0r + x2r;
		a[j0 + 3] = x0i + x2i;
		a[j1 + 2] = x0r - x2r;
		a[j1 + 3] = x0i - x2i;
		x0r = x1r - x3i;
		x0i = x1i + x3r;
		a[j2 + 2] = wk1i * x0r - wk1r * x0i;
		a[j2 + 3] = wk1i * x0i + wk1r * x0r;
		x0r = x1r + x3i;
		x0i = x1i - x3r;
		a[j3 + 2] = wk3i * x0r + wk3r * x0i;
		a[j3 + 3] = wk3i * x0i - wk3r * x0r;
	}


	void cftmdl2(int n, float *a)
	{
		int i, i0, j, j0, j1, j2, j3, m, mh;
		float ew, w1r, w1i, wn4r, wk1r, wk1i, wk3r, wk3i,
			wl1r, wl1i, wl3r, wl3i, wd1r, wd1i, wd3r, wd3i,
			we1r, we1i, we3r, we3i, ss1, ss3;
		float x0r, x0i, x1r, x1i, x2r, x2i, x3r, x3i, y0r, y0i, y2r, y2i;

		mh = n >> 3;
		m = 2 * mh;
		wn4r = WR5000;
		j1 = m;
		j2 = j1 + m;
		j3 = j2 + m;
		x0r = a[0] - a[j2 + 1];
		x0i = a[1] + a[j2];
		x1r = a[0] + a[j2 + 1];
		x1i = a[1] - a[j2];
		x2r = a[j1] - a[j3 + 1];
		x2i = a[j1 + 1] + a[j3];
		x3r = a[j1] + a[j3 + 1];
		x3i = a[j1 + 1] - a[j3];
		y0r = wn4r * (x2r - x2i);
		y0i = wn4r * (x2i + x2r);
		a[0] = x0r + y0r;
		a[1] = x0i + y0i;
		a[j1] = x0r - y0r;
		a[j1 + 1] = x0i - y0i;
		y0r = wn4r * (x3r - x3i);
		y0i = wn4r * (x3i + x3r);
		a[j2] = x1r - y0i;
		a[j2 + 1] = x1i + y0r;
		a[j3] = x1r + y0i;
		a[j3 + 1] = x1i - y0r;
		wl1r = 1;
		wl1i = 0;
		wl3r = 1;
		wl3i = 0;
		we1r = wn4r;
		we1i = wn4r;
		we3r = -wn4r;
		we3i = -wn4r;
		ew = M_PI_2 / (2 * m);
		w1r = (float)cos(2 * ew);
		w1i = (float)sin(2 * ew);
		wk1r = w1r;
		wk1i = w1i;
		wd1r = wn4r * (w1r - w1i);
		wd1i = wn4r * (w1i + w1r);
		ss1 = 2 * w1i;
		wk3i = 2 * ss1 * wk1r;
		wk3r = wk1r - wk3i * wk1i;
		wk3i = wk1i - wk3i * wk1r;
		ss3 = 2 * wk3i;
		wd3r = -wn4r * (wk3r - wk3i);
		wd3i = -wn4r * (wk3i + wk3r);
		i = 0;
		for (;;) {
			i0 = i + 4 * CDFT_LOOP_DIV;
			if (i0 > mh - 4) {
				i0 = mh - 4;
			}
			for (j = i + 2; j < i0; j += 4) {
				wl1r -= ss1 * wk1i;
				wl1i += ss1 * wk1r;
				wl3r -= ss3 * wk3i;
				wl3i += ss3 * wk3r;
				we1r -= ss1 * wd1i;
				we1i += ss1 * wd1r;
				we3r -= ss3 * wd3i;
				we3i += ss3 * wd3r;
				j1 = j + m;
				j2 = j1 + m;
				j3 = j2 + m;
				x0r = a[j] - a[j2 + 1];
				x0i = a[j + 1] + a[j2];
				x1r = a[j] + a[j2 + 1];
				x1i = a[j + 1] - a[j2];
				x2r = a[j1] - a[j3 + 1];
				x2i = a[j1 + 1] + a[j3];
				x3r = a[j1] + a[j3 + 1];
				x3i = a[j1 + 1] - a[j3];
				y0r = wk1r * x0r - wk1i * x0i;
				y0i = wk1r * x0i + wk1i * x0r;
				y2r = wd1r * x2r - wd1i * x2i;
				y2i = wd1r * x2i + wd1i * x2r;
				a[j] = y0r + y2r;
				a[j + 1] = y0i + y2i;
				a[j1] = y0r - y2r;
				a[j1 + 1] = y0i - y2i;
				y0r = wk3r * x1r + wk3i * x1i;
				y0i = wk3r * x1i - wk3i * x1r;
				y2r = wd3r * x3r + wd3i * x3i;
				y2i = wd3r * x3i - wd3i * x3r;
				a[j2] = y0r + y2r;
				a[j2 + 1] = y0i + y2i;
				a[j3] = y0r - y2r;
				a[j3 + 1] = y0i - y2i;
				x0r = a[j + 2] - a[j2 + 3];
				x0i = a[j + 3] + a[j2 + 2];
				x1r = a[j + 2] + a[j2 + 3];
				x1i = a[j + 3] - a[j2 + 2];
				x2r = a[j1 + 2] - a[j3 + 3];
				x2i = a[j1 + 3] + a[j3 + 2];
				x3r = a[j1 + 2] + a[j3 + 3];
				x3i = a[j1 + 3] - a[j3 + 2];
				y0r = wl1r * x0r - wl1i * x0i;
				y0i = wl1r * x0i + wl1i * x0r;
				y2r = we1r * x2r - we1i * x2i;
				y2i = we1r * x2i + we1i * x2r;
				a[j + 2] = y0r + y2r;
				a[j + 3] = y0i + y2i;
				a[j1 + 2] = y0r - y2r;
				a[j1 + 3] = y0i - y2i;
				y0r = wl3r * x1r + wl3i * x1i;
				y0i = wl3r * x1i - wl3i * x1r;
				y2r = we3r * x3r + we3i * x3i;
				y2i = we3r * x3i - we3i * x3r;
				a[j2 + 2] = y0r + y2r;
				a[j2 + 3] = y0i + y2i;
				a[j3 + 2] = y0r - y2r;
				a[j3 + 3] = y0i - y2i;
				j0 = m - j;
				j1 = j0 + m;
				j2 = j1 + m;
				j3 = j2 + m;
				x0r = a[j0] - a[j2 + 1];
				x0i = a[j0 + 1] + a[j2];
				x1r = a[j0] + a[j2 + 1];
				x1i = a[j0 + 1] - a[j2];
				x2r = a[j1] - a[j3 + 1];
				x2i = a[j1 + 1] + a[j3];
				x3r = a[j1] + a[j3 + 1];
				x3i = a[j1 + 1] - a[j3];
				y0r = wd1i * x0r - wd1r * x0i;
				y0i = wd1i * x0i + wd1r * x0r;
				y2r = wk1i * x2r - wk1r * x2i;
				y2i = wk1i * x2i + wk1r * x2r;
				a[j0] = y0r + y2r;
				a[j0 + 1] = y0i + y2i;
				a[j1] = y0r - y2r;
				a[j1 + 1] = y0i - y2i;
				y0r = wd3i * x1r + wd3r * x1i;
				y0i = wd3i * x1i - wd3r * x1r;
				y2r = wk3i * x3r + wk3r * x3i;
				y2i = wk3i * x3i - wk3r * x3r;
				a[j2] = y0r + y2r;
				a[j2 + 1] = y0i + y2i;
				a[j3] = y0r - y2r;
				a[j3 + 1] = y0i - y2i;
				x0r = a[j0 - 2] - a[j2 - 1];
				x0i = a[j0 - 1] + a[j2 - 2];
				x1r = a[j0 - 2] + a[j2 - 1];
				x1i = a[j0 - 1] - a[j2 - 2];
				x2r = a[j1 - 2] - a[j3 - 1];
				x2i = a[j1 - 1] + a[j3 - 2];
				x3r = a[j1 - 2] + a[j3 - 1];
				x3i = a[j1 - 1] - a[j3 - 2];
				y0r = we1i * x0r - we1r * x0i;
				y0i = we1i * x0i + we1r * x0r;
				y2r = wl1i * x2r - wl1r * x2i;
				y2i = wl1i * x2i + wl1r * x2r;
				a[j0 - 2] = y0r + y2r;
				a[j0 - 1] = y0i + y2i;
				a[j1 - 2] = y0r - y2r;
				a[j1 - 1] = y0i - y2i;
				y0r = we3i * x1r + we3r * x1i;
				y0i = we3i * x1i - we3r * x1r;
				y2r = wl3i * x3r + wl3r * x3i;
				y2i = wl3i * x3i - wl3r * x3r;
				a[j2 - 2] = y0r + y2r;
				a[j2 - 1] = y0i + y2i;
				a[j3 - 2] = y0r - y2r;
				a[j3 - 1] = y0i - y2i;
				wk1r -= ss1 * wl1i;
				wk1i += ss1 * wl1r;
				wk3r -= ss3 * wl3i;
				wk3i += ss3 * wl3r;
				wd1r -= ss1 * we1i;
				wd1i += ss1 * we1r;
				wd3r -= ss3 * we3i;
				wd3i += ss3 * we3r;
			}
			if (i0 == mh - 4) {
				break;
			}
			wl1r = (float)cos(ew * i0);
			wl1i = (float)sin(ew * i0);
			wl3i = 4 * wl1i * wl1r;
			wl3r = wl1r - wl3i * wl1i;
			wl3i = wl1i - wl3i * wl1r;
			we1r = wn4r * (wl1r - wl1i);
			we1i = wn4r * (wl1i + wl1r);
			we3r = -wn4r * (wl3r - wl3i);
			we3i = -wn4r * (wl3i + wl3r);
			wk1r = w1r * wl1r - w1i * wl1i;
			wk1i = w1r * wl1i + w1i * wl1r;
			wk3i = 4 * wk1i * wk1r;
			wk3r = wk1r - wk3i * wk1i;
			wk3i = wk1i - wk3i * wk1r;
			wd1r = wn4r * (wk1r - wk1i);
			wd1i = wn4r * (wk1i + wk1r);
			wd3r = -wn4r * (wk3r - wk3i);
			wd3i = -wn4r * (wk3i + wk3r);
			i = i0;
		}
		wl1r = WR2500;
		wl1i = WI2500;
		j0 = mh;
		j1 = j0 + m;
		j2 = j1 + m;
		j3 = j2 + m;
		x0r = a[j0 - 2] - a[j2 - 1];
		x0i = a[j0 - 1] + a[j2 - 2];
		x1r = a[j0 - 2] + a[j2 - 1];
		x1i = a[j0 - 1] - a[j2 - 2];
		x2r = a[j1 - 2] - a[j3 - 1];
		x2i = a[j1 - 1] + a[j3 - 2];
		x3r = a[j1 - 2] + a[j3 - 1];
		x3i = a[j1 - 1] - a[j3 - 2];
		y0r = wk1r * x0r - wk1i * x0i;
		y0i = wk1r * x0i + wk1i * x0r;
		y2r = wd1r * x2r - wd1i * x2i;
		y2i = wd1r * x2i + wd1i * x2r;
		a[j0 - 2] = y0r + y2r;
		a[j0 - 1] = y0i + y2i;
		a[j1 - 2] = y0r - y2r;
		a[j1 - 1] = y0i - y2i;
		y0r = wk3r * x1r + wk3i * x1i;
		y0i = wk3r * x1i - wk3i * x1r;
		y2r = wd3r * x3r + wd3i * x3i;
		y2i = wd3r * x3i - wd3i * x3r;
		a[j2 - 2] = y0r + y2r;
		a[j2 - 1] = y0i + y2i;
		a[j3 - 2] = y0r - y2r;
		a[j3 - 1] = y0i - y2i;
		x0r = a[j0] - a[j2 + 1];
		x0i = a[j0 + 1] + a[j2];
		x1r = a[j0] + a[j2 + 1];
		x1i = a[j0 + 1] - a[j2];
		x2r = a[j1] - a[j3 + 1];
		x2i = a[j1 + 1] + a[j3];
		x3r = a[j1] + a[j3 + 1];
		x3i = a[j1 + 1] - a[j3];
		y0r = wl1r * x0r - wl1i * x0i;
		y0i = wl1r * x0i + wl1i * x0r;
		y2r = wl1i * x2r - wl1r * x2i;
		y2i = wl1i * x2i + wl1r * x2r;
		a[j0] = y0r + y2r;
		a[j0 + 1] = y0i + y2i;
		a[j1] = y0r - y2r;
		a[j1 + 1] = y0i - y2i;
		y0r = wl1i * x1r - wl1r * x1i;
		y0i = wl1i * x1i + wl1r * x1r;
		y2r = wl1r * x3r - wl1i * x3i;
		y2i = wl1r * x3i + wl1i * x3r;
		a[j2] = y0r - y2r;
		a[j2 + 1] = y0i - y2i;
		a[j3] = y0r + y2r;
		a[j3 + 1] = y0i + y2i;
		x0r = a[j0 + 2] - a[j2 + 3];
		x0i = a[j0 + 3] + a[j2 + 2];
		x1r = a[j0 + 2] + a[j2 + 3];
		x1i = a[j0 + 3] - a[j2 + 2];
		x2r = a[j1 + 2] - a[j3 + 3];
		x2i = a[j1 + 3] + a[j3 + 2];
		x3r = a[j1 + 2] + a[j3 + 3];
		x3i = a[j1 + 3] - a[j3 + 2];
		y0r = wd1i * x0r - wd1r * x0i;
		y0i = wd1i * x0i + wd1r * x0r;
		y2r = wk1i * x2r - wk1r * x2i;
		y2i = wk1i * x2i + wk1r * x2r;
		a[j0 + 2] = y0r + y2r;
		a[j0 + 3] = y0i + y2i;
		a[j1 + 2] = y0r - y2r;
		a[j1 + 3] = y0i - y2i;
		y0r = wd3i * x1r + wd3r * x1i;
		y0i = wd3i * x1i - wd3r * x1r;
		y2r = wk3i * x3r + wk3r * x3i;
		y2i = wk3i * x3i - wk3r * x3r;
		a[j2 + 2] = y0r + y2r;
		a[j2 + 3] = y0i + y2i;
		a[j3 + 2] = y0r - y2r;
		a[j3 + 3] = y0i - y2i;
	}

	void cftf161(float *a)
	{
		float wn4r, wk1r, wk1i,
			x0r, x0i, x1r, x1i, x2r, x2i, x3r, x3i,
			y0r, y0i, y1r, y1i, y2r, y2i, y3r, y3i,
			y4r, y4i, y5r, y5i, y6r, y6i, y7r, y7i,
			y8r, y8i, y9r, y9i, y10r, y10i, y11r, y11i,
			y12r, y12i, y13r, y13i, y14r, y14i, y15r, y15i;

		wn4r = WR5000;
		wk1r = WR2500;
		wk1i = WI2500;
		x0r = a[0] + a[16];
		x0i = a[1] + a[17];
		x1r = a[0] - a[16];
		x1i = a[1] - a[17];
		x2r = a[8] + a[24];
		x2i = a[9] + a[25];
		x3r = a[8] - a[24];
		x3i = a[9] - a[25];
		y0r = x0r + x2r;
		y0i = x0i + x2i;
		y4r = x0r - x2r;
		y4i = x0i - x2i;
		y8r = x1r - x3i;
		y8i = x1i + x3r;
		y12r = x1r + x3i;
		y12i = x1i - x3r;
		x0r = a[2] + a[18];
		x0i = a[3] + a[19];
		x1r = a[2] - a[18];
		x1i = a[3] - a[19];
		x2r = a[10] + a[26];
		x2i = a[11] + a[27];
		x3r = a[10] - a[26];
		x3i = a[11] - a[27];
		y1r = x0r + x2r;
		y1i = x0i + x2i;
		y5r = x0r - x2r;
		y5i = x0i - x2i;
		x0r = x1r - x3i;
		x0i = x1i + x3r;
		y9r = wk1r * x0r - wk1i * x0i;
		y9i = wk1r * x0i + wk1i * x0r;
		x0r = x1r + x3i;
		x0i = x1i - x3r;
		y13r = wk1i * x0r - wk1r * x0i;
		y13i = wk1i * x0i + wk1r * x0r;
		x0r = a[4] + a[20];
		x0i = a[5] + a[21];
		x1r = a[4] - a[20];
		x1i = a[5] - a[21];
		x2r = a[12] + a[28];
		x2i = a[13] + a[29];
		x3r = a[12] - a[28];
		x3i = a[13] - a[29];
		y2r = x0r + x2r;
		y2i = x0i + x2i;
		y6r = x0r - x2r;
		y6i = x0i - x2i;
		x0r = x1r - x3i;
		x0i = x1i + x3r;
		y10r = wn4r * (x0r - x0i);
		y10i = wn4r * (x0i + x0r);
		x0r = x1r + x3i;
		x0i = x1i - x3r;
		y14r = wn4r * (x0r + x0i);
		y14i = wn4r * (x0i - x0r);
		x0r = a[6] + a[22];
		x0i = a[7] + a[23];
		x1r = a[6] - a[22];
		x1i = a[7] - a[23];
		x2r = a[14] + a[30];
		x2i = a[15] + a[31];
		x3r = a[14] - a[30];
		x3i = a[15] - a[31];
		y3r = x0r + x2r;
		y3i = x0i + x2i;
		y7r = x0r - x2r;
		y7i = x0i - x2i;
		x0r = x1r - x3i;
		x0i = x1i + x3r;
		y11r = wk1i * x0r - wk1r * x0i;
		y11i = wk1i * x0i + wk1r * x0r;
		x0r = x1r + x3i;
		x0i = x1i - x3r;
		y15r = wk1r * x0r - wk1i * x0i;
		y15i = wk1r * x0i + wk1i * x0r;
		x0r = y12r - y14r;
		x0i = y12i - y14i;
		x1r = y12r + y14r;
		x1i = y12i + y14i;
		x2r = y13r - y15r;
		x2i = y13i - y15i;
		x3r = y13r + y15r;
		x3i = y13i + y15i;
		a[24] = x0r + x2r;
		a[25] = x0i + x2i;
		a[26] = x0r - x2r;
		a[27] = x0i - x2i;
		a[28] = x1r - x3i;
		a[29] = x1i + x3r;
		a[30] = x1r + x3i;
		a[31] = x1i - x3r;
		x0r = y8r + y10r;
		x0i = y8i + y10i;
		x1r = y8r - y10r;
		x1i = y8i - y10i;
		x2r = y9r + y11r;
		x2i = y9i + y11i;
		x3r = y9r - y11r;
		x3i = y9i - y11i;
		a[16] = x0r + x2r;
		a[17] = x0i + x2i;
		a[18] = x0r - x2r;
		a[19] = x0i - x2i;
		a[20] = x1r - x3i;
		a[21] = x1i + x3r;
		a[22] = x1r + x3i;
		a[23] = x1i - x3r;
		x0r = y5r - y7i;
		x0i = y5i + y7r;
		x2r = wn4r * (x0r - x0i);
		x2i = wn4r * (x0i + x0r);
		x0r = y5r + y7i;
		x0i = y5i - y7r;
		x3r = wn4r * (x0r - x0i);
		x3i = wn4r * (x0i + x0r);
		x0r = y4r - y6i;
		x0i = y4i + y6r;
		x1r = y4r + y6i;
		x1i = y4i - y6r;
		a[8] = x0r + x2r;
		a[9] = x0i + x2i;
		a[10] = x0r - x2r;
		a[11] = x0i - x2i;
		a[12] = x1r - x3i;
		a[13] = x1i + x3r;
		a[14] = x1r + x3i;
		a[15] = x1i - x3r;
		x0r = y0r + y2r;
		x0i = y0i + y2i;
		x1r = y0r - y2r;
		x1i = y0i - y2i;
		x2r = y1r + y3r;
		x2i = y1i + y3i;
		x3r = y1r - y3r;
		x3i = y1i - y3i;
		a[0] = x0r + x2r;
		a[1] = x0i + x2i;
		a[2] = x0r - x2r;
		a[3] = x0i - x2i;
		a[4] = x1r - x3i;
		a[5] = x1i + x3r;
		a[6] = x1r + x3i;
		a[7] = x1i - x3r;
	}


	void cftf162(float *a)
	{
		float wn4r, wk1r, wk1i, wk2r, wk2i, wk3r, wk3i,
			x0r, x0i, x1r, x1i, x2r, x2i,
			y0r, y0i, y1r, y1i, y2r, y2i, y3r, y3i,
			y4r, y4i, y5r, y5i, y6r, y6i, y7r, y7i,
			y8r, y8i, y9r, y9i, y10r, y10i, y11r, y11i,
			y12r, y12i, y13r, y13i, y14r, y14i, y15r, y15i;

		wn4r = WR5000;
		wk1r = WR1250;
		wk1i = WI1250;
		wk2r = WR2500;
		wk2i = WI2500;
		wk3r = WR3750;
		wk3i = WI3750;
		x1r = a[0] - a[17];
		x1i = a[1] + a[16];
		x0r = a[8] - a[25];
		x0i = a[9] + a[24];
		x2r = wn4r * (x0r - x0i);
		x2i = wn4r * (x0i + x0r);
		y0r = x1r + x2r;
		y0i = x1i + x2i;
		y4r = x1r - x2r;
		y4i = x1i - x2i;
		x1r = a[0] + a[17];
		x1i = a[1] - a[16];
		x0r = a[8] + a[25];
		x0i = a[9] - a[24];
		x2r = wn4r * (x0r - x0i);
		x2i = wn4r * (x0i + x0r);
		y8r = x1r - x2i;
		y8i = x1i + x2r;
		y12r = x1r + x2i;
		y12i = x1i - x2r;
		x0r = a[2] - a[19];
		x0i = a[3] + a[18];
		x1r = wk1r * x0r - wk1i * x0i;
		x1i = wk1r * x0i + wk1i * x0r;
		x0r = a[10] - a[27];
		x0i = a[11] + a[26];
		x2r = wk3i * x0r - wk3r * x0i;
		x2i = wk3i * x0i + wk3r * x0r;
		y1r = x1r + x2r;
		y1i = x1i + x2i;
		y5r = x1r - x2r;
		y5i = x1i - x2i;
		x0r = a[2] + a[19];
		x0i = a[3] - a[18];
		x1r = wk3r * x0r - wk3i * x0i;
		x1i = wk3r * x0i + wk3i * x0r;
		x0r = a[10] + a[27];
		x0i = a[11] - a[26];
		x2r = wk1r * x0r + wk1i * x0i;
		x2i = wk1r * x0i - wk1i * x0r;
		y9r = x1r - x2r;
		y9i = x1i - x2i;
		y13r = x1r + x2r;
		y13i = x1i + x2i;
		x0r = a[4] - a[21];
		x0i = a[5] + a[20];
		x1r = wk2r * x0r - wk2i * x0i;
		x1i = wk2r * x0i + wk2i * x0r;
		x0r = a[12] - a[29];
		x0i = a[13] + a[28];
		x2r = wk2i * x0r - wk2r * x0i;
		x2i = wk2i * x0i + wk2r * x0r;
		y2r = x1r + x2r;
		y2i = x1i + x2i;
		y6r = x1r - x2r;
		y6i = x1i - x2i;
		x0r = a[4] + a[21];
		x0i = a[5] - a[20];
		x1r = wk2i * x0r - wk2r * x0i;
		x1i = wk2i * x0i + wk2r * x0r;
		x0r = a[12] + a[29];
		x0i = a[13] - a[28];
		x2r = wk2r * x0r - wk2i * x0i;
		x2i = wk2r * x0i + wk2i * x0r;
		y10r = x1r - x2r;
		y10i = x1i - x2i;
		y14r = x1r + x2r;
		y14i = x1i + x2i;
		x0r = a[6] - a[23];
		x0i = a[7] + a[22];
		x1r = wk3r * x0r - wk3i * x0i;
		x1i = wk3r * x0i + wk3i * x0r;
		x0r = a[14] - a[31];
		x0i = a[15] + a[30];
		x2r = wk1i * x0r - wk1r * x0i;
		x2i = wk1i * x0i + wk1r * x0r;
		y3r = x1r + x2r;
		y3i = x1i + x2i;
		y7r = x1r - x2r;
		y7i = x1i - x2i;
		x0r = a[6] + a[23];
		x0i = a[7] - a[22];
		x1r = wk1i * x0r + wk1r * x0i;
		x1i = wk1i * x0i - wk1r * x0r;
		x0r = a[14] + a[31];
		x0i = a[15] - a[30];
		x2r = wk3i * x0r - wk3r * x0i;
		x2i = wk3i * x0i + wk3r * x0r;
		y11r = x1r + x2r;
		y11i = x1i + x2i;
		y15r = x1r - x2r;
		y15i = x1i - x2i;
		x1r = y0r + y2r;
		x1i = y0i + y2i;
		x2r = y1r + y3r;
		x2i = y1i + y3i;
		a[0] = x1r + x2r;
		a[1] = x1i + x2i;
		a[2] = x1r - x2r;
		a[3] = x1i - x2i;
		x1r = y0r - y2r;
		x1i = y0i - y2i;
		x2r = y1r - y3r;
		x2i = y1i - y3i;
		a[4] = x1r - x2i;
		a[5] = x1i + x2r;
		a[6] = x1r + x2i;
		a[7] = x1i - x2r;
		x1r = y4r - y6i;
		x1i = y4i + y6r;
		x0r = y5r - y7i;
		x0i = y5i + y7r;
		x2r = wn4r * (x0r - x0i);
		x2i = wn4r * (x0i + x0r);
		a[8] = x1r + x2r;
		a[9] = x1i + x2i;
		a[10] = x1r - x2r;
		a[11] = x1i - x2i;
		x1r = y4r + y6i;
		x1i = y4i - y6r;
		x0r = y5r + y7i;
		x0i = y5i - y7r;
		x2r = wn4r * (x0r - x0i);
		x2i = wn4r * (x0i + x0r);
		a[12] = x1r - x2i;
		a[13] = x1i + x2r;
		a[14] = x1r + x2i;
		a[15] = x1i - x2r;
		x1r = y8r + y10r;
		x1i = y8i + y10i;
		x2r = y9r - y11r;
		x2i = y9i - y11i;
		a[16] = x1r + x2r;
		a[17] = x1i + x2i;
		a[18] = x1r - x2r;
		a[19] = x1i - x2i;
		x1r = y8r - y10r;
		x1i = y8i - y10i;
		x2r = y9r + y11r;
		x2i = y9i + y11i;
		a[20] = x1r - x2i;
		a[21] = x1i + x2r;
		a[22] = x1r + x2i;
		a[23] = x1i - x2r;
		x1r = y12r - y14i;
		x1i = y12i + y14r;
		x0r = y13r + y15i;
		x0i = y13i - y15r;
		x2r = wn4r * (x0r - x0i);
		x2i = wn4r * (x0i + x0r);
		a[24] = x1r + x2r;
		a[25] = x1i + x2i;
		a[26] = x1r - x2r;
		a[27] = x1i - x2i;
		x1r = y12r + y14i;
		x1i = y12i - y14r;
		x0r = y13r - y15i;
		x0i = y13i + y15r;
		x2r = wn4r * (x0r - x0i);
		x2i = wn4r * (x0i + x0r);
		a[28] = x1r - x2i;
		a[29] = x1i + x2r;
		a[30] = x1r + x2i;
		a[31] = x1i - x2r;
	}


	void cftf081(float *a)
	{
		float wn4r, x0r, x0i, x1r, x1i, x2r, x2i, x3r, x3i,
			y0r, y0i, y1r, y1i, y2r, y2i, y3r, y3i,
			y4r, y4i, y5r, y5i, y6r, y6i, y7r, y7i;

		wn4r = WR5000;
		x0r = a[0] + a[8];
		x0i = a[1] + a[9];
		x1r = a[0] - a[8];
		x1i = a[1] - a[9];
		x2r = a[4] + a[12];
		x2i = a[5] + a[13];
		x3r = a[4] - a[12];
		x3i = a[5] - a[13];
		y0r = x0r + x2r;
		y0i = x0i + x2i;
		y2r = x0r - x2r;
		y2i = x0i - x2i;
		y1r = x1r - x3i;
		y1i = x1i + x3r;
		y3r = x1r + x3i;
		y3i = x1i - x3r;
		x0r = a[2] + a[10];
		x0i = a[3] + a[11];
		x1r = a[2] - a[10];
		x1i = a[3] - a[11];
		x2r = a[6] + a[14];
		x2i = a[7] + a[15];
		x3r = a[6] - a[14];
		x3i = a[7] - a[15];
		y4r = x0r + x2r;
		y4i = x0i + x2i;
		y6r = x0r - x2r;
		y6i = x0i - x2i;
		x0r = x1r - x3i;
		x0i = x1i + x3r;
		x2r = x1r + x3i;
		x2i = x1i - x3r;
		y5r = wn4r * (x0r - x0i);
		y5i = wn4r * (x0r + x0i);
		y7r = wn4r * (x2r - x2i);
		y7i = wn4r * (x2r + x2i);
		a[8] = y1r + y5r;
		a[9] = y1i + y5i;
		a[10] = y1r - y5r;
		a[11] = y1i - y5i;
		a[12] = y3r - y7i;
		a[13] = y3i + y7r;
		a[14] = y3r + y7i;
		a[15] = y3i - y7r;
		a[0] = y0r + y4r;
		a[1] = y0i + y4i;
		a[2] = y0r - y4r;
		a[3] = y0i - y4i;
		a[4] = y2r - y6i;
		a[5] = y2i + y6r;
		a[6] = y2r + y6i;
		a[7] = y2i - y6r;
	}


	void cftf082(float *a)
	{
		float wn4r, wk1r, wk1i, x0r, x0i, x1r, x1i,
			y0r, y0i, y1r, y1i, y2r, y2i, y3r, y3i,
			y4r, y4i, y5r, y5i, y6r, y6i, y7r, y7i;

		wn4r = WR5000;
		wk1r = WR2500;
		wk1i = WI2500;
		y0r = a[0] - a[9];
		y0i = a[1] + a[8];
		y1r = a[0] + a[9];
		y1i = a[1] - a[8];
		x0r = a[4] - a[13];
		x0i = a[5] + a[12];
		y2r = wn4r * (x0r - x0i);
		y2i = wn4r * (x0i + x0r);
		x0r = a[4] + a[13];
		x0i = a[5] - a[12];
		y3r = wn4r * (x0r - x0i);
		y3i = wn4r * (x0i + x0r);
		x0r = a[2] - a[11];
		x0i = a[3] + a[10];
		y4r = wk1r * x0r - wk1i * x0i;
		y4i = wk1r * x0i + wk1i * x0r;
		x0r = a[2] + a[11];
		x0i = a[3] - a[10];
		y5r = wk1i * x0r - wk1r * x0i;
		y5i = wk1i * x0i + wk1r * x0r;
		x0r = a[6] - a[15];
		x0i = a[7] + a[14];
		y6r = wk1i * x0r - wk1r * x0i;
		y6i = wk1i * x0i + wk1r * x0r;
		x0r = a[6] + a[15];
		x0i = a[7] - a[14];
		y7r = wk1r * x0r - wk1i * x0i;
		y7i = wk1r * x0i + wk1i * x0r;
		x0r = y0r + y2r;
		x0i = y0i + y2i;
		x1r = y4r + y6r;
		x1i = y4i + y6i;
		a[0] = x0r + x1r;
		a[1] = x0i + x1i;
		a[2] = x0r - x1r;
		a[3] = x0i - x1i;
		x0r = y0r - y2r;
		x0i = y0i - y2i;
		x1r = y4r - y6r;
		x1i = y4i - y6i;
		a[4] = x0r - x1i;
		a[5] = x0i + x1r;
		a[6] = x0r + x1i;
		a[7] = x0i - x1r;
		x0r = y1r - y3i;
		x0i = y1i + y3r;
		x1r = y5r - y7r;
		x1i = y5i - y7i;
		a[8] = x0r + x1r;
		a[9] = x0i + x1i;
		a[10] = x0r - x1r;
		a[11] = x0i - x1i;
		x0r = y1r + y3i;
		x0i = y1i - y3r;
		x1r = y5r + y7r;
		x1i = y5i + y7i;
		a[12] = x0r - x1i;
		a[13] = x0i + x1r;
		a[14] = x0r + x1i;
		a[15] = x0i - x1r;
	}

	void cftf040(float *a)
	{
		float x0r, x0i, x1r, x1i, x2r, x2i, x3r, x3i;

		x0r = a[0] + a[4];
		x0i = a[1] + a[5];
		x1r = a[0] - a[4];
		x1i = a[1] - a[5];
		x2r = a[2] + a[6];
		x2i = a[3] + a[7];
		x3r = a[2] - a[6];
		x3i = a[3] - a[7];
		a[0] = x0r + x2r;
		a[1] = x0i + x2i;
		a[2] = x1r - x3i;
		a[3] = x1i + x3r;
		a[4] = x0r - x2r;
		a[5] = x0i - x2i;
		a[6] = x1r + x3i;
		a[7] = x1i - x3r;
	}

	void cftb040(float *a)
	{
		float x0r, x0i, x1r, x1i, x2r, x2i, x3r, x3i;

		x0r = a[0] + a[4];
		x0i = a[1] + a[5];
		x1r = a[0] - a[4];
		x1i = a[1] - a[5];
		x2r = a[2] + a[6];
		x2i = a[3] + a[7];
		x3r = a[2] - a[6];
		x3i = a[3] - a[7];
		a[0] = x0r + x2r;
		a[1] = x0i + x2i;
		a[2] = x1r + x3i;
		a[3] = x1i - x3r;
		a[4] = x0r - x2r;
		a[5] = x0i - x2i;
		a[6] = x1r - x3i;
		a[7] = x1i + x3r;
	}

	void cftx020(float *a)
	{
		float x0r, x0i;

		x0r = a[0] - a[2];
		x0i = a[1] - a[3];
		a[0] += a[2];
		a[1] += a[3];
		a[2] = x0r;
		a[3] = x0i;
	}

	void cftfx41(int n, float *a)
	{
		if (n == 128) {
			cftf161(a);
			cftf162(&a[32]);
			cftf161(&a[64]);
			cftf161(&a[96]);
		}
		else {
			cftf081(a);
			cftf082(&a[16]);
			cftf081(&a[32]);
			cftf081(&a[48]);
		}
	}

	int cfttree(int n, int j, int k, float *a)
	{
		int i, isplt, m;

		if ((k & 3) != 0) {
			isplt = k & 1;
			if (isplt != 0) {
				cftmdl1(n, &a[j - n]);
			}
			else {
				cftmdl2(n, &a[j - n]);
			}
		}
		else {
			m = n;
			for (i = k; (i & 3) == 0; i >>= 2) {
				m <<= 2;
			}
			isplt = i & 1;
			if (isplt != 0) {
				while (m > 128) {
					cftmdl1(m, &a[j - m]);
					m >>= 2;
				}
			}
			else {
				while (m > 128) {
					cftmdl2(m, &a[j - m]);
					m >>= 2;
				}
			}
		}
		return isplt;
	}

	void cftleaf(int n, int isplt, float *a)
	{
		if (n == 512) {
			cftmdl1(128, a);
			cftf161(a);
			cftf162(&a[32]);
			cftf161(&a[64]);
			cftf161(&a[96]);
			cftmdl2(128, &a[128]);
			cftf161(&a[128]);
			cftf162(&a[160]);
			cftf161(&a[192]);
			cftf162(&a[224]);
			cftmdl1(128, &a[256]);
			cftf161(&a[256]);
			cftf162(&a[288]);
			cftf161(&a[320]);
			cftf161(&a[352]);
			if (isplt != 0) {
				cftmdl1(128, &a[384]);
				cftf161(&a[480]);
			}
			else {
				cftmdl2(128, &a[384]);
				cftf162(&a[480]);
			}
			cftf161(&a[384]);
			cftf162(&a[416]);
			cftf161(&a[448]);
		}
		else {
			cftmdl1(64, a);
			cftf081(a);
			cftf082(&a[16]);
			cftf081(&a[32]);
			cftf081(&a[48]);
			cftmdl2(64, &a[64]);
			cftf081(&a[64]);
			cftf082(&a[80]);
			cftf081(&a[96]);
			cftf082(&a[112]);
			cftmdl1(64, &a[128]);
			cftf081(&a[128]);
			cftf082(&a[144]);
			cftf081(&a[160]);
			cftf081(&a[176]);
			if (isplt != 0) {
				cftmdl1(64, &a[192]);
				cftf081(&a[240]);
			}
			else {
				cftmdl2(64, &a[192]);
				cftf082(&a[240]);
			}
			cftf081(&a[192]);
			cftf082(&a[208]);
			cftf081(&a[224]);
		}
	}

	void cftrec4(int n, float *a)
	{
		int isplt, j, k, m;

		m = n;
		while (m > 512) {
			m >>= 2;
			cftmdl1(m, &a[n - m]);
		}
		cftleaf(m, 1, &a[n - m]);
		k = 0;
		for (j = n - m; j > 0; j -= m) {
			k++;
			isplt = cfttree(m, j, k, a);
			cftleaf(m, isplt, &a[j - m]);
		}
	}

	void cftfsub(int n, float *a)
	{
		if (n > 8) {
			if (n > 32) {
				cftmdl1(n, a);
				if (n > 512) {
					cftrec4(n, a);
				}
				else if (n > 128) {
					cftleaf(n, 1, a);
				}
				else {
					cftfx41(n, a);
				}
				bitrv2(n, a);
			}
			else if (n == 32) {
				cftf161(a);
				bitrv216(a);
			}
			else {
				cftf081(a);
				bitrv208(a);
			}
		}
		else if (n == 8) {
			cftf040(a);
		}
		else if (n == 4) {
			cftx020(a);
		}
	}


	void cftbsub(int n, float *a)
	{
		if (n > 8) {
			if (n > 32) {
				cftb1st(n, a);
				if (n > 512) {
					cftrec4(n, a);
				}
				else if (n > 128) {
					cftleaf(n, 1, a);
				}
				else {
					cftfx41(n, a);
				}
				bitrv2conj(n, a);
			}
			else if (n == 32) {
				cftf161(a);
				bitrv216neg(a);
			}
			else {
				cftf081(a);
				bitrv208neg(a);
			}
		}
		else if (n == 8) {
			cftb040(a);
		}
		else if (n == 4) {
			cftx020(a);
		}
	}
} // fftimpl


namespace SoLoud
{
    namespace FFT
    {
        void fft1024(float *aBuffer)
        {
			fft(aBuffer, 1024);			
        }    

        void fft256(float *aBuffer)
        {
			fft(aBuffer, 256);
        }
        
        void ifft256(float *aBuffer)
        {
			ifft(aBuffer, 256);
		}

		void fft(float *aBuffer, unsigned int aBufferLength)
		{
			fftimpl::cftbsub(aBufferLength, aBuffer);
		}

		void ifft(float *aBuffer, unsigned int aBufferLength)
		{
			unsigned int i;
			fftimpl::cftfsub(aBufferLength, aBuffer);
			for (i = 0; i < aBufferLength; i++)
				aBuffer[i] *= 1.0f / float(aBufferLength / 2);
		}
    };
};
