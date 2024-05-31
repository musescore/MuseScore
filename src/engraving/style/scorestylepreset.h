/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2021 MuseScore Limited
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
#ifndef MU_ENGRAVING_SCORESTYLEPRESET_H
#define MU_ENGRAVING_SCORESTYLEPRESET_H

#include "global/iglobalconfiguration.h"
#include "engraving/iengravingconfiguration.h"
#include "modularity/ioc.h"

namespace mu::engraving {
enum class ScoreStylePreset {
    CUSTOM,
    DEFAULT,
    MSN_16MM,
    MSN_18MM,
    MSN_20MM,
    MSN_24MM,
    MSN_25MM
};

class ScoreStylePresetHelper
{
    INJECT(muse::IGlobalConfiguration, globalConfiguration);
    INJECT(engraving::IEngravingConfiguration, engravingConfiguration);
public:
    muse::io::path_t getStyleFilePath(ScoreStylePreset preset) const;
};
} // namespace mu::engraving

#endif // MU_ENGRAVING_SCORESTYLEPRESET_H
