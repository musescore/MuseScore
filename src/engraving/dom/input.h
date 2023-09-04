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

#ifndef __INPUT_H__
#define __INPUT_H__

#include <set>

#include "durationtype.h"
#include "mscore.h"
#include "types.h"

#include "types/types.h"

namespace mu::engraving {
class ChordRest;
class Drumset;
class EngravingItem;
class Note;
class Score;
class Segment;
class Selection;
class Slur;

// no ordinal for the visual repres. of string
// (topmost in TAB varies according to visual order and presence of bass strings)
static constexpr int VISUAL_INVALID_STRING_INDEX = -100;

//---------------------------------------------------------
//   NoteEntryMethod
//---------------------------------------------------------

enum class NoteEntryMethod : char {
    UNKNOWN, STEPTIME, REPITCH, RHYTHM, REALTIME_AUTO, REALTIME_MANUAL, TIMEWISE
};

//---------------------------------------------------------
//   InputState
//---------------------------------------------------------

class InputState
{
    track_idx_t _track     { 0 };
    track_idx_t _prevTrack { 0 }; // used for navigation

    int _drumNote { -1 };
    int _string   { VISUAL_INVALID_STRING_INDEX }; // visual string selected for input (TAB staves only)

    Segment* _lastSegment = nullptr;
    Segment* _segment = nullptr; // current segment

    bool _noteEntryMode { false };
    NoteEntryMethod _noteEntryMethod { NoteEntryMethod::STEPTIME };

    TDuration _duration { DurationType::V_INVALID }; // currently duration
    bool _rest { false }; // rest mode

    NoteType _noteType { NoteType::NORMAL };
    BeamMode _beamMode { BeamMode::AUTO };

    AccidentalType _accidentalType { AccidentalType::NONE };
    Slur* _slur = nullptr;

    std::set<SymId> _articulationIds;

    Segment* nextInputPos() const;

public:
    ChordRest* cr() const;

    Fraction tick() const;

    void setDuration(const TDuration& d) { _duration = d; }
    TDuration duration() const { return _duration; }
    void setDots(int n);
    Fraction ticks() const { return _duration.ticks(); }

    Segment* segment() const { return _segment; }
    void setSegment(Segment* s);

    Segment* lastSegment() const { return _lastSegment; }
    void setLastSegment(Segment* s) { _lastSegment = s; }

    const Drumset* drumset() const;

    int drumNote() const { return _drumNote; }
    void setDrumNote(int v) { _drumNote = v; }

    voice_idx_t voice() const { return _track == mu::nidx ? 0 : (_track % VOICES); }
    void setVoice(voice_idx_t v) { setTrack((_track / VOICES) * VOICES + v); }
    track_idx_t track() const { return _track; }
    void setTrack(track_idx_t v) { _prevTrack = _track; _track = v; }
    track_idx_t prevTrack() const { return _prevTrack; }

    int string() const { return _string; }
    void setString(int val) { _string = val; }

    StaffGroup staffGroup() const;

    bool rest() const { return _rest; }
    void setRest(bool v) { _rest = v; }

    NoteType noteType() const { return _noteType; }
    void setNoteType(NoteType t) { _noteType = t; }

    BeamMode beamMode() const { return _beamMode; }
    void setBeamMode(BeamMode m) { _beamMode = m; }

    bool noteEntryMode() const { return _noteEntryMode; }
    void setNoteEntryMode(bool v) { _noteEntryMode = v; }

    NoteEntryMethod noteEntryMethod() const { return _noteEntryMethod; }
    void setNoteEntryMethod(NoteEntryMethod m) { _noteEntryMethod = m; }
    bool usingNoteEntryMethod(NoteEntryMethod m) const { return m == noteEntryMethod(); }

    AccidentalType accidentalType() const { return _accidentalType; }
    void setAccidentalType(AccidentalType val) { _accidentalType = val; }

    std::set<SymId> articulationIds() const { return _articulationIds; }
    void setArticulationIds(const std::set<SymId>& ids) { _articulationIds = ids; }

    Slur* slur() const { return _slur; }
    void setSlur(Slur* s) { _slur = s; }

    void update(Selection& selection);
    void moveInputPos(EngravingItem* e);
    void moveToNextInputPos();
    bool endOfScore() const;

    // TODO: unify with Selection::cr()?
    static Note* note(EngravingItem*);
    static ChordRest* chordRest(EngravingItem*);
};
} // namespace mu::engraving
#endif
