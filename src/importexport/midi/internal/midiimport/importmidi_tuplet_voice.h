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
#ifndef IMPORTMIDI_TUPLET_VOICE_H
#define IMPORTMIDI_TUPLET_VOICE_H

#include <vector>
#include <map>
#include <list>

namespace mu::iex::midi {
class ReducedFraction;
class MidiChord;

namespace MidiTuplet {
struct TupletInfo;

struct TiedTuplet
{
    int tupletId;
    int voice;
    std::pair<const ReducedFraction, MidiChord>* chord;    // chord the tuplet is tied with
    std::vector<int> tiedNoteIndexes;     // indexes of tied notes of that chord
};

int tupletVoiceLimit();

bool excludeExtraVoiceTuplets(
    std::vector<TupletInfo>& tuplets, std::list<std::multimap<ReducedFraction, MidiChord>::iterator>& nonTuplets,
    std::list<TiedTuplet>& backTiedTuplets, const std::multimap<ReducedFraction, MidiChord>& chords, const ReducedFraction& basicQuant,
    const ReducedFraction& barStart, int barIndex);

std::list<TiedTuplet>
findBackTiedTuplets(
    const std::multimap<ReducedFraction, MidiChord>& chords, const std::vector<TupletInfo>& tuplets, const ReducedFraction& prevBarStart,
    const ReducedFraction& startBarTick, const ReducedFraction& basicQuant, int currentBarIndex);

void assignVoices(
    std::vector<TupletInfo>& tuplets, std::list<std::multimap<ReducedFraction, MidiChord>::iterator>& nonTuplets,
    std::list<TiedTuplet>& backTiedTuplets, const std::multimap<ReducedFraction, MidiChord>& chords, const ReducedFraction& basicQuant,
    const ReducedFraction& barStart, int barIndex);

std::pair<ReducedFraction, ReducedFraction>
chordInterval(const std::pair<const ReducedFraction, MidiChord>& chord, const std::multimap<ReducedFraction, MidiChord>& chords,
              const ReducedFraction& basicQuant, const ReducedFraction& barStart);

#ifdef QT_DEBUG

bool haveOverlappingVoices(
    const std::list<std::multimap<ReducedFraction, MidiChord>::iterator>& nonTuplets, const std::vector<TupletInfo>& tuplets,
    const std::list<TiedTuplet>& backTiedTuplets, const std::multimap<ReducedFraction, MidiChord>& chords,
    const ReducedFraction& basicQuant, const ReducedFraction& barStart);

#endif
} // namespace MidiTuplet
} // namespace mu::iex::midi

#endif // IMPORTMIDI_TUPLET_VOICE_H
