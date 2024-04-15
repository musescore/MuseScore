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
#ifndef IMPORTMIDI_TUPLET_H
#define IMPORTMIDI_TUPLET_H

#include "importmidi_fraction.h"

namespace mu::engraving {
class TimeSigMap;
class DurationElement;
}

namespace mu::iex::midi {
class MidiChord;
class MidiNote;
class MTrack;

namespace MidiTuplet {
struct TupletInfo;

struct TupletData
{
    int voice;
    ReducedFraction onTime;
    ReducedFraction len;
    int tupletNumber;
    std::vector<engraving::DurationElement*> elements;
};

struct TupletLimits
{
    // ratio - for conversion from tuplet durations to regular durations
    // for example, 8th note in triplet * 3/2 = regular 8th note
    ReducedFraction ratio;
    int minNoteCount;
    int minNoteCountAddVoice;
    int minNoteCountStaccato;
    int minNoteCountHuman;
};

const TupletLimits& tupletLimits(int tupletNumber);

void removeEmptyTuplets(MTrack& track);

bool hasNonTrivialChord(
    const ReducedFraction& chordOnTime, const QList<MidiNote>& notes, const ReducedFraction& tupletOnTime,
    const ReducedFraction& tupletLen);

bool isTupletUseless(
    int voice, const ReducedFraction& onTime, const ReducedFraction& len, const ReducedFraction& maxChordLength,
    const std::multimap<ReducedFraction, MidiChord>& chords);

std::multimap<ReducedFraction, TupletData>::iterator
removeTuplet(
    const std::multimap<ReducedFraction, TupletData>::iterator& tupletIt, std::multimap<ReducedFraction, TupletData>& tuplets,
    const ReducedFraction& maxChordLength, std::multimap<ReducedFraction, MidiChord>& chords);

std::multimap<ReducedFraction, TupletData>::iterator
removeTupletIfEmpty(
    const std::multimap<ReducedFraction, TupletData>::iterator& tupletIt, std::multimap<ReducedFraction, TupletData>& tuplets,
    const ReducedFraction& maxChordLength, std::multimap<ReducedFraction, MidiChord>& chords);

const TupletInfo& tupletFromId(int id, const std::vector<TupletInfo>& tuplets);
TupletInfo& tupletFromId(int id, std::vector<TupletInfo>& tuplets);

std::pair<ReducedFraction, ReducedFraction>
tupletInterval(const TupletInfo& tuplet, const ReducedFraction& basicQuant);

std::vector<std::pair<ReducedFraction, ReducedFraction> >
findTupletIntervals(const std::vector<TupletInfo>& tuplets, const ReducedFraction& basicQuant);

std::vector<TupletData>
findTupletsInBarForDuration(int voice, const ReducedFraction& barStartTick, const ReducedFraction& durationOnTime,
                            const ReducedFraction& durationLen, const std::multimap<ReducedFraction, TupletData>& tupletEvents);

std::vector<std::multimap<ReducedFraction, TupletData>::const_iterator>
findTupletsForTimeRange(int voice, const ReducedFraction& onTime, const ReducedFraction& len, const std::multimap<ReducedFraction,
                                                                                                                  TupletData>& tupletEvents,
                        bool strictComparison);

std::multimap<ReducedFraction, TupletData>::const_iterator
findTupletContainingTime(int voice, const ReducedFraction& time, const std::multimap<ReducedFraction, TupletData>& tupletEvents,
                         bool strictComparison);

// Find tuplets and set bar indexes

void findAllTuplets(
    std::multimap<ReducedFraction, TupletData>& tuplets, std::multimap<ReducedFraction, MidiChord>& chords,
    const engraving::TimeSigMap* sigmap, const ReducedFraction& basicQuant);

ReducedFraction findOnTimeBetweenChords(
    const std::pair<const ReducedFraction, MidiChord>& chord, const std::multimap<ReducedFraction, MidiChord>& chords,
    const ReducedFraction& basicQuant, const ReducedFraction& barStart);

#ifdef QT_DEBUG

bool areAllTupletsReferenced(
    const std::multimap<ReducedFraction, MidiChord>& chords, const std::multimap<ReducedFraction, TupletData>& tupletEvents);

bool areTupletReferencesValid(const std::multimap<ReducedFraction, MidiChord>& chords);

bool isTupletRangeOk(
    const std::pair<const ReducedFraction, MidiChord>& chord, const std::multimap<ReducedFraction, TupletData>& tuplets);

bool areTupletRangesOk(
    const std::multimap<ReducedFraction, MidiChord>& chords, const std::multimap<ReducedFraction, TupletData>& tuplets);

bool areAllTupletsDifferent(const std::multimap<ReducedFraction, TupletData>& tuplets);

#endif
} // namespace MidiTuplet
} // namespace mu::iex::midi

#endif // IMPORTMIDI_TUPLET_H
