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

#ifndef __SELECT_H__
#define __SELECT_H__

#include "durationtype.h"
#include "mscore.h"
#include "pitchspelling.h"
#include "types.h"

namespace mu::engraving {
class Score;
class Page;
class System;
class ChordRest;
class EngravingItem;
class Segment;
class Note;
class Measure;
class Chord;

//---------------------------------------------------------
//   ElementPattern
//---------------------------------------------------------

struct ElementPattern {
    std::vector<EngravingItem*> el;
    int type = 0;
    int subtype = 0;
    staff_idx_t staffStart = 0;
    staff_idx_t staffEnd = 0;   // exclusive
    voice_idx_t voice = 0;
    const System* system = nullptr;
    bool subtypeValid = false;
    Fraction durationTicks;
    Fraction beat { 0, 0 };
    const Measure* measure = nullptr;
};

//---------------------------------------------------------
//   NotePattern
//---------------------------------------------------------

struct NotePattern {
    std::list<Note*> el;
    int pitch = -1;
    int string = INVALID_STRING_INDEX;
    int tpc = Tpc::TPC_INVALID;
    NoteHeadGroup notehead = NoteHeadGroup::HEAD_INVALID;
    TDuration durationType = TDuration();
    Fraction durationTicks;
    NoteType type = NoteType::INVALID;
    staff_idx_t staffStart;
    staff_idx_t staffEnd;   // exclusive
    voice_idx_t voice;
    Fraction beat { 0, 0 };
    const Measure* measure = nullptr;
    const System* system = nullptr;
};

//---------------------------------------------------------
//   SelState
//---------------------------------------------------------

enum class SelState : char {
    NONE,     // nothing is selected
    LIST,     // disjoint selection
    RANGE,    // adjacent selection, a range in one or more staves
    // is selected
};

//---------------------------------------------------------
//   SelectionFilterType
//---------------------------------------------------------

static constexpr size_t NUMBER_OF_SELECTION_FILTER_TYPES = 23;

enum class SelectionFilterType : unsigned int {
    NONE                    = 0,
    FIRST_VOICE             = 1 << 0,
    SECOND_VOICE            = 1 << 1,
    THIRD_VOICE             = 1 << 2,
    FOURTH_VOICE            = 1 << 3,
    DYNAMIC                 = 1 << 4,
    HAIRPIN                 = 1 << 5,
    FINGERING               = 1 << 6,
    LYRICS                  = 1 << 7,
    CHORD_SYMBOL            = 1 << 8,
    OTHER_TEXT              = 1 << 9,
    ARTICULATION            = 1 << 10,
    ORNAMENT                = 1 << 11,
    SLUR                    = 1 << 12,
    FIGURED_BASS            = 1 << 13,
    OTTAVA                  = 1 << 14,
    PEDAL_LINE              = 1 << 15,
    OTHER_LINE              = 1 << 16,
    ARPEGGIO                = 1 << 17,
    GLISSANDO               = 1 << 18,
    FRET_DIAGRAM            = 1 << 19,
    BREATH                  = 1 << 20,
    TREMOLO                 = 1 << 21,
    GRACE_NOTE              = 1 << 22,
    ALL                     = ~(~0u << NUMBER_OF_SELECTION_FILTER_TYPES)
};

//---------------------------------------------------------
//   SelectionFilter
//---------------------------------------------------------

class SelectionFilter
{
public:
    SelectionFilter() = default;
    SelectionFilter(SelectionFilterType type);

    int filteredTypes() const;
    bool isFiltered(SelectionFilterType type) const;
    void setFiltered(SelectionFilterType type, bool filtered);

    bool canSelect(const EngravingItem* element) const;
    bool canSelectVoice(track_idx_t track) const;

private:
    unsigned int m_filteredTypes = static_cast<unsigned int>(SelectionFilterType::ALL);
};

//-------------------------------------------------------------------
//   Selection
//    For SelState::LIST state only visible elements can be selected
//    (no Chord element etc.).
//-------------------------------------------------------------------

class Selection
{
    Score* _score;
    SelState _state;
    std::vector<EngravingItem*> _el;            // valid in mode SelState::LIST

    staff_idx_t _staffStart = 0;            // valid if selState is SelState::RANGE
    staff_idx_t _staffEnd = 0;
    Segment* _startSegment = nullptr;
    Segment* _endSegment = nullptr; // next segment after selection

