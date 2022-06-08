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
#include "string.h"
#include <cstring>
#include <cstdlib>
#include <locale>
#include <cctype>

#include "global/thirdparty/utfcpp-3.2.1/utf8.h"

#include "log.h"

using namespace mu;

// ============================
// Char
// ============================
char Char::ascii(bool* ok) const
{
    if (m_ch > 0xff) {
        if (ok) {
            *ok = false;
        }
        return '?';
    } else {
        if (ok) {
            *ok = true;
        }
        return static_cast<char>(m_ch);
    }
}

// ============================
// UtfCodec
// ============================
void UtfCodec::utf8to16(std::string_view src, std::u16string& dst)
{
    utf8::utf8to16(src.begin(), src.end(), std::back_inserter(dst));
}

void UtfCodec::utf16to8(std::u16string_view src, std::string& dst)
{
    utf8::utf16to8(src.begin(), src.end(), std::back_inserter(dst));
}

// ============================
// String
// ============================

String::String()
{
    m_data = std::make_shared<std::u16string>();
}

String::String(const char16_t* str)
{
    m_data = std::make_shared<std::u16string>(str);
}

const std::u16string& String::constStr() const
{
    return *m_data.get();
}

std::u16string& String::mutStr()
{
    detach();
    return *m_data.get();
}

void String::detach()
{
    if (!m_data) {
        return;
    }

    if (m_data.use_count() == 1) {
        return;
    }

    m_data = std::make_shared<std::u16string>(*m_data);
}

String& String::operator=(const char16_t* str)
{
    m_data = std::make_shared<std::u16string>(str);
    return *this;
}

String& String::operator +=(const String& s)
{
    mutStr() += s.constStr();
    return *this;
}

String& String::operator +=(const char16_t* s)
{
    mutStr() += s;
    return *this;
}

String String::fromUtf8(const char* str)
{
    String s;
    UtfCodec::utf8to16(std::string_view(str), *s.m_data.get());
    return s;
}

ByteArray String::toUtf8() const
{
    std::string str;
    UtfCodec::utf16to8(std::u16string_view(*m_data.get()), str);
    return ByteArray(reinterpret_cast<const uint8_t*>(str.c_str()), str.size());
}

ByteArray String::toAscii(bool* ok) const
{
    ByteArray ba;
    ba.resize(size());

    if (ok) {
        *ok = true;
    }

    for (size_t i = 0; i < size(); ++i) {
        bool cok = false;
        char ch = Char(m_data->at(i)).ascii(&cok);
        if (!cok && ok) {
            *ok = false;
        }
        ba[i] = static_cast<uint8_t>(ch);
    }
    return ba;
}

String String::fromStdString(const std::string& str)
{
    String s;
    UtfCodec::utf8to16(std::string_view(str), *s.m_data.get());
    return s;
}

std::string String::toStdString() const
{
    std::string s;
    UtfCodec::utf16to8(std::u16string_view(*m_data.get()), s);
    return s;
}

String String::fromQString(const QString& str)
{
    const QChar* qu = str.unicode();
    static_assert(sizeof(QChar) == sizeof(char16_t));
    const char16_t* u = reinterpret_cast<const char16_t*>(qu);

    String s;
    s.m_data.reset(new std::u16string(u, u + str.size()));
    return s;
}

QString String::toQString() const
{
    const char16_t* u = &m_data->front();
    static_assert(sizeof(QChar) == sizeof(char16_t));
    return QString(reinterpret_cast<const QChar*>(u), static_cast<int>(size()));
}

size_t String::size() const
{
    return m_data->size();
}

bool String::empty() const
{
    return m_data->empty();
}

Char String::at(size_t i) const
{
    IF_ASSERT_FAILED(i < size()) {
        return Char();
    }
    return Char(m_data->at(i));
}

bool String::contains(const Char& ch) const
{
    return constStr().find(ch.unicode()) != std::u16string::npos;
}

