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
#ifndef MUSE_GLOBAL_URI_H
#define MUSE_GLOBAL_URI_H

#include <string>
#include <vector>
#include <map>

#include "val.h"
#include "global/logstream.h"

namespace muse {
class Uri
{
public:
    Uri() = default;
    explicit Uri(const std::string& str);
    explicit Uri(const muse::String& str);

    using Scheme = std::string;
    static const Scheme MuseScore;
    static const Scheme Http;
    static const Scheme Https;

    bool isValid() const;

    Scheme scheme() const;
    void setScheme(const Scheme& scheme);

    std::string path() const;

    inline bool operator==(const Uri& uri) const { return m_path == uri.m_path && m_scheme == uri.m_scheme; }
    inline bool operator!=(const Uri& uri) const { return !(*this == uri); }
    inline bool operator <(const Uri& uri) const
    {
        if (m_scheme != uri.m_scheme) {
            return m_scheme < uri.m_scheme;
        }
        return m_path < uri.m_path;
    }

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
    explicit UriQuery(const String& str);
    explicit UriQuery(const Uri& uri);

    bool isValid() const;
    const Uri& uri() const;
    void setScheme(const Uri::Scheme& scheme);

    const Params& params() const;
    Val param(const std::string& key, const Val& def = Val()) const;
    void addParam(const std::string& key, const Val& val);
    UriQuery addingParam(const std::string& key, const Val& val) const;
    bool contains(const std::string& key) const;

    std::string toString() const;

    bool operator==(const UriQuery& query) const;
    bool operator!=(const UriQuery& query) const;

private:

    void parseParams(const std::string& str, Params& out) const;
    void extractQuotedStrings(const std::string& str, std::vector<std::string>& out) const;

    Uri m_uri;
    Params m_params;
};
}

inline muse::logger::Stream& operator<<(muse::logger::Stream& s, const muse::Uri& uri)
{
    s << uri.toString();
    return s;
}

inline muse::logger::Stream& operator<<(muse::logger::Stream& s, const muse::UriQuery& q)
{
    s << q.toString();
    return s;
}

#endif // MUSE_GLOBAL_URI_H
