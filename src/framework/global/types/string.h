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
#ifndef MU_GLOBAL_STRING_H
#define MU_GLOBAL_STRING_H

#include <memory>
#include <cstring>
#include <vector>
#include <string>
#include <string_view>

#include "bytearray.h"
#include "global/logstream.h"

namespace mu {
// ============================
// AsciiChar (ASCII)
// ============================
struct AsciiChar
{
public:
    AsciiChar() = default;
    explicit AsciiChar(char c)
        : ch(c) {}

    inline char ascii() const noexcept { return ch; }
    inline char16_t unicode() const noexcept { return char16_t(ch); }

private:
    char ch = 0;
};

// ============================
// Char (UTF-16)
// ============================
class Char
{
public:

    Char() = default;
    Char(char16_t c)
        : ch(c) {}
    Char(AsciiChar c)
        : ch(c.unicode()) {}

    inline bool operator ==(Char c) const { return ch == c.ch; }
    inline bool operator ==(char16_t c) const { return ch == c; }
    inline bool operator ==(AsciiChar c) const { return ch == c.unicode(); }

private:
    char16_t ch = 0;
};

// ============================
// UtfCodec
// ============================
class UtfCodec
{
public:
    static void utf8to16(std::string_view src, std::u16string& dst);
    static void utf16to8(std::u16string_view src, std::string& dst);
};

// ============================
// String (UTF-16)
// ============================
class String
{
public:

    static const size_t npos = static_cast<size_t>(-1);

    String();

    String(const char16_t* str);
    String& operator=(const char16_t* str);

    bool operator ==(const String& s) const { return *m_data.get() == *s.m_data.get(); }
    bool operator !=(const String& s) const { return !operator ==(s); }

    size_t size() const;
    bool empty() const;
    Char at(size_t i) const;

    static String fromUtf8(const char* str);
    ByteArray toUtf8() const;
    ByteArray toAscii(bool* ok = nullptr) const;

    static String fromStdString(const std::string& str);
    std::string toStdString() const;

//#ifndef NO_QT_SUPPORT
    String(const QString& str) { *this = fromQString(str); }
    String& operator=(const QString& str) { *this = fromQString(str); return *this; }
    static String fromQString(const QString& str);
    QString toQString() const;
//#endif

private:
    std::shared_ptr<std::u16string> m_data;
};

// ============================
// AsciiStringView (ASCII)
// Be carefully!!, this class just hold pointer to string (no copy), so source string should be present, while use view
// ============================
class AsciiStringView
{
public:

    static const size_t npos = static_cast<size_t>(-1);

    constexpr AsciiStringView() = default;
    constexpr AsciiStringView(const char* str)
        : m_size(str ? std::char_traits<char>::length(str) : 0), m_data(str) {}
    constexpr AsciiStringView(const char* str, size_t size)
        : m_size(size), m_data(str) {}

//#ifndef NO_QT_SUPPORT
    static AsciiStringView fromQLatin1String(const QLatin1String& str) { return AsciiStringView(str.latin1(), str.size()); }
    QLatin1String toQLatin1String() const { return QLatin1String(m_data, static_cast<int>(m_size)); }
//#endif

    inline bool operator ==(const AsciiStringView& s) const { return m_size == s.m_size && std::memcmp(m_data, s.m_data, m_size) == 0; }
    inline bool operator !=(const AsciiStringView& s) const { return !this->operator ==(s); }
    inline bool operator ==(const char* s) const
    {
        size_t sz = (s ? std::char_traits<char>::length(s) : 0);
        return m_size == sz && (s ? std::memcmp(m_data, s, m_size) == 0 : true);
    }

    inline bool operator !=(const char* s) const { return !this->operator ==(s); }

    inline bool operator <(const AsciiStringView& s) const
    {
        if (m_size != s.m_size) {
            return m_size < s.m_size;
        }
        return std::memcmp(m_data, s.m_data, m_size) < 0;
    }

    const char* ascii() const;
    size_t size() const;
    bool empty() const;
    AsciiChar at(size_t i) const;
    bool contains(char ch) const;

    int toInt(bool* ok = nullptr) const;
    double toDouble(bool* ok = nullptr) const;

private:
    size_t m_size = 0;
    const char* m_data = nullptr;
};
}

// ============================
// AsciiStringView (ASCII)
// ============================
inline bool operator ==(const char* s1, const mu::AsciiStringView& s2) { return s2 == s1; }
inline bool operator !=(const char* s1, const mu::AsciiStringView& s2) { return s2 != s1; }

inline mu::logger::Stream& operator<<(mu::logger::Stream& s, const mu::AsciiStringView& str)
{
    s << str.ascii();
    return s;
}

#endif // MU_GLOBAL_STRING_H
