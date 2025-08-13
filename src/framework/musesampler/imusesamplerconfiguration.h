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

#pragma once

#include "modularity/imoduleinterface.h"

#include "io/path.h"
#include "types/version.h"

namespace muse::musesampler {
class IMuseSamplerConfiguration : MODULE_EXPORT_INTERFACE
{
    INTERFACE_ID(IMuseSamplerConfiguration)

public:
    virtual ~IMuseSamplerConfiguration() = default;

    virtual io::path_t libraryPath() const = 0;

    virtual Version minSupportedVersion() const = 0;

    virtual bool shouldShowBuildNumber() const = 0;
    virtual bool useLegacyAudition() const = 0;
};
}
