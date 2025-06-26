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
#pragma once

#include <map>

#include "importexport/midi/internal/midishared/generalmidi.h"

#include "importmidi_fraction.h"
#include "importmidi_tuplet.h"

namespace mu::engraving {
class Tie;
class TimeSigMap;
}

namespace mu::iex::midi {
class MidiNote
{
public:
    int pitch = 0;
    int velo = 0;
    ReducedFraction offTime;
    engraving::Tie* tie = nullptr;
    bool staccato = false;
    bool isInTuplet = false;
    // for offTime quantization
    std::multimap<ReducedFraction, MidiTuplet::TupletData>::iterator tuplet;
    // for notation simplification - final quant value
    ReducedFraction offTimeQuant = ReducedFraction(-1, 1);         // invalid by default
    // to assign lyrics
    ReducedFraction origOnTime;
};

class MidiChord
{
public:
    int voice = 0;
    QList<MidiNote> notes;
    bool isInTuplet = false;
    int barIndex = -1;
    // for onTime quantization
    std::multimap<ReducedFraction, MidiTuplet::TupletData>::iterator tuplet;

    bool isStaccato() const
    {
        for (const auto& note: notes) {
            if (note.staccato) {
                return true;
            }
        }
        return false;
    }
};

class MTrack;

namespace MChord {
bool isGrandStaffProgram(GM1Program);

std::multimap<ReducedFraction, MidiChord>::iterator
findFirstChordInRange(std::multimap<ReducedFraction, MidiChord>& chords, const ReducedFraction& startRangeTick,
                      const ReducedFraction& endRangeTick);

std::multimap<ReducedFraction, MidiChord>::const_iterator
findFirstChordInRange(const std::multimap<ReducedFraction, MidiChord>& chords, const ReducedFraction& startRangeTick,
                      const ReducedFraction& endRangeTick);

template<typename Iter>
Iter findFirstChordInRange(const ReducedFraction& startRangeTick,
                           const ReducedFraction& endRangeTick,
                           const Iter& startChordIt,
                           const Iter& endChordIt)
{
    auto it = startChordIt;
    for (; it != endChordIt; ++it) {
        if (it->first >= startRangeTick) {
            if (it->first >= endRangeTick) {
                it = endChordIt;
            }
            break;
        }
    }
    return it;
}

template<typename Iter>
Iter findEndChordInRange(const ReducedFraction& endRangeTick,
                         const Iter& startChordIt,
                         const Iter& endChordIt)
{
    auto it = startChordIt;
    for (; it != endChordIt; ++it) {
        if (it->first >= endRangeTick) {
            break;
        }
    }
    return it;
}

ReducedFraction minNoteOffTime(const QList<MidiNote>& notes);
ReducedFraction maxNoteOffTime(const QList<MidiNote>& notes);
ReducedFraction minNoteLen(const std::pair<const ReducedFraction, MidiChord>& chord);
ReducedFraction maxNoteLen(const std::pair<const ReducedFraction, MidiChord>& chord);

const ReducedFraction& minAllowedDuration();
ReducedFraction findMinDuration(const ReducedFraction& onTime, const QList<MidiChord>& midiChords, const ReducedFraction& length);
void sortNotesByPitch(std::multimap<ReducedFraction, MidiChord>& chords);
void sortNotesByLength(std::multimap<ReducedFraction, MidiChord>& chords);

void collectChords(
    std::multimap<int, MTrack>& tracks, const ReducedFraction& humanTolCoeff, const ReducedFraction& nonHumanTolCoeff);

void collectChords(
    MTrack& track, const ReducedFraction& humanTolCoeff, const ReducedFraction& nonHumanTolCoeff);

void removeOverlappingNotes(std::multimap<int, MTrack>& tracks);
void mergeChordsWithEqualOnTimeAndVoice(std::multimap<int, MTrack>& tracks);
void splitUnequalChords(std::multimap<int, MTrack>& tracks);
int chordAveragePitch(const QList<MidiNote>& notes, int beg, int end);
int chordAveragePitch(const QList<MidiNote>& notes);

ReducedFraction findMaxChordLength(const std::multimap<ReducedFraction, MidiChord>& chords);

std::vector<std::multimap<ReducedFraction, MidiChord>::const_iterator>
findChordsForTimeRange(
    int voice, const ReducedFraction& onTime, const ReducedFraction& offTime, const std::multimap<ReducedFraction, MidiChord>& chords,
    const ReducedFraction& maxChordLength);

void setBarIndexes(
    std::multimap<ReducedFraction, MidiChord>& chords, const ReducedFraction& basicQuant, const ReducedFraction& lastTick,
    const engraving::TimeSigMap* sigmap);

#ifdef QT_DEBUG

bool areOnTimeValuesDifferent(const std::multimap<ReducedFraction, MidiChord>& chords);
bool areBarIndexesSuccessive(const std::multimap<ReducedFraction, MidiChord>& chords);
bool areNotesLongEnough(const std::multimap<ReducedFraction, MidiChord>& chords);
bool isLastTickValid(const ReducedFraction& lastTick, const std::multimap<ReducedFraction, MidiChord>& chords);
bool isLastTickValid(const ReducedFraction& lastTick, const std::multimap<int, MTrack>& tracks);
bool areBarIndexesSet(const std::multimap<ReducedFraction, MidiChord>& chords);

#endif
} // namespace MChord
} // namespace mu::iex::midi
