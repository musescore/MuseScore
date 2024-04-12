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
#ifndef IMPORTMIDI_METER_H
#define IMPORTMIDI_METER_H

#include <vector>
#include <QList>

namespace mu::engraving {
class TDuration;
}

namespace mu::iex::midi {
class ReducedFraction;

namespace MidiTuplet {
struct TupletData;
}

namespace Meter {
enum class DurationType : char
{
    NOTE,
    REST
};

bool isSimple(const ReducedFraction& barFraction);
bool isCompound(const ReducedFraction& barFraction);
bool isComplex(const ReducedFraction& barFraction);
bool isDuple(const ReducedFraction& barFraction);
bool isTriple(const ReducedFraction& barFraction);
bool isQuadruple(const ReducedFraction& barFraction);
bool isQuintuple(const ReducedFraction& barFraction);
bool isSeptuple(const ReducedFraction& barFraction);

ReducedFraction beatLength(const ReducedFraction& barFraction);

struct DivisionInfo;

DivisionInfo metricDivisionsOfBar(const ReducedFraction& barFraction);
DivisionInfo metricDivisionsOfTuplet(const MidiTuplet::TupletData& tuplet, int tupletStartLevel);

// result in vector: first elements - all tuplets info, one at the end - bar division info
std::vector<DivisionInfo> divisionInfo(const ReducedFraction& barFraction, const std::vector<MidiTuplet::TupletData>& tupletsInBar);

// tick is counted from the beginning of bar
int levelOfTick(const ReducedFraction& tick, const std::vector<DivisionInfo>& divsInfo);

std::vector<int> metricLevelsOfBar(const ReducedFraction& barFraction, const std::vector<DivisionInfo>& divsInfo,
                                   const ReducedFraction& minDuration);

bool isSimpleNoteDuration(const ReducedFraction& duration);   // quarter, half, eighth, 16th ...

// division lengths of bar, each can be a tuplet length
std::vector<ReducedFraction> divisionsOfBarForTuplets(const ReducedFraction& barFraction);

// duration and all tuplets should belong to the same voice
// nested tuplets are not allowed
QList<std::pair<ReducedFraction, engraving::TDuration> >
toDurationList(const ReducedFraction& startTickInBar, const ReducedFraction& endTickInBar, const ReducedFraction& barFraction,
               const std::vector<MidiTuplet::TupletData>& tupletsInBar, DurationType durationType, bool useDots,
               bool printRestRemains = true);
} // namespace Meter
} // namespace mu::iex::midi

#endif // IMPORTMIDI_METER_H
