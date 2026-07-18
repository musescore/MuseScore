/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2026 MuseScore Limited
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 3 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <https://www.gnu.org/licenses/>.
 */

#pragma once

#include <set>
#include <vector>
#include <utility>

#include "global/types/ret.h"
#include "draw/types/geometry.h"

#include "../types/types.h"

namespace mu::engraving {
class Chord;
class ChordRest;
class Fraction;
class InputState;
class Note;
class Score;
class Segment;
class Transaction;

struct NoteVal;
struct Position;

enum class AccidentalType : unsigned char;
enum class SymId;

struct NoteInputParams {
    int step = 0;
    int drumPitch = -1;
};

enum class Pad : char {
    NOTE00,
    NOTE0,
    NOTE1,
    NOTE2,
    NOTE4,
    NOTE8,
    NOTE16,
    NOTE32,
    NOTE64,
    NOTE128,
    NOTE256,
    NOTE512,
    NOTE1024,
    //--------------------
    REST,
    DOT,
    DOT2,
    DOT3,
    DOT4
};

class NoteInput
{
public:
    //! NOTE: Methods related to entering notes/rests into the score.

    static bool resolveNoteInputParams(const Score* score, int note, bool addFlag, NoteInputParams& out);

    static NoteVal noteVal(const Score* score, int pitch, staff_idx_t staffIdx, bool allowTransposition);
    static NoteVal noteValForPosition(const Score* score, Position pos, AccidentalType at, bool& error);

    static Note* addNote(Transaction& tx, Score* score, Chord* chord, const NoteVal& noteVal, bool forceAccidental = false,
                         const std::set<SymId>& articulationIds = {}, InputState* externalInputState = nullptr);
    static Note* addNoteToTiedChord(Transaction& tx, Score* score, Chord* chord, const NoteVal& noteVal, bool forceAccidental = false,
                                    const std::set<SymId>& articulationIds = {});

    static Note* addPitch(Transaction& tx, Score* score, NoteVal& nval, bool addFlag, InputState* externalInputState = nullptr);
    static Note* addPitchToChord(Transaction& tx, Score* score, NoteVal& nval, Chord* chord, InputState* externalInputState = nullptr,
                                 bool forceAccidental = false);

    static Note* addMidiPitch(Transaction& tx, Score* score, int pitch, bool addFlag, bool allowTransposition);
    static Note* addTiedMidiPitch(Transaction& tx, Score* score, int pitch, bool addFlag, Chord* prevChord, bool allowTransposition);

    static std::pair<Note*, Note*> repitchReplaceNote(Transaction& tx, Score* score, Chord* chord, const NoteVal& nval,
                                                      bool forceAccidental = false);

    static muse::Ret putNote(Transaction& tx, Score* score, const muse::PointF& pos, bool replace, bool insert);
    static muse::Ret putNote(Transaction& tx, Score* score, const Position& pos, bool replace);
    static muse::Ret repitchNote(Transaction& tx, Score* score, const Position& pos, bool replace);
    static muse::Ret insertChordByInsertingTime(Transaction& tx, Score* score, const Position& pos);

    static void truncateChordRest(Transaction& tx, Score* score, ChordRest* cr, const Fraction& tick, bool fillWithRest);

    static void nextInputPos(Transaction& tx, Score* score, const ChordRest* cr, bool doSelect);

    // Command entry points
    static void addPitch(Transaction& tx, Score* score, const NoteInputParams& params, bool addFlag, bool insert);
    static void addPitch(Transaction& tx, Score* score, int step, bool addFlag, bool insert);
    static void addFret(Transaction& tx, Score* score, int fret);

    static void padToggle(Transaction& tx, Score* score, Pad p, bool toggleForSelectionOnly = false);
    static void padNoteIncreaseTAB(Transaction& tx, Score* score);
    static void padNoteDecreaseTAB(Transaction& tx, Score* score);

    static void realtimeAdvance(Transaction& tx, Score* score, bool allowTransposition, const std::vector<int>& activeMidiPitches);
};
}
