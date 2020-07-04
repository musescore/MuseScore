//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2020 MuseScore BVBA and others
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
//=============================================================================
#include "stringutils.h"

#include <cctype>
#include <algorithm>

bool mu::strings::replace(std::string& str, const std::string& from, const std::string& to)
{
    size_t start_pos = str.find(from);
    if (start_pos == std::string::npos) {
        return false;
    }
    str.replace(start_pos, from.length(), to);
    return true;
}

void mu::strings::split(const std::string& str, std::vector<std::string>& out, const std::string& delim)
{
    std::size_t current, previous = 0;
    current = str.find(delim);
    while (current != std::string::npos) {
        out.push_back(str.substr(previous, current - previous));
        previous = current + 1;
        current = str.find(delim, previous);
    }
    out.push_back(str.substr(previous, current - previous));
}

void mu::strings::ltrim(std::string& s)
{
    s.erase(s.begin(), std::find_if(s.begin(), s.end(), [](unsigned char ch) {
        return !std::isspace(ch);
    }));
}

void mu::strings::rtrim(std::string& s)
{
    s.erase(std::find_if(s.rbegin(), s.rend(), [](int ch) {
        return !std::isspace(ch);
    }).base(), s.end());
}

void mu::strings::trim(std::string& s)
{
    ltrim(s);
    rtrim(s);
}

std::string mu::strings::toLower(const std::string& source)
{
    std::string str = source;
    std::for_each(str.begin(), str.end(), [](char& c) {
        c = ::tolower(c);
    });
    return str;
}

bool mu::strings::endsWith(const std::string& str, const std::string& ending)
{
    if (ending.size() > str.size()) {
        return false;
    }
    std::string ss = str.substr(str.size() - ending.size());
    return ss.compare(ending.c_str()) == 0;
}
