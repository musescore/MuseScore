/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2021 MuseScore Limited and others
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

#ifndef MU_ENGRAVING_INPUT_H
#define MU_ENGRAVING_INPUT_H

#include <optional>
#include <set>

#include "accidental.h"
#include "chord.h"
#include "durationtype.h"
#include "noteval.h"

#include "../types/types.h"

namespace mu::engraving {
class ChordRest;
class Drumset;
class EngravingItem;
class Note;
class Score;
class Segment;
class Selection;
class Slur;
class Staff;
enum class TupletNumberType : unsigned char;
enum class TupletBracketType : unsigned char;

// no ordinal for the visual repres. of string
// (topmost in TAB varies according to visual order and presence of bass strings)
static constexpr int VISUAL_INVALID_STRING_INDEX = -100;

//---------------------------------------------------------
//   NoteEntryMethod
//---------------------------------------------------------

enum class NoteEntryMethod : char {
    UNKNOWN, DEFAULT, BY_NOTE_NAME, BY_DURATION, REPITCH, RHYTHM, REALTIME_AUTO, REALTIME_MANUAL, TIMEWISE
};

//---------------------------------------------------------
//   PendingTuplet
//    A tuplet waiting at the input position for the next note
//    or rest. The next duration sizes it in By duration and
//    Rhythm input; the input duration sizes it when it is
//    asked for in By note name and Insert input.
//---------------------------------------------------------

struct PendingTuplet {
    Fraction ratio;
    TupletNumberType numberType {};
    TupletBracketType bracketType {};
    bool autoBaseLen = false;
    // the duration is the subdivision, rather than the length divided
    bool durationAsSubdivision = false;
    // By note name and Insert: the duration that sized it
    std::optional<TDuration> duration;
    // given when it starts waiting, see InputState::forgetPendingTuplets()
    int number = 0;

    Fraction tick;
    track_idx_t track = muse::nidx;
    bool beyondScore = false;
};

//---------------------------------------------------------
//   InputState
//---------------------------------------------------------

class InputState
{
public:
    bool isValid() const;

    ChordRest* cr() const;

    Fraction tick() const;

    void setDuration(const TDuration& d) { m_duration = d; }
    TDuration duration() const { return m_duration; }
    void setDots(int n);
    Fraction ticks() const { return m_duration.ticks(); }

    Segment* segment() const { return m_segment; }
    void setSegment(Segment* s);

    Segment* lastSegment() const { return m_lastSegment; }
    void setLastSegment(Segment* s) { m_lastSegment = s; }

    Staff* staff() const;
    staff_idx_t staffIdx() const;

    Drumset* drumset() const;

    int drumNote() const { return m_drumNote; }
    void setDrumNote(int v) { m_drumNote = v; }

    // Used in the input-by-duration mode
    const NoteValList& notes() const { return m_notes; }
    void setNotes(const NoteValList& v) { m_notes = v; }

    voice_idx_t voice() const { return m_track == muse::nidx ? 0 : (m_track % MAX_VOICES); }
    void setVoice(voice_idx_t v);
    track_idx_t track() const { return m_track; }
    void setTrack(track_idx_t v) { m_prevTrack = m_track; m_track = v; }
    track_idx_t prevTrack() const { return m_prevTrack; }

    int string() const { return m_string; }
    void setString(int val) { m_string = val; }

    StaffGroup staffGroup() const;

    bool rest() const { return m_rest; }
    void setRest(bool v) { m_rest = v; }

    NoteType noteType() const { return m_noteType; }
    void setNoteType(NoteType t) { m_noteType = t; }

    BeamMode beamMode() const { return m_beamMode; }
    void setBeamMode(BeamMode m) { m_beamMode = m; }

    bool noteEntryMode() const { return m_noteEntryMode; }
    void setNoteEntryMode(bool v);

    NoteEntryMethod noteEntryMethod() const { return m_noteEntryMethod; }
    void setNoteEntryMethod(NoteEntryMethod m);
    bool usingNoteEntryMethod(NoteEntryMethod m) const { return m == noteEntryMethod(); }

    AccidentalType accidentalType() const { return m_accidentalType; }
    void setAccidentalType(AccidentalType val) { m_accidentalType = val; }

    const std::set<SymId>& articulationIds() const { return m_articulationIds; }
    void setArticulationIds(const std::set<SymId>& ids) { m_articulationIds = ids; }

    Slur* slur() const { return m_slur; }
    void setSlur(Slur* s) { m_slur = s; }

    const std::optional<PendingTuplet>& pendingTuplet() const { return m_pendingTuplet; }
    void setPendingTuplet(const std::optional<PendingTuplet>& t);
    bool pendingTupletHere() const;
    void movePendingTupletHere();
    void moveToDrumVoice(voice_idx_t v);
    std::optional<voice_idx_t> otherDrumVoice() const;
    void forgetPendingTuplets();
    void restoreFromUndoStack(const InputState& state);
    void keepPendingTupletFrom(const InputState& state);
    void setInsertedTuplet(const Tuplet* tuplet);
    bool inInsertedTuplet(const ChordRest* cr) const;

    void update(Selection& selection);
    void moveInputPos(EngravingItem* e);
    void moveToNextInputPos();

    bool beyondScore() const { return m_beyondScore; }
    void setBeyondScore(bool val) { m_beyondScore = val; }

    // TODO: unify with Selection::cr()?
    static Note* note(EngravingItem*);
    static ChordRest* chordRest(EngravingItem*);

private:

    Segment* nextInputPos() const;

    track_idx_t m_track = 0;
    track_idx_t m_prevTrack = 0; // used for navigation

    int m_drumNote = -1;
    int m_string = VISUAL_INVALID_STRING_INDEX; // visual string selected for input (TAB staves only)

    NoteValList m_notes;

    Segment* m_lastSegment = nullptr;
    Segment* m_segment = nullptr; // current segment

    bool m_noteEntryMode = false;
    NoteEntryMethod m_noteEntryMethod = NoteEntryMethod::BY_NOTE_NAME;

    TDuration m_duration = DurationType::V_INVALID; // currently duration
    bool m_rest = false; // rest mode

    NoteType m_noteType = NoteType::NORMAL;
    BeamMode m_beamMode = BeamMode::AUTO;

    AccidentalType m_accidentalType = AccidentalType::NONE;
    Slur* m_slur = nullptr;

    std::set<SymId> m_articulationIds;

    bool m_beyondScore = false;

    std::optional<PendingTuplet> m_pendingTuplet;
    // Insert: the start of the tuplet a waiting tuplet inserted
    std::optional<std::pair<Fraction, track_idx_t> > m_insertedTuplet;

    // kept through undo and redo: the last number given to a waiting tuplet, and the last one cancelled
    int m_pendingTupletNumber = 0;
    int m_cancelledPendingTupletNumber = 0;
};
} // namespace mu::engraving
#endif
