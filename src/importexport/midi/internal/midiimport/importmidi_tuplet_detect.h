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
#ifndef IMPORTMIDI_TUPLET_DETECT_H
#define IMPORTMIDI_TUPLET_DETECT_H

#include <vector>
#include <map>

namespace mu::iex::midi {
class ReducedFraction;
class MidiChord;

namespace MidiTuplet {
struct TupletInfo;

std::vector<TupletInfo> detectTuplets(
    const std::multimap<ReducedFraction, MidiChord>::iterator& startBarChordIt, const std::multimap<ReducedFraction,
                                                                                                    MidiChord>::iterator& endBarChordIt,
    const ReducedFraction& startBarTick, const ReducedFraction& barFraction, std::multimap<ReducedFraction, MidiChord>& chords,
    const ReducedFraction& basicQuant, int barIndex);
} // namespace MidiTuplet
} // namespace mu::iex::midi

#endif // IMPORTMIDI_TUPLET_DETECT_H
