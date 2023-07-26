/* libFLAC - Free Lossless Audio Codec library
 * Copyright (C) 2000-2009  Josh Coalson
 * Copyright (C) 2011-2023  Xiph.Org Foundation
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * - Redistributions of source code must retain the above copyright
 * notice, this list of conditions and the following disclaimer.
 *
 * - Redistributions in binary form must reproduce the above copyright
 * notice, this list of conditions and the following disclaimer in the
 * documentation and/or other materials provided with the distribution.
 *
 * - Neither the name of the Xiph.org Foundation nor the names of its
 * contributors may be used to endorse or promote products derived from
 * this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * ``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE FOUNDATION OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
 * LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#include <math.h>
#include <stdlib.h>

#include "FLAC/assert.h"
#include "FLAC/format.h"
#include "share/compat.h"
#include "private/bitmath.h"
#include "private/lpc.h"
#include "private/macros.h"

#if !defined(NDEBUG) || defined FLAC__OVERFLOW_DETECT || defined FLAC__OVERFLOW_DETECT_VERBOSE
#include <stdio.h>
#endif

/* OPT: #undef'ing this may improve the speed on some architectures */
#define FLAC__LPC_UNROLLED_FILTER_LOOPS

#ifndef FLAC__INTEGER_ONLY_LIBRARY

#if defined(_MSC_VER) && (_MSC_VER < 1800)
#include <float.h>
static inline long int lround(double x) {
	return (long)(x + _copysign(0.5, x));
}
#elif !defined(HAVE_LROUND) && defined(__GNUC__)
static inline long int lround(double x) {
	return (long)(x + __builtin_copysign(0.5, x));
}
/* If this fails, we are in the presence of a mid 90's compiler, move along... */
#endif

void FLAC__lpc_window_data(const FLAC__int32 in[], const FLAC__real window[], FLAC__real out[], uint32_t data_len)
{
	uint32_t i;
	for(i = 0; i < data_len; i++)
		out[i] = in[i] * window[i];
}

void FLAC__lpc_window_data_wide(const FLAC__int64 in[], const FLAC__real window[], FLAC__real out[], uint32_t data_len)
{
	uint32_t i;
	for(i = 0; i < data_len; i++)
		out[i] = in[i] * window[i];
}

void FLAC__lpc_window_data_partial(const FLAC__int32 in[], const FLAC__real window[], FLAC__real out[], uint32_t data_len, uint32_t part_size, uint32_t data_shift)
{
	uint32_t i, j;
	if((part_size + data_shift) < data_len){
		for(i = 0; i < part_size; i++)
			out[i] = in[data_shift+i] * window[i];
		i = flac_min(i,data_len - part_size - data_shift);
		for(j = data_len - part_size; j < data_len; i++, j++)
			out[i] = in[data_shift+i] * window[j];
		if(i < data_len)
			out[i] = 0.0f;
	}
}

void FLAC__lpc_window_data_partial_wide(const FLAC__int64 in[], const FLAC__real window[], FLAC__real out[], uint32_t data_len, uint32_t part_size, uint32_t data_shift)
{
	uint32_t i, j;
	if((part_size + data_shift) < data_len){
		for(i = 0; i < part_size; i++)
			out[i] = in[data_shift+i] * window[i];
		i = flac_min(i,data_len - part_size - data_shift);
		for(j = data_len - part_size; j < data_len; i++, j++)
			out[i] = in[data_shift+i] * window[j];
		if(i < data_len)
			out[i] = 0.0f;
	}
}

void FLAC__lpc_compute_autocorrelation(const FLAC__real data[], uint32_t data_len, uint32_t lag, double autoc[])
{
	/* a readable, but slower, version */
#if 0
	double d;
	uint32_t i;

	FLAC__ASSERT(lag > 0);
	FLAC__ASSERT(lag <= data_len);

	/*
	 * Technically we should subtract the mean first like so:
	 *   for(i = 0; i < data_len; i++)
	 *     data[i] -= mean;
	 * but it appears not to make enough of a difference to matter, and
	 * most signals are already closely centered around zero
	 */
	while(lag--) {
		for(i = lag, d = 0.0; i < data_len; i++)
			d += data[i] * (double)data[i - lag];
		autoc[lag] = d;
	}
#endif
	if (data_len < FLAC__MAX_LPC_ORDER || lag > 16) {
		/*
		 * this version tends to run faster because of better data locality
		 * ('data_len' is usually much larger than 'lag')
		 */
		double d;
		uint32_t sample, coeff;
		const uint32_t limit = data_len - lag;

		FLAC__ASSERT(lag > 0);
		FLAC__ASSERT(lag <= data_len);

		for(coeff = 0; coeff < lag; coeff++)
			autoc[coeff] = 0.0;
		for(sample = 0; sample <= limit; sample++) {
			d = data[sample];
			for(coeff = 0; coeff < lag; coeff++)
				autoc[coeff] += d * data[sample+coeff];
		}
		for(; sample < data_len; sample++) {
			d = data[sample];
			for(coeff = 0; coeff < data_len - sample; coeff++)
				autoc[coeff] += d * data[sample+coeff];
		}
	}
	else if(lag <= 8) {
		#undef MAX_LAG
		#define MAX_LAG 8
		#include "deduplication/lpc_compute_autocorrelation_intrin.c"
	}
	else if(lag <= 12) {
		#undef MAX_LAG
		#define MAX_LAG 12
		#include "deduplication/lpc_compute_autocorrelation_intrin.c"
	}
	else if(lag <= 16) {
		#undef MAX_LAG
		#define MAX_LAG 16
		#include "deduplication/lpc_compute_autocorrelation_intrin.c"
	}

}

void FLAC__lpc_compute_lp_coefficients(const double autoc[], uint32_t *max_order, FLAC__real lp_coeff[][FLAC__MAX_LPC_ORDER], double error[])
{
	uint32_t i, j;
	double r, err, lpc[FLAC__MAX_LPC_ORDER];

	FLAC__ASSERT(0 != max_order);
	FLAC__ASSERT(0 < *max_order);
	FLAC__ASSERT(*max_order <= FLAC__MAX_LPC_ORDER);
	FLAC__ASSERT(autoc[0] != 0.0);

	err = autoc[0];

	for(i = 0; i < *max_order; i++) {
		/* Sum up this iteration's reflection coefficient. */
		r = -autoc[i+1];
		for(j = 0; j < i; j++)
			r -= lpc[j] * autoc[i-j];
		r /= err;

		/* Update LPC coefficients and total error. */
		lpc[i]=r;
		for(j = 0; j < (i>>1); j++) {
			double tmp = lpc[j];
			lpc[j] += r * lpc[i-1-j];
			lpc[i-1-j] += r * tmp;
		}
		if(i & 1)
			lpc[j] += lpc[j] * r;

		err *= (1.0 - r * r);

		/* save this order */
		for(j = 0; j <= i; j++)
			lp_coeff[i][j] = (FLAC__real)(-lpc[j]); /* negate FIR filter coeff to get predictor coeff */
		error[i] = err;

		/* see SF bug https://sourceforge.net/p/flac/bugs/234/ */
		if(err == 0.0) {
			*max_order = i+1;
			return;
		}
	}
}

int FLAC__lpc_quantize_coefficients(const FLAC__real lp_coeff[], uint32_t order, uint32_t precision, FLAC__int32 qlp_coeff[], int *shift)
{
	uint32_t i;
	double cmax;
	FLAC__int32 qmax, qmin;

	FLAC__ASSERT(precision > 0);
	FLAC__ASSERT(precision >= FLAC__MIN_QLP_COEFF_PRECISION);

	/* drop one bit for the sign; from here on out we consider only |lp_coeff[i]| */
	precision--;
	qmax = 1 << precision;
	qmin = -qmax;
	qmax--;

	/* calc cmax = max( |lp_coeff[i]| ) */
	cmax = 0.0;
	for(i = 0; i < order; i++) {
		const double d = fabs(lp_coeff[i]);
		if(d > cmax)
			cmax = d;
	}

	if(cmax <= 0.0) {
		/* => coefficients are all 0, which means our constant-detect didn't work */
		return 2;
	}
	else {
		const int max_shiftlimit = (1 << (FLAC__SUBFRAME_LPC_QLP_SHIFT_LEN-1)) - 1;
		const int min_shiftlimit = -max_shiftlimit - 1;
		int log2cmax;

		(void)frexp(cmax, &log2cmax);
		log2cmax--;
		*shift = (int)precision - log2cmax - 1;

		if(*shift > max_shiftlimit)
			*shift = max_shiftlimit;
		else if(*shift < min_shiftlimit)
			return 1;
	}

	if(*shift >= 0) {
		double error = 0.0;
		FLAC__int32 q;
		for(i = 0; i < order; i++) {
			error += lp_coeff[i] * (1 << *shift);
			q = lround(error);

#ifdef FLAC__OVERFLOW_DETECT
			if(q > qmax+1) /* we expect q==qmax+1 occasionally due to rounding */
				fprintf(stderr,"FLAC__lpc_quantize_coefficients: quantizer overflow: q>qmax %d>%d shift=%d cmax=%f precision=%u lpc[%u]=%f\n",q,qmax,*shift,cmax,precision+1,i,lp_coeff[i]);
			else if(q < qmin)
				fprintf(stderr,"FLAC__lpc_quantize_coefficients: quantizer overflow: q<qmin %d<%d shift=%d cmax=%f precision=%u lpc[%u]=%f\n",q,qmin,*shift,cmax,precision+1,i,lp_coeff[i]);
#endif
			if(q > qmax)
				q = qmax;
			else if(q < qmin)
				q = qmin;
			error -= q;
			qlp_coeff[i] = q;
		}
	}
	/* negative shift is very rare but due to design flaw, negative shift is
	 * not allowed in the decoder, so it must be handled specially by scaling
	 * down coeffs
	 */
	else {
		const int nshift = -(*shift);
		double error = 0.0;
		FLAC__int32 q;
#ifndef NDEBUG
		fprintf(stderr,"FLAC__lpc_quantize_coefficients: negative shift=%d order=%u cmax=%f\n", *shift, order, cmax);
#endif
		for(i = 0; i < order; i++) {
			error += lp_coeff[i] / (1 << nshift);
			q = lround(error);
#ifdef FLAC__OVERFLOW_DETECT
			if(q > qmax+1) /* we expect q==qmax+1 occasionally due to rounding */
				fprintf(stderr,"FLAC__lpc_quantize_coefficients: quantizer overflow: q>qmax %d>%d shift=%d cmax=%f precision=%u lpc[%u]=%f\n",q,qmax,*shift,cmax,precision+1,i,lp_coeff[i]);
			else if(q < qmin)
				fprintf(stderr,"FLAC__lpc_quantize_coefficients: quantizer overflow: q<qmin %d<%d shift=%d cmax=%f precision=%u lpc[%u]=%f\n",q,qmin,*shift,cmax,precision+1,i,lp_coeff[i]);
#endif
			if(q > qmax)
				q = qmax;
			else if(q < qmin)
				q = qmin;
			error -= q;
			qlp_coeff[i] = q;
		}
		*shift = 0;
	}

	return 0;
}

