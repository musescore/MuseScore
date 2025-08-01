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

#ifndef MUSE_AUDIO_SIMDTYPES_NEON_H
#define MUSE_AUDIO_SIMDTYPES_NEON_H

#if _MSC_VER
#include <arm64_neon.h>
#define __finl __forceinline
#define __vecc __vectorcall
#else
#include <arm_neon.h>
#define __finl inline __attribute__((always_inline))
#define __vecc
#endif

/*
  Neon version of SIMD types.
 */

namespace muse::audio::fx::simd {
struct float_x4
{
    float32x4_t s;

    __finl float_x4()
    {
    }

    /// enables math like: float_x4 a = 0.5f * float_x4{1.f, 2.f, 3.f, 4.f};
    __finl float_x4(float val)
    {
        s = vdupq_n_f32(val);
    }

    __finl float_x4(const float32x4_t& val)
        : s(val)
    {
    }

    /// enables assignments like: float_x4 a = {1.f, 2.f, 3.f, 4.f};
    __finl float_x4(float v0, float v1, float v2, float v3)
    {
#if _MSC_VER // aggregate initializer won't work unless we have {.n128_f32 = ..} in c++20
        s.n128_f32[0] = v0;
        s.n128_f32[1] = v1;
        s.n128_f32[2] = v2;
        s.n128_f32[3] = v3;
#elif defined(__GNUC__) // gcc (12.2.0) doesn't seem to allow initializer list.
        const float init[4] = { v0, v1, v2, v3 };
        s = vld1q_f32(init);
#else
        s = { v0, v1, v2, v3 };
#endif
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
        float32x4_t& val;
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

#elif _MSC_VER
    // on msvc returning a ref to a sub-register is possible
    __finl float& operator[](int n)
    {
        return s.n128_f32[n];
    }

    __finl const float operator[](int n) const
    {
        return s.n128_f32[n];
    }

#endif
};

__finl float_x4 __vecc operator+(float_x4 a, float_x4 b)
{
    return vaddq_f32(a.s, b.s);
}

__finl float_x4 __vecc operator-(float_x4 a, float_x4 b)
{
    return vsubq_f32(a.s, b.s);
}

__finl float_x4 __vecc operator*(float_x4 a, float_x4 b)
{
    return vmulq_f32(a.s, b.s);
}
} // namespace muse::audio::fx

#endif // MUSE_AUDIO_SIMDTYPES_NEON_H
