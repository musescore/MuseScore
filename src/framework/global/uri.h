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
#ifndef MU_FRAMEWORK_URI_H
#define MU_FRAMEWORK_URI_H

#include <string>
#include <vector>
#include <map>

#include "val.h"

namespace mu {
class Uri
{
public:
    Uri() = default;
    explicit Uri(const std::string& str);

    using Scheme = std::string;
    static const Scheme MuseScore;
    static const Scheme Http;
    static const Scheme Https;

    bool isValid() const;

    Scheme scheme() const;
    std::string path() const;

    inline bool operator==(const Uri& uri) const { return m_path == uri.m_path && m_scheme == uri.m_scheme; }
    inline bool operator!=(const Uri& uri) const { return !(*this == uri); }

    std::string toString() const;

private:

    Scheme m_scheme;
    std::string m_path;
};

class UriQuery
{
public:

    using Params = std::map<std::string /*key*/, Val>;

    UriQuery() = default;
    explicit UriQuery(const std::string& str);

    const Uri& uri() const;
    bool isValid() const;

    const Params& params() const;
    Val param(const std::string& key, const Val& def = Val()) const;
    void addParam(const std::string& key, const Val& val);

    std::string toString() const;

private:

    void parceParams(const std::string& str, Params& out) const;
    void extractQuotedStrings(const std::string& str, std::vector<std::string>& out) const;

    Uri m_uri;
    Params m_params;
};
}

#endif // MU_FRAMEWORK_URI_H
