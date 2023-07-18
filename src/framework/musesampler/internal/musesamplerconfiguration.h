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

#ifndef MU_MUSESAMPLER_MUSESAMPLERCONFIGURATION_H
#define MU_MUSESAMPLER_MUSESAMPLERCONFIGURATION_H

#include "modularity/ioc.h"
#include "iglobalconfiguration.h"

#include "imusesamplerconfiguration.h"

namespace mu::musesampler {
class MuseSamplerConfiguration : public IMuseSamplerConfiguration
{
    INJECT(framework::IGlobalConfiguration, globalConfig)

public:
    void init();

    // Preferred local user install path; try this first.
    io::path_t userLibraryPath() const override;

    // Backup location for system-wide sampler install
    io::path_t fallbackLibraryPath() const override;

private:
    io::path_t defaultPath() const;
};
}

#endif // MU_MUSESAMPLER_MUSESAMPLERCONFIGURATION_H
