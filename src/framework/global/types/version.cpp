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
#include "version.h"

#include <algorithm>
#include <array>
#include <utility>
#include <vector>

#include "log.h"

static const muse::Char SUFFIX_DELIMITER = '-';

using namespace muse;

static std::array<int, 3> parseVersion(const muse::String& versionString, bool& ok)
{
    std::array<int, 3> result { 0, 0, 0 };

    if (versionString.isEmpty()) {
        ok = false;
        return result;
    }

    size_t componentIdx = 0;
    int curNum = 0;

    for (size_t i = 0; i < versionString.size(); ++i) {
        char ch = versionString[i];
        if (ch == '.' || ch == '\0') {
            result.at(componentIdx++) = curNum;
            curNum = 0;
        } else if ('0' <= ch && ch <= '9') {
            curNum = curNum * 10 + (ch - '0');
        } else {
            ok = false;
            return result;
        }
    }

    result.at(componentIdx) = curNum;

    ok = true;
    return result;
}

static std::pair<String, int> parseVersionSuffix(const String& suffix, bool& ok)
{
    if (suffix.isEmpty()) {
        ok = false;
        return std::make_pair(String(), 0);
    }

    StringList suffixComponents = suffix.split('.');

    ok = true;
    return std::make_pair(suffixComponents.front(), (suffixComponents.size() > 1 ? suffixComponents[1].toInt() : 0));
}

Version::Version(int major, int minor, int patch, const String& suffix, int suffixVersion)
    : m_major(major), m_minor(minor), m_patch(patch), m_suffix(suffix), m_suffixVersion(suffixVersion)
{
}

Version::Version(const String& versionStr)
{
    String version = versionStr.left(versionStr.indexOf(SUFFIX_DELIMITER));

    bool ok = true;
    std::array<int, 3> versionComponents = parseVersion(version, ok);
    if (!ok) {
        return;
    }

    m_major = versionComponents[0];
    m_minor = versionComponents[1];
    m_patch = versionComponents[2];

    if (!versionStr.contains(SUFFIX_DELIMITER)) {
        return;
    }

    setSuffix(versionStr.right(versionStr.size() - versionStr.indexOf(SUFFIX_DELIMITER) - 1));
}

Version::Version(const std::string& versionStr)
    : Version(String::fromStdString(versionStr))
{
}

int Version::major() const
{
    return m_major;
}

int Version::minor() const
{
    return m_minor;
}

int Version::patch() const
{
    return m_patch;
}

String Version::suffix() const
{
    return m_suffix;
}

int Version::suffixVersion() const
{
    return m_suffixVersion;
}

void Version::setSuffix(const String& suffix)
{
    bool ok = true;
    std::pair<String, int> versionSuffix = parseVersionSuffix(suffix, ok);
    if (!ok) {
        return;
    }

    m_suffix = versionSuffix.first;
    m_suffixVersion = versionSuffix.second;
}

bool Version::isNull() const
{
    return m_major == 0 && m_minor == 0 && m_patch == 0;
}

bool Version::preRelease() const
{
    return !suffix().isEmpty();
}

String Version::toString() const
{
    String res = String(u"%1.%2.%3").arg(m_major, m_minor, m_patch);

    if (!m_suffix.isEmpty()) {
        res.append(SUFFIX_DELIMITER);
        res.append(m_suffix + (m_suffixVersion > 0 ? u"." + String::number(m_suffixVersion) : u""));
    }

    return res;
}

std::string Version::toStdString() const
{
    return toString().toStdString();
}

bool Version::operator <(const Version& other) const
{
    if (m_major > other.major()) {
        return false;
    } else if (m_major == other.major()) {
        if (m_minor > other.minor()) {
            return false;
        } else if (m_minor == other.minor()) {
            if (m_patch > other.patch()) {
                return false;
            } else if (m_patch == other.patch()) {
                if (m_suffix.isEmpty()) {
                    return false;
                }

                if (other.suffix().isEmpty()) {
                    return true;
                }

                static StringList suffixes {
                    u"dev",
                    u"alpha",
                    u"beta",
                    u"rc"
                };

                auto currentIt = std::find_if(suffixes.cbegin(), suffixes.cend(), [suffix=m_suffix](const String& s) {
                    return s.startsWith(suffix);
                });

                auto updateIt = std::find_if(suffixes.cbegin(), suffixes.cend(), [suffix=other.suffix()](const String& s) {
                    return s.startsWith(suffix);
                });

                if (currentIt == suffixes.cend() || updateIt == suffixes.cend()) {
                    LOGE() << "Invalid version suffix; current " << m_suffix << ", update " << other.suffix();
                    return true;
                }

                if (currentIt < updateIt) {
                    return true;
                }

                if (currentIt == updateIt) {
                    return m_suffixVersion < other.suffixVersion();
                }

                return false;
            }
        }
    }

    return true;
}

bool Version::operator ==(const Version& other) const
{
    return m_major == other.major()
           && m_minor == other.minor()
           && m_patch == other.patch()
           && m_suffix == other.suffix()
           && m_suffixVersion == other.suffixVersion();
}

bool Version::operator <=(const Version& other) const
{
    if (operator ==(other)) {
        return true;
    }

    return operator <(other);
}
