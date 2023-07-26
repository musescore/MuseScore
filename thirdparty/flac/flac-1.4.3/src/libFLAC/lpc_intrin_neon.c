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

#include "private/cpu.h"

#ifndef FLAC__INTEGER_ONLY_LIBRARY
#ifndef FLAC__NO_ASM
#if defined FLAC__CPU_ARM64 && FLAC__HAS_NEONINTRIN
#include "private/lpc.h"
#include "FLAC/assert.h"
#include "FLAC/format.h"
#include "private/macros.h"
#include <arm_neon.h>

#if FLAC__HAS_A64NEONINTRIN
void FLAC__lpc_compute_autocorrelation_intrin_neon_lag_14(const FLAC__real data[], uint32_t data_len, uint32_t lag, double autoc[])
{
#undef MAX_LAG
#define MAX_LAG 14
#include "deduplication/lpc_compute_autocorrelation_intrin_neon.c"
}

void FLAC__lpc_compute_autocorrelation_intrin_neon_lag_10(const FLAC__real data[], uint32_t data_len, uint32_t lag, double autoc[])
{
#undef MAX_LAG
#define MAX_LAG 10
#include "deduplication/lpc_compute_autocorrelation_intrin_neon.c"
}

void FLAC__lpc_compute_autocorrelation_intrin_neon_lag_8(const FLAC__real data[], uint32_t data_len, uint32_t lag, double autoc[])
{
#undef MAX_LAG
#define MAX_LAG 8
#include "deduplication/lpc_compute_autocorrelation_intrin_neon.c"
}

#endif /* ifdef FLAC__HAS_A64NEONINTRIN */


#define MUL_32_BIT_LOOP_UNROOL_3(qlp_coeff_vec, lane) \
                        summ_0 = vmulq_laneq_s32(tmp_vec[0], qlp_coeff_vec, lane); \
                        summ_1 = vmulq_laneq_s32(tmp_vec[4], qlp_coeff_vec, lane); \
                        summ_2 = vmulq_laneq_s32(tmp_vec[8], qlp_coeff_vec, lane); 
                        

#define MACC_32BIT_LOOP_UNROOL_3(tmp_vec_ind, qlp_coeff_vec, lane) \
                        summ_0 = vmlaq_laneq_s32(summ_0,tmp_vec[tmp_vec_ind] ,qlp_coeff_vec, lane); \
                        summ_1 = vmlaq_laneq_s32(summ_1,tmp_vec[tmp_vec_ind+4] ,qlp_coeff_vec, lane); \
                        summ_2 = vmlaq_laneq_s32(summ_2,tmp_vec[tmp_vec_ind+8] ,qlp_coeff_vec, lane);
                        