bool String::startsWith(const AsciiStringView& str, CaseSensitivity cs) const
{
    if (str.size() > size()) {
        return false;
    }

    for (size_t i = 0; i < str.size(); ++i) {
        if (Char(m_data->at(i)).ascii() == str.at(i).ascii()) {
            continue;
        }

        if (cs == CaseInsensitive) {
            if (AsciiChar::toLower(Char(m_data->at(i)).ascii()) == AsciiChar::toLower(str.at(i).ascii())) {
                continue;
            }
        }

        return false;
    }

    return true;
}

bool String::endsWith(const AsciiStringView& str, CaseSensitivity cs) const
{
    if (str.size() > size()) {
        return false;
    }

    size_t start = size() - str.size();
    for (size_t i = 0; i < str.size(); ++i) {
        if (Char(m_data->at(start + i)).ascii() == str.at(i).ascii()) {
            continue;
        }

        if (cs == CaseInsensitive) {
            if (AsciiChar::toLower(Char(m_data->at(start + i)).ascii()) == AsciiChar::toLower(str.at(i).ascii())) {
                continue;
            }
        }

        return false;
    }

    return true;
}

StringList String::split(const Char& ch) const
{
    StringList out;
    std::size_t current, previous = 0;
    current = constStr().find(ch.unicode());
    while (current != std::string::npos) {
        String sub = mid(previous, current - previous);
        out.push_back(std::move(sub));
        previous = current + 1;
        current = constStr().find(ch.unicode(), previous);
    }
    String sub = mid(previous, current - previous);
    out.push_back(std::move(sub));

    return out;
}

String& String::replace(const String& before, const String& after)
{
    size_t start_pos = 0;
    while ((start_pos = m_data->find(before.constStr(), start_pos)) != std::string::npos) {
        m_data->replace(start_pos, before.size(), after.constStr());
        start_pos += after.size(); // Handles case where 'after' is a substring of 'before'
    }
    return *this;
}

String String::mid(size_t pos, size_t count) const
{
    String s;
    s.mutStr() = constStr().substr(pos, count);
    return s;
}

// ============================
// AsciiStringView
// ============================

char AsciiChar::toLower(char ch)
{
    return static_cast<char>(std::tolower(static_cast<unsigned char>(ch)));
}

char AsciiChar::toUpper(char ch)
{
    return static_cast<char>(std::toupper(static_cast<unsigned char>(ch)));
}

// ============================
// AsciiStringView
// ============================
const char* AsciiStringView::ascii() const
{
    return m_data;
}

size_t AsciiStringView::size() const
{
    return m_size;
}

bool AsciiStringView::empty() const
{
    return m_size == 0;
}

AsciiChar AsciiStringView::at(size_t i) const
{
    IF_ASSERT_FAILED(i < size()) {
        return AsciiChar();
    }
    return AsciiChar(m_data[i]);
}

bool AsciiStringView::contains(char ch) const
{
    for (size_t i = 0; i < m_size; ++i) {
        if (m_data[i] == ch) {
            return true;
        }
    }
    return false;
}

int AsciiStringView::toInt(bool* ok) const
{
    const char* currentLoc = setlocale(LC_NUMERIC, "C");
    char* end = nullptr;
    long int v = static_cast<int>(std::strtol(m_data, &end, 10));
    setlocale(LC_NUMERIC, currentLoc);
    if (ok) {
        size_t sz = std::strlen(end);
        *ok = sz != m_size;
    }
    return v;
}

double AsciiStringView::toDouble(bool* ok) const
{
    const char* currentLoc = setlocale(LC_NUMERIC, "C");
    char* end = nullptr;
    double v = std::strtod(m_data, &end);
    setlocale(LC_NUMERIC, currentLoc);
    if (ok) {
        size_t sz = std::strlen(end);
        *ok = sz != m_size;
    }
    return v;
}
