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

#include "mscore.h"
#include "durationtype.h"
#include "beam.h"

namespace Ms {
class Element;
class Slur;
class ChordRest;
class Drumset;
class Segment;
class Score;
class Selection;

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
    TDuration _duration    { TDuration::DurationType::V_INVALID };      // currently duration
    int _drumNote    { -1 };
    int _track       { 0 };
    int _prevTrack   { 0 };                                 // used for navigation
    Segment* _lastSegment { 0 };
    Segment* _segment     { 0 };                            // current segment
    int _string      { VISUAL_STRING_NONE };                // visual string selected for input (TAB staves only)
    bool _rest               { false };                // rest mode
    NoteType _noteType       { NoteType::NORMAL };
    Beam::Mode _beamMode       { Beam::Mode::AUTO };
    bool _noteEntryMode      { false };
    NoteEntryMethod _noteEntryMethod { NoteEntryMethod::STEPTIME };
    AccidentalType _accidentalType { AccidentalType::NONE };
    Slur* _slur              { 0 };
    bool _insertMode         { false };

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

    int voice() const { return _track == -1 ? 0 : (_track % VOICES); }
    void setVoice(int v) { setTrack((_track / VOICES) * VOICES + v); }
    int track() const { return _track; }
    void setTrack(int v) { _prevTrack = _track; _track = v; }
    int prevTrack() const { return _prevTrack; }

    int string() const { return _string; }
    void setString(int val) { _string = val; }

    StaffGroup staffGroup() const;

    bool rest() const { return _rest; }
    void setRest(bool v) { _rest = v; }

    NoteType noteType() const { return _noteType; }
    void setNoteType(NoteType t) { _noteType = t; }

    Beam::Mode beamMode() const { return _beamMode; }
    void setBeamMode(Beam::Mode m) { _beamMode = m; }

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

    bool insertMode() const { return _insertMode; }
    void setInsertMode(bool val) { _insertMode = val; }

    void update(Selection& selection);
    void moveInputPos(Element* e);
    void moveToNextInputPos();
    bool endOfScore() const;

    // TODO: unify with Selection::cr()?
    static Note* note(Element*);
    static ChordRest* chordRest(Element*);
};
}     // namespace Ms
#endif
