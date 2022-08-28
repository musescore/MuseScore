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

#include <array>

#include "config.h"
#include "stringutils.h"

#include "log.h"

using namespace mu::framework;

using version_components_t = std::array<int, 3>;

static constexpr char versionString[] = VERSION;

static constexpr std::pair<version_components_t, bool> parse_version()
{
    version_components_t result { 0, 0, 0 };

    size_t componentIdx = 0;
    int curNum = 0;

    for (const char ch : versionString) {
        if (ch == '.' || ch == '\0') {
            result.at(componentIdx++) = curNum;
            curNum = 0;
        } else if ('0' <= ch && ch <= '9') {
            curNum = curNum * 10 + (ch - '0');
        } else {
            return { result, false };
        }
    }

    return { result, true };
}

static constexpr std::pair<version_components_t, bool> parse_result = parse_version();
static constexpr version_components_t version_components = parse_result.first;

static_assert(parse_result.second, "Invalid VERSION");

bool Version::unstable()
{
#ifdef MSCORE_UNSTABLE
    return true;
#else
    return false;
#endif
}

std::string Version::version()
{
    return versionString;
}

std::string Version::fullVersion()
{
    std::string version(versionString);
    std::string versionLabel(VERSION_LABEL);
    strings::replace(versionLabel, " ", "");
    if (!versionLabel.empty()) {
        version.append("-").append(versionLabel);
    }
    return version;
}

std::string Version::revision()
{
    return MUSESCORE_REVISION;
}

int Version::majorVersion()
{
    return version_components.at(0);
}

int Version::minorVersion()
{
    return version_components.at(1);
}

int Version::patchVersion()
{
    return version_components.at(2);
}
