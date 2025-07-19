/*
MIT License

Copyright (c) 2025 Igor Korsukov

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/
#pragma once

#include <cassert>
#include <cstdint>
#include <cstring>
#include <vector>
#include <limits>
#include <cmath>
#include <string>
#include <utility>
#include <type_traits>

namespace kors::msgpack {

enum Format : uint8_t {
    // positive fixint = 0x00 - 0x7f
    fixmap = 0x80,   //0x80 - 0x8f
    fixarray = 0x90, //0x90 - 0x9a
    fixstr = 0xa0,   //0xa0 - 0xbf
    // negative fixint = 0xe0 - 0xff

    nil = 0xc0,
    false_bool = 0xc2,
    true_bool = 0xc3,
    bin8 = 0xc4,
    bin16 = 0xc5,
    bin32 = 0xc6,
    ext8 = 0xc7,
    ext16 = 0xc8,
    ext32 = 0xc9,
    float32 = 0xca,
    float64 = 0xcb,
    uint8 = 0xcc,
    uint16 = 0xcd,
    uint32 = 0xce,
    uint64 = 0xcf,
    int8 = 0xd0,
    int16 = 0xd1,
    int32 = 0xd2,
    int64 = 0xd3,
    fixext1 = 0xd4,
    fixext2 = 0xd5,
    fixext4 = 0xd6,
    fixext8 = 0xd7,
    fixext16 = 0xd8,
    str8 = 0xd9,
    str16 = 0xda,
    str32 = 0xdb,
    array16 = 0xdc,
    array32 = 0xdd,
    map16 = 0xde,
    map32 = 0xdf
};

struct Cursor {
    const uint8_t* current = nullptr;
    const uint8_t* end = nullptr;
    bool error = false;

    Cursor(const uint8_t* start, size_t size)
        : current(start), end(start + size) {}

    inline size_t remain() const {
        return end - current;
    }

    inline const uint8_t* pdata() {
        if (current < end) {
            return current;
        }
        error = true;
        return nullptr;
    }

    inline uint8_t data() {
        if (current < end) {
            return *current;
        }
        error = true;
        return 0;
    }

    inline void next(int64_t bytes = 1) {
        if (static_cast<int64_t>(remain()) >= bytes) {
            current += bytes;
        } else {
            error = true;
        }
    }

    inline uint8_t read() {
        uint8_t b = data();
        next();
        return b;
    }
};

template <typename T, typename = void>
struct is_container : std::false_type {};

template <typename T>
struct is_container<T, std::void_t<
        typename T::value_type,
        typename T::iterator,
        typename T::size_type,
        decltype(std::declval<T>().begin()),
decltype(std::declval<T>().end()),
decltype(std::declval<T>().size())
>> : std::true_type {};

template <typename T, typename = void>
struct is_map : std::false_type {};

template <typename T>
struct is_map<T, std::void_t<
        typename T::key_type,
        typename T::mapped_type,
        decltype(std::declval<typename T::value_type>().first),
decltype(std::declval<typename T::value_type>().second)
>> : std::true_type {};

template <typename T, typename = void>
struct has_reserve : std::false_type {};

template <typename T>
struct has_reserve<T, std::void_t<
        decltype(std::declval<T>().reserve())
>> : std::true_type {};

template <typename T, typename = void>
struct has_emplace_back : std::false_type {};

template <typename T>
struct has_emplace_back<T, std::void_t<
        decltype(std::declval<T>().emplace_back())
>> : std::true_type {};

class Packer;

template <typename T, typename = void>
struct has_pack_packer : std::false_type {};

template <typename T>
struct has_pack_packer<T, std::void_t<
        decltype(std::declval<T&>().pack(std::declval<Packer&>()))
>> : std::true_type {};

template <typename T, typename = void>
struct has_pack_const_packer : std::false_type {};

template <typename T>
struct has_pack_const_packer<T, std::void_t<
        decltype(std::declval<const T&>().pack(std::declval<Packer&>()))
>> : std::true_type {};

template <typename T, typename = void>
struct has_pack_data : std::false_type {};

template <typename T>
struct has_pack_data<T, std::void_t<
        decltype(std::declval<const T&>().pack(std::declval<std::vector<uint8_t>&>()))
>> : std::true_type {};

template <typename T, typename = void>
struct has_func_pack_custom_packer : std::false_type {};

template <typename T>
struct has_func_pack_custom_packer<T,
           std::void_t<decltype(pack_custom(std::declval<Packer&>(), std::declval<T&>()))>
       > : std::true_type {};

class UnPacker;

template <typename T, typename = void>
struct has_pack_unpacker : std::false_type {};

template <typename T>
struct has_pack_unpacker<T, std::void_t<
        decltype(std::declval<T&>().pack(std::declval<UnPacker&>()))
>> : std::true_type {};

template <typename T, typename = void>
struct has_unpack_unpacker : std::false_type {};

template <typename T>
struct has_unpack_unpacker<T, std::void_t<
        decltype(std::declval<T>().unpack(std::declval<UnPacker&>()))
>> : std::true_type {};

template <typename T, typename = void>
struct has_unpack_cursor : std::false_type {};

template <typename T>
struct has_unpack_cursor<T, std::void_t<
        decltype(std::declval<T>().unpack(std::declval<Cursor&>()))
>> : std::true_type {};

template <typename T, typename = void>
struct has_func_unpack_custom_unpacker : std::false_type {};

template <typename T>
struct has_func_unpack_custom_unpacker<T,
           std::void_t<decltype(unpack_custom(std::declval<UnPacker&>(), std::declval<T&>()))>
       > : std::true_type {};

// Nil
inline void pack_type(std::vector<uint8_t>& data, const std::nullptr_t&/*value*/) {
    data.emplace_back(nil);
}