#if defined(_MSC_VER)
// silence MSVC warnings about __restrict modifier
#pragma warning ( disable : 4028 )
#endif

void FLAC__lpc_compute_residual_from_qlp_coefficients(const FLAC__int32 * flac_restrict data, uint32_t data_len, const FLAC__int32 * flac_restrict qlp_coeff, uint32_t order, int lp_quantization, FLAC__int32 * flac_restrict residual)
#if defined(FLAC__OVERFLOW_DETECT) || !defined(FLAC__LPC_UNROLLED_FILTER_LOOPS)
{
	FLAC__int64 sumo;
	uint32_t i, j;
	FLAC__int32 sum;
	const FLAC__int32 *history;

#ifdef FLAC__OVERFLOW_DETECT_VERBOSE
	fprintf(stderr,"FLAC__lpc_compute_residual_from_qlp_coefficients: data_len=%d, order=%u, lpq=%d",data_len,order,lp_quantization);
	for(i=0;i<order;i++)
		fprintf(stderr,", q[%u]=%d",i,qlp_coeff[i]);
	fprintf(stderr,"\n");
#endif
	FLAC__ASSERT(order > 0);

	for(i = 0; i < data_len; i++) {
		sumo = 0;
		sum = 0;
		history = data;
		for(j = 0; j < order; j++) {
			sum += qlp_coeff[j] * (*(--history));
			sumo += (FLAC__int64)qlp_coeff[j] * (FLAC__int64)(*history);
			if(sumo > 2147483647ll || sumo < -2147483648ll)
				fprintf(stderr,"FLAC__lpc_compute_residual_from_qlp_coefficients: OVERFLOW, i=%u, j=%u, c=%d, d=%d, sumo=%" PRId64 "\n",i,j,qlp_coeff[j],*history,sumo);
		}
		*(residual++) = *(data++) - (sum >> lp_quantization);
	}

	/* Here's a slower but clearer version:
	for(i = 0; i < data_len; i++) {
		sum = 0;
		for(j = 0; j < order; j++)
			sum += qlp_coeff[j] * data[i-j-1];
		residual[i] = data[i] - (sum >> lp_quantization);
	}
	*/
}
#else /* fully unrolled version for normal use */
{
	int i;
	FLAC__int32 sum;

	FLAC__ASSERT(order > 0);
	FLAC__ASSERT(order <= 32);

	/*
	 * We do unique versions up to 12th order since that's the subset limit.
	 * Also they are roughly ordered to match frequency of occurrence to
	 * minimize branching.
	 */
	if(order <= 12) {
		if(order > 8) {
			if(order > 10) {
				if(order == 12) {
					for(i = 0; i < (int)data_len; i++) {
						sum = 0;
						sum += qlp_coeff[11] * data[i-12];
						sum += qlp_coeff[10] * data[i-11];
						sum += qlp_coeff[9] * data[i-10];
						sum += qlp_coeff[8] * data[i-9];
						sum += qlp_coeff[7] * data[i-8];
						sum += qlp_coeff[6] * data[i-7];
						sum += qlp_coeff[5] * data[i-6];
						sum += qlp_coeff[4] * data[i-5];
						sum += qlp_coeff[3] * data[i-4];
						sum += qlp_coeff[2] * data[i-3];
						sum += qlp_coeff[1] * data[i-2];
						sum += qlp_coeff[0] * data[i-1];
						residual[i] = data[i] - (sum >> lp_quantization);
					}
				}
				else { /* order == 11 */
					for(i = 0; i < (int)data_len; i++) {
						sum = 0;
						sum += qlp_coeff[10] * data[i-11];
						sum += qlp_coeff[9] * data[i-10];
						sum += qlp_coeff[8] * data[i-9];
						sum += qlp_coeff[7] * data[i-8];
						sum += qlp_coeff[6] * data[i-7];
						sum += qlp_coeff[5] * data[i-6];
						sum += qlp_coeff[4] * data[i-5];
						sum += qlp_coeff[3] * data[i-4];
						sum += qlp_coeff[2] * data[i-3];
						sum += qlp_coeff[1] * data[i-2];
						sum += qlp_coeff[0] * data[i-1];
						residual[i] = data[i] - (sum >> lp_quantization);
					}
				}
			}
			else {
				if(order == 10) {
					for(i = 0; i < (int)data_len; i++) {
						sum = 0;
						sum += qlp_coeff[9] * data[i-10];
						sum += qlp_coeff[8] * data[i-9];
						sum += qlp_coeff[7] * data[i-8];
						sum += qlp_coeff[6] * data[i-7];
						sum += qlp_coeff[5] * data[i-6];
						sum += qlp_coeff[4] * data[i-5];
						sum += qlp_coeff[3] * data[i-4];
						sum += qlp_coeff[2] * data[i-3];
						sum += qlp_coeff[1] * data[i-2];
						sum += qlp_coeff[0] * data[i-1];
						residual[i] = data[i] - (sum >> lp_quantization);
					}
				}
				else { /* order == 9 */
					for(i = 0; i < (int)data_len; i++) {
						sum = 0;
						sum += qlp_coeff[8] * data[i-9];
						sum += qlp_coeff[7] * data[i-8];
						sum += qlp_coeff[6] * data[i-7];
						sum += qlp_coeff[5] * data[i-6];
						sum += qlp_coeff[4] * data[i-5];
						sum += qlp_coeff[3] * data[i-4];
						sum += qlp_coeff[2] * data[i-3];
						sum += qlp_coeff[1] * data[i-2];
						sum += qlp_coeff[0] * data[i-1];
						residual[i] = data[i] - (sum >> lp_quantization);
					}
				}
			}
		}
		else if(order > 4) {
			if(order > 6) {
				if(order == 8) {
					for(i = 0; i < (int)data_len; i++) {
						sum = 0;
						sum += qlp_coeff[7] * data[i-8];
						sum += qlp_coeff[6] * data[i-7];
						sum += qlp_coeff[5] * data[i-6];
						sum += qlp_coeff[4] * data[i-5];
						sum += qlp_coeff[3] * data[i-4];
						sum += qlp_coeff[2] * data[i-3];
						sum += qlp_coeff[1] * data[i-2];
						sum += qlp_coeff[0] * data[i-1];
						residual[i] = data[i] - (sum >> lp_quantization);
					}
				}
				else { /* order == 7 */
					for(i = 0; i < (int)data_len; i++) {
						sum = 0;
						sum += qlp_coeff[6] * data[i-7];
						sum += qlp_coeff[5] * data[i-6];
						sum += qlp_coeff[4] * data[i-5];
						sum += qlp_coeff[3] * data[i-4];
						sum += qlp_coeff[2] * data[i-3];
						sum += qlp_coeff[1] * data[i-2];
						sum += qlp_coeff[0] * data[i-1];
						residual[i] = data[i] - (sum >> lp_quantization);
					}
				}
			}
			else {
				if(order == 6) {
					for(i = 0; i < (int)data_len; i++) {
						sum = 0;
						sum += qlp_coeff[5] * data[i-6];
						sum += qlp_coeff[4] * data[i-5];
						sum += qlp_coeff[3] * data[i-4];
						sum += qlp_coeff[2] * data[i-3];
						sum += qlp_coeff[1] * data[i-2];
						sum += qlp_coeff[0] * data[i-1];
						residual[i] = data[i] - (sum >> lp_quantization);
					}
				}
				else { /* order == 5 */
					for(i = 0; i < (int)data_len; i++) {
						sum = 0;
						sum += qlp_coeff[4] * data[i-5];
						sum += qlp_coeff[3] * data[i-4];
						sum += qlp_coeff[2] * data[i-3];
						sum += qlp_coeff[1] * data[i-2];
						sum += qlp_coeff[0] * data[i-1];
						residual[i] = data[i] - (sum >> lp_quantization);
					}
				}
			}
		}
		else {
			if(order > 2) {
				if(order == 4) {
					for(i = 0; i < (int)data_len; i++) {
						sum = 0;
						sum += qlp_coeff[3] * data[i-4];
						sum += qlp_coeff[2] * data[i-3];
						sum += qlp_coeff[1] * data[i-2];
						sum += qlp_coeff[0] * data[i-1];
						residual[i] = data[i] - (sum >> lp_quantization);
					}
				}
				else { /* order == 3 */
					for(i = 0; i < (int)data_len; i++) {
						sum = 0;
						sum += qlp_coeff[2] * data[i-3];
						sum += qlp_coeff[1] * data[i-2];
						sum += qlp_coeff[0] * data[i-1];
						residual[i] = data[i] - (sum >> lp_quantization);
					}
				}
			}
			else {
				if(order == 2) {
					for(i = 0; i < (int)data_len; i++) {
						sum = 0;
						sum += qlp_coeff[1] * data[i-2];
						sum += qlp_coeff[0] * data[i-1];
						residual[i] = data[i] - (sum >> lp_quantization);
					}
				}
				else { /* order == 1 */
					for(i = 0; i < (int)data_len; i++)
						residual[i] = data[i] - ((qlp_coeff[0] * data[i-1]) >> lp_quantization);
				}
			}
		}
	}
	else { /* order > 12 */
		for(i = 0; i < (int)data_len; i++) {
			sum = 0;
			switch(order) {
				case 32: sum += qlp_coeff[31] * data[i-32]; /* Falls through. */
				case 31: sum += qlp_coeff[30] * data[i-31]; /* Falls through. */
				case 30: sum += qlp_coeff[29] * data[i-30]; /* Falls through. */
				case 29: sum += qlp_coeff[28] * data[i-29]; /* Falls through. */
				case 28: sum += qlp_coeff[27] * data[i-28]; /* Falls through. */
				case 27: sum += qlp_coeff[26] * data[i-27]; /* Falls through. */
				case 26: sum += qlp_coeff[25] * data[i-26]; /* Falls through. */
				case 25: sum += qlp_coeff[24] * data[i-25]; /* Falls through. */
				case 24: sum += qlp_coeff[23] * data[i-24]; /* Falls through. */
				case 23: sum += qlp_coeff[22] * data[i-23]; /* Falls through. */
				case 22: sum += qlp_coeff[21] * data[i-22]; /* Falls through. */
				case 21: sum += qlp_coeff[20] * data[i-21]; /* Falls through. */
				case 20: sum += qlp_coeff[19] * data[i-20]; /* Falls through. */
				case 19: sum += qlp_coeff[18] * data[i-19]; /* Falls through. */
				case 18: sum += qlp_coeff[17] * data[i-18]; /* Falls through. */
				case 17: sum += qlp_coeff[16] * data[i-17]; /* Falls through. */
				case 16: sum += qlp_coeff[15] * data[i-16]; /* Falls through. */
				case 15: sum += qlp_coeff[14] * data[i-15]; /* Falls through. */
				case 14: sum += qlp_coeff[13] * data[i-14]; /* Falls through. */
				case 13: sum += qlp_coeff[12] * data[i-13];
				         sum += qlp_coeff[11] * data[i-12];
				         sum += qlp_coeff[10] * data[i-11];
				         sum += qlp_coeff[ 9] * data[i-10];
				         sum += qlp_coeff[ 8] * data[i- 9];
				         sum += qlp_coeff[ 7] * data[i- 8];
				         sum += qlp_coeff[ 6] * data[i- 7];
				         sum += qlp_coeff[ 5] * data[i- 6];
				         sum += qlp_coeff[ 4] * data[i- 5];
				         sum += qlp_coeff[ 3] * data[i- 4];
				         sum += qlp_coeff[ 2] * data[i- 3];
				         sum += qlp_coeff[ 1] * data[i- 2];
				         sum += qlp_coeff[ 0] * data[i- 1];
			}
			residual[i] = data[i] - (sum >> lp_quantization);
		}
	}
}
#endif

