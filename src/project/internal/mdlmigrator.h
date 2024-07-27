/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2024 MuseScore Limited
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

#include "modularity/ioc.h"
#include "engraving/types/types.h"
#include "iglobalconfiguration.h"

namespace mu::engraving {
class Drumset;
class Instrument;
class MasterScore;
class Score;
}

namespace mu::project {
using RepitchFunc = std::function<int (int)>;

class MdlMigrator
{
    INJECT(muse::IGlobalConfiguration, globalConfiguration)

public:
    MdlMigrator(mu::engraving::MasterScore* score)
        : m_score(score) {}
    void remapPercussion();

private:
    bool loadDrumset(mu::engraving::Drumset* drumset, muse::io::path_t path);
    void remapPitches(mu::engraving::track_idx_t startTrack, mu::engraving::track_idx_t endTrack, mu::engraving::Fraction startTick,
                      mu::engraving::Fraction endTick, const RepitchFunc& repitch);
    bool needToRemap(const mu::engraving::Instrument& instr, RepitchFunc& repitch, muse::io::path_t& drumsetPath);

    // Repitch functions
    static int repitchMdlSnares(int pitch);
    static int repitchMdlTenors(int pitch);
    static int repitchMdlBassline10(int pitch);
    static int repitchMdlBassline5(int pitch);
    static int repitchMdlCymballine(int pitch);

    mu::engraving::MasterScore* m_score = nullptr;
};
}