inline bool unpack_type(Cursor& cursor, std::nullptr_t& /*value*/) {
    cursor.next();
    return true;
}

// Bool
inline void pack_type(std::vector<uint8_t>& data, const bool& value) {
    if (value) {
        data.emplace_back(true_bool);
    } else {
        data.emplace_back(false_bool);
    }
}

inline bool unpack_type(Cursor& cursor, bool& value) {
    value = cursor.read() != 0xc2;
    return true;
}

// Int family
template<typename T>
inline bool pack_fixint(std::vector<uint8_t>& data, const T& value) {

    // positive fixint
    if (value >= 0 && value <= 127) {
        data.emplace_back(static_cast<uint8_t>(value));
        return true;
    }

    // negative fixint
    if (value >= -32 && value <= -1) {
        data.emplace_back(static_cast<uint8_t>(0xE0 | (static_cast<uint8_t>(value + 32))));
        return true;
    }

    return false;
}

template<typename T>
inline bool unpack_fixint(Cursor& cursor, T& value) {

    // positive fixint
    uint8_t byte = cursor.data();
    if ((byte & 0x80) == 0) {
        value = static_cast<T>(byte);
        cursor.next();
        return true;
    }

    // negative fixint
    if ((byte & 0xE0) == 0xE0) {
        value = static_cast<T>(static_cast<int8_t>(byte & 0x1F) - 32);
        cursor.next();
        return true;
    }

    return false;
}

inline void pack_type(std::vector<uint8_t>& data, const int8_t& value) {
    if (pack_fixint(data, value)) {
        return;
    }

    data.emplace_back(int8);
    data.emplace_back(static_cast<uint8_t>(value));
}

inline bool unpack_type(Cursor& cursor, int8_t& value) {
    if (unpack_fixint(cursor, value)) {
        return true;
    } else if (cursor.data() == int8) {
        cursor.next();
        value = static_cast<int8_t>(cursor.read());
        return true;
    }
    return false;
}

