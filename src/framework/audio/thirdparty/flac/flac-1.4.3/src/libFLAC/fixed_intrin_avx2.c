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
#if (defined FLAC__CPU_IA32 || defined FLAC__CPU_X86_64) && FLAC__HAS_X86INTRIN
#include "private/fixed.h"
#ifdef FLAC__AVX2_SUPPORTED

#include <immintrin.h>
#include <math.h>
#include "private/macros.h"
#include "share/compat.h"
#include "FLAC/assert.h"

#ifdef local_abs
#undef local_abs
#endif
#define local_abs(x) ((uint32_t)((x)<0? -(x) : (x)))

FLAC__SSE_TARGET("avx2")
uint32_t FLAC__fixed_compute_best_predictor_wide_intrin_avx2(const FLAC__int32 data[], uint32_t data_len, float residual_bits_per_sample[FLAC__MAX_FIXED_ORDER + 1])
{
	FLAC__uint64 total_error_0, total_error_1, total_error_2, total_error_3, total_error_4;
	FLAC__int32 i, data_len_int;
	uint32_t order;
	__m256i total_err0, total_err1, total_err2, total_err3, total_err4;
	__m256i prev_err0,  prev_err1,  prev_err2,  prev_err3;
	__m256i tempA, tempB, bitmask;
	FLAC__int64 data_scalar[4];
	FLAC__int64 prev_err0_scalar[4];
	FLAC__int64 prev_err1_scalar[4];
	FLAC__int64 prev_err2_scalar[4];
	FLAC__int64 prev_err3_scalar[4];
	total_err0 = _mm256_setzero_si256();
	total_err1 = _mm256_setzero_si256();
	total_err2 = _mm256_setzero_si256();
	total_err3 = _mm256_setzero_si256();
	total_err4 = _mm256_setzero_si256();
	data_len_int = data_len;

	for(i = 0; i < 4; i++){
		prev_err0_scalar[i] = data[-1+i*(data_len_int/4)];
		prev_err1_scalar[i] = data[-1+i*(data_len_int/4)] - data[-2+i*(data_len_int/4)];
		prev_err2_scalar[i] = prev_err1_scalar[i] - (data[-2+i*(data_len_int/4)] - data[-3+i*(data_len_int/4)]);
		prev_err3_scalar[i] = prev_err2_scalar[i] - (data[-2+i*(data_len_int/4)] - 2*data[-3+i*(data_len_int/4)] + data[-4+i*(data_len_int/4)]);
	}
	prev_err0 = _mm256_loadu_si256((const __m256i*)(void*)prev_err0_scalar);
	prev_err1 = _mm256_loadu_si256((const __m256i*)(void*)prev_err1_scalar);
	prev_err2 = _mm256_loadu_si256((const __m256i*)(void*)prev_err2_scalar);
	prev_err3 = _mm256_loadu_si256((const __m256i*)(void*)prev_err3_scalar);
	for(i = 0; i < data_len_int / 4; i++){
		data_scalar[0] = data[i];
		data_scalar[1] = data[i+data_len/4];
		data_scalar[2] = data[i+2*data_len/4];
		data_scalar[3] = data[i+3*data_len/4];
		tempA = _mm256_loadu_si256((const __m256i*)(void*)data_scalar);
		/* Next three intrinsics calculate tempB as abs of tempA */
		bitmask = _mm256_cmpgt_epi64(_mm256_set1_epi64x(0), tempA);
		tempB = _mm256_xor_si256(tempA, bitmask);
		tempB = _mm256_sub_epi64(tempB, bitmask);
		total_err0 = _mm256_add_epi64(total_err0,tempB);
		tempB = _mm256_sub_epi64(tempA,prev_err0);
		prev_err0 = tempA;
		/* Next three intrinsics calculate tempA as abs of tempB */
		bitmask = _mm256_cmpgt_epi64(_mm256_set1_epi64x(0), tempB);
		tempA = _mm256_xor_si256(tempB, bitmask);
		tempA = _mm256_sub_epi64(tempA, bitmask);
		total_err1 = _mm256_add_epi64(total_err1,tempA);
		tempA = _mm256_sub_epi64(tempB,prev_err1);
		prev_err1 = tempB;
		/* Next three intrinsics calculate tempB as abs of tempA */
		bitmask = _mm256_cmpgt_epi64(_mm256_set1_epi64x(0), tempA);
		tempB = _mm256_xor_si256(tempA, bitmask);
		tempB = _mm256_sub_epi64(tempB, bitmask);
		total_err2 = _mm256_add_epi64(total_err2,tempB);
		tempB = _mm256_sub_epi64(tempA,prev_err2);
		prev_err2 = tempA;
		/* Next three intrinsics calculate tempA as abs of tempB */
		bitmask = _mm256_cmpgt_epi64(_mm256_set1_epi64x(0), tempB);
		tempA = _mm256_xor_si256(tempB, bitmask);
		tempA = _mm256_sub_epi64(tempA, bitmask);
		total_err3 = _mm256_add_epi64(total_err3,tempA);
		tempA = _mm256_sub_epi64(tempB,prev_err3);
		prev_err3 = tempB;
		/* Next three intrinsics calculate tempB as abs of tempA */
		bitmask = _mm256_cmpgt_epi64(_mm256_set1_epi64x(0), tempA);
		tempB = _mm256_xor_si256(tempA, bitmask);
		tempB = _mm256_sub_epi64(tempB, bitmask);
		total_err4 = _mm256_add_epi64(total_err4,tempB);
	}
	_mm256_storeu_si256((__m256i*)(void*)data_scalar,total_err0);
	total_error_0 = data_scalar[0] + data_scalar[1] + data_scalar[2] + data_scalar[3];
	_mm256_storeu_si256((__m256i*)(void*)data_scalar,total_err1);
	total_error_1 = data_scalar[0] + data_scalar[1] + data_scalar[2] + data_scalar[3];
	_mm256_storeu_si256((__m256i*)(void*)data_scalar,total_err2);
	total_error_2 = data_scalar[0] + data_scalar[1] + data_scalar[2] + data_scalar[3];
	_mm256_storeu_si256((__m256i*)(void*)data_scalar,total_err3);
	total_error_3 = data_scalar[0] + data_scalar[1] + data_scalar[2] + data_scalar[3];
	_mm256_storeu_si256((__m256i*)(void*)data_scalar,total_err4);
	total_error_4 = data_scalar[0] + data_scalar[1] + data_scalar[2] + data_scalar[3];

	/* Ignore the remainder, we're ignore the first few samples too */

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

#ifdef local_abs64
#undef local_abs64
#endif
#define local_abs64(x) ((uint64_t)((x)<0? -(x) : (x)))

#define CHECK_ORDER_IS_VALID(macro_order)  \
if(shadow_error_##macro_order <= INT32_MAX) { \
	if(total_error_##macro_order < smallest_error) { \
		order = macro_order; \
		smallest_error = total_error_##macro_order ; \
	} \
	residual_bits_per_sample[ macro_order ] = (float)((total_error_0 > 0) ? log(M_LN2 * (double)total_error_0 / (double)data_len) / M_LN2 : 0.0); \
} \
else \
	residual_bits_per_sample[ macro_order ] = 34.0f;

FLAC__SSE_TARGET("avx2")
uint32_t FLAC__fixed_compute_best_predictor_limit_residual_intrin_avx2(const FLAC__int32 data[], uint32_t data_len, float residual_bits_per_sample[FLAC__MAX_FIXED_ORDER + 1])
{
	FLAC__uint64 total_error_0 = 0, total_error_1 = 0, total_error_2 = 0, total_error_3 = 0, total_error_4 = 0, smallest_error = UINT64_MAX;
	FLAC__uint64 shadow_error_0 = 0, shadow_error_1 = 0, shadow_error_2 = 0, shadow_error_3 = 0, shadow_error_4 = 0;
	FLAC__uint64 error_0, error_1, error_2, error_3, error_4;
	FLAC__int32 i, data_len_int;
	uint32_t order = 0;
	__m256i total_err0, total_err1, total_err2, total_err3, total_err4;
	__m256i shadow_err0, shadow_err1, shadow_err2, shadow_err3, shadow_err4;
	__m256i prev_err0,  prev_err1,  prev_err2,  prev_err3;
	__m256i tempA, tempB, bitmask;
	FLAC__int64 data_scalar[4];
	FLAC__int64 prev_err0_scalar[4];
	FLAC__int64 prev_err1_scalar[4];
	FLAC__int64 prev_err2_scalar[4];
	FLAC__int64 prev_err3_scalar[4];
	total_err0 = _mm256_setzero_si256();
	total_err1 = _mm256_setzero_si256();
	total_err2 = _mm256_setzero_si256();
	total_err3 = _mm256_setzero_si256();
	total_err4 = _mm256_setzero_si256();
	shadow_err0 = _mm256_setzero_si256();
	shadow_err1 = _mm256_setzero_si256();
	shadow_err2 = _mm256_setzero_si256();
	shadow_err3 = _mm256_setzero_si256();
	shadow_err4 = _mm256_setzero_si256();
	data_len_int = data_len;

	/* First take care of preceding samples */
	for(i = -4; i < 0; i++) {
		error_0 = local_abs64((FLAC__int64)data[i]);
		error_1 = (i > -4) ? local_abs64((FLAC__int64)data[i] - data[i-1]) : 0 ;
		error_2 = (i > -3) ? local_abs64((FLAC__int64)data[i] - 2 * (FLAC__int64)data[i-1] + data[i-2]) : 0;
		error_3 = (i > -2) ? local_abs64((FLAC__int64)data[i] - 3 * (FLAC__int64)data[i-1] + 3 * (FLAC__int64)data[i-2] - data[i-3]) : 0;

		total_error_0 += error_0;
		total_error_1 += error_1;
		total_error_2 += error_2;
		total_error_3 += error_3;

		shadow_error_0 |= error_0;
		shadow_error_1 |= error_1;
		shadow_error_2 |= error_2;
		shadow_error_3 |= error_3;
	}

	for(i = 0; i < 4; i++){
		prev_err0_scalar[i] = data[-1+i*(data_len_int/4)];
		prev_err1_scalar[i] = (FLAC__int64)(data[-1+i*(data_len_int/4)]) - data[-2+i*(data_len_int/4)];
		prev_err2_scalar[i] = prev_err1_scalar[i] - ((FLAC__int64)(data[-2+i*(data_len_int/4)]) - data[-3+i*(data_len_int/4)]);
		prev_err3_scalar[i] = prev_err2_scalar[i] - ((FLAC__int64)(data[-2+i*(data_len_int/4)]) - 2*(FLAC__int64)(data[-3+i*(data_len_int/4)]) + data[-4+i*(data_len_int/4)]);
	}
	prev_err0 = _mm256_loadu_si256((const __m256i*)(void*)prev_err0_scalar);
	prev_err1 = _mm256_loadu_si256((const __m256i*)(void*)prev_err1_scalar);
	prev_err2 = _mm256_loadu_si256((const __m256i*)(void*)prev_err2_scalar);
	prev_err3 = _mm256_loadu_si256((const __m256i*)(void*)prev_err3_scalar);
	for(i = 0; i < data_len_int / 4; i++){
		data_scalar[0] = data[i];
		data_scalar[1] = data[i+data_len/4];
		data_scalar[2] = data[i+2*data_len/4];
		data_scalar[3] = data[i+3*data_len/4];
		tempA = _mm256_loadu_si256((const __m256i*)(void*)data_scalar);
		/* Next three intrinsics calculate tempB as abs of tempA */
		bitmask = _mm256_cmpgt_epi64(_mm256_set1_epi64x(0), tempA);
		tempB = _mm256_xor_si256(tempA, bitmask);
		tempB = _mm256_sub_epi64(tempB, bitmask);
		total_err0 = _mm256_add_epi64(total_err0,tempB);
		shadow_err0 = _mm256_or_si256(shadow_err0,tempB);
		tempB = _mm256_sub_epi64(tempA,prev_err0);
		prev_err0 = tempA;
		/* Next three intrinsics calculate tempA as abs of tempB */
		bitmask = _mm256_cmpgt_epi64(_mm256_set1_epi64x(0), tempB);
		tempA = _mm256_xor_si256(tempB, bitmask);
		tempA = _mm256_sub_epi64(tempA, bitmask);
		total_err1 = _mm256_add_epi64(total_err1,tempA);
		shadow_err1 = _mm256_or_si256(shadow_err1,tempA);
		tempA = _mm256_sub_epi64(tempB,prev_err1);
		prev_err1 = tempB;
		/* Next three intrinsics calculate tempB as abs of tempA */
		bitmask = _mm256_cmpgt_epi64(_mm256_set1_epi64x(0), tempA);
		tempB = _mm256_xor_si256(tempA, bitmask);
		tempB = _mm256_sub_epi64(tempB, bitmask);
		total_err2 = _mm256_add_epi64(total_err2,tempB);
		shadow_err2 = _mm256_or_si256(shadow_err2,tempB);
		tempB = _mm256_sub_epi64(tempA,prev_err2);
		prev_err2 = tempA;
		/* Next three intrinsics calculate tempA as abs of tempB */
		bitmask = _mm256_cmpgt_epi64(_mm256_set1_epi64x(0), tempB);
		tempA = _mm256_xor_si256(tempB, bitmask);
		tempA = _mm256_sub_epi64(tempA, bitmask);
		total_err3 = _mm256_add_epi64(total_err3,tempA);
		shadow_err3 = _mm256_or_si256(shadow_err3,tempA);
		tempA = _mm256_sub_epi64(tempB,prev_err3);
		prev_err3 = tempB;
		/* Next three intrinsics calculate tempB as abs of tempA */
		bitmask = _mm256_cmpgt_epi64(_mm256_set1_epi64x(0), tempA);
		tempB = _mm256_xor_si256(tempA, bitmask);
		tempB = _mm256_sub_epi64(tempB, bitmask);
		total_err4 = _mm256_add_epi64(total_err4,tempB);
		shadow_err4 = _mm256_or_si256(shadow_err4,tempB);
	}
	_mm256_storeu_si256((__m256i*)(void*)data_scalar,total_err0);
	total_error_0 += data_scalar[0] + data_scalar[1] + data_scalar[2] + data_scalar[3];
	_mm256_storeu_si256((__m256i*)(void*)data_scalar,total_err1);
	total_error_1 += data_scalar[0] + data_scalar[1] + data_scalar[2] + data_scalar[3];
	_mm256_storeu_si256((__m256i*)(void*)data_scalar,total_err2);
	total_error_2 += data_scalar[0] + data_scalar[1] + data_scalar[2] + data_scalar[3];
	_mm256_storeu_si256((__m256i*)(void*)data_scalar,total_err3);
	total_error_3 += data_scalar[0] + data_scalar[1] + data_scalar[2] + data_scalar[3];
	_mm256_storeu_si256((__m256i*)(void*)data_scalar,total_err4);
	total_error_4 += data_scalar[0] + data_scalar[1] + data_scalar[2] + data_scalar[3];
	_mm256_storeu_si256((__m256i*)(void*)data_scalar,shadow_err0);
	shadow_error_0 |= data_scalar[0] | data_scalar[1] | data_scalar[2] | data_scalar[3];
	_mm256_storeu_si256((__m256i*)(void*)data_scalar,shadow_err1);
	shadow_error_1 |= data_scalar[0] | data_scalar[1] | data_scalar[2] | data_scalar[3];
	_mm256_storeu_si256((__m256i*)(void*)data_scalar,shadow_err2);
	shadow_error_2 |= data_scalar[0] | data_scalar[1] | data_scalar[2] | data_scalar[3];
	_mm256_storeu_si256((__m256i*)(void*)data_scalar,shadow_err3);
	shadow_error_3 |= data_scalar[0] | data_scalar[1] | data_scalar[2] | data_scalar[3];
	_mm256_storeu_si256((__m256i*)(void*)data_scalar,shadow_err4);
	shadow_error_4 |= data_scalar[0] | data_scalar[1] | data_scalar[2] | data_scalar[3];

	/* Take care of remaining sample */
	for(i = (data_len/4)*4; i < data_len_int; i++) {
		error_0 = local_abs64((FLAC__int64)data[i]);
		error_1 = local_abs64((FLAC__int64)data[i] - data[i-1]);
		error_2 = local_abs64((FLAC__int64)data[i] - 2 * (FLAC__int64)data[i-1] + data[i-2]);
		error_3 = local_abs64((FLAC__int64)data[i] - 3 * (FLAC__int64)data[i-1] + 3 * (FLAC__int64)data[i-2] - data[i-3]);
		error_4 = local_abs64((FLAC__int64)data[i] - 4 * (FLAC__int64)data[i-1] + 6 * (FLAC__int64)data[i-2] - 4 * (FLAC__int64)data[i-3] + data[i-4]);

		total_error_0 += error_0;
		total_error_1 += error_1;
		total_error_2 += error_2;
		total_error_3 += error_3;
		total_error_4 += error_4;

		shadow_error_0 |= error_0;
		shadow_error_1 |= error_1;
		shadow_error_2 |= error_2;
		shadow_error_3 |= error_3;
		shadow_error_4 |= error_4;
	}


	CHECK_ORDER_IS_VALID(0);
	CHECK_ORDER_IS_VALID(1);
	CHECK_ORDER_IS_VALID(2);
	CHECK_ORDER_IS_VALID(3);
	CHECK_ORDER_IS_VALID(4);

	return order;
}

#endif /* FLAC__AVX2_SUPPORTED */
#endif /* (FLAC__CPU_IA32 || FLAC__CPU_X86_64) && FLAC__HAS_X86INTRIN */
#endif /* FLAC__NO_ASM */
#endif /* FLAC__INTEGER_ONLY_LIBRARY */
