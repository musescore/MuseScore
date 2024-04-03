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

#ifndef MUSE_AUDIO_SIMDTYPES_H
#define MUSE_AUDIO_SIMDTYPES_H

#include <cassert>
#include <cstdlib>

#if _MSC_VER
#define __finl __forceinline
#define __vecc __vectorcall
#else
#define __finl inline __attribute__((always_inline))
#define __vecc
#endif

#if defined(__SSE2__) || (defined(_M_AMD64) || defined(_M_X64))
#include "simdtypes_sse2.h"
#elif defined(__arm64__) || defined(__aarch64__) || defined(_M_ARM64)
#include "simdtypes_neon.h"
#else
#include "simdtypes_scalar.h"
#endif

/*
  Simd-types for parallel dsp processing.
  Aligned memory allocation for simd vectors.
 */

namespace muse::audio::fx::simd {
/// reserve aligned memory. Needs to be freed with aligned_free()
inline void* aligned_malloc(size_t required_bytes, size_t alignment)
{
    auto offset = alignment - 1 + sizeof(void*);
    auto p1 = std::malloc(required_bytes + offset);
    if (p1 == nullptr) {
        return nullptr;
    }
    // figure out aligned position
    void* p2 = (void*)(((size_t)(p1) + offset) & ~(alignment - 1));
    // write malloced pointer in front of aligned data
    ((void**)p2)[-1] = p1;
    return p2;
}

/// free memory allocated with aligned_malloc
inline void aligned_free(void* p)
{
    if (p) {
        free(((void**)p)[-1]);
    }
}

/// create a c++ class at an memory-aligned spot that needs to be deleted using aligned_delete
template<typename cls>
inline cls* aligned_new(int alignment)
{
    void* mem = aligned_malloc(sizeof(cls), alignment);
    return new (mem) cls();
}

/** delete objects created using aligned_new */
template<typename cls>
inline void aligned_delete(cls* obj)
{
    if (obj != nullptr) {
        obj->~cls();
        aligned_free((void*)obj);
    }
}
} // namespace muse::audio::fx

#endif // MUSE_AUDIO_SIMDTYPES_H
