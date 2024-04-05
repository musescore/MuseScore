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

#include "private/cpu.h"

#ifndef FLAC__INTEGER_ONLY_LIBRARY
#ifndef FLAC__NO_ASM
#if (defined FLAC__CPU_IA32 || defined FLAC__CPU_X86_64) && defined FLAC__HAS_X86INTRIN
#include "private/fixed.h"
#ifdef FLAC__SSE2_SUPPORTED

#include <emmintrin.h> /* SSE2 */
#include <math.h>
#include "private/macros.h"
#include "share/compat.h"
#include "FLAC/assert.h"

#ifdef FLAC__CPU_IA32
#define m128i_to_i64(dest, src) _mm_storel_epi64((__m128i*)&dest, src)
#else
#define m128i_to_i64(dest, src) dest = _mm_cvtsi128_si64(src)
#endif

#ifdef local_abs
#undef local_abs
#endif
#define local_abs(x) ((uint32_t)((x)<0? -(x) : (x)))

FLAC__SSE_TARGET("sse2")
uint32_t FLAC__fixed_compute_best_predictor_intrin_sse2(const FLAC__int32 data[], uint32_t data_len, float residual_bits_per_sample[FLAC__MAX_FIXED_ORDER + 1])
{
	FLAC__uint32 total_error_0, total_error_1, total_error_2, total_error_3, total_error_4;
	FLAC__int32 i, data_len_int;
	uint32_t order;
	__m128i total_err0, total_err1, total_err2, total_err3, total_err4;
	__m128i prev_err0,  prev_err1,  prev_err2,  prev_err3;
	__m128i tempA, tempB, bitmask;
	FLAC__int32 data_scalar[4];
	FLAC__int32 prev_err0_scalar[4];
	FLAC__int32 prev_err1_scalar[4];
	FLAC__int32 prev_err2_scalar[4];
	FLAC__int32 prev_err3_scalar[4];
	total_err0 = _mm_setzero_si128();
	total_err1 = _mm_setzero_si128();
	total_err2 = _mm_setzero_si128();
	total_err3 = _mm_setzero_si128();
	total_err4 = _mm_setzero_si128();
	data_len_int = data_len;

	for(i = 0; i < 4; i++){
		prev_err0_scalar[i] = data[-1+i*(data_len_int/4)];
		prev_err1_scalar[i] = data[-1+i*(data_len_int/4)] - data[-2+i*(data_len_int/4)];
		prev_err2_scalar[i] = prev_err1_scalar[i] - (data[-2+i*(data_len_int/4)] - data[-3+i*(data_len_int/4)]);
		prev_err3_scalar[i] = prev_err2_scalar[i] - (data[-2+i*(data_len_int/4)] - 2*data[-3+i*(data_len_int/4)] + data[-4+i*(data_len_int/4)]);
	}
	prev_err0 = _mm_loadu_si128((const __m128i*)prev_err0_scalar);
	prev_err1 = _mm_loadu_si128((const __m128i*)prev_err1_scalar);
	prev_err2 = _mm_loadu_si128((const __m128i*)prev_err2_scalar);
	prev_err3 = _mm_loadu_si128((const __m128i*)prev_err3_scalar);
	for(i = 0; i < data_len_int / 4; i++){
		data_scalar[0] = data[i];
		data_scalar[1] = data[i+data_len/4];
		data_scalar[2] = data[i+2*(data_len/4)];
		data_scalar[3] = data[i+3*(data_len/4)];
		tempA = _mm_loadu_si128((const __m128i*)data_scalar);
		/* Next three intrinsics calculate tempB as abs of tempA */
		bitmask = _mm_srai_epi32(tempA, 31);
		tempB   = _mm_xor_si128(tempA, bitmask);
		tempB   = _mm_sub_epi32(tempB, bitmask);
		total_err0 = _mm_add_epi32(total_err0,tempB);
		tempB = _mm_sub_epi32(tempA,prev_err0);
		prev_err0 = tempA;
		/* Next three intrinsics calculate tempA as abs of tempB */
		bitmask = _mm_srai_epi32(tempB, 31);
		tempA   = _mm_xor_si128(tempB, bitmask);
		tempA   = _mm_sub_epi32(tempA, bitmask);
		total_err1 = _mm_add_epi32(total_err1,tempA);
		tempA = _mm_sub_epi32(tempB,prev_err1);
		prev_err1 = tempB;
		/* Next three intrinsics calculate tempB as abs of tempA */
		bitmask = _mm_srai_epi32(tempA, 31);
		tempB   = _mm_xor_si128(tempA, bitmask);
		tempB   = _mm_sub_epi32(tempB, bitmask);
		total_err2 = _mm_add_epi32(total_err2,tempB);
		tempB = _mm_sub_epi32(tempA,prev_err2);
		prev_err2 = tempA;
		/* Next three intrinsics calculate tempA as abs of tempB */
		bitmask = _mm_srai_epi32(tempB, 31);
		tempA   = _mm_xor_si128(tempB, bitmask);
		tempA   = _mm_sub_epi32(tempA, bitmask);
		total_err3 = _mm_add_epi32(total_err3,tempA);
		tempA = _mm_sub_epi32(tempB,prev_err3);
		prev_err3 = tempB;
		/* Next three intrinsics calculate tempB as abs of tempA */
		bitmask = _mm_srai_epi32(tempA, 31);
		tempB   = _mm_xor_si128(tempA, bitmask);
		tempB   = _mm_sub_epi32(tempB, bitmask);
		total_err4 = _mm_add_epi32(total_err4,tempB);
	}
	_mm_storeu_si128((__m128i*)data_scalar,total_err0);
	total_error_0 = data_scalar[0] + data_scalar[1] + data_scalar[2] + data_scalar[3];
	_mm_storeu_si128((__m128i*)data_scalar,total_err1);
	total_error_1 = data_scalar[0] + data_scalar[1] + data_scalar[2] + data_scalar[3];
	_mm_storeu_si128((__m128i*)data_scalar,total_err2);
	total_error_2 = data_scalar[0] + data_scalar[1] + data_scalar[2] + data_scalar[3];
	_mm_storeu_si128((__m128i*)data_scalar,total_err3);
	total_error_3 = data_scalar[0] + data_scalar[1] + data_scalar[2] + data_scalar[3];
	_mm_storeu_si128((__m128i*)data_scalar,total_err4);
	total_error_4 = data_scalar[0] + data_scalar[1] + data_scalar[2] + data_scalar[3];

	/* Now the remainder of samples needs to be processed */
	i *= 4;
	if(data_len % 4 > 0){
		FLAC__int32 last_error_0 = data[i-1];
		FLAC__int32 last_error_1 = data[i-1] - data[i-2];
		FLAC__int32 last_error_2 = last_error_1 - (data[i-2] - data[i-3]);
		FLAC__int32 last_error_3 = last_error_2 - (data[i-2] - 2*data[i-3] + data[i-4]);
		FLAC__int32 error, save;
		for(; i < data_len_int; i++) {
			error  = data[i]     ; total_error_0 += local_abs(error);                      save = error;
			error -= last_error_0; total_error_1 += local_abs(error); last_error_0 = save; save = error;
			error -= last_error_1; total_error_2 += local_abs(error); last_error_1 = save; save = error;
			error -= last_error_2; total_error_3 += local_abs(error); last_error_2 = save; save = error;
			error -= last_error_3; total_error_4 += local_abs(error); last_error_3 = save;
		}
	}

	/* prefer lower order */
	if(total_error_0 <= flac_min(flac_min(flac_min(total_error_1, total_error_2), total_error_3), total_error_4))
		order = 0;
	else if(total_error_1 <= flac_min(flac_min(total_error_2, total_error_3), total_error_4))
		order = 1;
	else if(total_error_2 <= flac_min(total_error_3, total_error_4))
		order = 2;
	else if(total_error_3 <= total_error_4)
		order = 3;
	else
		order = 4;

	/* Estimate the expected number of bits per residual signal sample. */
	/* 'total_error*' is linearly related to the variance of the residual */
	/* signal, so we use it directly to compute E(|x|) */
	FLAC__ASSERT(data_len > 0 || total_error_0 == 0);
	FLAC__ASSERT(data_len > 0 || total_error_1 == 0);
	FLAC__ASSERT(data_len > 0 || total_error_2 == 0);
	FLAC__ASSERT(data_len > 0 || total_error_3 == 0);
	FLAC__ASSERT(data_len > 0 || total_error_4 == 0);

	residual_bits_per_sample[0] = (float)((total_error_0 > 0) ? log(M_LN2 * (double)total_error_0 / (double)data_len) / M_LN2 : 0.0);
	residual_bits_per_sample[1] = (float)((total_error_1 > 0) ? log(M_LN2 * (double)total_error_1 / (double)data_len) / M_LN2 : 0.0);
	residual_bits_per_sample[2] = (float)((total_error_2 > 0) ? log(M_LN2 * (double)total_error_2 / (double)data_len) / M_LN2 : 0.0);
	residual_bits_per_sample[3] = (float)((total_error_3 > 0) ? log(M_LN2 * (double)total_error_3 / (double)data_len) / M_LN2 : 0.0);
	residual_bits_per_sample[4] = (float)((total_error_4 > 0) ? log(M_LN2 * (double)total_error_4 / (double)data_len) / M_LN2 : 0.0);

	return order;
}

#endif /* FLAC__SSE2_SUPPORTED */
#endif /* (FLAC__CPU_IA32 || FLAC__CPU_X86_64) && FLAC__HAS_X86INTRIN */
#endif /* FLAC__NO_ASM */
#endif /* FLAC__INTEGER_ONLY_LIBRARY */
