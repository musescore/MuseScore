/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-CLA-applies
 *
 * MuseScore
 * Music Composition & Notation
 *
 * Copyright (C) 2025 MuseScore BVBA and others
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

using namespace muse::musesampler;

static const muse::Settings::Key USE_LEGACY_AUDITION("musesampler", "museSampler/useLegacyAudition");

void MuseSamplerConfiguration::init()
{
    settings()->setDefaultValue(USE_LEGACY_AUDITION, Val(false));
}

#if defined(Q_OS_LINUX)
muse::io::path_t MuseSamplerConfiguration::libraryPath() const
{
    return globalConfig()->genericDataPath() + "/MuseSampler/lib/libMuseSamplerCoreLib.so";
}

#elif defined(Q_OS_MAC)
muse::io::path_t MuseSamplerConfiguration::libraryPath() const
{
    return globalConfig()->genericDataPath() + "/MuseSampler/lib/libMuseSamplerCoreLib.dylib";
}

#else
muse::io::path_t MuseSamplerConfiguration::libraryPath() const
{
    return globalConfig()->genericDataPath() + "/MuseSampler/lib/MuseSamplerCoreLib.dll";
}

#endif

muse::Version MuseSamplerConfiguration::minSupportedVersion() const
{
    return Version(0, 101, 0);
}

bool MuseSamplerConfiguration::shouldShowBuildNumber() const
{
    return globalConfig()->devModeEnabled();
}

bool MuseSamplerConfiguration::useLegacyAudition() const
{
    return settings()->value(USE_LEGACY_AUDITION).toBool();
}
