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

// if SPLIT_CHORD_DURATIONS is defined: chord with bend is split to several chords, tied chords may be removed or replaced according to new ticks
// otherwise - "grace note after" is placed where the "small note head" was in guitar pro, and normal note is placed where normal note was in guitar pro
// #define SPLIT_CHORD_DURATIONS 1

namespace mu::engraving {
class Chord;
}

namespace mu::iex::guitarpro {
struct BendDataContext {
    struct BendNoteData {
        double startFactor = 0.0;
        double endFactor = 1.0;
        int quarterTones = 0;
    };

    using bend_data_map_t       = std::map<size_t /* idx in chord */, BendNoteData>;
    using grace_bend_data_map_t = std::map<size_t /* idx in chord */, std::vector<BendNoteData> >; // each note can have multiple grace notes connected

#ifdef SPLIT_CHORD_DURATIONS
    struct BendChordData {
        mu::engraving::Fraction startTick;
        bend_data_map_t noteDataByIdx;
    };

    std::unordered_map<mu::engraving::track_idx_t, std::map<int, BendChordData> > bendDataByEndTick;
    std::unordered_map<mu::engraving::track_idx_t, std::map<int, std::vector<mu::engraving::Fraction> > > bendChordDurations;
    std::unordered_map<mu::engraving::track_idx_t, std::set<mu::engraving::Fraction> > reduntantChordTicks;
    std::unordered_map<mu::engraving::track_idx_t, std::set<mu::engraving::Fraction> > chordTicksForTieBack;
#else
    std::unordered_map<mu::engraving::track_idx_t, std::map<mu::engraving::Fraction, grace_bend_data_map_t > > graceAfterBendData;
    std::unordered_map<mu::engraving::track_idx_t, std::map<mu::engraving::Fraction, bend_data_map_t > > tiedNotesBendsData;
#endif
    std::unordered_map<mu::engraving::track_idx_t, std::map<mu::engraving::Fraction, bend_data_map_t > > prebendData;
    std::unordered_map<mu::engraving::track_idx_t, std::map<mu::engraving::Fraction, bend_data_map_t > > slightBendData;
};
} // mu::iex::guitarpro
