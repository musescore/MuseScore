/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2025 MuseScore Limited
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

#include <engraving/types/types.h>
#include "../guitarbendimporttypes.h"

namespace mu::iex::guitarpro {
struct BendDataContextSplitChord {
    std::unordered_map<mu::engraving::track_idx_t, std::map<int, BendChordData> > bendDataByEndTick;
    std::unordered_map<mu::engraving::track_idx_t, std::map<int, std::vector<mu::engraving::Fraction> > > bendChordDurations;
    std::unordered_map<mu::engraving::track_idx_t, std::set<mu::engraving::Fraction> > reduntantChordTicks;
    std::unordered_map<mu::engraving::track_idx_t, std::set<mu::engraving::Fraction> > chordTicksForTieBack;
};
} // mu::iex::guitarpro