inline void pack_type(std::vector<uint8_t>& data, const uint8_t& value) {
    if (pack_fixint(data, value)) {
        return;
    }

    data.emplace_back(uint8);
    data.emplace_back(value);
}

inline bool unpack_type(Cursor& cursor, uint8_t& value) {
    if (unpack_fixint(cursor, value)) {
        return true;
    } else if (cursor.data() == uint8) {
        cursor.next();
        value = cursor.read();
        return true;
    }
    return false;
}

template <typename T, typename U>
constexpr bool is_fit_in_type(U value) noexcept {
    return value >= std::numeric_limits<T>::min() &&
            value <= std::numeric_limits<T>::max();
}

inline void pack_type(std::vector<uint8_t>& data, const int16_t& value) {
    if (is_fit_in_type<int8_t>(value)) {
        pack_type(data, static_cast<int8_t>(value));
    } else {
        data.emplace_back(int16);
        uint16_t uvalue = static_cast<uint16_t>(value);
        data.emplace_back(static_cast<uint8_t>((uvalue >> 8) & 0xFF));
        data.emplace_back(static_cast<uint8_t>(uvalue & 0xFF));
    }
}

inline bool unpack_type(Cursor& cursor, int16_t& value) {

    if (int8_t i8 = 0; unpack_type(cursor, i8)) {
        value = static_cast<int16_t>(i8);
        return true;
    } else if (cursor.data() == int16) {
        cursor.next();
        uint8_t high = cursor.read();
        uint8_t low = cursor.read();
        uint16_t uvalue = (static_cast<uint16_t>(high) << 8) | low;
        value = static_cast<int16_t>(uvalue);
        return true;
    }

    return false;
}

inline void pack_type(std::vector<uint8_t>& data, const uint16_t& value) {
    if (is_fit_in_type<uint8_t>(value)) {
        pack_type(data, static_cast<uint8_t>(value));
    } else {
        data.emplace_back(uint16);
        data.emplace_back(static_cast<uint8_t>((value >> 8) & 0xFF));
        data.emplace_back(static_cast<uint8_t>(value & 0xFF));
    }
}

inline bool unpack_type(Cursor& cursor, uint16_t& value) {

    if (uint8_t ui8 = 0; unpack_type(cursor, ui8)) {
        value = static_cast<uint16_t>(ui8);
        return true;
    } else if (cursor.data() == uint16) {
        cursor.next();
        uint8_t high = cursor.read();
        uint8_t low = cursor.read();
        value = (static_cast<uint16_t>(high) << 8) | low;
        return true;
    }

    return false;
}

inline void pack_type(std::vector<uint8_t>& data, const int32_t& value) {
    if (is_fit_in_type<int16_t>(value)) {
        pack_type(data, static_cast<int16_t>(value));
    } else {
        data.emplace_back(int32);
        uint32_t uvalue = static_cast<uint32_t>(value);
        data.emplace_back(static_cast<uint8_t>((uvalue >> 24) & 0xFF));
        data.emplace_back(static_cast<uint8_t>((uvalue >> 16) & 0xFF));
        data.emplace_back(static_cast<uint8_t>((uvalue >> 8) & 0xFF));
        data.emplace_back(static_cast<uint8_t>(uvalue & 0xFF));
    }
}

inline bool unpack_type(Cursor& cursor, int32_t& value) {

    if (int16_t i16 = 0; unpack_type(cursor, i16)) {
        value = static_cast<int32_t>(i16);
        return true;
    } else if (cursor.data() == int32) {
        cursor.next();
        uint32_t ui32 = 0;
        for (int i = 0; i < 4; ++i) {
            uint8_t byte = cursor.read();
            ui32 = (ui32 << 8) | byte;
        }
        value = static_cast<int32_t>(ui32);
        return true;
    }

    return false;
}