void FLAC__lpc_compute_residual_from_qlp_coefficients_wide(const FLAC__int32 * flac_restrict data, uint32_t data_len, const FLAC__int32 * flac_restrict qlp_coeff, uint32_t order, int lp_quantization, FLAC__int32 * flac_restrict residual)
#if defined(FLAC__OVERFLOW_DETECT) || !defined(FLAC__LPC_UNROLLED_FILTER_LOOPS)
{
	uint32_t i, j;
	FLAC__int64 sum;
	const FLAC__int32 *history;

#ifdef FLAC__OVERFLOW_DETECT_VERBOSE
	fprintf(stderr,"FLAC__lpc_compute_residual_from_qlp_coefficients_wide: data_len=%d, order=%u, lpq=%d",data_len,order,lp_quantization);
	for(i=0;i<order;i++)
		fprintf(stderr,", q[%u]=%d",i,qlp_coeff[i]);
	fprintf(stderr,"\n");
#endif
	FLAC__ASSERT(order > 0);

	for(i = 0; i < data_len; i++) {
		sum = 0;
		history = data;
		for(j = 0; j < order; j++)
			sum += (FLAC__int64)qlp_coeff[j] * (FLAC__int64)(*(--history));
		if(FLAC__bitmath_silog2((FLAC__int64)(*data) - (sum >> lp_quantization)) > 32) {
			fprintf(stderr,"FLAC__lpc_compute_residual_from_qlp_coefficients_wide: OVERFLOW, i=%u, data=%d, sum=%" PRId64 ", residual=%" PRId64 "\n", i, *data, (int64_t)(sum >> lp_quantization), ((FLAC__int64)(*data) - (sum >> lp_quantization)));
			break;
		}
		*(residual++) = *(data++) - (FLAC__int32)(sum >> lp_quantization);
	}
}
#else /* fully unrolled version for normal use */
{
	int i;
	FLAC__int64 sum;

	FLAC__ASSERT(order > 0);
	FLAC__ASSERT(order <= 32);

	/*
	 * We do unique versions up to 12th order since that's the subset limit.
	 * Also they are roughly ordered to match frequency of occurrence to
	 * minimize branching.
	 */
	if(order <= 12) {
		if(order > 8) {
			if(order > 10) {
				if(order == 12) {
					for(i = 0; i < (int)data_len; i++) {
						sum = 0;
						sum += qlp_coeff[11] * (FLAC__int64)data[i-12];
						sum += qlp_coeff[10] * (FLAC__int64)data[i-11];
						sum += qlp_coeff[9] * (FLAC__int64)data[i-10];
						sum += qlp_coeff[8] * (FLAC__int64)data[i-9];
						sum += qlp_coeff[7] * (FLAC__int64)data[i-8];
						sum += qlp_coeff[6] * (FLAC__int64)data[i-7];
						sum += qlp_coeff[5] * (FLAC__int64)data[i-6];
						sum += qlp_coeff[4] * (FLAC__int64)data[i-5];
						sum += qlp_coeff[3] * (FLAC__int64)data[i-4];
						sum += qlp_coeff[2] * (FLAC__int64)data[i-3];
						sum += qlp_coeff[1] * (FLAC__int64)data[i-2];
						sum += qlp_coeff[0] * (FLAC__int64)data[i-1];
						residual[i] = data[i] - (sum >> lp_quantization);
					}
				}
				else { /* order == 11 */
					for(i = 0; i < (int)data_len; i++) {
						sum = 0;
						sum += qlp_coeff[10] * (FLAC__int64)data[i-11];
						sum += qlp_coeff[9] * (FLAC__int64)data[i-10];
						sum += qlp_coeff[8] * (FLAC__int64)data[i-9];
						sum += qlp_coeff[7] * (FLAC__int64)data[i-8];
						sum += qlp_coeff[6] * (FLAC__int64)data[i-7];
						sum += qlp_coeff[5] * (FLAC__int64)data[i-6];
						sum += qlp_coeff[4] * (FLAC__int64)data[i-5];
						sum += qlp_coeff[3] * (FLAC__int64)data[i-4];
						sum += qlp_coeff[2] * (FLAC__int64)data[i-3];
						sum += qlp_coeff[1] * (FLAC__int64)data[i-2];
						sum += qlp_coeff[0] * (FLAC__int64)data[i-1];
						residual[i] = data[i] - (sum >> lp_quantization);
					}
				}
			}
			else {
				if(order == 10) {
					for(i = 0; i < (int)data_len; i++) {
						sum = 0;
						sum += qlp_coeff[9] * (FLAC__int64)data[i-10];
						sum += qlp_coeff[8] * (FLAC__int64)data[i-9];
						sum += qlp_coeff[7] * (FLAC__int64)data[i-8];
						sum += qlp_coeff[6] * (FLAC__int64)data[i-7];
						sum += qlp_coeff[5] * (FLAC__int64)data[i-6];
						sum += qlp_coeff[4] * (FLAC__int64)data[i-5];
						sum += qlp_coeff[3] * (FLAC__int64)data[i-4];
						sum += qlp_coeff[2] * (FLAC__int64)data[i-3];
						sum += qlp_coeff[1] * (FLAC__int64)data[i-2];
						sum += qlp_coeff[0] * (FLAC__int64)data[i-1];
						residual[i] = data[i] - (sum >> lp_quantization);
					}
				}
				else { /* order == 9 */
					for(i = 0; i < (int)data_len; i++) {
						sum = 0;
						sum += qlp_coeff[8] * (FLAC__int64)data[i-9];
						sum += qlp_coeff[7] * (FLAC__int64)data[i-8];
						sum += qlp_coeff[6] * (FLAC__int64)data[i-7];
						sum += qlp_coeff[5] * (FLAC__int64)data[i-6];
						sum += qlp_coeff[4] * (FLAC__int64)data[i-5];
						sum += qlp_coeff[3] * (FLAC__int64)data[i-4];
						sum += qlp_coeff[2] * (FLAC__int64)data[i-3];
						sum += qlp_coeff[1] * (FLAC__int64)data[i-2];
						sum += qlp_coeff[0] * (FLAC__int64)data[i-1];
						residual[i] = data[i] - (sum >> lp_quantization);
					}
				}
			}
		}
		else if(order > 4) {
			if(order > 6) {
				if(order == 8) {
					for(i = 0; i < (int)data_len; i++) {
						sum = 0;
						sum += qlp_coeff[7] * (FLAC__int64)data[i-8];
						sum += qlp_coeff[6] * (FLAC__int64)data[i-7];
						sum += qlp_coeff[5] * (FLAC__int64)data[i-6];
						sum += qlp_coeff[4] * (FLAC__int64)data[i-5];
						sum += qlp_coeff[3] * (FLAC__int64)data[i-4];
						sum += qlp_coeff[2] * (FLAC__int64)data[i-3];
						sum += qlp_coeff[1] * (FLAC__int64)data[i-2];
						sum += qlp_coeff[0] * (FLAC__int64)data[i-1];
						residual[i] = data[i] - (sum >> lp_quantization);
					}
				}
				else { /* order == 7 */
					for(i = 0; i < (int)data_len; i++) {
						sum = 0;
						sum += qlp_coeff[6] * (FLAC__int64)data[i-7];
						sum += qlp_coeff[5] * (FLAC__int64)data[i-6];
						sum += qlp_coeff[4] * (FLAC__int64)data[i-5];
						sum += qlp_coeff[3] * (FLAC__int64)data[i-4];
						sum += qlp_coeff[2] * (FLAC__int64)data[i-3];
						sum += qlp_coeff[1] * (FLAC__int64)data[i-2];
						sum += qlp_coeff[0] * (FLAC__int64)data[i-1];
						residual[i] = data[i] - (sum >> lp_quantization);
					}
				}
			}
			else {
				if(order == 6) {
					for(i = 0; i < (int)data_len; i++) {
						sum = 0;
						sum += qlp_coeff[5] * (FLAC__int64)data[i-6];
						sum += qlp_coeff[4] * (FLAC__int64)data[i-5];
						sum += qlp_coeff[3] * (FLAC__int64)data[i-4];
						sum += qlp_coeff[2] * (FLAC__int64)data[i-3];
						sum += qlp_coeff[1] * (FLAC__int64)data[i-2];
						sum += qlp_coeff[0] * (FLAC__int64)data[i-1];
						residual[i] = data[i] - (sum >> lp_quantization);
					}
				}
				else { /* order == 5 */
					for(i = 0; i < (int)data_len; i++) {
						sum = 0;
						sum += qlp_coeff[4] * (FLAC__int64)data[i-5];
						sum += qlp_coeff[3] * (FLAC__int64)data[i-4];
						sum += qlp_coeff[2] * (FLAC__int64)data[i-3];
						sum += qlp_coeff[1] * (FLAC__int64)data[i-2];
						sum += qlp_coeff[0] * (FLAC__int64)data[i-1];
						residual[i] = data[i] - (sum >> lp_quantization);
					}
				}
			}
		}
		else {
			if(order > 2) {
				if(order == 4) {
					for(i = 0; i < (int)data_len; i++) {
						sum = 0;
						sum += qlp_coeff[3] * (FLAC__int64)data[i-4];
						sum += qlp_coeff[2] * (FLAC__int64)data[i-3];
						sum += qlp_coeff[1] * (FLAC__int64)data[i-2];
						sum += qlp_coeff[0] * (FLAC__int64)data[i-1];
						residual[i] = data[i] - (sum >> lp_quantization);
					}
				}
				else { /* order == 3 */
					for(i = 0; i < (int)data_len; i++) {
						sum = 0;
						sum += qlp_coeff[2] * (FLAC__int64)data[i-3];
						sum += qlp_coeff[1] * (FLAC__int64)data[i-2];
						sum += qlp_coeff[0] * (FLAC__int64)data[i-1];
						residual[i] = data[i] - (sum >> lp_quantization);
					}
				}
			}
			else {
				if(order == 2) {
					for(i = 0; i < (int)data_len; i++) {
						sum = 0;
						sum += qlp_coeff[1] * (FLAC__int64)data[i-2];
						sum += qlp_coeff[0] * (FLAC__int64)data[i-1];
						residual[i] = data[i] - (sum >> lp_quantization);
					}
				}
				else { /* order == 1 */
					for(i = 0; i < (int)data_len; i++)
						residual[i] = data[i] - ((qlp_coeff[0] * (FLAC__int64)data[i-1]) >> lp_quantization);
				}
			}
		}
	}
	else { /* order > 12 */
		for(i = 0; i < (int)data_len; i++) {
			sum = 0;
			switch(order) {
				case 32: sum += qlp_coeff[31] * (FLAC__int64)data[i-32]; /* Falls through. */
				case 31: sum += qlp_coeff[30] * (FLAC__int64)data[i-31]; /* Falls through. */
				case 30: sum += qlp_coeff[29] * (FLAC__int64)data[i-30]; /* Falls through. */
				case 29: sum += qlp_coeff[28] * (FLAC__int64)data[i-29]; /* Falls through. */
				case 28: sum += qlp_coeff[27] * (FLAC__int64)data[i-28]; /* Falls through. */
				case 27: sum += qlp_coeff[26] * (FLAC__int64)data[i-27]; /* Falls through. */
				case 26: sum += qlp_coeff[25] * (FLAC__int64)data[i-26]; /* Falls through. */
				case 25: sum += qlp_coeff[24] * (FLAC__int64)data[i-25]; /* Falls through. */
				case 24: sum += qlp_coeff[23] * (FLAC__int64)data[i-24]; /* Falls through. */
				case 23: sum += qlp_coeff[22] * (FLAC__int64)data[i-23]; /* Falls through. */
				case 22: sum += qlp_coeff[21] * (FLAC__int64)data[i-22]; /* Falls through. */
				case 21: sum += qlp_coeff[20] * (FLAC__int64)data[i-21]; /* Falls through. */
				case 20: sum += qlp_coeff[19] * (FLAC__int64)data[i-20]; /* Falls through. */
				case 19: sum += qlp_coeff[18] * (FLAC__int64)data[i-19]; /* Falls through. */
				case 18: sum += qlp_coeff[17] * (FLAC__int64)data[i-18]; /* Falls through. */
				case 17: sum += qlp_coeff[16] * (FLAC__int64)data[i-17]; /* Falls through. */
				case 16: sum += qlp_coeff[15] * (FLAC__int64)data[i-16]; /* Falls through. */
				case 15: sum += qlp_coeff[14] * (FLAC__int64)data[i-15]; /* Falls through. */
				case 14: sum += qlp_coeff[13] * (FLAC__int64)data[i-14]; /* Falls through. */
				case 13: sum += qlp_coeff[12] * (FLAC__int64)data[i-13];
				         sum += qlp_coeff[11] * (FLAC__int64)data[i-12];
				         sum += qlp_coeff[10] * (FLAC__int64)data[i-11];
				         sum += qlp_coeff[ 9] * (FLAC__int64)data[i-10];
				         sum += qlp_coeff[ 8] * (FLAC__int64)data[i- 9];
				         sum += qlp_coeff[ 7] * (FLAC__int64)data[i- 8];
				         sum += qlp_coeff[ 6] * (FLAC__int64)data[i- 7];
				         sum += qlp_coeff[ 5] * (FLAC__int64)data[i- 6];
				         sum += qlp_coeff[ 4] * (FLAC__int64)data[i- 5];
				         sum += qlp_coeff[ 3] * (FLAC__int64)data[i- 4];
				         sum += qlp_coeff[ 2] * (FLAC__int64)data[i- 3];
				         sum += qlp_coeff[ 1] * (FLAC__int64)data[i- 2];
				         sum += qlp_coeff[ 0] * (FLAC__int64)data[i- 1];
			}
			residual[i] = data[i] - (sum >> lp_quantization);
		}
	}
}
#endif

