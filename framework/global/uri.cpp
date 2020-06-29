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
#include "uri.h"

#include "log.h"
#include "stringutils.h"

using namespace mu;

const Uri::Scheme Uri::MuseScore("musescore");
const Uri::Scheme Uri::Http("http");
const Uri::Scheme Uri::Https("https");

// musescore://module/target/name

Uri::Uri(const std::string& str)
{
    auto schemePos = str.find(':');
    if (schemePos != std::string::npos) {
        m_scheme = str.substr(0, schemePos);
    }

    auto paramsPos = str.find('?');
    auto pathPos= (schemePos != std::string::npos) ? (schemePos + 3) : 0;
    int pathN = (paramsPos != std::string::npos) ? (paramsPos - pathPos) : std::string::npos;

    m_path = str.substr(pathPos, pathN);
}

bool Uri::isValid() const
{
    if (m_scheme.empty()) {
        return false;
    }

    if (m_path.empty()) {
        return false;
    }

    return true;
}

Uri::Scheme Uri::scheme() const
{
    return m_scheme;
}

std::string Uri::path() const
{
    return m_path;
}

std::string Uri::toString() const
{
    return m_scheme + "://" + m_path;
}

// musescore://module/target/name?param1=value1&paramn=valuen

UriQuery::UriQuery(const std::string& str)
    : m_uri(str)
{
    parceParams(str, m_params);
}

void UriQuery::parceParams(const std::string& uri, Params& out) const
{
    auto paramsPos = uri.find('?');
    if (paramsPos == std::string::npos) {
        return;
    }

    std::string paramsStr = uri.substr(paramsPos + 1);

    strings::trim(paramsStr);

    std::vector<std::string> paramsPairs;
    strings::split(paramsStr, paramsPairs, "&");

    for (const std::string& pair : paramsPairs) {
        std::vector<std::string> param;
        strings::split(pair, param, "=");
        if (param.size() != 2) {
            LOGE() << "Invalid param:" << pair << ", in uri:" << uri;
            continue;
        }
        std::string key = param.at(0);
        strings::trim(key);

        std::string val = param.at(1);
        strings::trim(val);

        out[key] = Val(val);
    }
}

std::string UriQuery::toString() const
{
    std::string str = m_uri.toString();
    if (!m_params.empty()) {
        str += "?";
        for (auto it = m_params.cbegin(); it != m_params.cend(); ++it) {
            str += it->first + "=" + it->second.toString() + "&";
        }

        str.erase(str.size() - 2);
    }
    return str;
}

const Uri& UriQuery::uri() const
{
    return m_uri;
}

bool UriQuery::isValid() const
{
    return m_uri.isValid();
}

const UriQuery::Params& UriQuery::params() const
{
    return m_params;
}

mu::Val UriQuery::param(const std::string& key, const Val& def) const
{
    auto it = m_params.find(key);
    if (it == m_params.end()) {
        return def;
    }
    return it->second;
}

void UriQuery::addParam(const std::string& key, const Val& val)
{
    m_params[key] = val;
}
