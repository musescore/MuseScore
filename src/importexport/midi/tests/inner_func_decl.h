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
#ifndef INNER_FUNC_DECL_H
#define INNER_FUNC_DECL_H

#include <set>

namespace Ms {
class MidiChord;
class ReducedFraction;

namespace MidiTuplet {
struct TupletInfo;

bool isTupletAllowed(const TupletInfo& tupletInfo);

std::vector<int> findTupletNumbers(const ReducedFraction& divLen, const ReducedFraction& barFraction);

TupletInfo findTupletApproximation(const ReducedFraction& tupletLen, int tupletNumber, const ReducedFraction& quantValue,
                                   const ReducedFraction& startTupletTime, const std::multimap<ReducedFraction,
                                                                                               MidiChord>::iterator& startChordIt,
                                   const std::multimap<ReducedFraction,
                                                       MidiChord>::iterator& endChordIt);

void splitFirstTupletChords(std::vector<TupletInfo>& tuplets, std::multimap<ReducedFraction, MidiChord>& chords);

std::set<int> findLongestUncommonGroup(const std::vector<TupletInfo>& tuplets, const ReducedFraction& basicQuant);
} // namespace MidiTuplet

namespace Meter {
struct MaxLevel;
struct DivisionInfo;

Meter::MaxLevel maxLevelBetween(const ReducedFraction& startTickInBar, const ReducedFraction& endTickInBar, const DivisionInfo& divInfo);

Meter::MaxLevel findMaxLevelBetween(const ReducedFraction& startTickInBar, const ReducedFraction& endTickInBar,
                                    const std::vector<DivisionInfo>& divsInfo);
} // namespace Meter
} // namespace Ms

#endif // INNER_FUNC_DECL_H