inline void pack_type(std::vector<uint8_t>& data, const uint32_t& value) {
    if (is_fit_in_type<uint16_t>(value)) {
        pack_type(data, static_cast<uint16_t>(value));
    } else {
        data.emplace_back(uint32);
        data.emplace_back(static_cast<uint8_t>((value >> 24) & 0xFF));
        data.emplace_back(static_cast<uint8_t>((value >> 16) & 0xFF));
        data.emplace_back(static_cast<uint8_t>((value >> 8) & 0xFF));
        data.emplace_back(static_cast<uint8_t>(value & 0xFF));
    }
}

inline bool unpack_type(Cursor& cursor, uint32_t& value) {

    if (uint16_t ui16 = 0; unpack_type(cursor, ui16)) {
        value = static_cast<uint32_t>(ui16);
        return true;
    } else if (cursor.data() == uint32) {
        cursor.next();
        uint32_t ui32 = 0;
        for (int i = 0; i < 4; ++i) {
            uint8_t byte = cursor.read();
            ui32 = (ui32 << 8) | byte;
        }
        value = ui32;
        return true;
    }

    return false;
}

inline void pack_type(std::vector<uint8_t>& data, const int64_t& value) {
    if (is_fit_in_type<int32_t>(value)) {
        pack_type(data, static_cast<int32_t>(value));
    } else {
        data.emplace_back(int64);
        uint64_t uvalue = static_cast<uint64_t>(value);
        for (int i = 7; i >= 0; --i) {
            data.emplace_back(static_cast<uint8_t>((uvalue >> (i * 8)) & 0xFF));
        }
    }
}

inline bool unpack_type(Cursor& cursor, int64_t& value) {

    if (int32_t i32 = 0; unpack_type(cursor, i32)) {
        value = static_cast<int64_t>(i32);
        return true;
    } else if (cursor.data() == int64) {
        cursor.next();
        uint64_t ui64 = 0;
        for (int i = 0; i < 4; ++i) {
            uint8_t byte = cursor.read();
            ui64 = (ui64 << 8) | byte;
        }
        value = static_cast<int64_t>(ui64);
        return true;
    }

    return false;
}

inline void pack_type(std::vector<uint8_t>& data, const uint64_t& value) {
    if (is_fit_in_type<uint32_t>(value)) {
        pack_type(data, static_cast<uint32_t>(value));
    } else {
        data.emplace_back(uint64);
        for (int i = 7; i >= 0; --i) {
            data.emplace_back(static_cast<uint8_t>((value >> (i * 8)) & 0xFF));
        }
    }
}

inline bool unpack_type(Cursor& cursor, uint64_t& value) {

    if (uint32_t ui32 = 0; unpack_type(cursor, ui32)) {
        value = static_cast<uint64_t>(ui32);
        return true;
    } else if (cursor.data() == uint64) {
        cursor.next();
        uint64_t ui64 = 0;
        for (int i = 0; i < 4; ++i) {
            uint8_t byte = cursor.read();
            ui64 = (ui64 << 8) | byte;
        }
        value = ui64;
        return true;
    }

    return false;
}

// Float/Double
inline void pack_type(std::vector<uint8_t>& data, const float& value) {

    data.emplace_back(float32);

    uint32_t bin = 0;
    static_assert(sizeof(float) == sizeof(uint32_t), "Float size mismatch");
    std::memcpy(&bin, &value, sizeof(float));

    data.emplace_back(static_cast<uint8_t>((bin >> 24) & 0xFF));
    data.emplace_back(static_cast<uint8_t>((bin >> 16) & 0xFF));
    data.emplace_back(static_cast<uint8_t>((bin >> 8) & 0xFF));
    data.emplace_back(static_cast<uint8_t>(bin & 0xFF));
}

