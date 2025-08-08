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

#include <optional>

#include "msgpack_forward.h"
#include "../types/bytearray.h"
#include "../types/string.h"
#include "../types/number.h"
#include "../types/flags.h"
#include "../types/retval.h"
#include "../io/path.h"

void pack_custom(std::vector<uint8_t>& data, const size_t& value);
bool unpack_custom(muse::msgpack::Cursor& cursor, size_t& value);

template<typename T>
void pack_custom(std::vector<uint8_t>& data, const std::optional<T>& value);
template<typename T>
bool unpack_custom(muse::msgpack::Cursor& cursor, std::optional<T>& value);

void pack_custom(std::vector<uint8_t>& data, const muse::String& value);
bool unpack_custom(muse::msgpack::Cursor& cursor, muse::String& value);

template<typename T>
void pack_custom(std::vector<uint8_t>& data, const muse::number_t<T>& value);
template<typename T>
bool unpack_custom(muse::msgpack::Cursor& cursor, muse::number_t<T>& value);

template<typename T>
void pack_custom(std::vector<uint8_t>& data, const muse::Flags<T>& value);
template<typename T>
bool unpack_custom(muse::msgpack::Cursor& cursor, muse::Flags<T>& value);

// Ret[Val]
void pack_custom(std::vector<uint8_t>& data, const muse::Ret& value);
bool unpack_custom(muse::msgpack::Cursor& cursor, muse::Ret& value);
template<typename T>
void pack_custom(std::vector<uint8_t>& data, const muse::RetVal<T>& value);
template<typename T>
bool unpack_custom(muse::msgpack::Cursor& cursor, muse::RetVal<T>& value);
template<typename T1, typename T2>
void pack_custom(std::vector<uint8_t>& data, const muse::RetVal2<T1, T2>& value);
template<typename T1, typename T2>
bool unpack_custom(muse::msgpack::Cursor& cursor, muse::RetVal2<T1, T2>& value);

// path_t
void pack_custom(std::vector<uint8_t>& data, const muse::io::path_t& value);
bool unpack_custom(muse::msgpack::Cursor& cursor, muse::io::path_t& value);

#include "../thirdparty/kors_msgpack/msgpack/msgpack.h"

// std standart
inline void pack_custom(std::vector<uint8_t>& data, const size_t& value)
{
    muse::msgpack::Packer::pack(data, static_cast<uint64_t>(value));
}

inline bool unpack_custom(muse::msgpack::Cursor& cursor, size_t& value)
{
    uint64_t v = 0;
    bool ok = muse::msgpack::UnPacker::unpack(cursor, v);
    value = static_cast<size_t>(v);
    return ok;
}

template<typename T>
inline void pack_custom(std::vector<uint8_t>& data, const std::optional<T>& value)
{
    bool has_value = value.has_value();
    muse::msgpack::Packer::pack(data, has_value);
    if (has_value) {
        muse::msgpack::Packer::pack(data, value.value());
    }
}

template<typename T>
inline bool unpack_custom(muse::msgpack::Cursor& cursor, std::optional<T>& value)
{
    bool has_value = false;
    bool ok = muse::msgpack::UnPacker::unpack(cursor, has_value);
    if (!ok) {
        return false;
    }

    if (!has_value) {
        value = std::nullopt;
        return true;
    }

    T val = {};
    ok = muse::msgpack::UnPacker::unpack(cursor, val);
    if (!ok) {
        return false;
    }

    value = std::make_optional<T>(val);
    return true;
}

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

template<typename T>
inline void pack_custom(std::vector<uint8_t>& data, const muse::Flags<T>& value)
{
    typename muse::Flags<T>::Int val = value;
    muse::msgpack::Packer::pack(data, val);
}

template<typename T>
inline bool unpack_custom(muse::msgpack::Cursor& cursor, muse::Flags<T>& value)
{
    typename muse::Flags<T>::Int val = 0;
    bool ok = muse::msgpack::UnPacker::unpack(cursor, val);
    value = muse::Flags<T>(val);
    return ok;
}

// Ret[Val]
inline void pack_custom(std::vector<uint8_t>& data, const muse::Ret& value)
{
    muse::msgpack::Packer::pack(data, value.code(), value.text());
}

inline bool unpack_custom(muse::msgpack::Cursor& cursor, muse::Ret& value)
{
    int code = 0;
    std::string text;
    bool ok = muse::msgpack::UnPacker::unpack(cursor, code, text);
    value = muse::Ret(code, text);
    return ok;
}

template<typename T>
inline void pack_custom(std::vector<uint8_t>& data, const muse::RetVal<T>& value)
{
    muse::msgpack::Packer::pack(data, value.ret, value.val);
}

template<typename T>
inline bool unpack_custom(muse::msgpack::Cursor& cursor, muse::RetVal<T>& value)
{
    return muse::msgpack::UnPacker::unpack(cursor, value.ret, value.val);
}

template<typename T1, typename T2>
inline void pack_custom(std::vector<uint8_t>& data, const muse::RetVal2<T1, T2>& value)
{
    muse::msgpack::Packer::pack(data, value.ret, value.val1, value.val2);
}

template<typename T1, typename T2>
inline bool unpack_custom(muse::msgpack::Cursor& cursor, muse::RetVal2<T1, T2>& value)
{
    return muse::msgpack::UnPacker::unpack(cursor, value.ret, value.val1, value.val2);
}

// path_t
inline void pack_custom(std::vector<uint8_t>& data, const muse::io::path_t& value)
{
    muse::msgpack::Packer::pack(data, value.toStdString());
}

inline bool unpack_custom(muse::msgpack::Cursor& cursor, muse::io::path_t& value)
{
    std::string str;
    bool ok = muse::msgpack::UnPacker::unpack(cursor, str);
    value = muse::io::path_t(str);
    return ok;
}

// pack / unpack
namespace muse::msgpack {
struct Options {
    size_t rezerveSize = 100;
};

template<class ... Types>
static inline void pack_to_data(std::vector<uint8_t>& data, const Types&... args)
{
    Packer::pack(data, args ...);
}

template<class ... Types>
static inline ByteArray pack(const Options& opt, const Types&... args)
{
    ByteArray ba;
    std::vector<uint8_t>& vdata = ba.vdata();
    vdata.clear();
    vdata.reserve(opt.rezerveSize);
    Packer::pack(vdata, args ...);
    vdata.emplace_back(0);
    return ba;
}

template<class ... Types>
static inline ByteArray pack(const Types&... args)
{
    return pack(Options {}, args ...);
}

template<class ... Types>
static inline bool unpack(muse::msgpack::Cursor& cursor, Types&... args)
{
    return UnPacker::unpack(cursor, args ...);
}

template<class ... Types>
static inline bool unpack(const ByteArray& data, Types&... args)
{
    return DataUnPacker<std::vector<uint8_t>>::unpack(data.constVData(), args ...);
}
}
