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

#ifndef MUSE_AUDIO_VECTOROPS_H
#define MUSE_AUDIO_VECTOROPS_H

#include <stdlib.h>

#include <cstdint>

//
// This header is provided for convenience, to easily wrap vector operations around
// their platform-specific optimised libraries (e.g. IPP, vDSP), if desired.
// This can be done by adding #if defined(...) compile-time branches to each function
// and calling the corresponding library function, e.g. ippsAdd_32f or vDSP_vadd
// inside add().
//

namespace muse::audio::fx {
namespace vo {
inline void* allocate(int32_t bytes)
{
    return ::malloc(bytes);
}

inline void free(void* ptr)
{
    return ::free(ptr);
}

template<class T>
void copy(const T* src, T* dst, int32_t n)
{
    memcpy(dst, src, n * sizeof(T));
}

template<class T>
void add(const T* src1, const T* src2, T* dst, int32_t n)
{
    for (int32_t i = 0; i < n; i++) {
        dst[i] = src1[i] + src2[i];
    }
}

template<class T>
void subtract(const T* src1, const T* src2, T* dst, int32_t n)
{
    for (int32_t i = 0; i < n; i++) {
        dst[i] = src2[i] - src1[i];
    }
}

template<class T>
void constantMultiply(const T* src, T constant, T* dst, int32_t n)
{
    for (int32_t i = 0; i < n; i++) {
        dst[i] = src[i] * constant;
    }
}

template<class T>
inline void constantMultiplyAndAdd(const T* src, T constant, T* dst, int32_t n)
{
    for (int32_t i = 0; i < n; i++) {
        dst[i] += src[i] * constant;
    }
}

template<class T>
void setToZero(T* dst, int32_t n)
{
    std::fill(dst, dst + n, 0.f);
}
} // namespace vo
}

#endif // MUSE_AUDIO_VECTOROPS_H