inline bool unpack_type(Cursor& cursor, float& value) {

    if (cursor.data() == float32) {
        cursor.next();

        uint32_t bin = 0;
        for (int i = 0; i < 4; ++i) {
            uint8_t byte = cursor.read();
            bin = (bin << 8) | byte;
        }

        static_assert(sizeof(float) == sizeof(uint32_t), "Float size mismatch");
        std::memcpy(&value, &bin, sizeof(float));

        return true;
    }

    return false;
}

inline void pack_type(std::vector<uint8_t>& data, const double& value) {

    data.emplace_back(float64);
    uint64_t bin = 0;
    static_assert(sizeof(double) == sizeof(uint64_t), "Double size mismatch");
    std::memcpy(&bin, &value, sizeof(double));

    for (int i = 7; i >= 0; --i) {
        data.emplace_back(static_cast<uint8_t>((bin >> (i * 8)) & 0xFF));
    }
}

inline bool unpack_type(Cursor& cursor, double& value) {

    if (float f32 = 0; unpack_type(cursor, f32)) {
        value = static_cast<double>(f32);
        return true;
    } else if (cursor.data() == float64) {
        cursor.next();

        uint64_t bin = 0;
        for (int i = 0; i < 8; ++i) {
            uint8_t byte = cursor.read();
            bin = (bin << 8) | byte;
        }

        static_assert(sizeof(double) == sizeof(uint64_t), "Double size mismatch");
        std::memcpy(&value, &bin, sizeof(double));

        return true;
    }

    return false;
}

// string
inline void pack_type(std::vector<uint8_t>& data, const std::string& value) {

    const size_t len = value.size();

    if (len <= 31) {
        data.emplace_back(static_cast<uint8_t>(fixstr | len));
    } else if (len <= 0xFF) {
        data.emplace_back(str8);
        data.emplace_back(static_cast<uint8_t>(len));
    }
    else if (len <= 0xFFFF) {
        data.emplace_back(str16);
        data.emplace_back(static_cast<uint8_t>((len >> 8) & 0xFF));
        data.emplace_back(static_cast<uint8_t>(len & 0xFF));
    }
    else if (len <= 0xFFFFFFFF) {
        data.emplace_back(str32);
        data.emplace_back(static_cast<uint8_t>((len >> 24) & 0xFF));
        data.emplace_back(static_cast<uint8_t>((len >> 16) & 0xFF));
        data.emplace_back(static_cast<uint8_t>((len >> 8) & 0xFF));
        data.emplace_back(static_cast<uint8_t>(len & 0xFF));
    } else {
        // too long for MessagePack
        return;
    }

    data.reserve(data.size() + len + 5);
    for (char i : value) {
        data.emplace_back(static_cast<uint8_t>(i));
    }
}

inline bool unpack_type(Cursor& cursor, std::string& value) {

    size_t len = 0;
    uint8_t marker = cursor.read();
    if ((marker & 0xE0) == fixstr) {
        len = marker & 0x1F;
    } else if (marker == str8) {
        len = cursor.read();;
    } else if (marker == str16) {
        uint16_t ui16 = 0;
        for (int i = 0; i < 2; ++i) {
            uint8_t byte = cursor.read();
            ui16 = (ui16 << 8) | byte;
        }
        len = ui16;
    } else if (marker == str32) {
        uint32_t ui32 = 0;
        for (int i = 0; i < 4; ++i) {
            uint8_t byte = cursor.read();
            ui32 = (ui32 << 8) | byte;
        }
        len = ui32;
    } else {
        // invalid string marker
        return false;
    }

    if (cursor.remain() < len) {
        cursor.error = true;
        return false;
    }

    value = std::string(cursor.pdata(), cursor.pdata() + len);
    cursor.next(len);

    return true;
}

