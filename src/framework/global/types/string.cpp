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

// Helpers

static int toInt_helper(const char* str, bool* ok, int base)
{
    if (!str) {
        return 0;
    }
    const char* currentLoc = setlocale(LC_NUMERIC, "C");
    char* end = nullptr;
    long int v = static_cast<int>(std::strtol(str, &end, base));
    setlocale(LC_NUMERIC, currentLoc);
    if (ok) {
        size_t sz = std::strlen(end);
        *ok = sz != std::strlen(str);
    }
    return v;
}

static double toDouble_helper(const char* str, bool* ok)
{
    if (!str) {
        return 0.0;
    }
    const char* currentLoc = setlocale(LC_NUMERIC, "C");
    char* end = nullptr;
    double v = std::strtod(str, &end);
    setlocale(LC_NUMERIC, currentLoc);
    if (ok) {
        size_t sz = std::strlen(end);
        *ok = sz != std::strlen(str);
    }
    return v;
}

static void ltrim_helper(std::u16string& s)
{
    s.erase(s.begin(), std::find_if(s.begin(), s.end(), [](char16_t ch) {
        return !std::isspace(ch);
    }));
}

static void rtrim_helper(std::u16string& s)
{
    s.erase(std::find_if(s.rbegin(), s.rend(), [](char16_t ch) {
        return !std::isspace(ch);
    }).base(), s.end());
}

static void trim_helper(std::u16string& s)
{
    ltrim_helper(s);
    rtrim_helper(s);
}

// ============================
// Char
// ============================
char Char::toAscii(char16_t c, bool* ok)
{
    if (c > 0xff) {
        if (ok) {
            *ok = false;
        }
        return '?';
    } else {
        if (ok) {
            *ok = true;
        }
        return static_cast<char>(c);
    }
}

char16_t Char::fromAscii(char c)
{
    return static_cast<char16_t>(c);
}

char Char::toAscii(bool* ok) const
{
    return toAscii(m_ch);
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

String::String(const Char& ch)
{
    m_data = std::make_shared<std::u16string>();
    *m_data.get() += ch.unicode();
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

void String::reserve(size_t i)
{
    mutStr().reserve(i);
}

bool String::operator ==(const AsciiStringView& s) const
{
    if (size() != s.size()) {
        return false;
    }

    for (size_t i = 0; i < s.size(); ++i) {
        if (at(i).toAscii() != s.at(i).ascii()) {
            return false;
        }
    }
    return true;
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

String& String::operator +=(const char16_t* s)
{
    mutStr() += s;
    return *this;
}

String& String::append(Char ch)
{
    mutStr() += ch.unicode();
    return *this;
}

String& String::append(const String& s)
{
    mutStr() += s.constStr();
    return *this;
}

String& String::prepend(Char ch)
{
    mutStr() = ch.unicode() + constStr();
    return *this;
}

String& String::prepend(const String& s)
{
    mutStr() = s.constStr() + constStr();
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

String String::fromAscii(const char* str)
{
    size_t sz = std::strlen(str);
    String s;
    s.m_data->resize(sz);
    std::u16string& data = *s.m_data.get();
    for (size_t i = 0; i < sz; ++i) {
        data[i] = Char::fromAscii(str[i]);
    }
    return s;
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
        char ch = Char::toAscii(m_data->at(i), &cok);
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

void String::clear()
{
    m_data->clear();
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
        if (Char(m_data->at(i)).toAscii() == str.at(i).ascii()) {
            continue;
        }

        if (cs == CaseInsensitive) {
            if (AsciiChar::toLower(Char(m_data->at(i)).toAscii()) == AsciiChar::toLower(str.at(i).ascii())) {
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
        if (Char(m_data->at(start + i)).toAscii() == str.at(i).ascii()) {
            continue;
        }

        if (cs == CaseInsensitive) {
            if (AsciiChar::toLower(Char(m_data->at(start + i)).toAscii()) == AsciiChar::toLower(str.at(i).ascii())) {
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

String String::trimmed() const
{
    String s = *this;
    trim_helper(s.mutStr());
    return s;
}

String String::toXmlEscaped(char16_t c)
{
    switch (c) {
    case u'<':
        return String(u"&lt;");
    case u'>':
        return String(u"&gt;");
    case u'&':
        return String(u"&amp;");
    case u'\"':
        return String(u"&quot;");
    default:
        // ignore invalid characters in xml 1.0
        if ((c < 0x0020 && c != 0x0009 && c != 0x000A && c != 0x000D)) {
            return String();
        }
        return String(Char(c));
    }
}

String String::toXmlEscaped(const String& s)
{
    String escaped;
    escaped.reserve(s.size());
    for (size_t i = 0; i < s.size(); ++i) {
        char16_t c = s.at(i).unicode();
        escaped += toXmlEscaped(c);
    }
    return escaped;
}

String String::toXmlEscaped() const
{
    return toXmlEscaped(*this);
}

int String::toInt(bool* ok, int base) const
{
    ByteArray ba = toUtf8();
    return toInt_helper(ba.constChar(), ok, base);
}

double String::toDouble(bool* ok) const
{
    ByteArray ba = toUtf8();
    return toDouble_helper(ba.constChar(), ok);
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

int AsciiStringView::toInt(bool* ok, int base) const
{
    return toInt_helper(m_data, ok, base);
}

double AsciiStringView::toDouble(bool* ok) const
{
    return toDouble_helper(m_data, ok);
}
