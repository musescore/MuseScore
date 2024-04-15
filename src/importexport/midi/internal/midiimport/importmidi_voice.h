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
#ifndef IMPORTMIDI_VOICE_H
#define IMPORTMIDI_VOICE_H

#include "importmidi_operation.h"

#include <map>

namespace mu::engraving {
class TimeSigMap;
}

namespace mu::iex::midi {
class MTrack;
class MidiChord;

namespace MidiTuplet {
struct TupletData;
}

namespace MidiVoice {
size_t toIntVoiceCount(MidiOperations::VoiceCount value);
int voiceLimit();
bool separateVoices(std::multimap<int, MTrack>& tracks, const engraving::TimeSigMap* sigmap);

bool splitChordToVoice(std::multimap<ReducedFraction, MidiChord>::iterator& chordIt, const QSet<int>& notesToMove, int newVoice,
                       std::multimap<ReducedFraction, MidiChord>& chords, std::multimap<ReducedFraction, MidiTuplet::TupletData>& tuplets,
                       std::multimap<ReducedFraction, std::multimap<ReducedFraction, MidiTuplet::TupletData>::iterator>& insertedTuplets,
                       const ReducedFraction& maxChordLength, bool allowParallelTuplets = false);

#ifdef QT_DEBUG

bool areVoicesSame(const std::multimap<ReducedFraction, MidiChord>& chords);

#endif
} // namespace MidiVoice
} // namespace mu::iex::midi

#endif // IMPORTMIDI_VOICE_H