// bin
inline void pack_type(std::vector<uint8_t>& data, const std::vector<uint8_t>& value) {

    const size_t len = value.size();

    if (len <= 0xFF) {
        data.emplace_back(bin8);
        data.emplace_back(static_cast<uint8_t>(len));
    }
    else if (len <= 0xFFFF) {
        data.emplace_back(bin16);
        data.emplace_back(static_cast<uint8_t>((len >> 8) & 0xFF));
        data.emplace_back(static_cast<uint8_t>(len & 0xFF));
    }
    else if (len <= 0xFFFFFFFF) {
        data.emplace_back(bin32);
        data.emplace_back(static_cast<uint8_t>((len >> 24) & 0xFF));
        data.emplace_back(static_cast<uint8_t>((len >> 16) & 0xFF));
        data.emplace_back(static_cast<uint8_t>((len >> 8) & 0xFF));
        data.emplace_back(static_cast<uint8_t>(len & 0xFF));
    } else {
        // too long for MessagePack
        return;
    }

    data.reserve(data.size() + len + 5);
    for (uint8_t i : value) {
        data.emplace_back(i);
    }
}

inline bool unpack_type(Cursor& cursor, std::vector<uint8_t>& value) {

    size_t len = 0;
    uint8_t marker = cursor.read();
    if (marker == bin8) {
        len = cursor.read();;
    } else if (marker == bin16) {
        uint16_t ui16 = 0;
        for (int i = 0; i < 2; ++i) {
            uint8_t byte = cursor.read();
            ui16 = (ui16 << 8) | byte;
        }
        len = ui16;
    } else if (marker == bin32) {
        uint32_t ui32 = 0;
        for (int i = 0; i < 4; ++i) {
            uint8_t byte = cursor.read();
            ui32 = (ui32 << 8) | byte;
        }
        len = ui32;
    } else {
        // invalid bin marker
        return false;
    }

    if (cursor.remain() < len) {
        cursor.error = true;
        return false;
    }

    value = std::vector<uint8_t>(cursor.pdata(), cursor.pdata() + len);
    cursor.next(len);

    return true;
}

// array
template<class T>
void pack_array(std::vector<uint8_t>& data, const T& array) {

    const size_t len = array.size();

    if (len < 16) {
        data.emplace_back(uint8_t(fixarray | len));
    } else if (len <= 0xFFFF) {
        data.emplace_back(array16);
        data.emplace_back(static_cast<uint8_t>((len >> 8) & 0xFF));
        data.emplace_back(static_cast<uint8_t>(len & 0xFF));
    } else if (len <= 0xFFFFFFFF) {
        data.emplace_back(array32);
        data.emplace_back(static_cast<uint8_t>((len >> 24) & 0xFF));
        data.emplace_back(static_cast<uint8_t>((len >> 16) & 0xFF));
        data.emplace_back(static_cast<uint8_t>((len >> 8) & 0xFF));
        data.emplace_back(static_cast<uint8_t>(len & 0xFF));
    } else {
        // too long for MessagePack
        return;
    }

    for (const auto& elem : array) {
        pack_type(data, elem);
    }
}

template<class T>
inline bool unpack_array(Cursor& cursor, T& array) {

    size_t len = 0;
    uint8_t marker = cursor.read();
    if ((marker & 0xF0) == fixarray) {
        len = marker & 0xF;
    } else if (marker == array16) {
        uint16_t ui16 = 0;
        for (int i = 0; i < 2; ++i) {
            uint8_t byte = cursor.read();
            ui16 = (ui16 << 8) | byte;
        }
        len = ui16;
    } else if (marker == array32) {
        uint32_t ui32 = 0;
        for (int i = 0; i < 4; ++i) {
            uint8_t byte = cursor.read();
            ui32 = (ui32 << 8) | byte;
        }
        len = ui32;
    } else {
        // invalid marker
        return false;
    }

    using ValueType = typename T::value_type;

    if constexpr (has_reserve<T>::value) {
        array.reserve(len);
    }

    for (size_t i = 0; i < len; ++i) {
        ValueType val{};
        bool ok = unpack_type(cursor, val);
        if (!ok) {
            return false;
        }

        if constexpr (has_emplace_back<T>::value) {
            array.emplace_back(std::move(val));
        } else {
            array.emplace(std::move(val));
        }
    }

    return true;
}

