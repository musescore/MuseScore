/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-CLA-applies
 *
 * MuseScore
 * Music Composition & Notation
 *
 * Copyright (C) 2021 MuseScore BVBA and others
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
#ifndef IMPORTMIDI_BEAT_H
#define IMPORTMIDI_BEAT_H

#include <set>
#include <map>

namespace mu::engraving {
class TimeSigMap;
class ReducedFraction;
class MidiChord;
class MTrack;
class ReducedFraction;
class Score;
}

namespace mu::iex::midi {
namespace MidiBeat {
void removeEvery2ndBeat(std::set<ReducedFraction>& beatSet);
void findBeatLocations(
    const std::multimap<ReducedFraction, MidiChord>& allChords, TimeSigMap* sigmap, double ticksPerSec);

void adjustChordsToBeats(std::multimap<int, MTrack>& tracks);
void setTimeSignature(TimeSigMap* sigmap);
} // namespace MidiBeat
} // namespace mu::iex::midi

#endif // IMPORTMIDI_BEAT_H