    Fraction _plannedTick1 { -1, 1 };   // Will be actually selected on updateSelectedElements() call.
    Fraction _plannedTick2 { -1, 1 };   // Used by setRangeTicks() to restore proper selection after
    // command end in case some changes are expected to segments'
    // structure (e.g. MMRests reconstruction).

    Segment* _activeSegment = nullptr;
    track_idx_t _activeTrack = 0;

    Fraction _currentTick;    // tracks the most recent selection
    track_idx_t _currentTrack = 0;

    String _lockReason;

    mu::ByteArray staffMimeData() const;
    mu::ByteArray symbolListMimeData() const;
    SelectionFilter selectionFilter() const;
    bool canSelect(EngravingItem* e) const { return selectionFilter().canSelect(e); }
    bool canSelectVoice(track_idx_t track) const { return selectionFilter().canSelectVoice(track); }
    void appendFiltered(EngravingItem* e);
    void appendChord(Chord* chord);

public:
    Selection() { _score = 0; _state = SelState::NONE; }
    Selection(Score*);
    Score* score() const { return _score; }
    SelState state() const { return _state; }
    bool isNone() const { return _state == SelState::NONE; }
    bool isRange() const { return _state == SelState::RANGE; }
    bool isList() const { return _state == SelState::LIST; }
    void setState(SelState s);

    //! NOTE If locked, the selected items should not be changed.
    void lock(const String& reason) { _lockReason = reason; }
    void unlock(const String& /*reason*/) { _lockReason.clear(); }    // reason for clarity
    bool isLocked() const { return !_lockReason.isEmpty(); }
    const String& lockReason() const { return _lockReason; }

    const std::vector<EngravingItem*>& elements() const { return _el; }
    std::vector<EngravingItem*> elements(ElementType type) const;
    std::vector<Note*> noteList(track_idx_t track = mu::nidx) const;

    const std::list<EngravingItem*> uniqueElements() const;
    std::list<Note*> uniqueNotes(track_idx_t track = mu::nidx) const;

    bool isSingle() const { return (_state == SelState::LIST) && (_el.size() == 1); }

    void add(EngravingItem*);
    void deselectAll();
    void remove(EngravingItem*);
    void clear();
    EngravingItem* element() const;
    ChordRest* cr() const;
    Segment* firstChordRestSegment() const;
    ChordRest* firstChordRest(track_idx_t track = mu::nidx) const;
    ChordRest* lastChordRest(track_idx_t track = mu::nidx) const;
    Measure* findMeasure() const;
    void update();
    void updateState();
    void dump();
    String mimeType() const;
    mu::ByteArray mimeData() const;

    Segment* startSegment() const { return _startSegment; }
    Segment* endSegment() const { return _endSegment; }
    void setStartSegment(Segment* s) { _startSegment = s; }
    void setEndSegment(Segment* s) { _endSegment = s; }
    void setRange(Segment* startSegment, Segment* endSegment, staff_idx_t staffStart, staff_idx_t staffEnd);
    void setRangeTicks(const Fraction& tick1, const Fraction& tick2, staff_idx_t staffStart, staff_idx_t staffEnd);
    Segment* activeSegment() const { return _activeSegment; }
    void setActiveSegment(Segment* s) { _activeSegment = s; }
    ChordRest* activeCR() const;
    bool isStartActive() const;
    bool isEndActive() const;
    ChordRest* currentCR() const;
    Fraction tickStart() const;
    Fraction tickEnd() const;
    staff_idx_t staffStart() const { return _staffStart; }
    staff_idx_t staffEnd() const { return _staffEnd; }
    track_idx_t activeTrack() const { return _activeTrack; }
    void setStaffStart(int v) { _staffStart = v; }
    void setStaffEnd(int v) { _staffEnd = v; }
    void setActiveTrack(track_idx_t v) { _activeTrack = v; }
    bool canCopy() const;
    void updateSelectedElements();
    bool measureRange(Measure** m1, Measure** m2) const;
    void extendRangeSelection(ChordRest* cr);
    void extendRangeSelection(Segment* seg, Segment* segAfter, staff_idx_t staffIdx, const Fraction& tick, const Fraction& etick);
};
} // namespace mu::engraving
#endif
