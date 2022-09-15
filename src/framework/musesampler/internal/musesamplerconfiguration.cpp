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

#include <cstdlib>

using namespace mu;
using namespace mu::musesampler;

#if defined(Q_OS_LINUX)
static const io::path_t DEFAULT_PATH("libMuseSamplerCoreLib.so");
#elif defined(Q_OS_MAC)
static const io::path_t DEFAULT_PATH("/usr/local/lib/libMuseSamplerCoreLib.dylib");
#else
static const io::path_t DEFAULT_PATH("MuseSamplerCoreLib.dll");
#endif

io::path_t MuseSamplerConfiguration::libraryPath() const
{
    if (const char* path = std::getenv("MUSESAMPLER_PATH")) {
        return io::path_t(path);
    }

    return DEFAULT_PATH;
}