FLAC__bool FLAC__lpc_compute_residual_from_qlp_coefficients_limit_residual(const FLAC__int32 * flac_restrict data, uint32_t data_len, const FLAC__int32 * flac_restrict qlp_coeff, uint32_t order, int lp_quantization, FLAC__int32 * flac_restrict residual)
{
	int i;
	FLAC__int64 sum, residual_to_check;

	FLAC__ASSERT(order > 0);
	FLAC__ASSERT(order <= 32);

	for(i = 0; i < (int)data_len; i++) {
		sum = 0;
		switch(order) {
			case 32: sum += qlp_coeff[31] * (FLAC__int64)data[i-32]; /* Falls through. */
			case 31: sum += qlp_coeff[30] * (FLAC__int64)data[i-31]; /* Falls through. */
			case 30: sum += qlp_coeff[29] * (FLAC__int64)data[i-30]; /* Falls through. */
			case 29: sum += qlp_coeff[28] * (FLAC__int64)data[i-29]; /* Falls through. */
			case 28: sum += qlp_coeff[27] * (FLAC__int64)data[i-28]; /* Falls through. */
			case 27: sum += qlp_coeff[26] * (FLAC__int64)data[i-27]; /* Falls through. */
			case 26: sum += qlp_coeff[25] * (FLAC__int64)data[i-26]; /* Falls through. */
			case 25: sum += qlp_coeff[24] * (FLAC__int64)data[i-25]; /* Falls through. */
			case 24: sum += qlp_coeff[23] * (FLAC__int64)data[i-24]; /* Falls through. */
			case 23: sum += qlp_coeff[22] * (FLAC__int64)data[i-23]; /* Falls through. */
			case 22: sum += qlp_coeff[21] * (FLAC__int64)data[i-22]; /* Falls through. */
			case 21: sum += qlp_coeff[20] * (FLAC__int64)data[i-21]; /* Falls through. */
			case 20: sum += qlp_coeff[19] * (FLAC__int64)data[i-20]; /* Falls through. */
			case 19: sum += qlp_coeff[18] * (FLAC__int64)data[i-19]; /* Falls through. */
			case 18: sum += qlp_coeff[17] * (FLAC__int64)data[i-18]; /* Falls through. */
			case 17: sum += qlp_coeff[16] * (FLAC__int64)data[i-17]; /* Falls through. */
			case 16: sum += qlp_coeff[15] * (FLAC__int64)data[i-16]; /* Falls through. */
			case 15: sum += qlp_coeff[14] * (FLAC__int64)data[i-15]; /* Falls through. */
			case 14: sum += qlp_coeff[13] * (FLAC__int64)data[i-14]; /* Falls through. */
			case 13: sum += qlp_coeff[12] * (FLAC__int64)data[i-13]; /* Falls through. */
			case 12: sum += qlp_coeff[11] * (FLAC__int64)data[i-12]; /* Falls through. */
			case 11: sum += qlp_coeff[10] * (FLAC__int64)data[i-11]; /* Falls through. */
			case 10: sum += qlp_coeff[ 9] * (FLAC__int64)data[i-10]; /* Falls through. */
			case  9: sum += qlp_coeff[ 8] * (FLAC__int64)data[i- 9]; /* Falls through. */
			case  8: sum += qlp_coeff[ 7] * (FLAC__int64)data[i- 8]; /* Falls through. */
			case  7: sum += qlp_coeff[ 6] * (FLAC__int64)data[i- 7]; /* Falls through. */
			case  6: sum += qlp_coeff[ 5] * (FLAC__int64)data[i- 6]; /* Falls through. */
			case  5: sum += qlp_coeff[ 4] * (FLAC__int64)data[i- 5]; /* Falls through. */
			case  4: sum += qlp_coeff[ 3] * (FLAC__int64)data[i- 4]; /* Falls through. */
			case  3: sum += qlp_coeff[ 2] * (FLAC__int64)data[i- 3]; /* Falls through. */
			case  2: sum += qlp_coeff[ 1] * (FLAC__int64)data[i- 2]; /* Falls through. */
			case  1: sum += qlp_coeff[ 0] * (FLAC__int64)data[i- 1];
		}
		residual_to_check = data[i] - (sum >> lp_quantization);
		 /* residual must not be INT32_MIN because abs(INT32_MIN) is undefined */
		if(residual_to_check <= INT32_MIN || residual_to_check > INT32_MAX)
			return false;
		else
			residual[i] = residual_to_check;
	}
	return true;
}

