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

#ifndef MUSE_AUDIO_SIMDTYPES_SSE2_H
#define MUSE_AUDIO_SIMDTYPES_SSE2_H

#if _MSC_VER
#define __finl __forceinline
#define __vecc __vectorcall
#else
#define __finl inline __attribute__((always_inline))
#define __vecc
#endif

#include <emmintrin.h>

/*
SSE2 simd types
*/

namespace muse::audio::fx::simd {
// this is jumping through some hoops to get the same level of support
// for clang and msvc. With clang, the sse2 types are built-in and have
// some arithmetic operators defined.
// On msvc the sse2 types are structs with no operators.
// to get to the same level and to be able to write algorithms "naturally",
// everything needs to be wrapped in a struct.
struct float_x4
{
    __m128 s;
    __finl float_x4()
    {
    }

    /// enables math like: float_x4 a = 0.5f * float_x4{1.f, 2.f, 3.f, 4.f};
    __finl float_x4(float val)
    {
        s = _mm_set1_ps(val);
    }

    __finl float_x4(const __m128& val)
        : s(val)
    {
    }

    /// enables assignments like: float_x4 a = {1.f, 2.f, 3.f, 4.f};
    __finl float_x4(float v0, float v1, float v2, float v3)
    {
        s = _mm_setr_ps(v0, v1, v2, v3);
    }

#if defined(__clang__) || defined(__GNUC__)
private:
    // this helper class allows writing to the single registers for clang
    // __mm128 is a built-in type -> we can't return a float& reference.
    // this is just syntax sugar and clang will remove it during builds.
    //
    // it allows to write
    // float_x4 a;
    // a[1] = 2.f;
    struct RegisterAccessWrapper
    {
        __m128& val;
        int i;

        void operator=(float x)
        {
            val[i] = x;
        }

        operator float() noexcept
        {
            return val[i];
        }
    };

public:
    __finl RegisterAccessWrapper operator[](int n)
    {
        RegisterAccessWrapper raw = { s, n };
        return raw;
    }

    __finl float operator[](int n) const
    {
        return s[n];
    }

#elif defined(_MSC_VER)
    // on msvc returning a ref to a sub-register is possible
    __finl float& operator[](int n)
    {
        return s.m128_f32[n];
    }

    __finl const float operator[](int n) const
    {
        return s.m128_f32[n];
    }

#endif
};

__finl float_x4 __vecc operator+(float_x4 a, float_x4 b)
{
    return _mm_add_ps(a.s, b.s);
}

__finl float_x4 __vecc operator-(float_x4 a, float_x4 b)
{
    return _mm_sub_ps(a.s, b.s);
}

__finl float_x4 __vecc operator*(float_x4 a, float_x4 b)
{
    return _mm_mul_ps(a.s, b.s);
}
} // namespace muse::audio::fx

#endif // MUSE_AUDIO_SIMDTYPES_SSE2_H
