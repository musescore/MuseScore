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
#include <vector>
#include <string>
#include <string_view>

namespace mu {
// ============================
// AsciiChar
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
// Char (utf16)
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
// String (utf16)
// ============================
class String
{
public:

    static const size_t npos = static_cast<size_t>(-1);

    String();

    String(const char* str) { *this = fromUtf8(str); }
    String& operator=(const char* str) { *this = fromUtf8(str); return *this; }
    static String fromUtf8(const char* str);

    static String fromStdString(const std::string& str);
    std::string toStdString() const;

//#ifndef NO_QT_SUPPORT
    String(const QString& str) { *this = fromQString(str); }
    String& operator=(const QString& str) { *this = fromQString(str); return *this; }
    static String fromQString(const QString& str);
    QString toQString() const;
//#endif

    size_t size() const;
    bool empty() const;
    Char at(size_t i) const;

private:
    std::shared_ptr<std::u16string> m_data;
};

// ============================
// AsciiString (ASCII)
// ============================
class AsciiString
{
public:

    static const size_t npos = static_cast<size_t>(-1);

    AsciiString() = default;
    AsciiString(const char* str)
        : m_size(str ? strlen(str) : 0), m_data(str) {}

//#ifndef NO_QT_SUPPORT
    AsciiString(const QLatin1String& str)
        : m_size(str.size()), m_data(str.latin1()) {}

    static AsciiString fromQLatin1String(const QLatin1String& str) { return AsciiString(str); }
    QLatin1String toQString() const { return QLatin1String(m_data, m_size); }
//#endif

    size_t size() const;
    bool empty() const;
    AsciiChar at(size_t i) const;

private:
    size_t m_size = 0;
    const char* m_data = nullptr;
};
}

#endif // MU_GLOBAL_STRING_H
