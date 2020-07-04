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
#ifndef MU_FRAMEWORK_STRINGUTILS_H
#define MU_FRAMEWORK_STRINGUTILS_H

#include <string>
#include <vector>
#include <sstream>

namespace mu {
namespace strings {
bool replace(std::string& source, const std::string& what, const std::string& to);
void split(const std::string& str, std::vector<std::string>& out, const std::string& delim);

void ltrim(std::string& s);
void rtrim(std::string& s);
void trim(std::string& s);

std::string toLower(const std::string& source);
bool endsWith(const std::string& str, const std::string& end);

// Locale-independent version of std::to_string
template<typename T>
std::string toString(const T& t)
{
    std::ostringstream oss;
    oss.imbue(std::locale::classic());
    oss << t;
    return oss.str();
}
}
}

#endif // MU_FRAMEWORK_STRINGUTILS_H
