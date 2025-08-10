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

#ifndef MUSE_AUDIO_SIMDTYPES_SCALAR_H
#define MUSE_AUDIO_SIMDTYPES_SCALAR_H

#include <algorithm>
#include <cmath>

#if _MSC_VER
#define __finl __forceinline
#define __vecc __vectorcall
#else
#define __finl inline __attribute__((always_inline))
#define __vecc
#endif

namespace muse::audio::fx::simd {
struct float_x4
{
    float v[4];

    __finl float_x4()
    {
    }

    /// enables math like: float_x4 a = 0.5f * float_x4{1.f, 2.f, 3.f, 4.f};
    __finl float_x4(float val)
    {
        v[0] = v[1] = v[2] = v[3] = val;
    }

    /// enables assignments like: float_x4 a = {1.f, 2.f, 3.f, 4.f};
    __finl float_x4(float v0, float v1, float v2, float v3)
    {
        v[0] = v0;
        v[1] = v1;
        v[2] = v2;
        v[3] = v3;
    }

    __finl float& operator[](int n)
    {
        return v[n];
    }

    __finl const float& operator[](int n) const
    {
        return v[n];
    }
};

__finl float_x4 __vecc operator+(float_x4 a, float_x4 b)
{
    return { a[0] + b[0], a[1] + b[1], a[2] + b[2], a[3] + b[3] };
}

__finl float_x4 __vecc operator-(float_x4 a, float_x4 b)
{
    return { a[0] - b[0], a[1] - b[1], a[2] - b[2], a[3] - b[3] };
}

__finl float_x4 __vecc operator*(float_x4 a, float_x4 b)
{
    return { a[0] * b[0], a[1] * b[1], a[2] * b[2], a[3] * b[3] };
}
} // namespace muse::audio::fx

#endif // MUSE_AUDIO_SIMDTYPES_SCALAR_H
