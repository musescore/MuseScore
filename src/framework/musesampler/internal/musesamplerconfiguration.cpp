/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-CLA-applies
 *
 * MuseScore
 * Music Composition & Notation
 *
 * Copyright (C) 2022 MuseScore BVBA and others
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

#include "musesamplerconfiguration.h"

#include "settings.h"

#include <cstdlib>

using namespace mu;
using namespace mu::musesampler;
using namespace mu::framework;

static const Settings::Key USER_MUSESAMPLER_PATH("musesampler", "application/paths/museSampler");

void MuseSamplerConfiguration::init()
{
    settings()->setDefaultValue(USER_MUSESAMPLER_PATH, Val(""));
}

#if defined(Q_OS_LINUX)
static const io::path_t LIB_NAME("libMuseSamplerCoreLib.so");
static const io::path_t FALLBACK_PATH = LIB_NAME;

io::path_t MuseSamplerConfiguration::defaultPath() const
{
    return globalConfig()->genericDataPath() + "/MuseSampler/lib/" + LIB_NAME;
}

#elif defined(Q_OS_MAC)
static const io::path_t LIB_NAME("libMuseSamplerCoreLib.dylib");
static const io::path_t FALLBACK_PATH = "/usr/local/lib/" + LIB_NAME;

io::path_t MuseSamplerConfiguration::defaultPath() const
{
    return globalConfig()->genericDataPath() + "/MuseSampler/lib/" + LIB_NAME;
}

#else
static const io::path_t LIB_NAME("MuseSamplerCoreLib.dll");
static const io::path_t FALLBACK_PATH = LIB_NAME;

io::path_t MuseSamplerConfiguration::defaultPath() const
{
    return globalConfig()->genericDataPath() + "\\MuseSampler\\lib\\" + LIB_NAME;
}

#endif

// Preferred location
io::path_t MuseSamplerConfiguration::userLibraryPath() const
{
    // Override for testing/dev:
    if (const char* path = std::getenv("MUSESAMPLER_PATH")) {
        return io::path_t(path);
    }

    io::path_t path = settings()->value(USER_MUSESAMPLER_PATH).toString();
    if (!path.empty()) {
        return path;
    }

    return defaultPath();
}

// If installed on the system instead of user dir...do this as a backup
io::path_t MuseSamplerConfiguration::fallbackLibraryPath() const
{
    return FALLBACK_PATH;
}