FLAC__bool FLAC__lpc_compute_residual_from_qlp_coefficients_limit_residual_33bit(const FLAC__int64 * flac_restrict data, uint32_t data_len, const FLAC__int32 * flac_restrict qlp_coeff, uint32_t order, int lp_quantization, FLAC__int32 * flac_restrict residual)
{
	int i;
	FLAC__int64 sum, residual_to_check;

	FLAC__ASSERT(order > 0);
	FLAC__ASSERT(order <= 32);

	for(i = 0; i < (int)data_len; i++) {
		sum = 0;
		switch(order) {
			case 32: sum += qlp_coeff[31] * data[i-32]; /* Falls through. */
			case 31: sum += qlp_coeff[30] * data[i-31]; /* Falls through. */
			case 30: sum += qlp_coeff[29] * data[i-30]; /* Falls through. */
			case 29: sum += qlp_coeff[28] * data[i-29]; /* Falls through. */
			case 28: sum += qlp_coeff[27] * data[i-28]; /* Falls through. */
			case 27: sum += qlp_coeff[26] * data[i-27]; /* Falls through. */
			case 26: sum += qlp_coeff[25] * data[i-26]; /* Falls through. */
			case 25: sum += qlp_coeff[24] * data[i-25]; /* Falls through. */
			case 24: sum += qlp_coeff[23] * data[i-24]; /* Falls through. */
			case 23: sum += qlp_coeff[22] * data[i-23]; /* Falls through. */
			case 22: sum += qlp_coeff[21] * data[i-22]; /* Falls through. */
			case 21: sum += qlp_coeff[20] * data[i-21]; /* Falls through. */
			case 20: sum += qlp_coeff[19] * data[i-20]; /* Falls through. */
			case 19: sum += qlp_coeff[18] * data[i-19]; /* Falls through. */
			case 18: sum += qlp_coeff[17] * data[i-18]; /* Falls through. */
			case 17: sum += qlp_coeff[16] * data[i-17]; /* Falls through. */
			case 16: sum += qlp_coeff[15] * data[i-16]; /* Falls through. */
			case 15: sum += qlp_coeff[14] * data[i-15]; /* Falls through. */
			case 14: sum += qlp_coeff[13] * data[i-14]; /* Falls through. */
			case 13: sum += qlp_coeff[12] * data[i-13]; /* Falls through. */
			case 12: sum += qlp_coeff[11] * data[i-12]; /* Falls through. */
			case 11: sum += qlp_coeff[10] * data[i-11]; /* Falls through. */
			case 10: sum += qlp_coeff[ 9] * data[i-10]; /* Falls through. */
			case  9: sum += qlp_coeff[ 8] * data[i- 9]; /* Falls through. */
			case  8: sum += qlp_coeff[ 7] * data[i- 8]; /* Falls through. */
			case  7: sum += qlp_coeff[ 6] * data[i- 7]; /* Falls through. */
			case  6: sum += qlp_coeff[ 5] * data[i- 6]; /* Falls through. */
			case  5: sum += qlp_coeff[ 4] * data[i- 5]; /* Falls through. */
			case  4: sum += qlp_coeff[ 3] * data[i- 4]; /* Falls through. */
			case  3: sum += qlp_coeff[ 2] * data[i- 3]; /* Falls through. */
			case  2: sum += qlp_coeff[ 1] * data[i- 2]; /* Falls through. */
			case  1: sum += qlp_coeff[ 0] * data[i- 1];
		}
		residual_to_check = data[i] - (sum >> lp_quantization);
		/* residual must not be INT32_MIN because abs(INT32_MIN) is undefined */
		if(residual_to_check <= INT32_MIN || residual_to_check > INT32_MAX)
			return false;
		else
			residual[i] = residual_to_check;
	}
	return true;
}

#endif /* !defined FLAC__INTEGER_ONLY_LIBRARY */

uint32_t FLAC__lpc_max_prediction_before_shift_bps(uint32_t subframe_bps, const FLAC__int32 * flac_restrict qlp_coeff, uint32_t order)
{
	/* This used to be subframe_bps + qlp_coeff_precision + FLAC__bitmath_ilog2(order)
	 * but that treats both the samples as well as the predictor as unknown. The
	 * predictor is known however, so taking the log2 of the sum of the absolute values
	 * of all coefficients is a more accurate representation of the predictor */
	FLAC__int32 abs_sum_of_qlp_coeff = 0;
	uint32_t i;
	for(i = 0; i < order; i++)
		abs_sum_of_qlp_coeff += abs(qlp_coeff[i]);
	if(abs_sum_of_qlp_coeff == 0)
		abs_sum_of_qlp_coeff = 1;
	return subframe_bps + FLAC__bitmath_silog2(abs_sum_of_qlp_coeff);
}


uint32_t FLAC__lpc_max_residual_bps(uint32_t subframe_bps, const FLAC__int32 * flac_restrict qlp_coeff, uint32_t order, int lp_quantization)
{
	FLAC__int32 predictor_sum_bps = FLAC__lpc_max_prediction_before_shift_bps(subframe_bps, qlp_coeff, order) - lp_quantization;
	if((int)subframe_bps > predictor_sum_bps)
		return subframe_bps + 1;
	else
		return predictor_sum_bps + 1;
}

#if defined(FUZZING_BUILD_MODE_UNSAFE_FOR_PRODUCTION) && !defined(FUZZING_BUILD_MODE_FLAC_SANITIZE_SIGNED_INTEGER_OVERFLOW)
/* The attribute below is to silence the undefined sanitizer of oss-fuzz.
 * Because fuzzing feeds bogus predictors and residual samples to the
 * decoder, having overflows in this section is unavoidable. Also,
 * because the calculated values are audio path only, there is no
 * potential for security problems */
