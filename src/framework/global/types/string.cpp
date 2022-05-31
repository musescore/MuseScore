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

#include "global/thirdparty/utfcpp-3.2.1/utf8.h"

#include "log.h"

using namespace mu;

// ============================
// Char
// ============================

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

String String::fromUtf8(const char* str)
{
    String s;
    UtfCodec::utf8to16(std::string_view(str), *s.m_data.get());
    return s;
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
