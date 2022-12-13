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

#ifndef MU_MUSESAMPLER_IMUSESAMPLERCONFIGURATION_H
#define MU_MUSESAMPLER_IMUSESAMPLERCONFIGURATION_H

#include "modularity/imoduleexport.h"

#include "io/path.h"

namespace mu::musesampler {
class IMuseSamplerConfiguration : MODULE_EXPORT_INTERFACE
{
    INTERFACE_ID(IMuseSamplerConfiguration)

public:
    virtual ~IMuseSamplerConfiguration() = default;

    virtual mu::io::path_t libraryPath() const = 0;

    virtual std::string minimumSupportedVersion() const = 0;
    virtual std::string maximumSupportedVersion() const = 0;
};
}

#endif // MU_MUSESAMPLER_IMUSESAMPLERCONFIGURATION_H