__attribute__((no_sanitize("signed-integer-overflow")))
#endif
void FLAC__lpc_restore_signal(const FLAC__int32 * flac_restrict residual, uint32_t data_len, const FLAC__int32 * flac_restrict qlp_coeff, uint32_t order, int lp_quantization, FLAC__int32 * flac_restrict data)
#if defined(FLAC__OVERFLOW_DETECT) || !defined(FLAC__LPC_UNROLLED_FILTER_LOOPS)
{
	FLAC__int64 sumo;
	uint32_t i, j;
	FLAC__int32 sum;
	const FLAC__int32 *r = residual, *history;

#ifdef FLAC__OVERFLOW_DETECT_VERBOSE
	fprintf(stderr,"FLAC__lpc_restore_signal: data_len=%d, order=%u, lpq=%d",data_len,order,lp_quantization);
	for(i=0;i<order;i++)
		fprintf(stderr,", q[%u]=%d",i,qlp_coeff[i]);
	fprintf(stderr,"\n");
#endif
	FLAC__ASSERT(order > 0);

	for(i = 0; i < data_len; i++) {
		sumo = 0;
		sum = 0;
		history = data;
		for(j = 0; j < order; j++) {
			sum += qlp_coeff[j] * (*(--history));
			sumo += (FLAC__int64)qlp_coeff[j] * (FLAC__int64)(*history);
#ifdef FLAC__OVERFLOW_DETECT
			if(sumo > 2147483647ll || sumo < -2147483648ll)
				fprintf(stderr,"FLAC__lpc_restore_signal: OVERFLOW, i=%u, j=%u, c=%d, d=%d, sumo=%" PRId64 "\n",i,j,qlp_coeff[j],*history,sumo);
#endif
		}
		*(data++) = *(r++) + (sum >> lp_quantization);
	}

	/* Here's a slower but clearer version:
	for(i = 0; i < data_len; i++) {
		sum = 0;
		for(j = 0; j < order; j++)
			sum += qlp_coeff[j] * data[i-j-1];
		data[i] = residual[i] + (sum >> lp_quantization);
	}
	*/
}
#else /* fully unrolled version for normal use */
{
	int i;
	FLAC__int32 sum;

	FLAC__ASSERT(order > 0);
	FLAC__ASSERT(order <= 32);

	/*
	 * We do unique versions up to 12th order since that's the subset limit.
	 * Also they are roughly ordered to match frequency of occurrence to
	 * minimize branching.
	 */
	if(order <= 12) {
		if(order > 8) {
			if(order > 10) {
				if(order == 12) {
					for(i = 0; i < (int)data_len; i++) {
						sum = 0;
						sum += qlp_coeff[11] * data[i-12];
						sum += qlp_coeff[10] * data[i-11];
						sum += qlp_coeff[9] * data[i-10];
						sum += qlp_coeff[8] * data[i-9];
						sum += qlp_coeff[7] * data[i-8];
						sum += qlp_coeff[6] * data[i-7];
						sum += qlp_coeff[5] * data[i-6];
						sum += qlp_coeff[4] * data[i-5];
						sum += qlp_coeff[3] * data[i-4];
						sum += qlp_coeff[2] * data[i-3];
						sum += qlp_coeff[1] * data[i-2];
						sum += qlp_coeff[0] * data[i-1];
						data[i] = residual[i] + (sum >> lp_quantization);
					}
				}
				else { /* order == 11 */
					for(i = 0; i < (int)data_len; i++) {
						sum = 0;
						sum += qlp_coeff[10] * data[i-11];
						sum += qlp_coeff[9] * data[i-10];
						sum += qlp_coeff[8] * data[i-9];
						sum += qlp_coeff[7] * data[i-8];
						sum += qlp_coeff[6] * data[i-7];
						sum += qlp_coeff[5] * data[i-6];
						sum += qlp_coeff[4] * data[i-5];
						sum += qlp_coeff[3] * data[i-4];
						sum += qlp_coeff[2] * data[i-3];
						sum += qlp_coeff[1] * data[i-2];
						sum += qlp_coeff[0] * data[i-1];
						data[i] = residual[i] + (sum >> lp_quantization);
					}
				}
			}
			else {
				if(order == 10) {
					for(i = 0; i < (int)data_len; i++) {
						sum = 0;
						sum += qlp_coeff[9] * data[i-10];
						sum += qlp_coeff[8] * data[i-9];
						sum += qlp_coeff[7] * data[i-8];
						sum += qlp_coeff[6] * data[i-7];
						sum += qlp_coeff[5] * data[i-6];
						sum += qlp_coeff[4] * data[i-5];
						sum += qlp_coeff[3] * data[i-4];
						sum += qlp_coeff[2] * data[i-3];
						sum += qlp_coeff[1] * data[i-2];
						sum += qlp_coeff[0] * data[i-1];
						data[i] = residual[i] + (sum >> lp_quantization);
					}
				}
				else { /* order == 9 */
					for(i = 0; i < (int)data_len; i++) {
						sum = 0;
						sum += qlp_coeff[8] * data[i-9];
						sum += qlp_coeff[7] * data[i-8];
						sum += qlp_coeff[6] * data[i-7];
						sum += qlp_coeff[5] * data[i-6];
						sum += qlp_coeff[4] * data[i-5];
						sum += qlp_coeff[3] * data[i-4];
						sum += qlp_coeff[2] * data[i-3];
						sum += qlp_coeff[1] * data[i-2];
						sum += qlp_coeff[0] * data[i-1];
						data[i] = residual[i] + (sum >> lp_quantization);
					}
				}
			}
		}
		else if(order > 4) {
			if(order > 6) {
				if(order == 8) {
					for(i = 0; i < (int)data_len; i++) {
						sum = 0;
						sum += qlp_coeff[7] * data[i-8];
						sum += qlp_coeff[6] * data[i-7];
						sum += qlp_coeff[5] * data[i-6];
						sum += qlp_coeff[4] * data[i-5];
						sum += qlp_coeff[3] * data[i-4];
						sum += qlp_coeff[2] * data[i-3];
						sum += qlp_coeff[1] * data[i-2];
						sum += qlp_coeff[0] * data[i-1];
						data[i] = residual[i] + (sum >> lp_quantization);
					}
				}
				else { /* order == 7 */
					for(i = 0; i < (int)data_len; i++) {
						sum = 0;
						sum += qlp_coeff[6] * data[i-7];
						sum += qlp_coeff[5] * data[i-6];
						sum += qlp_coeff[4] * data[i-5];
						sum += qlp_coeff[3] * data[i-4];
						sum += qlp_coeff[2] * data[i-3];
						sum += qlp_coeff[1] * data[i-2];
						sum += qlp_coeff[0] * data[i-1];
						data[i] = residual[i] + (sum >> lp_quantization);
					}
				}
			}
			else {
				if(order == 6) {
					for(i = 0; i < (int)data_len; i++) {
						sum = 0;
						sum += qlp_coeff[5] * data[i-6];
						sum += qlp_coeff[4] * data[i-5];
						sum += qlp_coeff[3] * data[i-4];
						sum += qlp_coeff[2] * data[i-3];
						sum += qlp_coeff[1] * data[i-2];
						sum += qlp_coeff[0] * data[i-1];
						data[i] = residual[i] + (sum >> lp_quantization);
					}
				}
				else { /* order == 5 */
					for(i = 0; i < (int)data_len; i++) {
						sum = 0;
						sum += qlp_coeff[4] * data[i-5];
						sum += qlp_coeff[3] * data[i-4];
						sum += qlp_coeff[2] * data[i-3];
						sum += qlp_coeff[1] * data[i-2];
						sum += qlp_coeff[0] * data[i-1];
						data[i] = residual[i] + (sum >> lp_quantization);
					}
				}
			}
		}
		else {
			if(order > 2) {
				if(order == 4) {
					for(i = 0; i < (int)data_len; i++) {
						sum = 0;
						sum += qlp_coeff[3] * data[i-4];
						sum += qlp_coeff[2] * data[i-3];
						sum += qlp_coeff[1] * data[i-2];
						sum += qlp_coeff[0] * data[i-1];
						data[i] = residual[i] + (sum >> lp_quantization);
					}
				}
				else { /* order == 3 */
					for(i = 0; i < (int)data_len; i++) {
						sum = 0;
						sum += qlp_coeff[2] * data[i-3];
						sum += qlp_coeff[1] * data[i-2];
						sum += qlp_coeff[0] * data[i-1];
						data[i] = residual[i] + (sum >> lp_quantization);
					}
				}
			}
			else {
				if(order == 2) {
					for(i = 0; i < (int)data_len; i++) {
						sum = 0;
						sum += qlp_coeff[1] * data[i-2];
						sum += qlp_coeff[0] * data[i-1];
						data[i] = residual[i] + (sum >> lp_quantization);
					}
				}
				else { /* order == 1 */
					for(i = 0; i < (int)data_len; i++)
						data[i] = residual[i] + ((qlp_coeff[0] * data[i-1]) >> lp_quantization);
				}
			}
		}
	}
	else { /* order > 12 */
		for(i = 0; i < (int)data_len; i++) {
			sum = 0;
			switch(order) {
				case 32: sum += qlp_coeff[31] * data[i-32]; /* Falls through. */
				case 31: sum += qlp_coeff[30] * data[i-31]; /* Falls through. */
				case 30: sum += qlp_coeff[29] * data[i-30]; /* Falls through. */
				case 29: sum += qlp_coeff[28] * data[i-29]; /* Falls through. */
				case 28: sum += qlp_coeff[27] * data[i-28]; /* Falls through. */
				case 27: sum += qlp_coeff[26] * data[i-27]; /* Falls through. */
				case 26: sum += qlp_coeff[25] * data[i-26]; /* Falls through. */
				case 25: sum += qlp_coeff[24] * data[i-25]; /* Falls through. */
				case 24: sum += qlp_coeff[23] * data[i-24]; /* Falls through. */
				case 23: sum += qlp_coeff[22] * data[i-23]; /* Falls through. */
				case 22: sum += qlp_coeff[21] * data[i-22]; /* Falls through. */
				case 21: sum += qlp_coeff[20] * data[i-21]; /* Falls through. */
				case 20: sum += qlp_coeff[19] * data[i-20]; /* Falls through. */
				case 19: sum += qlp_coeff[18] * data[i-19]; /* Falls through. */
				case 18: sum += qlp_coeff[17] * data[i-18]; /* Falls through. */
				case 17: sum += qlp_coeff[16] * data[i-17]; /* Falls through. */
				case 16: sum += qlp_coeff[15] * data[i-16]; /* Falls through. */
				case 15: sum += qlp_coeff[14] * data[i-15]; /* Falls through. */
				case 14: sum += qlp_coeff[13] * data[i-14]; /* Falls through. */
				case 13: sum += qlp_coeff[12] * data[i-13];
				         sum += qlp_coeff[11] * data[i-12];
				         sum += qlp_coeff[10] * data[i-11];
				         sum += qlp_coeff[ 9] * data[i-10];
				         sum += qlp_coeff[ 8] * data[i- 9];
				         sum += qlp_coeff[ 7] * data[i- 8];
				         sum += qlp_coeff[ 6] * data[i- 7];
				         sum += qlp_coeff[ 5] * data[i- 6];
				         sum += qlp_coeff[ 4] * data[i- 5];
				         sum += qlp_coeff[ 3] * data[i- 4];
				         sum += qlp_coeff[ 2] * data[i- 3];
				         sum += qlp_coeff[ 1] * data[i- 2];
				         sum += qlp_coeff[ 0] * data[i- 1];
			}
			data[i] = residual[i] + (sum >> lp_quantization);
		}
	}
}
#endif

