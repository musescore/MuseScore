/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-CLA-applies
 *
 * MuseScore
 * Music Composition & Notation
 *
 * Copyright (C) 2025 MuseScore BVBA and others
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
#pragma once

#include "msgpack_forward.h"
#include "../types/bytearray.h"
#include "../types/string.h"
#include "../types/number.h"

void pack_custom(std::vector<uint8_t>& data, const muse::String& value);
bool unpack_custom(muse::msgpack::Cursor& cursor, muse::String& value);

template<typename T>
void pack_custom(std::vector<uint8_t>& data, const muse::number_t<T>& value);
template<typename T>
bool unpack_custom(muse::msgpack::Cursor& cursor, muse::number_t<T>& value);

#include "../thirdparty/kors_msgpack/msgpack/msgpack.h"

// muse standart types
inline void pack_custom(std::vector<uint8_t>& data, const muse::String& value)
{
    muse::msgpack::Packer::pack(data, value.toStdString());
}

inline bool unpack_custom(muse::msgpack::Cursor& cursor, muse::String& value)
{
    std::string str;
    bool ok = muse::msgpack::UnPacker::unpack(cursor, str);
    value = muse::String::fromStdString(str);
    return ok;
}

template<typename T>
inline void pack_custom(std::vector<uint8_t>& data, const muse::number_t<T>& value)
{
    muse::msgpack::Packer::pack(data, value.raw());
}

template<typename T>
inline bool unpack_custom(muse::msgpack::Cursor& cursor, muse::number_t<T>& value)
{
    T val = {};
    bool ok = muse::msgpack::UnPacker::unpack(cursor, val);
    value = muse::number_t<T>(val);
    return ok;
}

// pack / unpack
namespace muse::msgpack {
template<class ... Types>
static inline void pack(std::vector<uint8_t>& data, const Types&... args)
{
    Packer::pack(data, args ...);
}

template<class ... Types>
static inline ByteArray pack(const Types&... args)
{
    ByteArray ba;
    std::vector<uint8_t>& vdata = ba.vdata();
    vdata.clear();
    Packer::pack(vdata, args ...);
    return ba;
}

template<class ... Types>
static inline bool unpack(muse::msgpack::Cursor& cursor, Types&... args)
{
    return UnPacker::unpack(cursor, args ...);
}

template<class ... Types>
static inline bool unpack(const ByteArray& data, Types&... args)
{
    return UnPacker::unpack(data.constVData(), args ...);
}
}
