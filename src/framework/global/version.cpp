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

#include "config.h"
#include "stringutils.h"

#include "log.h"

using namespace mu::framework;

static int _majorVersion = -1, _minorVersion = -1, _patchVersion = -1;

static void parseVersion()
{
    std::vector<std::string> versionComponents;
    mu::strings::split(VERSION, versionComponents, ".");

    IF_ASSERT_FAILED(versionComponents.size() == 3) {
        _majorVersion = _minorVersion = _patchVersion = 0;
    } else {
        _majorVersion = std::stoi(versionComponents.at(0));
        _minorVersion = std::stoi(versionComponents.at(1));
        _patchVersion = std::stoi(versionComponents.at(2));
    }
}

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
    return VERSION;
}

std::string Version::fullVersion()
{
    std::string version(VERSION);
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
    if (_majorVersion < 0) {
        parseVersion();
    }

    return _majorVersion;
}

int Version::minorVersion()
{
    if (_minorVersion < 0) {
        parseVersion();
    }

    return _minorVersion;
}

int Version::patchVersion()
{
    if (_patchVersion < 0) {
        parseVersion();
    }

    return _patchVersion;
}