void FLAC__lpc_restore_signal_wide(const FLAC__int32 * flac_restrict residual, uint32_t data_len, const FLAC__int32 * flac_restrict qlp_coeff, uint32_t order, int lp_quantization, FLAC__int32 * flac_restrict data)
#if defined(FLAC__OVERFLOW_DETECT) || !defined(FLAC__LPC_UNROLLED_FILTER_LOOPS)
{
	uint32_t i, j;
	FLAC__int64 sum;
	const FLAC__int32 *r = residual, *history;

#ifdef FLAC__OVERFLOW_DETECT_VERBOSE
	fprintf(stderr,"FLAC__lpc_restore_signal_wide: data_len=%d, order=%u, lpq=%d",data_len,order,lp_quantization);
	for(i=0;i<order;i++)
		fprintf(stderr,", q[%u]=%d",i,qlp_coeff[i]);
	fprintf(stderr,"\n");
#endif
	FLAC__ASSERT(order > 0);

	for(i = 0; i < data_len; i++) {
		sum = 0;
		history = data;
		for(j = 0; j < order; j++)
			sum += (FLAC__int64)qlp_coeff[j] * (FLAC__int64)(*(--history));
#ifdef FLAC__OVERFLOW_DETECT
		if(FLAC__bitmath_silog2((FLAC__int64)(*r) + (sum >> lp_quantization)) > 32) {
			fprintf(stderr,"FLAC__lpc_restore_signal_wide: OVERFLOW, i=%u, residual=%d, sum=%" PRId64 ", data=%" PRId64 "\n", i, *r, (sum >> lp_quantization), ((FLAC__int64)(*r) + (sum >> lp_quantization)));
			break;
		}
#endif
		*(data++) = (FLAC__int32)(*(r++) + (sum >> lp_quantization));
	}
}
#else /* fully unrolled version for normal use */
{
	int i;
	FLAC__int64 sum;

	FLAC__ASSERT(order > 0);
	FLAC__ASSERT(order <= 32);

	/*
	 * We do unique versions up to 12th order since that's the subset limit.
	 * Also they are roughly ordered to match frequency of occurrence to
	 * minimize branching.
	 */
	if(order <= 12) {
		if(order > 8) {
			if(order > 10) {
				if(order == 12) {
					for(i = 0; i < (int)data_len; i++) {
						sum = 0;
						sum += qlp_coeff[11] * (FLAC__int64)data[i-12];
						sum += qlp_coeff[10] * (FLAC__int64)data[i-11];
						sum += qlp_coeff[9] * (FLAC__int64)data[i-10];
						sum += qlp_coeff[8] * (FLAC__int64)data[i-9];
						sum += qlp_coeff[7] * (FLAC__int64)data[i-8];
						sum += qlp_coeff[6] * (FLAC__int64)data[i-7];
						sum += qlp_coeff[5] * (FLAC__int64)data[i-6];
						sum += qlp_coeff[4] * (FLAC__int64)data[i-5];
						sum += qlp_coeff[3] * (FLAC__int64)data[i-4];
						sum += qlp_coeff[2] * (FLAC__int64)data[i-3];
						sum += qlp_coeff[1] * (FLAC__int64)data[i-2];
						sum += qlp_coeff[0] * (FLAC__int64)data[i-1];
						data[i] = (FLAC__int32) (residual[i] + (sum >> lp_quantization));
					}
				}
				else { /* order == 11 */
					for(i = 0; i < (int)data_len; i++) {
						sum = 0;
						sum += qlp_coeff[10] * (FLAC__int64)data[i-11];
						sum += qlp_coeff[9] * (FLAC__int64)data[i-10];
						sum += qlp_coeff[8] * (FLAC__int64)data[i-9];
						sum += qlp_coeff[7] * (FLAC__int64)data[i-8];
						sum += qlp_coeff[6] * (FLAC__int64)data[i-7];
						sum += qlp_coeff[5] * (FLAC__int64)data[i-6];
						sum += qlp_coeff[4] * (FLAC__int64)data[i-5];
						sum += qlp_coeff[3] * (FLAC__int64)data[i-4];
						sum += qlp_coeff[2] * (FLAC__int64)data[i-3];
						sum += qlp_coeff[1] * (FLAC__int64)data[i-2];
						sum += qlp_coeff[0] * (FLAC__int64)data[i-1];
						data[i] = (FLAC__int32) (residual[i] + (sum >> lp_quantization));
					}
				}
			}
			else {
				if(order == 10) {
					for(i = 0; i < (int)data_len; i++) {
						sum = 0;
						sum += qlp_coeff[9] * (FLAC__int64)data[i-10];
						sum += qlp_coeff[8] * (FLAC__int64)data[i-9];
						sum += qlp_coeff[7] * (FLAC__int64)data[i-8];
						sum += qlp_coeff[6] * (FLAC__int64)data[i-7];
						sum += qlp_coeff[5] * (FLAC__int64)data[i-6];
						sum += qlp_coeff[4] * (FLAC__int64)data[i-5];
						sum += qlp_coeff[3] * (FLAC__int64)data[i-4];
						sum += qlp_coeff[2] * (FLAC__int64)data[i-3];
						sum += qlp_coeff[1] * (FLAC__int64)data[i-2];
						sum += qlp_coeff[0] * (FLAC__int64)data[i-1];
						data[i] = (FLAC__int32) (residual[i] + (sum >> lp_quantization));
					}
				}
				else { /* order == 9 */
					for(i = 0; i < (int)data_len; i++) {
						sum = 0;
						sum += qlp_coeff[8] * (FLAC__int64)data[i-9];
						sum += qlp_coeff[7] * (FLAC__int64)data[i-8];
						sum += qlp_coeff[6] * (FLAC__int64)data[i-7];
						sum += qlp_coeff[5] * (FLAC__int64)data[i-6];
						sum += qlp_coeff[4] * (FLAC__int64)data[i-5];
						sum += qlp_coeff[3] * (FLAC__int64)data[i-4];
						sum += qlp_coeff[2] * (FLAC__int64)data[i-3];
						sum += qlp_coeff[1] * (FLAC__int64)data[i-2];
						sum += qlp_coeff[0] * (FLAC__int64)data[i-1];
						data[i] = (FLAC__int32) (residual[i] + (sum >> lp_quantization));
					}
				}
			}
		}
		else if(order > 4) {
			if(order > 6) {
				if(order == 8) {
					for(i = 0; i < (int)data_len; i++) {
						sum = 0;
						sum += qlp_coeff[7] * (FLAC__int64)data[i-8];
						sum += qlp_coeff[6] * (FLAC__int64)data[i-7];
						sum += qlp_coeff[5] * (FLAC__int64)data[i-6];
						sum += qlp_coeff[4] * (FLAC__int64)data[i-5];
						sum += qlp_coeff[3] * (FLAC__int64)data[i-4];
						sum += qlp_coeff[2] * (FLAC__int64)data[i-3];
						sum += qlp_coeff[1] * (FLAC__int64)data[i-2];
						sum += qlp_coeff[0] * (FLAC__int64)data[i-1];
						data[i] = (FLAC__int32) (residual[i] + (sum >> lp_quantization));
					}
				}
				else { /* order == 7 */
					for(i = 0; i < (int)data_len; i++) {
						sum = 0;
						sum += qlp_coeff[6] * (FLAC__int64)data[i-7];
						sum += qlp_coeff[5] * (FLAC__int64)data[i-6];
						sum += qlp_coeff[4] * (FLAC__int64)data[i-5];
						sum += qlp_coeff[3] * (FLAC__int64)data[i-4];
						sum += qlp_coeff[2] * (FLAC__int64)data[i-3];
						sum += qlp_coeff[1] * (FLAC__int64)data[i-2];
						sum += qlp_coeff[0] * (FLAC__int64)data[i-1];
						data[i] = (FLAC__int32) (residual[i] + (sum >> lp_quantization));
					}
				}
			}
			else {
				if(order == 6) {
					for(i = 0; i < (int)data_len; i++) {
						sum = 0;
						sum += qlp_coeff[5] * (FLAC__int64)data[i-6];
						sum += qlp_coeff[4] * (FLAC__int64)data[i-5];
						sum += qlp_coeff[3] * (FLAC__int64)data[i-4];
						sum += qlp_coeff[2] * (FLAC__int64)data[i-3];
						sum += qlp_coeff[1] * (FLAC__int64)data[i-2];
						sum += qlp_coeff[0] * (FLAC__int64)data[i-1];
						data[i] = (FLAC__int32) (residual[i] + (sum >> lp_quantization));
					}
				}
				else { /* order == 5 */
					for(i = 0; i < (int)data_len; i++) {
						sum = 0;
						sum += qlp_coeff[4] * (FLAC__int64)data[i-5];
						sum += qlp_coeff[3] * (FLAC__int64)data[i-4];
						sum += qlp_coeff[2] * (FLAC__int64)data[i-3];
						sum += qlp_coeff[1] * (FLAC__int64)data[i-2];
						sum += qlp_coeff[0] * (FLAC__int64)data[i-1];
						data[i] = (FLAC__int32) (residual[i] + (sum >> lp_quantization));
					}
				}
			}
		}
		else {
			if(order > 2) {
				if(order == 4) {
					for(i = 0; i < (int)data_len; i++) {
						sum = 0;
						sum += qlp_coeff[3] * (FLAC__int64)data[i-4];
						sum += qlp_coeff[2] * (FLAC__int64)data[i-3];
						sum += qlp_coeff[1] * (FLAC__int64)data[i-2];
						sum += qlp_coeff[0] * (FLAC__int64)data[i-1];
						data[i] = (FLAC__int32) (residual[i] + (sum >> lp_quantization));
					}
				}
				else { /* order == 3 */
					for(i = 0; i < (int)data_len; i++) {
						sum = 0;
						sum += qlp_coeff[2] * (FLAC__int64)data[i-3];
						sum += qlp_coeff[1] * (FLAC__int64)data[i-2];
						sum += qlp_coeff[0] * (FLAC__int64)data[i-1];
						data[i] = (FLAC__int32) (residual[i] + (sum >> lp_quantization));
					}
				}
			}
			else {
				if(order == 2) {
					for(i = 0; i < (int)data_len; i++) {
						sum = 0;
						sum += qlp_coeff[1] * (FLAC__int64)data[i-2];
						sum += qlp_coeff[0] * (FLAC__int64)data[i-1];
						data[i] = (FLAC__int32) (residual[i] + (sum >> lp_quantization));
					}
				}
				else { /* order == 1 */
					for(i = 0; i < (int)data_len; i++)
						data[i] = (FLAC__int32)(residual[i] + ((qlp_coeff[0] * (FLAC__int64)data[i-1]) >> lp_quantization));
				}
			}
		}
	}
	else { /* order > 12 */
		for(i = 0; i < (int)data_len; i++) {
			sum = 0;
			switch(order) {
				case 32: sum += qlp_coeff[31] * (FLAC__int64)data[i-32]; /* Falls through. */
				case 31: sum += qlp_coeff[30] * (FLAC__int64)data[i-31]; /* Falls through. */
				case 30: sum += qlp_coeff[29] * (FLAC__int64)data[i-30]; /* Falls through. */
				case 29: sum += qlp_coeff[28] * (FLAC__int64)data[i-29]; /* Falls through. */
				case 28: sum += qlp_coeff[27] * (FLAC__int64)data[i-28]; /* Falls through. */
				case 27: sum += qlp_coeff[26] * (FLAC__int64)data[i-27]; /* Falls through. */
				case 26: sum += qlp_coeff[25] * (FLAC__int64)data[i-26]; /* Falls through. */
				case 25: sum += qlp_coeff[24] * (FLAC__int64)data[i-25]; /* Falls through. */
				case 24: sum += qlp_coeff[23] * (FLAC__int64)data[i-24]; /* Falls through. */
				case 23: sum += qlp_coeff[22] * (FLAC__int64)data[i-23]; /* Falls through. */
				case 22: sum += qlp_coeff[21] * (FLAC__int64)data[i-22]; /* Falls through. */
				case 21: sum += qlp_coeff[20] * (FLAC__int64)data[i-21]; /* Falls through. */
				case 20: sum += qlp_coeff[19] * (FLAC__int64)data[i-20]; /* Falls through. */
				case 19: sum += qlp_coeff[18] * (FLAC__int64)data[i-19]; /* Falls through. */
				case 18: sum += qlp_coeff[17] * (FLAC__int64)data[i-18]; /* Falls through. */
				case 17: sum += qlp_coeff[16] * (FLAC__int64)data[i-17]; /* Falls through. */
				case 16: sum += qlp_coeff[15] * (FLAC__int64)data[i-16]; /* Falls through. */
				case 15: sum += qlp_coeff[14] * (FLAC__int64)data[i-15]; /* Falls through. */
				case 14: sum += qlp_coeff[13] * (FLAC__int64)data[i-14]; /* Falls through. */
				case 13: sum += qlp_coeff[12] * (FLAC__int64)data[i-13];
				         sum += qlp_coeff[11] * (FLAC__int64)data[i-12];
				         sum += qlp_coeff[10] * (FLAC__int64)data[i-11];
				         sum += qlp_coeff[ 9] * (FLAC__int64)data[i-10];
				         sum += qlp_coeff[ 8] * (FLAC__int64)data[i- 9];
				         sum += qlp_coeff[ 7] * (FLAC__int64)data[i- 8];
				         sum += qlp_coeff[ 6] * (FLAC__int64)data[i- 7];
				         sum += qlp_coeff[ 5] * (FLAC__int64)data[i- 6];
				         sum += qlp_coeff[ 4] * (FLAC__int64)data[i- 5];
				         sum += qlp_coeff[ 3] * (FLAC__int64)data[i- 4];
				         sum += qlp_coeff[ 2] * (FLAC__int64)data[i- 3];
				         sum += qlp_coeff[ 1] * (FLAC__int64)data[i- 2];
				         sum += qlp_coeff[ 0] * (FLAC__int64)data[i- 1];
			}
			data[i] = (FLAC__int32) (residual[i] + (sum >> lp_quantization));
		}
	}
}
#endif