void FLAC__lpc_compute_residual_from_qlp_coefficients_intrin_neon(const FLAC__int32 *data, uint32_t data_len, const FLAC__int32 qlp_coeff[], uint32_t order, int lp_quantization, FLAC__int32 residual[])
{
    int i;
    FLAC__int32 sum;
    int32x4_t tmp_vec[20];

    FLAC__ASSERT(order > 0);
    FLAC__ASSERT(order <= 32);

    // Using prologue reads is valid as encoder->private_->local_lpc_compute_residual_from_qlp_coefficients(signal+order,....)
    if(order <= 12) {
        if(order > 8) {
            if(order > 10) {
                if (order == 12) {
                    int32x4_t qlp_coeff_0 = {qlp_coeff[0], qlp_coeff[1], qlp_coeff[2], qlp_coeff[3]};
                    int32x4_t qlp_coeff_1 = {qlp_coeff[4], qlp_coeff[5], qlp_coeff[6], qlp_coeff[7]};
                    int32x4_t qlp_coeff_2 = {qlp_coeff[8], qlp_coeff[9], qlp_coeff[10], qlp_coeff[11]};

                    tmp_vec[0] = vld1q_s32(data - 12);
                    tmp_vec[1] = vld1q_s32(data - 11);
                    tmp_vec[2] = vld1q_s32(data - 10);
                    tmp_vec[3] = vld1q_s32(data - 9);
                    tmp_vec[4] = vld1q_s32(data - 8);
                    tmp_vec[5] = vld1q_s32(data - 7);
                    tmp_vec[6] = vld1q_s32(data - 6);
                    tmp_vec[7] = vld1q_s32(data - 5);

                    for (i = 0; i < (int)data_len - 11; i += 12)
                    {
                        int32x4_t summ_0, summ_1, summ_2;

                        tmp_vec[8] = vld1q_s32(data + i - 4);
                        tmp_vec[9] = vld1q_s32(data+i-3);
                        tmp_vec[10] = vld1q_s32(data+i-2);
                        tmp_vec[11] = vld1q_s32(data+i-1);
                        tmp_vec[12] = vld1q_s32(data+i);
                        tmp_vec[13] = vld1q_s32(data+i+1);
                        tmp_vec[14] = vld1q_s32(data+i+2);
                        tmp_vec[15] = vld1q_s32(data+i+3);
                        tmp_vec[16] = vld1q_s32(data + i + 4);
                        tmp_vec[17] = vld1q_s32(data + i + 5);
                        tmp_vec[18] = vld1q_s32(data + i + 6);
                        tmp_vec[19] = vld1q_s32(data + i + 7);

                        MUL_32_BIT_LOOP_UNROOL_3(qlp_coeff_2, 3)
                        MACC_32BIT_LOOP_UNROOL_3(1, qlp_coeff_2, 2)
                        MACC_32BIT_LOOP_UNROOL_3(2, qlp_coeff_2, 1)
                        MACC_32BIT_LOOP_UNROOL_3(3, qlp_coeff_2, 0)
                        MACC_32BIT_LOOP_UNROOL_3(4, qlp_coeff_1, 3)
                        MACC_32BIT_LOOP_UNROOL_3(5, qlp_coeff_1, 2)
                        MACC_32BIT_LOOP_UNROOL_3(6, qlp_coeff_1, 1)
                        MACC_32BIT_LOOP_UNROOL_3(7, qlp_coeff_1, 0)
                        MACC_32BIT_LOOP_UNROOL_3(8, qlp_coeff_0, 3)
                        MACC_32BIT_LOOP_UNROOL_3(9, qlp_coeff_0, 2)
                        MACC_32BIT_LOOP_UNROOL_3(10, qlp_coeff_0, 1)
                        MACC_32BIT_LOOP_UNROOL_3(11, qlp_coeff_0, 0)

                        vst1q_s32(residual+i + 0, vsubq_s32(vld1q_s32(data+i + 0) , vshlq_s32(summ_0,vdupq_n_s32(-lp_quantization))));
                        vst1q_s32(residual+i + 4, vsubq_s32(vld1q_s32(data+i + 4) , vshlq_s32(summ_1,vdupq_n_s32(-lp_quantization))));
                        vst1q_s32(residual+i + 8, vsubq_s32(vld1q_s32(data+i + 8) , vshlq_s32(summ_2,vdupq_n_s32(-lp_quantization))));

                        tmp_vec[0] = tmp_vec[12];
                        tmp_vec[1] = tmp_vec[13];
                        tmp_vec[2] = tmp_vec[14];
                        tmp_vec[3] = tmp_vec[15];
                        tmp_vec[4] = tmp_vec[16];
                        tmp_vec[5] = tmp_vec[17];
                        tmp_vec[6] = tmp_vec[18];
                        tmp_vec[7] = tmp_vec[19];
                    }
                }

                else { /* order == 11 */
                    int32x4_t qlp_coeff_0 = {qlp_coeff[0], qlp_coeff[1], qlp_coeff[2], qlp_coeff[3]};
                    int32x4_t qlp_coeff_1 = {qlp_coeff[4], qlp_coeff[5], qlp_coeff[6], qlp_coeff[7]};
                    int32x4_t qlp_coeff_2 = {qlp_coeff[8], qlp_coeff[9], qlp_coeff[10], 0};

                    tmp_vec[0] = vld1q_s32(data - 11);
                    tmp_vec[1] = vld1q_s32(data - 10);
                    tmp_vec[2] = vld1q_s32(data - 9);
                    tmp_vec[3] = vld1q_s32(data - 8);
                    tmp_vec[4] = vld1q_s32(data - 7);
                    tmp_vec[5] = vld1q_s32(data - 6);
                    tmp_vec[6] = vld1q_s32(data - 5);
                    
                    for (i = 0; i < (int)data_len - 11; i += 12)
                    {
                        int32x4_t summ_0, summ_1, summ_2;
                        tmp_vec[7] = vld1q_s32(data + i - 4);
                        tmp_vec[8] = vld1q_s32(data + i - 3);
                        tmp_vec[9] = vld1q_s32(data + i - 2);
                        tmp_vec[10] = vld1q_s32(data + i - 1);
                        tmp_vec[11] = vld1q_s32(data + i - 0);
                        tmp_vec[12] = vld1q_s32(data + i + 1);
                        tmp_vec[13] = vld1q_s32(data + i + 2);
                        tmp_vec[14] = vld1q_s32(data + i + 3);
                        tmp_vec[15] = vld1q_s32(data + i + 4);
                        tmp_vec[16] = vld1q_s32(data + i + 5);
                        tmp_vec[17] = vld1q_s32(data + i + 6);
                        tmp_vec[18] = vld1q_s32(data + i + 7);
                        
                      
                        MUL_32_BIT_LOOP_UNROOL_3(qlp_coeff_2, 2)
                        MACC_32BIT_LOOP_UNROOL_3(1, qlp_coeff_2, 1)
                        MACC_32BIT_LOOP_UNROOL_3(2, qlp_coeff_2, 0)
                        MACC_32BIT_LOOP_UNROOL_3(3, qlp_coeff_1, 3)
                        MACC_32BIT_LOOP_UNROOL_3(4, qlp_coeff_1, 2)
                        MACC_32BIT_LOOP_UNROOL_3(5, qlp_coeff_1, 1)
                        MACC_32BIT_LOOP_UNROOL_3(6, qlp_coeff_1, 0)
                        MACC_32BIT_LOOP_UNROOL_3(7, qlp_coeff_0, 3)
                        MACC_32BIT_LOOP_UNROOL_3(8, qlp_coeff_0, 2)
                        MACC_32BIT_LOOP_UNROOL_3(9, qlp_coeff_0, 1)
                        MACC_32BIT_LOOP_UNROOL_3(10, qlp_coeff_0, 0)
                        
                        vst1q_s32(residual+i + 0, vsubq_s32(vld1q_s32(data+i + 0) , vshlq_s32(summ_0,vdupq_n_s32(-lp_quantization))));
                        vst1q_s32(residual+i + 4, vsubq_s32(vld1q_s32(data+i + 4) , vshlq_s32(summ_1,vdupq_n_s32(-lp_quantization))));
                        vst1q_s32(residual+i + 8, vsubq_s32(vld1q_s32(data+i + 8) , vshlq_s32(summ_2,vdupq_n_s32(-lp_quantization))));

                        
                        tmp_vec[0] = tmp_vec[12];
                        tmp_vec[1] = tmp_vec[13];
                        tmp_vec[2] = tmp_vec[14];
                        tmp_vec[3] = tmp_vec[15];
                        tmp_vec[4] = tmp_vec[16];
                        tmp_vec[5] = tmp_vec[17];
                        tmp_vec[6] = tmp_vec[18];
                    }
                }
            }
            else {
                if(order == 10) {
                    int32x4_t qlp_coeff_0 = {qlp_coeff[0], qlp_coeff[1], qlp_coeff[2], qlp_coeff[3]};
                    int32x4_t qlp_coeff_1 = {qlp_coeff[4], qlp_coeff[5], qlp_coeff[6], qlp_coeff[7]};
                    int32x4_t qlp_coeff_2 = {qlp_coeff[8], qlp_coeff[9], 0, 0};

                    tmp_vec[0] = vld1q_s32(data - 10);
                    tmp_vec[1] = vld1q_s32(data - 9);
                    tmp_vec[2] = vld1q_s32(data - 8);
                    tmp_vec[3] = vld1q_s32(data - 7);
                    tmp_vec[4] = vld1q_s32(data - 6);
                    tmp_vec[5] = vld1q_s32(data - 5);
                    
                    for (i = 0; i < (int)data_len - 11; i += 12)
                    {
                        int32x4_t summ_0, summ_1, summ_2;
                        tmp_vec[6] = vld1q_s32(data + i - 4);
                        tmp_vec[7] = vld1q_s32(data + i - 3);
                        tmp_vec[8] = vld1q_s32(data + i - 2);
                        tmp_vec[9] = vld1q_s32(data + i - 1);
                        tmp_vec[10] = vld1q_s32(data + i - 0);
                        tmp_vec[11] = vld1q_s32(data + i + 1);
                        tmp_vec[12] = vld1q_s32(data + i + 2);
                        tmp_vec[13] = vld1q_s32(data + i + 3);
                        tmp_vec[14] = vld1q_s32(data + i + 4);
                        tmp_vec[15] = vld1q_s32(data + i + 5);
                        tmp_vec[16] = vld1q_s32(data + i + 6);
                        tmp_vec[17] = vld1q_s32(data + i + 7);
                        
                            
                        MUL_32_BIT_LOOP_UNROOL_3(qlp_coeff_2, 1)
                        MACC_32BIT_LOOP_UNROOL_3(1, qlp_coeff_2, 0)
                        MACC_32BIT_LOOP_UNROOL_3(2, qlp_coeff_1, 3)
                        MACC_32BIT_LOOP_UNROOL_3(3, qlp_coeff_1, 2)
                        MACC_32BIT_LOOP_UNROOL_3(4, qlp_coeff_1, 1)
                        MACC_32BIT_LOOP_UNROOL_3(5, qlp_coeff_1, 0)
                        MACC_32BIT_LOOP_UNROOL_3(6, qlp_coeff_0, 3)
                        MACC_32BIT_LOOP_UNROOL_3(7, qlp_coeff_0, 2)
                        MACC_32BIT_LOOP_UNROOL_3(8, qlp_coeff_0, 1)
                        MACC_32BIT_LOOP_UNROOL_3(9, qlp_coeff_0, 0)
                        
                        vst1q_s32(residual+i + 0, vsubq_s32(vld1q_s32(data+i + 0) , vshlq_s32(summ_0,vdupq_n_s32(-lp_quantization))));
                        vst1q_s32(residual+i + 4, vsubq_s32(vld1q_s32(data+i + 4) , vshlq_s32(summ_1,vdupq_n_s32(-lp_quantization))));
                        vst1q_s32(residual+i + 8, vsubq_s32(vld1q_s32(data+i + 8) , vshlq_s32(summ_2,vdupq_n_s32(-lp_quantization))));

                        
                        tmp_vec[0] = tmp_vec[12];
                        tmp_vec[1] = tmp_vec[13];
                        tmp_vec[2] = tmp_vec[14];
                        tmp_vec[3] = tmp_vec[15];
                        tmp_vec[4] = tmp_vec[16];
                        tmp_vec[5] = tmp_vec[17];
                    }
                }
                else { /* order == 9 */
                    int32x4_t qlp_coeff_0 = {qlp_coeff[0], qlp_coeff[1], qlp_coeff[2], qlp_coeff[3]};
                    int32x4_t qlp_coeff_1 = {qlp_coeff[4], qlp_coeff[5], qlp_coeff[6], qlp_coeff[7]};
                    int32x4_t qlp_coeff_2 = {qlp_coeff[8], 0, 0, 0};

                    tmp_vec[0] = vld1q_s32(data - 9);
                    tmp_vec[1] = vld1q_s32(data - 8);
                    tmp_vec[2] = vld1q_s32(data - 7);
                    tmp_vec[3] = vld1q_s32(data - 6);
                    tmp_vec[4] = vld1q_s32(data - 5);
                    
                    for (i = 0; i < (int)data_len - 11; i += 12)
                    {
                        int32x4_t summ_0, summ_1, summ_2;
                        tmp_vec[5] = vld1q_s32(data + i - 4);
                        tmp_vec[6] = vld1q_s32(data + i - 3);
                        tmp_vec[7] = vld1q_s32(data + i - 2);
                        tmp_vec[8] = vld1q_s32(data + i - 1);
                        tmp_vec[9] = vld1q_s32(data + i - 0);
                        tmp_vec[10] = vld1q_s32(data + i + 1);
                        tmp_vec[11] = vld1q_s32(data + i + 2);
                        tmp_vec[12] = vld1q_s32(data + i + 3);
                        tmp_vec[13] = vld1q_s32(data + i + 4);
                        tmp_vec[14] = vld1q_s32(data + i + 5);
                        tmp_vec[15] = vld1q_s32(data + i + 6);
                        tmp_vec[16] = vld1q_s32(data + i + 7);
                        
                        MUL_32_BIT_LOOP_UNROOL_3(qlp_coeff_2, 0)
                        MACC_32BIT_LOOP_UNROOL_3(1, qlp_coeff_1, 3)
                        MACC_32BIT_LOOP_UNROOL_3(2, qlp_coeff_1, 2)
                        MACC_32BIT_LOOP_UNROOL_3(3, qlp_coeff_1, 1)
                        MACC_32BIT_LOOP_UNROOL_3(4, qlp_coeff_1, 0)
                        MACC_32BIT_LOOP_UNROOL_3(5, qlp_coeff_0, 3)
                        MACC_32BIT_LOOP_UNROOL_3(6, qlp_coeff_0, 2)
                        MACC_32BIT_LOOP_UNROOL_3(7, qlp_coeff_0, 1)
                        MACC_32BIT_LOOP_UNROOL_3(8, qlp_coeff_0, 0)

                        vst1q_s32(residual+i + 0, vsubq_s32(vld1q_s32(data+i + 0) , vshlq_s32(summ_0,vdupq_n_s32(-lp_quantization))));
                        vst1q_s32(residual+i + 4, vsubq_s32(vld1q_s32(data+i + 4) , vshlq_s32(summ_1,vdupq_n_s32(-lp_quantization))));
                        vst1q_s32(residual+i + 8, vsubq_s32(vld1q_s32(data+i + 8) , vshlq_s32(summ_2,vdupq_n_s32(-lp_quantization))));

                        tmp_vec[0] = tmp_vec[12];
                        tmp_vec[1] = tmp_vec[13];
                        tmp_vec[2] = tmp_vec[14];
                        tmp_vec[3] = tmp_vec[15];
                        tmp_vec[4] = tmp_vec[16];
                    }
                }
            }
        }
        else if(order > 4) {
            if(order > 6) {
                if(order == 8) {
                    int32x4_t qlp_coeff_0 = {qlp_coeff[0], qlp_coeff[1], qlp_coeff[2], qlp_coeff[3]};
                    int32x4_t qlp_coeff_1 = {qlp_coeff[4], qlp_coeff[5], qlp_coeff[6], qlp_coeff[7]};

                    tmp_vec[0] = vld1q_s32(data - 8);
                    tmp_vec[1] = vld1q_s32(data - 7);
                    tmp_vec[2] = vld1q_s32(data - 6);
                    tmp_vec[3] = vld1q_s32(data - 5);
                    
                    for (i = 0; i < (int)data_len - 11; i += 12)
                    {
                        int32x4_t summ_0, summ_1, summ_2;
                        tmp_vec[4] = vld1q_s32(data + i - 4);
                        tmp_vec[5] = vld1q_s32(data + i - 3);
                        tmp_vec[6] = vld1q_s32(data + i - 2);
                        tmp_vec[7] = vld1q_s32(data + i - 1);
                        tmp_vec[8] = vld1q_s32(data + i - 0);
                        tmp_vec[9] = vld1q_s32(data + i + 1);
                        tmp_vec[10] = vld1q_s32(data + i + 2);
                        tmp_vec[11] = vld1q_s32(data + i + 3);
                        tmp_vec[12] = vld1q_s32(data + i + 4);
                        tmp_vec[13] = vld1q_s32(data + i + 5);
                        tmp_vec[14] = vld1q_s32(data + i + 6);
                        tmp_vec[15] = vld1q_s32(data + i + 7);
                        
                        MUL_32_BIT_LOOP_UNROOL_3(qlp_coeff_1, 3)
                        MACC_32BIT_LOOP_UNROOL_3(1, qlp_coeff_1, 2)
                        MACC_32BIT_LOOP_UNROOL_3(2, qlp_coeff_1, 1)
                        MACC_32BIT_LOOP_UNROOL_3(3, qlp_coeff_1, 0)
                        MACC_32BIT_LOOP_UNROOL_3(4, qlp_coeff_0, 3)
                        MACC_32BIT_LOOP_UNROOL_3(5, qlp_coeff_0, 2)
                        MACC_32BIT_LOOP_UNROOL_3(6, qlp_coeff_0, 1)
                        MACC_32BIT_LOOP_UNROOL_3(7, qlp_coeff_0, 0)

                        vst1q_s32(residual+i + 0, vsubq_s32(vld1q_s32(data+i + 0) , vshlq_s32(summ_0,vdupq_n_s32(-lp_quantization))));
                        vst1q_s32(residual+i + 4, vsubq_s32(vld1q_s32(data+i + 4) , vshlq_s32(summ_1,vdupq_n_s32(-lp_quantization))));
                        vst1q_s32(residual+i + 8, vsubq_s32(vld1q_s32(data+i + 8) , vshlq_s32(summ_2,vdupq_n_s32(-lp_quantization))));

                        tmp_vec[0] = tmp_vec[12];
                        tmp_vec[1] = tmp_vec[13];
                        tmp_vec[2] = tmp_vec[14];
                        tmp_vec[3] = tmp_vec[15];
                    }
                }
                else { /* order == 7 */
                    int32x4_t qlp_coeff_0 = {qlp_coeff[0], qlp_coeff[1], qlp_coeff[2], qlp_coeff[3]};
                    int32x4_t qlp_coeff_1 = {qlp_coeff[4], qlp_coeff[5], qlp_coeff[6], 0};

                    tmp_vec[0] = vld1q_s32(data - 7);
                    tmp_vec[1] = vld1q_s32(data - 6);
                    tmp_vec[2] = vld1q_s32(data - 5);
                    
                    for (i = 0; i < (int)data_len - 11; i += 12)
                    {
                        int32x4_t summ_0, summ_1, summ_2;
                        tmp_vec[3] = vld1q_s32(data + i - 4);
                        tmp_vec[4] = vld1q_s32(data + i - 3);
                        tmp_vec[5] = vld1q_s32(data + i - 2);
                        tmp_vec[6] = vld1q_s32(data + i - 1);
                        tmp_vec[7] = vld1q_s32(data + i - 0);
                        tmp_vec[8] = vld1q_s32(data + i + 1);
                        tmp_vec[9] = vld1q_s32(data + i + 2);
                        tmp_vec[10] = vld1q_s32(data + i + 3);
                        tmp_vec[11] = vld1q_s32(data + i + 4);
                        tmp_vec[12] = vld1q_s32(data + i + 5);
                        tmp_vec[13] = vld1q_s32(data + i + 6);
                        tmp_vec[14] = vld1q_s32(data + i + 7);
                        
                        MUL_32_BIT_LOOP_UNROOL_3(qlp_coeff_1, 2)
                        MACC_32BIT_LOOP_UNROOL_3(1, qlp_coeff_1, 1)
                        MACC_32BIT_LOOP_UNROOL_3(2, qlp_coeff_1, 0)
                        MACC_32BIT_LOOP_UNROOL_3(3, qlp_coeff_0, 3)
                        MACC_32BIT_LOOP_UNROOL_3(4, qlp_coeff_0, 2)
                        MACC_32BIT_LOOP_UNROOL_3(5, qlp_coeff_0, 1)
                        MACC_32BIT_LOOP_UNROOL_3(6, qlp_coeff_0, 0)
                        
                        vst1q_s32(residual+i + 0, vsubq_s32(vld1q_s32(data+i + 0) , vshlq_s32(summ_0,vdupq_n_s32(-lp_quantization))));
                        vst1q_s32(residual+i + 4, vsubq_s32(vld1q_s32(data+i + 4) , vshlq_s32(summ_1,vdupq_n_s32(-lp_quantization))));
                        vst1q_s32(residual+i + 8, vsubq_s32(vld1q_s32(data+i + 8) , vshlq_s32(summ_2,vdupq_n_s32(-lp_quantization))));

                        tmp_vec[0] = tmp_vec[12];
                        tmp_vec[1] = tmp_vec[13];
                        tmp_vec[2] = tmp_vec[14];
                    }
                }
            }
            else {
                if(order == 6) {
                    int32x4_t qlp_coeff_0 = {qlp_coeff[0], qlp_coeff[1], qlp_coeff[2], qlp_coeff[3]};
                    int32x4_t qlp_coeff_1 = {qlp_coeff[4], qlp_coeff[5], 0, 0};

                    tmp_vec[0] = vld1q_s32(data - 6);
                    tmp_vec[1] = vld1q_s32(data - 5);
                    
                    for (i = 0; i < (int)data_len - 11; i += 12)
                    {
                        int32x4_t summ_0, summ_1, summ_2;
                        tmp_vec[2] = vld1q_s32(data + i - 4);
                        tmp_vec[3] = vld1q_s32(data + i - 3);
                        tmp_vec[4] = vld1q_s32(data + i - 2);
                        tmp_vec[5] = vld1q_s32(data + i - 1);
                        tmp_vec[6] = vld1q_s32(data + i - 0);
                        tmp_vec[7] = vld1q_s32(data + i + 1);
                        tmp_vec[8] = vld1q_s32(data + i + 2);
                        tmp_vec[9] = vld1q_s32(data + i + 3);
                        tmp_vec[10] = vld1q_s32(data + i + 4);
                        tmp_vec[11] = vld1q_s32(data + i + 5);
                        tmp_vec[12] = vld1q_s32(data + i + 6);
                        tmp_vec[13] = vld1q_s32(data + i + 7);
                        
                        MUL_32_BIT_LOOP_UNROOL_3(qlp_coeff_1, 1)
                        MACC_32BIT_LOOP_UNROOL_3(1, qlp_coeff_1, 0)
                        MACC_32BIT_LOOP_UNROOL_3(2, qlp_coeff_0, 3)
                        MACC_32BIT_LOOP_UNROOL_3(3, qlp_coeff_0, 2)
                        MACC_32BIT_LOOP_UNROOL_3(4, qlp_coeff_0, 1)
                        MACC_32BIT_LOOP_UNROOL_3(5, qlp_coeff_0, 0)
                        
                        vst1q_s32(residual+i + 0, vsubq_s32(vld1q_s32(data+i + 0) , vshlq_s32(summ_0,vdupq_n_s32(-lp_quantization))));
                        vst1q_s32(residual+i + 4, vsubq_s32(vld1q_s32(data+i + 4) , vshlq_s32(summ_1,vdupq_n_s32(-lp_quantization))));
                        vst1q_s32(residual+i + 8, vsubq_s32(vld1q_s32(data+i + 8) , vshlq_s32(summ_2,vdupq_n_s32(-lp_quantization))));

                        tmp_vec[0] = tmp_vec[12];
                        tmp_vec[1] = tmp_vec[13];
                    }
                }
                else { /* order == 5 */
                    int32x4_t qlp_coeff_0 = {qlp_coeff[0], qlp_coeff[1], qlp_coeff[2], qlp_coeff[3]};
                    int32x4_t qlp_coeff_1 = {qlp_coeff[4], 0, 0, 0};

                    tmp_vec[0] = vld1q_s32(data - 5);
                    
                    for (i = 0; i < (int)data_len - 11; i += 12)
                    {
                        int32x4_t summ_0, summ_1, summ_2;

                        tmp_vec[1] = vld1q_s32(data + i - 4);
                        tmp_vec[2] = vld1q_s32(data + i - 3);
                        tmp_vec[3] = vld1q_s32(data + i - 2);
                        tmp_vec[4] = vld1q_s32(data + i - 1);
                        tmp_vec[5] = vld1q_s32(data + i - 0);
                        tmp_vec[6] = vld1q_s32(data + i + 1);
                        tmp_vec[7] = vld1q_s32(data + i + 2);
                        tmp_vec[8] = vld1q_s32(data + i + 3);
                        tmp_vec[9] = vld1q_s32(data + i + 4);
                        tmp_vec[10] = vld1q_s32(data + i + 5);
                        tmp_vec[11] = vld1q_s32(data + i + 6);
                        tmp_vec[12] = vld1q_s32(data + i + 7);
                        
                        MUL_32_BIT_LOOP_UNROOL_3(qlp_coeff_1, 0)
                        MACC_32BIT_LOOP_UNROOL_3(1, qlp_coeff_0, 3)
                        MACC_32BIT_LOOP_UNROOL_3(2, qlp_coeff_0, 2)
                        MACC_32BIT_LOOP_UNROOL_3(3, qlp_coeff_0, 1)
                        MACC_32BIT_LOOP_UNROOL_3(4, qlp_coeff_0, 0)
                        
                        vst1q_s32(residual+i + 0, vsubq_s32(vld1q_s32(data+i + 0) , vshlq_s32(summ_0,vdupq_n_s32(-lp_quantization))));
                        vst1q_s32(residual+i + 4, vsubq_s32(vld1q_s32(data+i + 4) , vshlq_s32(summ_1,vdupq_n_s32(-lp_quantization))));
                        vst1q_s32(residual+i + 8, vsubq_s32(vld1q_s32(data+i + 8) , vshlq_s32(summ_2,vdupq_n_s32(-lp_quantization))));

                        tmp_vec[0] = tmp_vec[12];
                    }
                }
            }
        }
        else {
            if(order > 2) {
                if(order == 4) {
                    int32x4_t qlp_coeff_0 = {qlp_coeff[0], qlp_coeff[1], qlp_coeff[2], qlp_coeff[3]};
                    
                    for (i = 0; i < (int)data_len - 11; i += 12)
                    {
                        int32x4_t summ_0, summ_1, summ_2;
                        tmp_vec[0] = vld1q_s32(data + i - 4);
                        tmp_vec[1] = vld1q_s32(data + i - 3);
                        tmp_vec[2] = vld1q_s32(data + i - 2);
                        tmp_vec[3] = vld1q_s32(data + i - 1);
                        tmp_vec[4] = vld1q_s32(data + i - 0);
                        tmp_vec[5] = vld1q_s32(data + i + 1);
                        tmp_vec[6] = vld1q_s32(data + i + 2);
                        tmp_vec[7] = vld1q_s32(data + i + 3);
                        tmp_vec[8] = vld1q_s32(data + i + 4);
                        tmp_vec[9] = vld1q_s32(data + i + 5);
                        tmp_vec[10] = vld1q_s32(data + i + 6);
                        tmp_vec[11] = vld1q_s32(data + i + 7);
                    
                        MUL_32_BIT_LOOP_UNROOL_3(qlp_coeff_0, 3)
                        MACC_32BIT_LOOP_UNROOL_3(1, qlp_coeff_0, 2)
                        MACC_32BIT_LOOP_UNROOL_3(2, qlp_coeff_0, 1)
                        MACC_32BIT_LOOP_UNROOL_3(3, qlp_coeff_0, 0)
                        
                        vst1q_s32(residual+i + 0, vsubq_s32(vld1q_s32(data+i + 0) , vshlq_s32(summ_0,vdupq_n_s32(-lp_quantization))));
                        vst1q_s32(residual+i + 4, vsubq_s32(vld1q_s32(data+i + 4) , vshlq_s32(summ_1,vdupq_n_s32(-lp_quantization))));
                        vst1q_s32(residual+i + 8, vsubq_s32(vld1q_s32(data+i + 8) , vshlq_s32(summ_2,vdupq_n_s32(-lp_quantization))));
                    }
                }
                else { /* order == 3 */
                    int32x4_t qlp_coeff_0 = {qlp_coeff[0], qlp_coeff[1], qlp_coeff[2], 0};

                    for (i = 0; i < (int)data_len - 11; i += 12)
                    {
                        int32x4_t summ_0, summ_1, summ_2;
                        tmp_vec[0] = vld1q_s32(data + i - 3);
                        tmp_vec[1] = vld1q_s32(data + i - 2);
                        tmp_vec[2] = vld1q_s32(data + i - 1);
                        tmp_vec[4] = vld1q_s32(data + i + 1);
                        tmp_vec[5] = vld1q_s32(data + i + 2);
                        tmp_vec[6] = vld1q_s32(data + i + 3);
                        tmp_vec[8] = vld1q_s32(data + i + 5);
                        tmp_vec[9] = vld1q_s32(data + i + 6);
                        tmp_vec[10] = vld1q_s32(data + i + 7);
                        
                        MUL_32_BIT_LOOP_UNROOL_3(qlp_coeff_0, 2)
                        MACC_32BIT_LOOP_UNROOL_3(1, qlp_coeff_0, 1)
                        MACC_32BIT_LOOP_UNROOL_3(2, qlp_coeff_0, 0)

                        vst1q_s32(residual+i + 0, vsubq_s32(vld1q_s32(data+i + 0) , vshlq_s32(summ_0,vdupq_n_s32(-lp_quantization))));
                        vst1q_s32(residual+i + 4, vsubq_s32(vld1q_s32(data+i + 4) , vshlq_s32(summ_1,vdupq_n_s32(-lp_quantization))));
                        vst1q_s32(residual+i + 8, vsubq_s32(vld1q_s32(data+i + 8) , vshlq_s32(summ_2,vdupq_n_s32(-lp_quantization))));
                    }
                }
            }
            else {
                if(order == 2) {
                    int32x4_t qlp_coeff_0 = {qlp_coeff[0], qlp_coeff[1], 0, 0};

                    for (i = 0; i < (int)data_len - 11; i += 12)
                    {
                        int32x4_t summ_0, summ_1, summ_2;
                        tmp_vec[0] = vld1q_s32(data + i - 2);
                        tmp_vec[1] = vld1q_s32(data + i - 1);
                        tmp_vec[4] = vld1q_s32(data + i + 2);
                        tmp_vec[5] = vld1q_s32(data + i + 3);
                        tmp_vec[8] = vld1q_s32(data + i + 6);
                        tmp_vec[9] = vld1q_s32(data + i + 7);
                        
                        MUL_32_BIT_LOOP_UNROOL_3(qlp_coeff_0, 1)
                        MACC_32BIT_LOOP_UNROOL_3(1, qlp_coeff_0, 0)
                        
                        vst1q_s32(residual+i + 0, vsubq_s32(vld1q_s32(data+i + 0) , vshlq_s32(summ_0,vdupq_n_s32(-lp_quantization))));
                        vst1q_s32(residual+i + 4, vsubq_s32(vld1q_s32(data+i + 4) , vshlq_s32(summ_1,vdupq_n_s32(-lp_quantization))));
                        vst1q_s32(residual+i + 8, vsubq_s32(vld1q_s32(data+i + 8) , vshlq_s32(summ_2,vdupq_n_s32(-lp_quantization))));
                    }
                }
                else { /* order == 1 */
                    int32x4_t qlp_coeff_0 = vdupq_n_s32(qlp_coeff[0]);

                    for (i = 0; i < (int)data_len - 11; i += 12)
                    {
                        int32x4_t summ_0, summ_1, summ_2;
                        tmp_vec[0] = vld1q_s32(data + i - 1);
                        tmp_vec[4] = vld1q_s32(data + i + 3);
                        tmp_vec[8] = vld1q_s32(data + i + 7);
                        
                        summ_0 = vmulq_s32(tmp_vec[0], qlp_coeff_0);
                        summ_1 = vmulq_s32(tmp_vec[4], qlp_coeff_0);
                        summ_2 = vmulq_s32(tmp_vec[8], qlp_coeff_0);

                        vst1q_s32(residual+i + 0, vsubq_s32(vld1q_s32(data+i + 0) , vshlq_s32(summ_0,vdupq_n_s32(-lp_quantization))));
                        vst1q_s32(residual+i + 4, vsubq_s32(vld1q_s32(data+i + 4) , vshlq_s32(summ_1,vdupq_n_s32(-lp_quantization))));
                        vst1q_s32(residual+i + 8, vsubq_s32(vld1q_s32(data+i + 8) , vshlq_s32(summ_2,vdupq_n_s32(-lp_quantization))));
                    }
                }
            }
        }
        for(; i < (int)data_len; i++) {
            sum = 0;
            switch(order) {
                case 12: sum += qlp_coeff[11] * data[i-12]; /* Falls through. */
                case 11: sum += qlp_coeff[10] * data[i-11]; /* Falls through. */
                case 10: sum += qlp_coeff[ 9] * data[i-10]; /* Falls through. */
                case 9:  sum += qlp_coeff[ 8] * data[i- 9]; /* Falls through. */
                case 8:  sum += qlp_coeff[ 7] * data[i- 8]; /* Falls through. */
                case 7:  sum += qlp_coeff[ 6] * data[i- 7]; /* Falls through. */
                case 6:  sum += qlp_coeff[ 5] * data[i- 6]; /* Falls through. */
                case 5:  sum += qlp_coeff[ 4] * data[i- 5]; /* Falls through. */
                case 4:  sum += qlp_coeff[ 3] * data[i- 4]; /* Falls through. */
                case 3:  sum += qlp_coeff[ 2] * data[i- 3]; /* Falls through. */
                case 2:  sum += qlp_coeff[ 1] * data[i- 2]; /* Falls through. */
                case 1:  sum += qlp_coeff[ 0] * data[i- 1];
            }
            residual[i] = data[i] - (sum >> lp_quantization);
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



#define MUL_64_BIT_LOOP_UNROOL_3(qlp_coeff_vec, lane) \
                        summ_l_0 = vmull_laneq_s32(vget_low_s32(tmp_vec[0]),qlp_coeff_vec, lane); \
                        summ_h_0 = vmull_high_laneq_s32(tmp_vec[0], qlp_coeff_vec, lane);\
                        summ_l_1 = vmull_laneq_s32(vget_low_s32(tmp_vec[4]),qlp_coeff_vec, lane); \
                        summ_h_1 = vmull_high_laneq_s32(tmp_vec[4], qlp_coeff_vec, lane);\
                        summ_l_2 = vmull_laneq_s32(vget_low_s32(tmp_vec[8]),qlp_coeff_vec, lane);\
                        summ_h_2 = vmull_high_laneq_s32(tmp_vec[8], qlp_coeff_vec, lane);


#define MACC_64_BIT_LOOP_UNROOL_3(tmp_vec_ind, qlp_coeff_vec, lane) \
                        summ_l_0 = vmlal_laneq_s32(summ_l_0,vget_low_s32(tmp_vec[tmp_vec_ind]),qlp_coeff_vec, lane); \
                        summ_h_0 = vmlal_high_laneq_s32(summ_h_0, tmp_vec[tmp_vec_ind], qlp_coeff_vec, lane); \
                        summ_l_1 = vmlal_laneq_s32(summ_l_1, vget_low_s32(tmp_vec[tmp_vec_ind+4]),qlp_coeff_vec, lane); \
                        summ_h_1 = vmlal_high_laneq_s32(summ_h_1, tmp_vec[tmp_vec_ind+4], qlp_coeff_vec, lane); \
                        summ_l_2 = vmlal_laneq_s32(summ_l_2, vget_low_s32(tmp_vec[tmp_vec_ind+8]),qlp_coeff_vec, lane);\
                        summ_h_2 = vmlal_high_laneq_s32(summ_h_2,tmp_vec[tmp_vec_ind+8], qlp_coeff_vec, lane);

#define SHIFT_SUMS_64BITS_AND_STORE_SUB() \
                        res0 = vuzp1q_s32(vreinterpretq_s32_s64(vshlq_s64(summ_l_0,lp_quantization_vec)), vreinterpretq_s32_s64(vshlq_s64(summ_h_0,lp_quantization_vec))); \
                        res1 = vuzp1q_s32(vreinterpretq_s32_s64(vshlq_s64(summ_l_1,lp_quantization_vec)), vreinterpretq_s32_s64(vshlq_s64(summ_h_1,lp_quantization_vec))); \
                        res2 = vuzp1q_s32(vreinterpretq_s32_s64(vshlq_s64(summ_l_2,lp_quantization_vec)), vreinterpretq_s32_s64(vshlq_s64(summ_h_2,lp_quantization_vec))); \
                        vst1q_s32(residual+i+0, vsubq_s32(vld1q_s32(data+i+0), res0));\
                        vst1q_s32(residual+i+4, vsubq_s32(vld1q_s32(data+i+4), res1));\
                        vst1q_s32(residual+i+8, vsubq_s32(vld1q_s32(data+i+8), res2));

void FLAC__lpc_compute_residual_from_qlp_coefficients_wide_intrin_neon(const FLAC__int32 *data, uint32_t data_len, const FLAC__int32 qlp_coeff[], uint32_t order, int lp_quantization, FLAC__int32 residual[]) {
	int i;
	FLAC__int64 sum;
	
    int32x4_t tmp_vec[20];
    int32x4_t res0, res1, res2;
    int64x2_t  lp_quantization_vec = vdupq_n_s64(-lp_quantization);

    FLAC__ASSERT(order > 0);
	FLAC__ASSERT(order <= 32);
    
    // Using prologue reads is valid as encoder->private_->local_lpc_compute_residual_from_qlp_coefficients_64bit(signal+order,....)
	if(order <= 12) {
		if(order > 8) {
			if(order > 10) {
				if(order == 12) {
                    int32x4_t qlp_coeff_0 = {qlp_coeff[0], qlp_coeff[1], qlp_coeff[2], qlp_coeff[3]};
                    int32x4_t qlp_coeff_1 = {qlp_coeff[4],qlp_coeff[5],qlp_coeff[6],qlp_coeff[7]};
                    int32x4_t qlp_coeff_2 = {qlp_coeff[8],qlp_coeff[9],qlp_coeff[10],qlp_coeff[11]};

                    tmp_vec[0] = vld1q_s32(data - 12);
                    tmp_vec[1] = vld1q_s32(data - 11);
                    tmp_vec[2] = vld1q_s32(data - 10);
                    tmp_vec[3] = vld1q_s32(data - 9);
                    tmp_vec[4] = vld1q_s32(data - 8);
                    tmp_vec[5] = vld1q_s32(data - 7);
                    tmp_vec[6] = vld1q_s32(data - 6);
                    tmp_vec[7] = vld1q_s32(data - 5);

                    for (i = 0; i < (int)data_len - 11; i += 12)
                    {
                        int64x2_t  summ_l_0, summ_h_0, summ_l_1, summ_h_1, summ_l_2, summ_h_2;
                        
                        tmp_vec[8] = vld1q_s32(data+i-4);
                        tmp_vec[9] = vld1q_s32(data+i-3);
                        tmp_vec[10] = vld1q_s32(data+i-2);
                        tmp_vec[11] = vld1q_s32(data+i-1);
                        tmp_vec[12] = vld1q_s32(data+i);
                        tmp_vec[13] = vld1q_s32(data+i+1);
                        tmp_vec[14] = vld1q_s32(data+i+2);
                        tmp_vec[15] = vld1q_s32(data+i+3);
                        tmp_vec[16] = vld1q_s32(data + i + 4);
                        tmp_vec[17] = vld1q_s32(data + i + 5);
                        tmp_vec[18] = vld1q_s32(data + i + 6);
                        tmp_vec[19] = vld1q_s32(data + i + 7);

                        MUL_64_BIT_LOOP_UNROOL_3(qlp_coeff_2, 3)
                        MACC_64_BIT_LOOP_UNROOL_3(1, qlp_coeff_2, 2) 
                        MACC_64_BIT_LOOP_UNROOL_3(2, qlp_coeff_2, 1) 
                        MACC_64_BIT_LOOP_UNROOL_3(3, qlp_coeff_2, 0) 
                        MACC_64_BIT_LOOP_UNROOL_3(4, qlp_coeff_1, 3) 
                        MACC_64_BIT_LOOP_UNROOL_3(5, qlp_coeff_1, 2) 
                        MACC_64_BIT_LOOP_UNROOL_3(6, qlp_coeff_1, 1) 
                        MACC_64_BIT_LOOP_UNROOL_3(7, qlp_coeff_1, 0) 
                        MACC_64_BIT_LOOP_UNROOL_3(8, qlp_coeff_0, 3) 
                        MACC_64_BIT_LOOP_UNROOL_3(9, qlp_coeff_0, 2) 
                        MACC_64_BIT_LOOP_UNROOL_3(10,qlp_coeff_0, 1) 
                        MACC_64_BIT_LOOP_UNROOL_3(11,qlp_coeff_0, 0) 

                        SHIFT_SUMS_64BITS_AND_STORE_SUB()
                        
                        tmp_vec[0] = tmp_vec[12];
                        tmp_vec[1] = tmp_vec[13];
                        tmp_vec[2] = tmp_vec[14];
                        tmp_vec[3] = tmp_vec[15];
                        tmp_vec[4] = tmp_vec[16];
                        tmp_vec[5] = tmp_vec[17];
                        tmp_vec[6] = tmp_vec[18];
                        tmp_vec[7] = tmp_vec[19];
                    }
                }
				else { /* order == 11 */			
                    int32x4_t qlp_coeff_0 = {qlp_coeff[0], qlp_coeff[1], qlp_coeff[2], qlp_coeff[3]};
                    int32x4_t qlp_coeff_1 = {qlp_coeff[4],qlp_coeff[5],qlp_coeff[6],qlp_coeff[7]};
                    int32x4_t qlp_coeff_2 = {qlp_coeff[8],qlp_coeff[9],qlp_coeff[10],0};

                    tmp_vec[0] = vld1q_s32(data - 11);
                    tmp_vec[1] = vld1q_s32(data - 10);
                    tmp_vec[2] = vld1q_s32(data - 9);
                    tmp_vec[3] = vld1q_s32(data - 8);
                    tmp_vec[4] = vld1q_s32(data - 7);
                    tmp_vec[5] = vld1q_s32(data - 6);
                    tmp_vec[6] = vld1q_s32(data - 5);

                    for (i = 0; i < (int)data_len - 11; i += 12)
                    {
                        int64x2_t  summ_l_0, summ_h_0, summ_l_1, summ_h_1, summ_l_2, summ_h_2;
                        
                        tmp_vec[7] = vld1q_s32(data+i-4);
                        tmp_vec[8] = vld1q_s32(data+i-3);
                        tmp_vec[9] = vld1q_s32(data+i-2);
                        tmp_vec[10] = vld1q_s32(data+i-1);
                        tmp_vec[11] = vld1q_s32(data+i);
                        tmp_vec[12] = vld1q_s32(data+i+1);
                        tmp_vec[13] = vld1q_s32(data+i+2);
                        tmp_vec[14] = vld1q_s32(data+i+3);
                        tmp_vec[15] = vld1q_s32(data + i + 4);
                        tmp_vec[16] = vld1q_s32(data + i + 5);
                        tmp_vec[17] = vld1q_s32(data + i + 6);
                        tmp_vec[18] = vld1q_s32(data + i + 7);

                        MUL_64_BIT_LOOP_UNROOL_3(qlp_coeff_2, 2)
                        MACC_64_BIT_LOOP_UNROOL_3(1, qlp_coeff_2, 1) 
                        MACC_64_BIT_LOOP_UNROOL_3(2, qlp_coeff_2, 0) 
                        MACC_64_BIT_LOOP_UNROOL_3(3, qlp_coeff_1, 3) 
                        MACC_64_BIT_LOOP_UNROOL_3(4, qlp_coeff_1, 2) 
                        MACC_64_BIT_LOOP_UNROOL_3(5, qlp_coeff_1, 1) 
                        MACC_64_BIT_LOOP_UNROOL_3(6, qlp_coeff_1, 0) 
                        MACC_64_BIT_LOOP_UNROOL_3(7, qlp_coeff_0, 3) 
                        MACC_64_BIT_LOOP_UNROOL_3(8, qlp_coeff_0, 2) 
                        MACC_64_BIT_LOOP_UNROOL_3(9, qlp_coeff_0, 1) 
                        MACC_64_BIT_LOOP_UNROOL_3(10,qlp_coeff_0, 0) 

                        SHIFT_SUMS_64BITS_AND_STORE_SUB()
                        
                        tmp_vec[0] = tmp_vec[12];
                        tmp_vec[1] = tmp_vec[13];
                        tmp_vec[2] = tmp_vec[14];
                        tmp_vec[3] = tmp_vec[15];
                        tmp_vec[4] = tmp_vec[16];
                        tmp_vec[5] = tmp_vec[17];
                        tmp_vec[6] = tmp_vec[18];
                    }
                }
            }
            else
            {
                if (order == 10) {
                    int32x4_t qlp_coeff_0 = {qlp_coeff[0], qlp_coeff[1], qlp_coeff[2], qlp_coeff[3]};
                    int32x4_t qlp_coeff_1 = {qlp_coeff[4], qlp_coeff[5], qlp_coeff[6], qlp_coeff[7]};
                    int32x4_t qlp_coeff_2 = {qlp_coeff[8], qlp_coeff[9], 0, 0};

                    tmp_vec[0] = vld1q_s32(data - 10);
                    tmp_vec[1] = vld1q_s32(data - 9);
                    tmp_vec[2] = vld1q_s32(data - 8);
                    tmp_vec[3] = vld1q_s32(data - 7);
                    tmp_vec[4] = vld1q_s32(data - 6);
                    tmp_vec[5] = vld1q_s32(data - 5);
                    

                    for (i = 0; i < (int)data_len - 11; i += 12)
                    {
                        int64x2_t summ_l_0, summ_h_0, summ_l_1, summ_h_1, summ_l_2, summ_h_2;
                        
                        tmp_vec[6] = vld1q_s32(data + i - 4);
                        tmp_vec[7] = vld1q_s32(data + i - 3);
                        tmp_vec[8] = vld1q_s32(data + i - 2);
                        tmp_vec[9] = vld1q_s32(data + i - 1);
                        tmp_vec[10] = vld1q_s32(data + i - 0);
                        tmp_vec[11] = vld1q_s32(data + i + 1);
                        tmp_vec[12] = vld1q_s32(data + i + 2);
                        tmp_vec[13] = vld1q_s32(data + i + 3);
                        tmp_vec[14] = vld1q_s32(data + i + 4);
                        tmp_vec[15] = vld1q_s32(data + i + 5);
                        tmp_vec[16] = vld1q_s32(data + i + 6);
                        tmp_vec[17] = vld1q_s32(data + i + 7);
                        
                        MUL_64_BIT_LOOP_UNROOL_3(qlp_coeff_2, 1)
                        MACC_64_BIT_LOOP_UNROOL_3(1, qlp_coeff_2, 0) 
                        MACC_64_BIT_LOOP_UNROOL_3(2, qlp_coeff_1, 3) 
                        MACC_64_BIT_LOOP_UNROOL_3(3, qlp_coeff_1, 2) 
                        MACC_64_BIT_LOOP_UNROOL_3(4, qlp_coeff_1, 1) 
                        MACC_64_BIT_LOOP_UNROOL_3(5, qlp_coeff_1, 0) 
                        MACC_64_BIT_LOOP_UNROOL_3(6, qlp_coeff_0, 3) 
                        MACC_64_BIT_LOOP_UNROOL_3(7, qlp_coeff_0, 2) 
                        MACC_64_BIT_LOOP_UNROOL_3(8, qlp_coeff_0, 1) 
                        MACC_64_BIT_LOOP_UNROOL_3(9, qlp_coeff_0, 0) 

                        SHIFT_SUMS_64BITS_AND_STORE_SUB()
                        
                        tmp_vec[0] = tmp_vec[12];
                        tmp_vec[1] = tmp_vec[13];
                        tmp_vec[2] = tmp_vec[14];
                        tmp_vec[3] = tmp_vec[15];
                        tmp_vec[4] = tmp_vec[16];
                        tmp_vec[5] = tmp_vec[17];
                    }
                }

                else /* order == 9 */ {
                    int32x4_t qlp_coeff_0 = {qlp_coeff[0], qlp_coeff[1], qlp_coeff[2], qlp_coeff[3]};
                    int32x4_t qlp_coeff_1 = {qlp_coeff[4], qlp_coeff[5], qlp_coeff[6], qlp_coeff[7]};
                    int32x4_t qlp_coeff_2 = {qlp_coeff[8], 0, 0, 0};

                    tmp_vec[0] = vld1q_s32(data - 9);
                    tmp_vec[1] = vld1q_s32(data - 8);
                    tmp_vec[2] = vld1q_s32(data - 7);
                    tmp_vec[3] = vld1q_s32(data - 6);
                    tmp_vec[4] = vld1q_s32(data - 5);

                    for (i = 0; i < (int)data_len - 11; i += 12)
                    {
                        int64x2_t summ_l_0, summ_h_0, summ_l_1, summ_h_1, summ_l_2, summ_h_2;

                        tmp_vec[5] = vld1q_s32(data + i - 4);
                        tmp_vec[6] = vld1q_s32(data + i - 3);
                        tmp_vec[7] = vld1q_s32(data + i - 2);
                        tmp_vec[8] = vld1q_s32(data + i - 1);
                        tmp_vec[9] = vld1q_s32(data + i - 0);
                        tmp_vec[10] = vld1q_s32(data + i + 1);
                        tmp_vec[11] = vld1q_s32(data + i + 2);
                        tmp_vec[12] = vld1q_s32(data + i + 3);
                        tmp_vec[13] = vld1q_s32(data + i + 4);
                        tmp_vec[14] = vld1q_s32(data + i + 5);
                        tmp_vec[15] = vld1q_s32(data + i + 6);
                        tmp_vec[16] = vld1q_s32(data + i + 7);

                        MUL_64_BIT_LOOP_UNROOL_3(qlp_coeff_2, 0)
                        MACC_64_BIT_LOOP_UNROOL_3(1, qlp_coeff_1, 3) 
                        MACC_64_BIT_LOOP_UNROOL_3(2, qlp_coeff_1, 2) 
                        MACC_64_BIT_LOOP_UNROOL_3(3, qlp_coeff_1, 1) 
                        MACC_64_BIT_LOOP_UNROOL_3(4, qlp_coeff_1, 0) 
                        MACC_64_BIT_LOOP_UNROOL_3(5, qlp_coeff_0, 3) 
                        MACC_64_BIT_LOOP_UNROOL_3(6, qlp_coeff_0, 2) 
                        MACC_64_BIT_LOOP_UNROOL_3(7, qlp_coeff_0, 1) 
                        MACC_64_BIT_LOOP_UNROOL_3(8, qlp_coeff_0, 0) 

                        SHIFT_SUMS_64BITS_AND_STORE_SUB()
                        
                        tmp_vec[0] = tmp_vec[12];
                        tmp_vec[1] = tmp_vec[13];
                        tmp_vec[2] = tmp_vec[14];
                        tmp_vec[3] = tmp_vec[15];
                        tmp_vec[4] = tmp_vec[16];
                    }
                }
            }
        }
        else if (order > 4)
        {
            if (order > 6)
            {
                if (order == 8)
                {
                    int32x4_t qlp_coeff_0 = {qlp_coeff[0], qlp_coeff[1], qlp_coeff[2], qlp_coeff[3]};
                    int32x4_t qlp_coeff_1 = {qlp_coeff[4], qlp_coeff[5], qlp_coeff[6], qlp_coeff[7]};
                 
                    tmp_vec[0] = vld1q_s32(data - 8);
                    tmp_vec[1] = vld1q_s32(data - 7);
                    tmp_vec[2] = vld1q_s32(data - 6);
                    tmp_vec[3] = vld1q_s32(data - 5);

                    for (i = 0; i < (int)data_len - 11; i += 12)
                    {
                        int64x2_t summ_l_0, summ_h_0, summ_l_1, summ_h_1, summ_l_2, summ_h_2;

                        tmp_vec[4] = vld1q_s32(data + i - 4);
                        tmp_vec[5] = vld1q_s32(data + i - 3);
                        tmp_vec[6] = vld1q_s32(data + i - 2);
                        tmp_vec[7] = vld1q_s32(data + i - 1);
                        tmp_vec[8] = vld1q_s32(data + i - 0);
                        tmp_vec[9] = vld1q_s32(data + i + 1);
                        tmp_vec[10] = vld1q_s32(data + i + 2);
                        tmp_vec[11] = vld1q_s32(data + i + 3);
                        tmp_vec[12] = vld1q_s32(data + i + 4);
                        tmp_vec[13] = vld1q_s32(data + i + 5);
                        tmp_vec[14] = vld1q_s32(data + i + 6);
                        tmp_vec[15] = vld1q_s32(data + i + 7);
                        
                      
                        MUL_64_BIT_LOOP_UNROOL_3(qlp_coeff_1, 3)
                        MACC_64_BIT_LOOP_UNROOL_3(1, qlp_coeff_1, 2) 
                        MACC_64_BIT_LOOP_UNROOL_3(2, qlp_coeff_1, 1) 
                        MACC_64_BIT_LOOP_UNROOL_3(3, qlp_coeff_1, 0) 
                        MACC_64_BIT_LOOP_UNROOL_3(4, qlp_coeff_0, 3) 
                        MACC_64_BIT_LOOP_UNROOL_3(5, qlp_coeff_0, 2) 
                        MACC_64_BIT_LOOP_UNROOL_3(6, qlp_coeff_0, 1) 
                        MACC_64_BIT_LOOP_UNROOL_3(7, qlp_coeff_0, 0) 

                        SHIFT_SUMS_64BITS_AND_STORE_SUB()
                        
                        tmp_vec[0] = tmp_vec[12];
                        tmp_vec[1] = tmp_vec[13];
                        tmp_vec[2] = tmp_vec[14];
                        tmp_vec[3] = tmp_vec[15];
                    }
                }
                else /* order == 7 */
                {
                    int32x4_t qlp_coeff_0 = {qlp_coeff[0], qlp_coeff[1], qlp_coeff[2], qlp_coeff[3]};
                    int32x4_t qlp_coeff_1 = {qlp_coeff[4], qlp_coeff[5], qlp_coeff[6], 0};

                    tmp_vec[0] = vld1q_s32(data - 7);
                    tmp_vec[1] = vld1q_s32(data - 6);
                    tmp_vec[2] = vld1q_s32(data - 5);
                    

                    for (i = 0; i < (int)data_len - 11; i += 12)
                    {
                        int64x2_t summ_l_0, summ_h_0, summ_l_1, summ_h_1, summ_l_2, summ_h_2;
                        tmp_vec[3] = vld1q_s32(data +i - 4);
                        tmp_vec[4] = vld1q_s32(data + i - 3);
                        tmp_vec[5] = vld1q_s32(data + i - 2);
                        tmp_vec[6] = vld1q_s32(data + i - 1);
                        tmp_vec[7] = vld1q_s32(data + i - 0);
                        tmp_vec[8] = vld1q_s32(data + i + 1);
                        tmp_vec[9] = vld1q_s32(data + i + 2);
                        tmp_vec[10] = vld1q_s32(data + i + 3);
                        tmp_vec[11] = vld1q_s32(data + i + 4);
                        tmp_vec[12] = vld1q_s32(data + i + 5);
                        tmp_vec[13] = vld1q_s32(data + i + 6);
                        tmp_vec[14] = vld1q_s32(data + i + 7);
                                              
                      
                        MUL_64_BIT_LOOP_UNROOL_3(qlp_coeff_1, 2)
                        MACC_64_BIT_LOOP_UNROOL_3(1, qlp_coeff_1, 1) 
                        MACC_64_BIT_LOOP_UNROOL_3(2, qlp_coeff_1, 0) 
                        MACC_64_BIT_LOOP_UNROOL_3(3, qlp_coeff_0, 3) 
                        MACC_64_BIT_LOOP_UNROOL_3(4, qlp_coeff_0, 2) 
                        MACC_64_BIT_LOOP_UNROOL_3(5, qlp_coeff_0, 1) 
                        MACC_64_BIT_LOOP_UNROOL_3(6, qlp_coeff_0, 0) 

                        SHIFT_SUMS_64BITS_AND_STORE_SUB()
                        
                        tmp_vec[0] = tmp_vec[12];
                        tmp_vec[1] = tmp_vec[13];
                        tmp_vec[2] = tmp_vec[14];
                    }
                }
            }
            else
            {
                if (order == 6) {
                    int32x4_t qlp_coeff_0 = {qlp_coeff[0], qlp_coeff[1], qlp_coeff[2], qlp_coeff[3]};
                    int32x4_t qlp_coeff_1 = {qlp_coeff[4], qlp_coeff[5], 0, 0};

                    tmp_vec[0] = vld1q_s32(data - 6);
                    tmp_vec[1] = vld1q_s32(data - 5);
                    
                    for (i = 0; i < (int)data_len - 11; i += 12)
                    {
                        int64x2_t summ_l_0, summ_h_0, summ_l_1, summ_h_1, summ_l_2, summ_h_2;

                        tmp_vec[2] = vld1q_s32(data + i - 4);
                        tmp_vec[3] = vld1q_s32(data + i - 3);
                        tmp_vec[4] = vld1q_s32(data + i - 2);
                        tmp_vec[5] = vld1q_s32(data + i - 1);
                        tmp_vec[6] = vld1q_s32(data + i - 0);
                        tmp_vec[7] = vld1q_s32(data + i + 1);
                        tmp_vec[8] = vld1q_s32(data + i + 2);
                        tmp_vec[9] = vld1q_s32(data + i + 3);
                        tmp_vec[10] = vld1q_s32(data + i + 4);
                        tmp_vec[11] = vld1q_s32(data + i + 5);
                        tmp_vec[12] = vld1q_s32(data + i + 6);
                        tmp_vec[13] = vld1q_s32(data + i + 7);
                        
                       
                        MUL_64_BIT_LOOP_UNROOL_3(qlp_coeff_1, 1)
                        MACC_64_BIT_LOOP_UNROOL_3(1, qlp_coeff_1, 0) 
                        MACC_64_BIT_LOOP_UNROOL_3(2, qlp_coeff_0, 3) 
                        MACC_64_BIT_LOOP_UNROOL_3(3, qlp_coeff_0, 2) 
                        MACC_64_BIT_LOOP_UNROOL_3(4, qlp_coeff_0, 1) 
                        MACC_64_BIT_LOOP_UNROOL_3(5, qlp_coeff_0, 0) 
                        
                        SHIFT_SUMS_64BITS_AND_STORE_SUB()
                        
                        tmp_vec[0] = tmp_vec[12];
                        tmp_vec[1] = tmp_vec[13];
                    }
                }

                else
                { /* order == 5 */
                    int32x4_t qlp_coeff_0 = {qlp_coeff[0], qlp_coeff[1], qlp_coeff[2], qlp_coeff[3]};
                    int32x4_t qlp_coeff_1 = {qlp_coeff[4], 0, 0, 0};

                    tmp_vec[0] = vld1q_s32(data - 5);
                    
                    for (i = 0; i < (int)data_len - 11; i += 12)
                    {
                        int64x2_t summ_l_0, summ_h_0, summ_l_1, summ_h_1, summ_l_2, summ_h_2;
                        tmp_vec[1] = vld1q_s32(data + i - 4);
                        tmp_vec[2] = vld1q_s32(data + i - 3);
                        tmp_vec[3] = vld1q_s32(data + i - 2);
                        tmp_vec[4] = vld1q_s32(data + i - 1);
                        tmp_vec[5] = vld1q_s32(data + i - 0);
                        tmp_vec[6] = vld1q_s32(data + i + 1);
                        tmp_vec[7] = vld1q_s32(data + i + 2);
                        tmp_vec[8] = vld1q_s32(data + i + 3);
                        tmp_vec[9] = vld1q_s32(data + i + 4);
                        tmp_vec[10] = vld1q_s32(data + i + 5);
                        tmp_vec[11] = vld1q_s32(data + i + 6);
                        tmp_vec[12] = vld1q_s32(data + i + 7);
                        
                        MUL_64_BIT_LOOP_UNROOL_3(qlp_coeff_1, 0)
                        MACC_64_BIT_LOOP_UNROOL_3(1, qlp_coeff_0, 3) 
                        MACC_64_BIT_LOOP_UNROOL_3(2, qlp_coeff_0, 2) 
                        MACC_64_BIT_LOOP_UNROOL_3(3, qlp_coeff_0, 1) 
                        MACC_64_BIT_LOOP_UNROOL_3(4, qlp_coeff_0, 0) 
                        
                        SHIFT_SUMS_64BITS_AND_STORE_SUB()
                        
                        tmp_vec[0] = tmp_vec[12];
                    }
                }
            }
        }
        else
        {
            if (order > 2)
            {
                if (order == 4)
                {
                    int32x4_t qlp_coeff_0 = {qlp_coeff[0], qlp_coeff[1], qlp_coeff[2], qlp_coeff[3]};
                    
                    for (i = 0; i < (int)data_len - 11; i += 12)
                    {
                        int64x2_t summ_l_0, summ_h_0, summ_l_1, summ_h_1, summ_l_2, summ_h_2;
                        tmp_vec[0] = vld1q_s32(data + i - 4);
                        tmp_vec[1] = vld1q_s32(data + i - 3);
                        tmp_vec[2] = vld1q_s32(data + i - 2);
                        tmp_vec[3] = vld1q_s32(data + i - 1);
                        tmp_vec[4] = vld1q_s32(data + i - 0);
                        tmp_vec[5] = vld1q_s32(data + i + 1);
                        tmp_vec[6] = vld1q_s32(data + i + 2);
                        tmp_vec[7] = vld1q_s32(data + i + 3);
                        tmp_vec[8] = vld1q_s32(data + i + 4);
                        tmp_vec[9] = vld1q_s32(data + i + 5);
                        tmp_vec[10] = vld1q_s32(data + i + 6);
                        tmp_vec[11] = vld1q_s32(data + i + 7);
                        
                        MUL_64_BIT_LOOP_UNROOL_3(qlp_coeff_0, 3)
                        MACC_64_BIT_LOOP_UNROOL_3(1, qlp_coeff_0, 2) 
                        MACC_64_BIT_LOOP_UNROOL_3(2, qlp_coeff_0, 1) 
                        MACC_64_BIT_LOOP_UNROOL_3(3, qlp_coeff_0, 0) 
                                               
                        SHIFT_SUMS_64BITS_AND_STORE_SUB()                        
                    }
                }
                else
                { /* order == 3 */

                    int32x4_t qlp_coeff_0 = {qlp_coeff[0], qlp_coeff[1], qlp_coeff[2], 0};
                    
                    for (i = 0; i < (int)data_len - 11; i += 12)
                    {
                        int64x2_t summ_l_0, summ_h_0, summ_l_1, summ_h_1, summ_l_2, summ_h_2;
                        tmp_vec[0] = vld1q_s32(data + i - 3);
                        tmp_vec[1] = vld1q_s32(data + i - 2);
                        tmp_vec[2] = vld1q_s32(data + i - 1);
                        tmp_vec[4] = vld1q_s32(data + i + 1);
                        tmp_vec[5] = vld1q_s32(data + i + 2);
                        tmp_vec[6] = vld1q_s32(data + i + 3);
                        tmp_vec[8] = vld1q_s32(data + i + 5);
                        tmp_vec[9] = vld1q_s32(data + i + 6);
                        tmp_vec[10] = vld1q_s32(data + i + 7);
                        
                        MUL_64_BIT_LOOP_UNROOL_3(qlp_coeff_0, 2)
                        MACC_64_BIT_LOOP_UNROOL_3(1, qlp_coeff_0, 1) 
                        MACC_64_BIT_LOOP_UNROOL_3(2, qlp_coeff_0, 0) 
                        
                        SHIFT_SUMS_64BITS_AND_STORE_SUB()                        
                    }
                }
            }
            else
            {
                if (order == 2)
                {
                    int32x4_t qlp_coeff_0 = {qlp_coeff[0], qlp_coeff[1], 0, 0};

                    for (i = 0; i < (int)data_len - 11; i += 12)
                    {
                        int64x2_t summ_l_0, summ_h_0, summ_l_1, summ_h_1, summ_l_2, summ_h_2;
                        tmp_vec[0] = vld1q_s32(data + i - 2);
                        tmp_vec[1] = vld1q_s32(data + i - 1);
                        tmp_vec[4] = vld1q_s32(data + i + 2);
                        tmp_vec[5] = vld1q_s32(data + i + 3);
                        tmp_vec[8] = vld1q_s32(data + i + 6);
                        tmp_vec[9] = vld1q_s32(data + i + 7);
                        
                        MUL_64_BIT_LOOP_UNROOL_3(qlp_coeff_0, 1)
                        MACC_64_BIT_LOOP_UNROOL_3(1, qlp_coeff_0, 0) 

                        SHIFT_SUMS_64BITS_AND_STORE_SUB()                        
                    }
                }

                else
                { /* order == 1 */

                    int32x2_t qlp_coeff_0_2 = vdup_n_s32(qlp_coeff[0]);
                    int32x4_t qlp_coeff_0_4 = vdupq_n_s32(qlp_coeff[0]);

                    for (i = 0; i < (int)data_len - 11; i += 12)
                    {
                        int64x2_t summ_l_0, summ_h_0, summ_l_1, summ_h_1, summ_l_2, summ_h_2;
                        tmp_vec[0] = vld1q_s32(data + i - 1);
                        tmp_vec[4] = vld1q_s32(data + i + 3);
                        tmp_vec[8] = vld1q_s32(data + i + 7);
                        
                        summ_l_0 = vmull_s32(vget_low_s32(tmp_vec[0]), qlp_coeff_0_2);
                        summ_h_0 = vmull_high_s32(tmp_vec[0], qlp_coeff_0_4);

                        summ_l_1 = vmull_s32(vget_low_s32(tmp_vec[4]), qlp_coeff_0_2);
                        summ_h_1 = vmull_high_s32(tmp_vec[4], qlp_coeff_0_4);

                        summ_l_2 = vmull_s32(vget_low_s32(tmp_vec[8]), qlp_coeff_0_2);
                        summ_h_2 = vmull_high_s32(tmp_vec[8], qlp_coeff_0_4);

                        SHIFT_SUMS_64BITS_AND_STORE_SUB()                        
                    }
                }
            }
        }
        for (; i < (int)data_len; i++)
        {
            sum = 0;
            switch (order)
            {
            case 12:
                sum += qlp_coeff[11] * (FLAC__int64)data[i - 12]; /* Falls through. */
            case 11:
                sum += qlp_coeff[10] * (FLAC__int64)data[i - 11]; /* Falls through. */
            case 10:
                sum += qlp_coeff[9] * (FLAC__int64)data[i - 10]; /* Falls through. */
            case 9:
                sum += qlp_coeff[8] * (FLAC__int64)data[i - 9]; /* Falls through. */
            case 8:
                sum += qlp_coeff[7] * (FLAC__int64)data[i - 8]; /* Falls through. */
            case 7:
                sum += qlp_coeff[6] * (FLAC__int64)data[i - 7]; /* Falls through. */
            case 6:
                sum += qlp_coeff[5] * (FLAC__int64)data[i - 6]; /* Falls through. */
            case 5:
                sum += qlp_coeff[4] * (FLAC__int64)data[i - 5]; /* Falls through. */
            case 4:
                sum += qlp_coeff[3] * (FLAC__int64)data[i - 4]; /* Falls through. */
            case 3:
                sum += qlp_coeff[2] * (FLAC__int64)data[i - 3]; /* Falls through. */
            case 2:
                sum += qlp_coeff[1] * (FLAC__int64)data[i - 2]; /* Falls through. */
            case 1:
                sum += qlp_coeff[0] * (FLAC__int64)data[i - 1];
            }
            residual[i] = data[i] - (sum >> lp_quantization);
        }
    }
    else
    { /* order > 12 */
        for (i = 0; i < (int)data_len; i++)
        {
            sum = 0;
            switch (order)
            {
            case 32:
                sum += qlp_coeff[31] * (FLAC__int64)data[i - 32]; /* Falls through. */
            case 31:
                sum += qlp_coeff[30] * (FLAC__int64)data[i - 31]; /* Falls through. */
            case 30:
                sum += qlp_coeff[29] * (FLAC__int64)data[i - 30]; /* Falls through. */
            case 29:
                sum += qlp_coeff[28] * (FLAC__int64)data[i - 29]; /* Falls through. */
            case 28:
                sum += qlp_coeff[27] * (FLAC__int64)data[i - 28]; /* Falls through. */
            case 27:
                sum += qlp_coeff[26] * (FLAC__int64)data[i - 27]; /* Falls through. */
            case 26:
                sum += qlp_coeff[25] * (FLAC__int64)data[i - 26]; /* Falls through. */
            case 25:
                sum += qlp_coeff[24] * (FLAC__int64)data[i - 25]; /* Falls through. */
            case 24:
                sum += qlp_coeff[23] * (FLAC__int64)data[i - 24]; /* Falls through. */
            case 23:
                sum += qlp_coeff[22] * (FLAC__int64)data[i - 23]; /* Falls through. */
            case 22:
                sum += qlp_coeff[21] * (FLAC__int64)data[i - 22]; /* Falls through. */
            case 21:
                sum += qlp_coeff[20] * (FLAC__int64)data[i - 21]; /* Falls through. */
            case 20:
                sum += qlp_coeff[19] * (FLAC__int64)data[i - 20]; /* Falls through. */
            case 19:
                sum += qlp_coeff[18] * (FLAC__int64)data[i - 19]; /* Falls through. */
            case 18:
                sum += qlp_coeff[17] * (FLAC__int64)data[i - 18]; /* Falls through. */
            case 17:
                sum += qlp_coeff[16] * (FLAC__int64)data[i - 17]; /* Falls through. */
            case 16:
                sum += qlp_coeff[15] * (FLAC__int64)data[i - 16]; /* Falls through. */
            case 15:
                sum += qlp_coeff[14] * (FLAC__int64)data[i - 15]; /* Falls through. */
            case 14:
                sum += qlp_coeff[13] * (FLAC__int64)data[i - 14]; /* Falls through. */
            case 13:
                sum += qlp_coeff[12] * (FLAC__int64)data[i - 13];
                sum += qlp_coeff[11] * (FLAC__int64)data[i - 12];
                sum += qlp_coeff[10] * (FLAC__int64)data[i - 11];
                sum += qlp_coeff[9] * (FLAC__int64)data[i - 10];	
                sum += qlp_coeff[8] * (FLAC__int64)data[i - 9];
                sum += qlp_coeff[7] * (FLAC__int64)data[i - 8];
                sum += qlp_coeff[6] * (FLAC__int64)data[i - 7];
                sum += qlp_coeff[5] * (FLAC__int64)data[i - 6];
                sum += qlp_coeff[4] * (FLAC__int64)data[i - 5];
                sum += qlp_coeff[3] * (FLAC__int64)data[i - 4];
                sum += qlp_coeff[2] * (FLAC__int64)data[i - 3];
                sum += qlp_coeff[1] * (FLAC__int64)data[i - 2];
                sum += qlp_coeff[0] * (FLAC__int64)data[i - 1];
            }
            residual[i] = data[i] - (sum >> lp_quantization);
        }
    }

    return;
}

#endif /* FLAC__CPU_ARM64 && FLAC__HAS_ARCH64INTRIN */
#endif /* FLAC__NO_ASM */
#endif /* FLAC__INTEGER_ONLY_LIBRARY */
