/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-CLA-applies
 *
 * MuseScore
 * Music Composition & Notation
 *
 * Copyright (C) 2021 MuseScore BVBA and others
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
#ifndef MUSE_GLOBAL_PTRUTILS_H
#define MUSE_GLOBAL_PTRUTILS_H

#include "runtime.h"
#include "stringutils.h"
#include "log.h"

namespace muse::ptr {
template<typename T, typename E> T* checked_cast(E* source)
{
#ifndef NDEBUG
    T* casted = dynamic_cast<T*>(source);
    if (source && !casted) {
        Q_ASSERT_X(false, "checked_cast", "bad cast");
    }
    return casted;
#else
    return static_cast<T*>(source);
#endif
}

template<typename T, typename E> const T* checked_cast(const E* source)
{
#ifndef NDEBUG
    T* casted = dynamic_cast<T*>(source);
    if (source && !casted) {
        Q_ASSERT_X(false, "checked_cast", "bad cast");
    }
    return casted;
#else
    return static_cast<T*>(source);
#endif
}

inline bool string_is_hex(const std::string& str)
{
    return strings::startsWith(str, "0x");
}

inline std::string ptr_to_string(void* p)
{
    std::stringstream ss;
    ss << p;
    return ss.str();
}

inline void* ptr_from_string(const std::string& str)
{
    uintptr_t num = 0;
    std::stringstream ss;
    ss << std::hex << str;
    ss >> num;
    return reinterpret_cast<void*>(num);
}
}

#endif // MUSE_GLOBAL_PTRUTILS_H