// map
template <typename T>
void pack_map(std::vector<uint8_t>& data, const T& map) {

    const size_t len = map.size();

    if (len < 16) {
        data.emplace_back(uint8_t(fixmap | len));
    } else if (len <= 0xFFFF) {
        data.emplace_back(map16);
        data.emplace_back(static_cast<uint8_t>((len >> 8) & 0xFF));
        data.emplace_back(static_cast<uint8_t>(len & 0xFF));
    } else if (len <= 0xFFFFFFFF) {
        data.emplace_back(map32);
        data.emplace_back(static_cast<uint8_t>((len >> 24) & 0xFF));
        data.emplace_back(static_cast<uint8_t>((len >> 16) & 0xFF));
        data.emplace_back(static_cast<uint8_t>((len >> 8) & 0xFF));
        data.emplace_back(static_cast<uint8_t>(len & 0xFF));
    } else {
        // too long for MessagePack
        return;
    }

    for (const auto &elem : map) {
        pack_type(data, std::get<0>(elem));
        pack_type(data, std::get<1>(elem));
    }
}

template<class T>
inline bool unpack_map(Cursor& cursor, T& map) {

    size_t len = 0;
    uint8_t marker = cursor.read();
    if ((marker & 0xF0) == fixmap) {
        len = marker & 0xF;
    } else if (marker == map16) {
        uint16_t ui16 = 0;
        for (int i = 0; i < 2; ++i) {
            uint8_t byte = cursor.read();
            ui16 = (ui16 << 8) | byte;
        }
        len = ui16;
    } else if (marker == map32) {
        uint32_t ui32 = 0;
        for (int i = 0; i < 4; ++i) {
            uint8_t byte = cursor.read();
            ui32 = (ui32 << 8) | byte;
        }
        len = ui32;
    } else {
        // invalid marker
        return false;
    }

    using KeyType = typename T::key_type;
    using MappedType = typename T::mapped_type;

    for (size_t i = 0; i < len; ++i) {
        KeyType key{};
        MappedType value{};
        bool ok = unpack_type(cursor, key);
        if (!ok) {
            return false;
        }

        ok = unpack_type(cursor, value);
        if (!ok) {
            return false;
        }

        map.insert_or_assign(key, value);
    }

    return true;
}

// pack by packer
template <typename T>
void pack_packer(std::vector<uint8_t>& data, const T& value);
template <typename T>
void pack_const_packer(std::vector<uint8_t>& data, const T& value);
template <typename T>
void pack_custom_packer(std::vector<uint8_t>& data, const T& value);

// unpack by unpacker
template <typename T>
bool pack_unpacker(Cursor& cursor, T& value);
template <typename T>
bool unpack_unpacker(Cursor& cursor, T& value);
template <typename T>
bool unpack_custom_unpacker(Cursor& cursor, T& value);

template<class T>
void pack_type(std::vector<uint8_t>& data, const T& value) {

    if constexpr(is_map<T>::value) {
        pack_map(data, value);
    } else if constexpr(is_container<T>::value) {
        pack_array(data, value);
    } else {
        // custom
        if constexpr(has_pack_data<T>::value && !has_pack_const_packer<T>::value) {
            value.pack(data);
        }  else if constexpr(has_pack_const_packer<T>::value) {
            pack_const_packer(data, value);
        } else if constexpr(has_pack_packer<T>::value) {
            pack_packer(data, value);
        } else if constexpr (has_func_pack_custom_packer<T>::value) {
            pack_custom_packer(data, value);
        } else {
            pack_custom(data, value);
        }
    }
}

