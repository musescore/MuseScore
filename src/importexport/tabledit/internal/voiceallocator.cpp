/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2025 MuseScore Limited
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

#include "importtef.h"
#include "voiceallocator.h"
#include "engraving/dom/mscore.h"

namespace mu::iex::tabledit {
bool VoiceAllocator::canAddTefNoteToVoice(const TefNote* const note, const int voice)
{
    // is there room after the previous note ?
    if (stopPosition(voice) <= note->position) {
        LOGN("add string %d fret %d to voice %d", note->string, note->fret, voice);
        return true;
    }
    // can the note go into a chord ?
    const auto notePlaying = notesPlaying.at(voice);
    if (notePlaying
        && !notePlaying->rest
        && !note->rest
        && notePlaying->position == note->position
        && notePlaying->duration == note->duration) {
        LOGN("add string %d fret %d to voice %d as chord", note->string, note->fret, voice);
        return true;
    }
    return false;
}

int VoiceAllocator::findFirstPossibleVoice(const TefNote* const note, const std::array<int, 3> voices)
{
    for (const auto v : voices) {
        if (canAddTefNoteToVoice(note, v)) {
            return v;
        }
    }
    return -1;
}

// return TablEdit note length in 64th (including triplets rounded down to nearest note length)
// TODO: remove code duplication with importtef.cpp duration2length()

static int durationToInt(uint8_t duration) // TODO duplicated code ?
{
    switch (duration) {
    case  0: return 64; //"whole";
    case  1: return 48; //"half dotted";
    case  2: return 32; //"whole triplet";
    case  3: return 32; //"half";
    case  4: return 24; //"quarter dotted";
    case  5: return 16; //"half triplet";
    case  6: return 16; //"quarter";
    case  7: return 12; //"eighth dotted";
    case  8: return 8; //"quarter triplet";
    case  9: return 8; //"eighth";
    case 10: return 6; //"16th dotted";
    case 11: return 4; //"eighth triplet";
    case 12: return 4; //"16th";
    case 13: return 3; //"32nd dotted";
    case 14: return 2; //"16th triplet";
    case 15: return 2; //"32nd";
    //case 16: return "64th dotted";
    case 17: return 1; //"32nd triplet";
    case 18: return 1; //"64th";
    case 19: return 56; //"half double dotted";
    //case 20: return "16th quintuplet";
    case 22: return 28; //"quarter double dotted";
    case 25: return 14; //"eighth double dotted";
    case 28: return 7; //"16th double dotted";
    default: return 0; //"undefined";
    }
}

int VoiceAllocator::stopPosition(const size_t voice)
{
    if (mu::engraving::VOICES <= voice) {
        LOGD("incorrect voice %zu", voice);
        return -1;
    }

    const auto note = notesPlaying.at(voice);
    if (note) {
        return note->position + durationToInt(note->duration);
    }
    return 0;
}

void VoiceAllocator::appendNoteToVoice(const TefNote* const note, int voice)
{
    LOGN("position %d string %d fret %d voice %d", note->position, note->string, note->fret, voice);
    const auto nChords { voiceContents[voice].size() };
    LOGN("voice %d nChords %zu", voice, nChords);
    if (nChords == 0) {
        LOGN("create first chord");
        std::vector<const TefNote*> chord;
        chord.push_back(note);
        voiceContents[voice].push_back(chord);
    } else {
        const auto position { voiceContents[voice].at(nChords - 1).at(0)->position };
        LOGN("chord %zu position %d", nChords - 1, position);
        if (position == note->position) {
            LOGN("add to last chord");
            voiceContents[voice].at(nChords - 1).push_back(note);
        } else {
            LOGN("create next chord at position %d", note->position);
            std::vector<const TefNote*> chord;
            chord.push_back(note);
            voiceContents[voice].push_back(chord);
        }
    }
    LOGN("done");
}

// debug: dump voices

void VoiceAllocator::dump()
{
    for (size_t i = 0; i < mu::engraving::VOICES; ++i) {
        LOGN("- voice %zu", i);
        for (size_t j = 0; j < voiceContents.at(i).size(); ++j) {
            LOGN("  - chord %zu", j);
            for (const auto note : voiceContents.at(i).at(j)) {
                LOGN("    - position %d string %d fret %d", note->position, note->string, note->fret);
            }
        }
    }
}

void VoiceAllocator::allocateVoice(const TefNote* const note, int voice)
{
    if (voice >= 0) {
        // do actual allocation
        // note chord info is lost
        if (allocations.count(note) == 0) {
            allocations[note] = voice;
            notesPlaying[voice] = note;
            appendNoteToVoice(note, voice);
        } else {
            LOGD("duplicate note allocation");
        }
    } else {
        LOGD("cannot add string %d fret %d to voice %d", note->string, note->fret, voice);
    }
}

void VoiceAllocator::addColumn(const std::vector<const TefNote*>& column)
{
    if (column.empty()) {
        return;
    }

    // first add the highest note to voice 0
    addNote(column.at(0), true);
    if (column.size() >= 2) {
        // then add the lowest note to voice 1
        addNote(column.at(column.size() - 1), false);
        // finally add the remaining notes where possible
        for (unsigned int i = 1; i < column.size() - 1; ++i) {
            addNote(column.at(i), true);
        }
    }
}

void VoiceAllocator::addNote(const TefNote* const note, const bool preferVoice0)
{
    int voice { -1 };
    LOGN("note position %d voice %d", note->position, static_cast<int>(note->voice));
    if (note->voice == Voice::UPPER) {
        voice = findFirstPossibleVoice(note, { 0, 2, 3 });
    } else if (note->voice == Voice::LOWER) {
        voice = findFirstPossibleVoice(note, { 1, 2, 3 });
    } else {
        if (preferVoice0) {
            voice = findFirstPossibleVoice(note, { 0, 1, 2 /* TODO , 3 */ });
        } else {
            voice = findFirstPossibleVoice(note, { 1, 0, 2 /* TODO , 3 */ });
        }
    }
    allocateVoice(note, voice);
}

int VoiceAllocator::voice(const TefNote* const note)
{
    int res { -1 }; // TODO -1 ?
    if (allocations.count(note) > 0) {
        res = allocations[note];
    } else {
        LOGD("no voice allocated for note %p", note);
    }

    LOGN("note %p voice %d res %d", note, static_cast<int>(note->voice), res);
    return res;
}
} // namespace mu::iex::tabledit
