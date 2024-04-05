/* libFLAC - Free Lossless Audio Codec library
 * Copyright (C) 2022-2023  Xiph.Org Foundation
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
#if defined FLAC__CPU_X86_64 && FLAC__HAS_X86INTRIN
#include "private/lpc.h"
#ifdef FLAC__FMA_SUPPORTED

#include "FLAC/assert.h"

FLAC__SSE_TARGET("fma")
void FLAC__lpc_compute_autocorrelation_intrin_fma_lag_8(const FLAC__real data[], uint32_t data_len, uint32_t lag, double autoc[])
{
#undef MAX_LAG
#define MAX_LAG 8
#include "deduplication/lpc_compute_autocorrelation_intrin.c"
}

FLAC__SSE_TARGET("fma")
void FLAC__lpc_compute_autocorrelation_intrin_fma_lag_12(const FLAC__real data[], uint32_t data_len, uint32_t lag, double autoc[])
{
#undef MAX_LAG
#define MAX_LAG 12
#include "deduplication/lpc_compute_autocorrelation_intrin.c"
}
FLAC__SSE_TARGET("fma")
void FLAC__lpc_compute_autocorrelation_intrin_fma_lag_16(const FLAC__real data[], uint32_t data_len, uint32_t lag, double autoc[])
{
#undef MAX_LAG
#define MAX_LAG 16
#include "deduplication/lpc_compute_autocorrelation_intrin.c"

}

#endif /* FLAC__FMA_SUPPORTED */
#endif /* FLAC__CPU_X86_64 && FLAC__HAS_X86INTRIN */
#endif /* FLAC__NO_ASM */
#endif /* FLAC__INTEGER_ONLY_LIBRARY */