template<class T>
bool unpack_type(Cursor& cursor, T& value) {

    if constexpr(is_map<T>::value) {
        return unpack_map(cursor, value);
    } else if constexpr(is_container<T>::value) {
        return unpack_array(cursor, value);
    } else {
        // custom
        if constexpr(has_unpack_cursor<T>::value && !has_unpack_unpacker<T>::value) {
            value.unpack(cursor);
        } else if constexpr(has_unpack_unpacker<T>::value) {
            return unpack_unpacker(cursor, value);
        } else if constexpr(has_pack_unpacker<T>::value) {
            return pack_unpacker(cursor, value);
        } else if constexpr (has_func_unpack_custom_unpacker<T>::value) {
            return unpack_custom_unpacker(cursor, value);
        } else {
            return unpack_custom(cursor, value);
        }
    }
}

class Packer
{
public:

    Packer(std::vector<uint8_t>& d)
        : m_ref(d) {}

    Packer()
        : m_ref(m_data) {}

    template<class ... Types>
    static void pack(std::vector<uint8_t>& data, const Types &... args) {
        do_pack(data, args...);
    }

    template<class ... Types>
    void operator()(const Types &... args) {
        do_pack(m_ref, args...);
    }

    template<class ... Types>
    void process(const Types &... args) {
        do_pack(m_ref, args...);
    }

    const std::vector<uint8_t>& data() const {
        return m_ref;
    }

private:
    template<class ... Types>
    static void do_pack(std::vector<uint8_t>& data, const Types &... args) {
        (pack_type(data, std::forward<const Types &>(args)), ...);
    }

    std::vector<uint8_t> m_data;
    std::vector<uint8_t>& m_ref;
};

class UnPacker
{
public:

    UnPacker(Cursor& cursor)
        : m_cursor(cursor) {}

    UnPacker(const uint8_t* start, std::size_t size)
        : m_cursor(Cursor(start, size)) {}

    UnPacker(const std::vector<uint8_t>& data)
        : m_cursor(Cursor(&data[0], data.size())) {}

    template<class ... Types>
    static bool unpack(Cursor& c, Types&... args) {
        return do_unpack(c, args...);
    }

    template<class ... Types>
    static bool unpack(const uint8_t* start, std::size_t size, Types&... args) {
        Cursor c(start, size);
        return do_unpack(c, args...);
    }

    template<class ... Types>
    static bool unpack(const std::vector<uint8_t>& data, Types&... args) {
        Cursor c(&data[0], data.size());
        return do_unpack(c, args...);
    }

    template<class ... Types>
    void operator()(Types&... args) {
        m_success = do_unpack(m_cursor, args...);
    }

    template<class ... Types>
    bool process(Types&... args) {
        m_success = do_unpack(m_cursor, args...);
        return success();
    }

    bool success() const { return m_success; }

private:
    template<class ... Types>
    static bool do_unpack(Cursor& c, Types&... args) {
        bool ok = (unpack_type(c, std::forward<Types&>(args)), ...);
        return ok && !c.error;
    }

    bool m_success = true;
    Cursor m_cursor;
};

template <typename T>
inline void pack_packer(std::vector<uint8_t>& data, const T& value)
{
    Packer p(data);
    const_cast<T&>(value).pack(p);
}

template <typename T>
inline void pack_const_packer(std::vector<uint8_t>& data, const T& value)
{
    Packer p(data);
    value.pack(p);
}

template <typename T>
inline void pack_custom_packer(std::vector<uint8_t>& data, const T& value)
{
    Packer p(data);
    pack_custom(p, value);
}

template <typename T>
inline bool pack_unpacker(Cursor& cursor, T& value)
{
    UnPacker u(cursor);
    value.pack(u);
    return u.success();
}

template <typename T>
inline bool unpack_unpacker(Cursor& cursor, T& value)
{
    UnPacker u(cursor);
    value.unpack(u);
    return u.success();
}

template <typename T>
inline bool unpack_custom_unpacker(Cursor& cursor, T& value)
{
    UnPacker u(cursor);
    unpack_custom(u, value);
    return u.success();
}
}
