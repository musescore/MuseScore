/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-CLA-applies
 *
 * MuseScore
 * Music Composition & Notation
 *
 * Copyright (C) 2021 MuseScore Limited and others
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

#include <locale>
#include <sstream>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

#include "types/string.h"

namespace muse::strings {
bool replace(std::string& source, const std::string& what, const std::string& to);

void split(const std::string& str, std::vector<std::string>& out, const std::string& delim);

template<typename CharT, typename Traits, typename Allocator, typename OutIt, typename StringViewLike,
         typename = decltype(*std::declval<OutIt>(), ++std::declval<OutIt>())> // restrict this overload to iterators only
void split(const std::basic_string<CharT, Traits, Allocator>& str,
           OutIt out, const StringViewLike& delim)
{
    const std::basic_string_view<CharT, Traits> delimStr = delim;
    const std::size_t delimLen = delimStr.length();
    const std::size_t extra = delimLen == 0 ? 1 : 0;

    std::size_t begin = 0;
    std::size_t end = str.find(delimStr);
    while (end != std::basic_string<CharT, Traits, Allocator>::npos) {
        *out++ = str.substr(begin, end - begin);

        begin = end + delimLen;
        end = str.find(delimStr, begin + extra);
    }

    *out++ = str.substr(begin);
}

std::string join(const std::vector<std::string>& strs, const std::string& sep = ",");

void ltrim(std::string& s);
void rtrim(std::string& s);
void trim(std::string& s);

std::string toLower(const std::string& source);
bool startsWith(const std::string& str, const std::string& start);
bool endsWith(const std::string& str, const std::string& end);
std::string leftJustified(const std::string& val, size_t width);

// Locale-independent version of std::to_string
template<typename T>
std::string toString(const T& t)
{
    std::ostringstream oss;
    oss.imbue(std::locale::classic());
    oss << t;
    return oss.str();
}

bool lessThanCaseInsensitive(const std::string& lhs, const std::string& rhs);
bool lessThanCaseInsensitive(const String& lhs, const String& rhs);

size_t levenshteinDistance(const std::string& s1, const std::string& s2);
}
