/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-CLA-applies
 *
 * MuseScore
 * Music Composition & Notation
 *
 * Copyright (C) 2023 MuseScore BVBA and others
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 3 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#ifndef MUSE_AUDIO_REVERBMATRICES_H
#define MUSE_AUDIO_REVERBMATRICES_H

#include "log.h"

namespace muse::audio::fx {
namespace reverb_matrices {
template<int n>
inline void Hadamard(const float i[n], float o[n])
{
    UNUSED(i);
    UNUSED(o);

    // Hadamard matrix not defined for this size
    assert(false);
}

template<>
inline void Hadamard<8>(const float i[8], float o[8])
{
    // log2(n)*n adds n muls = 24 adds, 8 muls

    const float fact = 1.f / std::sqrt(8.f);

    float a[] = { i[7] + i[0], i[6] + i[1], i[5] + i[3], i[2] + i[4] };
    float b[] = { i[7] - i[0], i[6] - i[1], i[5] - i[3], i[2] - i[4] };

    float c[] = { a[0] + a[1], a[2] + a[3], b[0] + b[1], b[2] + b[3] };
    float d[] = { a[0] - a[1], a[2] - a[3], b[0] - b[1], b[2] - b[3] };

    o[0] = fact * (c[0] + c[1]);
    o[1] = fact * (c[2] + c[3]);
    o[2] = fact * (d[0] + d[1]);
    o[3] = fact * (d[2] + d[3]);
    o[4] = fact * (c[0] - c[1]);
    o[5] = fact * (c[2] - c[3]);
    o[6] = fact * (d[0] - d[1]);
    o[7] = fact * (d[2] - d[3]);
}

template<>
inline void Hadamard<12>(const float i[12], float o[12])
{
    // adds 12 + 24 + 24 = 60 adds, 12 muls

    // this might seem a bit crazy and it took me 4 attempts to get all signs and numbers right.. I don't have enough
    // autism. The 12x12 hadamard matrix is quite irregular (there exists only one).
    // I grouped operations as simd-friendly as I could.

    const float fact = 1.f / std::sqrt(12.f);

    float a[] = { i[7] + i[4], i[10] + i[11], i[2] + i[5], i[0] + i[1], i[8] + i[9], i[6] + i[3] };
    float b[] = { i[7] - i[4], i[10] - i[11], i[2] - i[5], i[0] - i[1], i[8] - i[9], i[6] - i[3] };

    float c[] = { a[0] + a[1], a[0] + b[1], b[0] + a[1], b[0] + b[1], a[0] - a[1], a[0] - b[1], b[0] - a[1], b[0] - b[1] };
    float d[] = { a[2] + a[3], a[2] + b[3], b[2] + a[3], b[2] + b[3], a[2] - a[3], a[2] - b[3], b[2] - a[3], b[2] - b[3] };
    float e[] = { a[4] + a[5], a[4] + b[5], b[4] + a[5], b[4] + b[5], a[4] - a[5], a[4] - b[5], b[4] - a[5], b[4] - b[5] };

    o[0] = fact * (c[0] + d[0] + e[0]);
    o[1] = fact * (c[3] + d[1] - e[5]);
    o[2] = fact * (c[7] - d[6] - e[1]);
    o[3] = fact * (c[4] + d[2] + e[6]);
    o[4] = fact * (c[3] - d[3] + e[4]);
    o[5] = fact * (c[7] - d[5] + e[1]);
    o[6] = fact * (c[6] + d[7] - e[6]);
    o[7] = fact * (c[4] - d[2] + e[2]);
    o[8] = fact * (c[1] - d[5] - e[3]);
    o[9] = fact * (c[0] - d[1] - e[5]);
    o[10] = fact * (c[2] + d[6] + e[7]);
    o[11] = fact * (c[5] + d[4] - e[2]);
}

template<>
inline void Hadamard<16>(const float i[16], float o[16])
{
    // 16 + 16 + 16 + 16 = 64 adds, 16 muls

    const float fact = 1.f / std::sqrt(16.f);

    float a[] = { i[10] + i[4], i[0] + i[5], i[15] + i[11], i[12] + i[7],
                  i[2] + i[14], i[1] + i[8], i[3] + i[9],   i[13] + i[6] };
    float b[] = { i[10] - i[4], i[0] - i[5], i[15] - i[11], i[12] - i[7],
                  i[2] - i[14], i[1] - i[8], i[3] - i[9],   i[13] - i[6] };

    float c[] = { a[0] + a[1], a[2] + a[3], a[4] + a[5], a[6] + a[7], b[0] + b[1], b[2] + b[3], b[4] + b[5], b[6] + b[7] };
    float d[] = { a[0] - a[1], a[2] - a[3], a[4] - a[5], a[6] - a[7], b[0] - b[1], b[2] - b[3], b[4] - b[5], b[6] - b[7] };

    float e[] = { c[0] + c[1], c[2] + c[3], d[0] + d[1], d[2] + d[3], c[4] + c[5], c[6] + c[7], d[4] + d[5], d[6] + d[7] };
    float f[] = { c[0] - c[1], c[2] - c[3], d[0] - d[1], d[2] - d[3], c[4] - c[5], c[6] - c[7], d[4] - d[5], d[6] - d[7] };

    o[0] = fact * (e[0] + e[1]);
    o[1] = fact * (e[2] + e[3]);
    o[2] = fact * (e[4] + e[5]);
    o[3] = fact * (e[6] + e[7]);
    o[4] = fact * (f[0] + f[1]);
    o[5] = fact * (f[2] + f[3]);
    o[6] = fact * (f[4] + f[5]);
    o[7] = fact * (f[6] + f[7]);
    o[8] = fact * (e[0] - e[1]);
    o[9] = fact * (e[2] - e[3]);
    o[10] = fact * (e[4] - e[5]);
    o[11] = fact * (e[6] - e[7]);
    o[12] = fact * (f[0] - f[1]);
    o[13] = fact * (f[2] - f[3]);
    o[14] = fact * (f[4] - f[5]);
    o[15] = fact * (f[6] - f[7]);
}

template<>
inline void Hadamard<24>(const float i[24], float o[24])
{
    // 24 + 48 + 48 = 120 adds, 24 muls
    const float fact = 1.f / std::sqrt(24.f);

    float a[] = { i[11] + i[12], i[23] + i[14], i[4] + i[2],   i[1] + i[22], i[3] + i[19], i[9] + i[0],
                  i[17] + i[7],  i[5] + i[10],  i[21] + i[13], i[6] + i[18], i[8] + i[15], i[20] + i[16] };
    float b[] = { i[11] - i[12], i[23] - i[14], i[4] - i[2],   i[1] - i[22], i[3] - i[19], i[9] - i[0],
                  i[17] - i[7],  i[5] - i[10],  i[21] - i[13], i[6] - i[18], i[8] - i[15], i[20] - i[16] };

    float c[] = { a[0] + a[1], a[0] + b[1], b[0] + a[1], b[0] + b[1], a[0] - a[1], a[0] - b[1], b[0] - a[1], b[0] - b[1] };
    float d[] = { a[2] + a[3], a[2] + b[3], b[2] + a[3], b[2] + b[3], a[2] - a[3], a[2] - b[3], b[2] - a[3], b[2] - b[3] };
    float e[] = { a[4] + a[5], a[4] + b[5], b[4] + a[5], b[4] + b[5], a[4] - a[5], a[4] - b[5], b[4] - a[5], b[4] - b[5] };
    float f[] = { a[6] + a[7], a[6] + b[7], b[6] + a[7], b[6] + b[7], a[6] - a[7], a[6] - b[7], b[6] - a[7], b[6] - b[7] };
    float g[] = { a[8] + a[9], a[8] + b[9], b[8] + a[9], b[8] + b[9], a[8] - a[9], a[8] - b[9], b[8] - a[9], b[8] - b[9] };
    float h[] = { a[10] + a[11], a[10] + b[11], b[10] + a[11], b[10] + b[11],
                  a[10] - a[11], a[10] - b[11], b[10] - a[11], b[10] - b[11] };

    o[0] = fact * (+c[0] + d[0] + e[0] + f[0] + g[0] + h[0]);
    o[1] = fact * (-c[2] - d[5] + e[7] + f[7] + g[3] + h[0]);
    o[2] = fact * (-c[7] - d[1] - e[2] + f[4] + g[5] - h[6]);
    o[3] = fact * (-c[6] - d[0] + e[3] - f[7] - g[7] + h[2]);
    o[4] = fact * (-c[6] + d[6] - e[3] - f[4] - g[4] - h[3]);
    o[5] = fact * (-c[6] + d[4] - e[5] + f[7] + g[7] + h[3]);
    o[6] = fact * (-c[4] + d[1] - e[1] - f[2] + g[4] + h[5]);
    o[7] = fact * (-c[3] + d[0] - e[0] + f[3] - g[7] - h[7]);
    o[8] = fact * (-c[5] + d[0] + e[6] - f[3] - g[4] - h[4]);
    o[9] = fact * (-c[3] - d[6] + e[4] - f[5] + g[7] + h[7]);
    o[10] = fact * (-c[7] + d[2] + e[1] - f[1] - g[2] + h[4]);
    o[11] = fact * (-c[4] - d[3] + e[0] - f[0] + g[3] - h[7]);
    o[12] = fact * (-c[1] + d[3] + e[0] + f[6] - g[3] - h[4]);
    o[13] = fact * (-c[2] + d[5] - e[6] + f[4] - g[5] + h[7]);
    o[14] = fact * (-c[7] - d[7] + e[2] + f[1] - g[1] - h[2]);
    o[15] = fact * (-c[4] - d[4] - e[3] + f[0] - g[0] + h[3]);
    o[16] = fact * (-c[1] + d[7] + e[3] + f[0] + g[6] - h[3]);
    o[17] = fact * (-c[2] + d[4] + e[5] - f[6] + g[4] - h[5]);
    o[18] = fact * (-c[5] - d[7] - e[7] + f[2] + g[1] - h[1]);
    o[19] = fact * (-c[3] - d[4] - e[4] - f[3] + g[0] - h[0]);
    o[20] = fact * (-c[5] + d[7] + e[7] + f[3] + g[0] + h[6]);
    o[21] = fact * (-c[1] - d[2] + e[4] + f[5] - g[6] + h[4]);
    o[22] = fact * (-c[0] + d[3] - e[7] - f[7] + g[2] + h[1]);
    o[23] = fact * (-c[0] - d[3] - e[4] - f[4] - g[3] + h[0]);
}
} // namespace reverb_matrices
} // namespace muse::audio::fx

#endif // MUSE_AUDIO_REVERBMATRICES_H
