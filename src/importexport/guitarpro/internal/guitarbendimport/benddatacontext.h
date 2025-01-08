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

#include <engraving/types/types.h>
#include <engraving/dom/types.h>

namespace mu::engraving {
class Chord;
}

namespace mu::iex::guitarpro {
struct BendDataContext {
    struct BendNoteData {
        double startFactor = 0.0;
        double endFactor = 1.0;
        int quarterTones = 0;
        mu::engraving::GuitarBendType type = mu::engraving::GuitarBendType::BEND;
    };

    struct BendChordData {
        mu::engraving::Fraction startTick;
        std::map<int /* idx in chord */, BendNoteData> noteDataByIdx;
    };

    std::unordered_map<mu::engraving::track_idx_t, std::map<int, std::vector<mu::engraving::Fraction> > > bendChordDurations;
    std::unordered_map<mu::engraving::track_idx_t, std::map<int, BendChordData> > bendDataByEndTick;
};
} // mu::iex::guitarpro
