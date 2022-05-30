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
struct Latin1Char
{
public:
    explicit Latin1Char(char c)
        : ch(c) {}
    inline char toLatin1() const noexcept { return ch; }
    inline char16_t unicode() const noexcept { return char16_t(ch); }

private:
    char ch;
};

class Char
{
public:

    Char() = default;
    Char(char16_t c)
        : ch(c) {}
    Char(Latin1Char c)
        : ch(c.unicode()) {}

    inline bool operator ==(Char c) const { return ch == c.ch; }
    inline bool operator ==(char16_t c) const { return ch == c; }
    inline bool operator ==(Latin1Char c) const { return ch == c.unicode(); }

private:
    char16_t ch = 0;
};

class UtfCodec
{
public:
    static void utf8to16(std::string_view src, std::u16string& dst);
    static void utf16to8(std::u16string_view src, std::string& dst);
};

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
}

#endif // MU_GLOBAL_STRING_H