#if defined(FUZZING_BUILD_MODE_UNSAFE_FOR_PRODUCTION) && !defined(FUZZING_BUILD_MODE_FLAC_SANITIZE_SIGNED_INTEGER_OVERFLOW)
/* The attribute below is to silence the undefined sanitizer of oss-fuzz.
 * Because fuzzing feeds bogus predictors and residual samples to the
 * decoder, having overflows in this section is unavoidable. Also,
 * because the calculated values are audio path only, there is no
 * potential for security problems */
__attribute__((no_sanitize("signed-integer-overflow")))
#endif
void FLAC__lpc_restore_signal_wide_33bit(const FLAC__int32 * flac_restrict residual, uint32_t data_len, const FLAC__int32 * flac_restrict qlp_coeff, uint32_t order, int lp_quantization, FLAC__int64 * flac_restrict data)
#if defined(FLAC__OVERFLOW_DETECT) || !defined(FLAC__LPC_UNROLLED_FILTER_LOOPS)
{
	uint32_t i, j;
	FLAC__int64 sum;
	const FLAC__int32 *r = residual;
	const FLAC__int64 *history;

	FLAC__ASSERT(order > 0);

	for(i = 0; i < data_len; i++) {
		sum = 0;
		history = data;
		for(j = 0; j < order; j++)
			sum += (FLAC__int64)qlp_coeff[j] * (FLAC__int64)(*(--history));
#ifdef FLAC__OVERFLOW_DETECT
		if(FLAC__bitmath_silog2((FLAC__int64)(*r) + (sum >> lp_quantization)) > 33) {
			fprintf(stderr,"FLAC__lpc_restore_signal_33bit: OVERFLOW, i=%u, residual=%d, sum=%" PRId64 ", data=%" PRId64 "\n", i, *r, (sum >> lp_quantization), ((FLAC__int64)(*r) + (sum >> lp_quantization)));
			break;
		}
#endif
		*(data++) = (FLAC__int64)(*(r++)) + (sum >> lp_quantization);
	}
}
#else /* unrolled version for normal use */
{
	int i;
	FLAC__int64 sum;

	FLAC__ASSERT(order > 0);
	FLAC__ASSERT(order <= 32);

	for(i = 0; i < (int)data_len; i++) {
		sum = 0;
		switch(order) {
			case 32: sum += qlp_coeff[31] * data[i-32]; /* Falls through. */
			case 31: sum += qlp_coeff[30] * data[i-31]; /* Falls through. */
			case 30: sum += qlp_coeff[29] * data[i-30]; /* Falls through. */
			case 29: sum += qlp_coeff[28] * data[i-29]; /* Falls through. */
			case 28: sum += qlp_coeff[27] * data[i-28]; /* Falls through. */
			case 27: sum += qlp_coeff[26] * data[i-27]; /* Falls through. */
			case 26: sum += qlp_coeff[25] * data[i-26]; /* Falls through. */
			case 25: sum += qlp_coeff[24] * data[i-25]; /* Falls through. */
			case 24: sum += qlp_coeff[23] * data[i-24]; /* Falls through. */
			case 23: sum += qlp_coeff[22] * data[i-23]; /* Falls through. */
			case 22: sum += qlp_coeff[21] * data[i-22]; /* Falls through. */
			case 21: sum += qlp_coeff[20] * data[i-21]; /* Falls through. */
			case 20: sum += qlp_coeff[19] * data[i-20]; /* Falls through. */
			case 19: sum += qlp_coeff[18] * data[i-19]; /* Falls through. */
			case 18: sum += qlp_coeff[17] * data[i-18]; /* Falls through. */
			case 17: sum += qlp_coeff[16] * data[i-17]; /* Falls through. */
			case 16: sum += qlp_coeff[15] * data[i-16]; /* Falls through. */
			case 15: sum += qlp_coeff[14] * data[i-15]; /* Falls through. */
			case 14: sum += qlp_coeff[13] * data[i-14]; /* Falls through. */
			case 13: sum += qlp_coeff[12] * data[i-13]; /* Falls through. */
			case 12: sum += qlp_coeff[11] * data[i-12]; /* Falls through. */
			case 11: sum += qlp_coeff[10] * data[i-11]; /* Falls through. */
			case 10: sum += qlp_coeff[ 9] * data[i-10]; /* Falls through. */
			case  9: sum += qlp_coeff[ 8] * data[i- 9]; /* Falls through. */
			case  8: sum += qlp_coeff[ 7] * data[i- 8]; /* Falls through. */
			case  7: sum += qlp_coeff[ 6] * data[i- 7]; /* Falls through. */
			case  6: sum += qlp_coeff[ 5] * data[i- 6]; /* Falls through. */
			case  5: sum += qlp_coeff[ 4] * data[i- 5]; /* Falls through. */
			case  4: sum += qlp_coeff[ 3] * data[i- 4]; /* Falls through. */
			case  3: sum += qlp_coeff[ 2] * data[i- 3]; /* Falls through. */
			case  2: sum += qlp_coeff[ 1] * data[i- 2]; /* Falls through. */
			case  1: sum += qlp_coeff[ 0] * data[i- 1];
		}
		data[i] = residual[i] + (sum >> lp_quantization);
	}
}
#endif

#if defined(_MSC_VER)
#pragma warning ( default : 4028 )
#endif

#ifndef FLAC__INTEGER_ONLY_LIBRARY

double FLAC__lpc_compute_expected_bits_per_residual_sample(double lpc_error, uint32_t total_samples)
{
	double error_scale;

	FLAC__ASSERT(total_samples > 0);

	error_scale = 0.5 / (double)total_samples;

	return FLAC__lpc_compute_expected_bits_per_residual_sample_with_error_scale(lpc_error, error_scale);
}

double FLAC__lpc_compute_expected_bits_per_residual_sample_with_error_scale(double lpc_error, double error_scale)
{
	if(lpc_error > 0.0) {
		double bps = (double)0.5 * log(error_scale * lpc_error) / M_LN2;
		if(bps >= 0.0)
			return bps;
		else
			return 0.0;
	}
	else if(lpc_error < 0.0) { /* error should not be negative but can happen due to inadequate floating-point resolution */
		return 1e32;
	}
	else {
		return 0.0;
	}
}

uint32_t FLAC__lpc_compute_best_order(const double lpc_error[], uint32_t max_order, uint32_t total_samples, uint32_t overhead_bits_per_order)
{
	uint32_t order, indx, best_index; /* 'index' the index into lpc_error; index==order-1 since lpc_error[0] is for order==1, lpc_error[1] is for order==2, etc */
	double bits, best_bits, error_scale;

	FLAC__ASSERT(max_order > 0);
	FLAC__ASSERT(total_samples > 0);

	error_scale = 0.5 / (double)total_samples;

	best_index = 0;
	best_bits = (uint32_t)(-1);

	for(indx = 0, order = 1; indx < max_order; indx++, order++) {
		bits = FLAC__lpc_compute_expected_bits_per_residual_sample_with_error_scale(lpc_error[indx], error_scale) * (double)(total_samples - order) + (double)(order * overhead_bits_per_order);
		if(bits < best_bits) {
			best_index = indx;
			best_bits = bits;
		}
	}

	return best_index+1; /* +1 since indx of lpc_error[] is order-1 */
}

#endif /* !defined FLAC__INTEGER_ONLY_LIBRARY */
