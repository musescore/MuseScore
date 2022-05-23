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

using namespace mu;
using namespace mu::musesampler;

#ifdef Q_OS_UNIX
static const io::path_t DEFAULT_PATH("/libMuseSamplerCoreLib.so");
#else
static const io::path_t DEFAULT_PATH("/libMuseSamplerCoreLib.dll");
#endif

io::path_t MuseSamplerConfiguration::libraryPath() const
{
    return globalConfig()->appBinPath() + DEFAULT_PATH;
}
