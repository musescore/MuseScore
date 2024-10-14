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

using namespace muse;
using namespace muse::musesampler;

static const Settings::Key USE_LEGACY_AUDITION("musesampler", "museSampler/useLegacyAudition");

void MuseSamplerConfiguration::init()
{
    settings()->setDefaultValue(USE_LEGACY_AUDITION, Val(false));
}

#if defined(Q_OS_LINUX)
static const muse::io::path_t LIB_NAME("libMuseSamplerCoreLib.so");
static const muse::io::path_t FALLBACK_PATH = LIB_NAME;

muse::io::path_t MuseSamplerConfiguration::defaultPath() const
{
    return globalConfig()->genericDataPath() + "/MuseSampler/lib/" + LIB_NAME;
}

#elif defined(Q_OS_MAC)
static const muse::io::path_t LIB_NAME("libMuseSamplerCoreLib.dylib");
static const muse::io::path_t FALLBACK_PATH = "/usr/local/lib/" + LIB_NAME;

muse::io::path_t MuseSamplerConfiguration::defaultPath() const
{
    return globalConfig()->genericDataPath() + "/MuseSampler/lib/" + LIB_NAME;
}

#else
static const muse::io::path_t LIB_NAME("MuseSamplerCoreLib.dll");
static const muse::io::path_t FALLBACK_PATH = LIB_NAME;

muse::io::path_t MuseSamplerConfiguration::defaultPath() const
{
    io::path_t path = globalConfig()->genericDataPath() + "/MuseSampler/lib/" + LIB_NAME;
    return path;
}

#endif

// Preferred location
muse::io::path_t MuseSamplerConfiguration::userLibraryPath() const
{
    // Override for testing/dev:
    if (const char* path = std::getenv("MUSESAMPLER_PATH")) {
        return io::path_t(path);
    }

    return defaultPath();
}

// If installed on the system instead of user dir...do this as a backup
muse::io::path_t MuseSamplerConfiguration::fallbackLibraryPath() const
{
    return FALLBACK_PATH;
}

bool MuseSamplerConfiguration::shouldShowBuildNumber() const
{
    return globalConfig()->devModeEnabled();
}

bool MuseSamplerConfiguration::useLegacyAudition() const
{
    return settings()->value(USE_LEGACY_AUDITION).toBool();
}
