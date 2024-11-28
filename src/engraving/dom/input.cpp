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

#include "input.h"

#include "articulation.h"
#include "chord.h"
#include "durationtype.h"
#include "hook.h"
#include "measure.h"
#include "note.h"
#include "part.h"
#include "rest.h"
#include "score.h"
#include "segment.h"
#include "select.h"
#include "staff.h"
#include "stem.h"

using namespace mu;

namespace mu::engraving {
class DrumSet;

bool InputState::isValid() const
{
    return m_segment != nullptr && m_track != muse::nidx;
}

//---------------------------------------------------------
//   drumset
//---------------------------------------------------------

Drumset* InputState::drumset() const
{
    if (!m_segment || m_track == muse::nidx) {
        return nullptr;
    }

    const Staff* staff = m_segment->score()->staff(m_track / VOICES);
    if (!staff) {
        return nullptr;
    }

    return staff->part()->instrument(m_segment->tick())->drumset();
}

//---------------------------------------------------------
//   staffGroup
//---------------------------------------------------------

StaffGroup InputState::staffGroup() const
{
    if (!m_segment || m_track == muse::nidx) {
        return StaffGroup::STANDARD;
    }

    Fraction tick = m_segment->tick();
    const Staff* staff = m_segment->score()->staff(m_track / VOICES);
    if (!staff) {
        return StaffGroup::STANDARD;
    }

    StaffGroup staffGroup = staff->staffType(tick)->group();
    const Instrument* instrument = staff->part()->instrument(tick);

    // if not tab, pitched/unpitched input depends on instrument, not staff (override StaffGroup)
    if (staffGroup != StaffGroup::TAB) {
        staffGroup = instrument->useDrumset() ? StaffGroup::PERCUSSION : StaffGroup::STANDARD;
    }

    return staffGroup;
}

//---------------------------------------------------------
//   tick
//---------------------------------------------------------

Fraction InputState::tick() const
{
    return m_segment ? m_segment->tick() : Fraction(0, 1);
}

//---------------------------------------------------------
//   cr
//---------------------------------------------------------

ChordRest* InputState::cr() const
{
    // _track could potentially be invalid, for instance after navigation through a frame
    return m_segment && m_track != muse::nidx ? toChordRest(m_segment->element(m_track)) : 0;
}

//---------------------------------------------------------
//   setDots
//---------------------------------------------------------

void InputState::setDots(int n)
{
    if (n && (!m_duration.isValid() || m_duration.isZero() || m_duration.isMeasure())) {
        m_duration = DurationType::V_QUARTER;
    }
    m_duration.setDots(n);
}

void InputState::setVoice(voice_idx_t v)
{
    if (v >= VOICES || m_track == muse::nidx) {
        return;
    }

    setTrack((m_track / VOICES) * VOICES + v);
}

//---------------------------------------------------------
//   note
//---------------------------------------------------------

Note* InputState::note(EngravingItem* e)
{
    return e && e->isNote() ? toNote(e) : nullptr;
}

//---------------------------------------------------------
//   chordRest
//---------------------------------------------------------

ChordRest* InputState::chordRest(EngravingItem* e)
{
    if (!e) {
        return nullptr;
    }
    if (e->isChordRest()) {
        return toChordRest(e);
    }
    if (e->isNote()) {
        return toNote(e)->chord();
    }
    if (e->isStem()) {
        return toStem(e)->chord();
    }
    if (e->isHook()) {
        return toHook(e)->chord();
    }
    return nullptr;
}

//---------------------------------------------------------
//   update
//---------------------------------------------------------

void InputState::update(Selection& selection)
{
    setDuration(DurationType::V_INVALID);
    setRest(false);
    setAccidentalType(AccidentalType::NONE);
    Note* n1 = nullptr;
    ChordRest* cr1 = nullptr;
    bool differentAccidentals = false;
    bool differentDurations = false;
    bool chordsAndRests = false;

    std::set<SymId> articulationSymbolIds;
    for (EngravingItem* e : selection.elements()) {
        if (Note* n = note(e)) {
            if (n1) {
                if (n->accidentalType() != n1->accidentalType()) {
                    setAccidentalType(AccidentalType::NONE);
                    differentAccidentals = true;
                }

                std::set<SymId> articulationsIds;
                for (Articulation* artic: n->chord()->articulations()) {
                    articulationsIds.insert(artic->symId());
                }

                articulationsIds = mu::engraving::splitArticulations(articulationsIds);
                articulationsIds = mu::engraving::flipArticulations(articulationsIds, PlacementV::ABOVE);
                for (const SymId& articulationSymbolId: articulationsIds) {
                    if (std::find(articulationSymbolIds.begin(), articulationSymbolIds.end(),
                                  articulationSymbolId) == articulationSymbolIds.end()) {
                        articulationSymbolIds.erase(articulationSymbolId);
                    }
                }
            } else {
                setAccidentalType(n->accidentalType());

                std::set<SymId> articulationsIds;
                for (Articulation* artic: n->chord()->articulations()) {
                    if (!artic->isOrnament()) {
                        articulationsIds.insert(artic->symId());
                    }
                }
                articulationSymbolIds = mu::engraving::flipArticulations(articulationsIds, PlacementV::ABOVE);

                n1 = n;
            }
        }

        if (ChordRest* cr = chordRest(e)) {
            if (cr1) {
                if (cr->durationType() != cr1->durationType()) {
                    setDuration(DurationType::V_INVALID);
                    differentDurations = true;
                }
                if ((cr->isRest() && !cr1->isRest()) || (!cr->isRest() && cr1->isRest())) {
                    setRest(false);
                    chordsAndRests = true;
                }
            } else {
                setDuration(cr->durationType());
                setRest(cr->isRest());
                cr1 = cr;
            }
        }

        if (differentAccidentals && differentDurations && chordsAndRests) {
            break;
        }
    }

    setArticulationIds(joinArticulations(articulationSymbolIds));

    EngravingItem* e = selection.element();
    if (!e) {
        if (!selection.isNone()) {
            setTrack(selection.activeTrack());
            setSegment(selection.startSegment());
        }
        return;
    }

    ChordRest* cr = chordRest(e);
    Note* n = note(e);
    if (!n && cr && cr->isChord()) {
        n = toChord(cr)->upNote();
    }

    if (cr) {
        setTrack(cr->track());
        setNoteType(n ? n->noteType() : NoteType::NORMAL);
        setBeamMode(cr->beamMode());
    }

    setDrumNote(-1);
    if (n) {
        const Instrument* instr = n->part()->instrument(n->tick());
        if (instr->useDrumset()) {
            setDrumNote(n->pitch());
        }
    }
}

//---------------------------------------------------------
//   moveInputPos
//---------------------------------------------------------

void InputState::moveInputPos(EngravingItem* e)
{
    if (e == 0) {
        return;
    }

    Segment* s;
    if (e->isChordRest()) {
        s = toChordRest(e)->segment();
    } else {
        s = toSegment(e);
    }

    if (s->isSegment()) {
        if (s->measure()->isMMRest()) {
            Measure* m = s->measure()->mmRestFirst();
            s = m->findSegment(SegmentType::ChordRest, m->tick());
        }
        m_lastSegment = m_segment;
        m_segment = s;
    }
}

//---------------------------------------------------------
//   setSegment
//---------------------------------------------------------

void InputState::setSegment(Segment* s)
{
    if (s && s->measure()->isMMRest()) {
        Measure* m = s->measure()->mmRestFirst();
        s = m->findSegment(SegmentType::ChordRest, m->tick());
    }
    m_segment = s;
    m_lastSegment = s;
}

//---------------------------------------------------------
//   nextInputPos
//---------------------------------------------------------

Segment* InputState::nextInputPos() const
{
    Measure* m = m_segment->measure();
    Segment* s = m_segment->next1(SegmentType::ChordRest);
    while (s) {
        if (s->element(m_track) || s->measure() != m) {
            return s;
        }
        s = s->next1(SegmentType::ChordRest);
    }
    return 0;
}

//---------------------------------------------------------
//   moveToNextInputPos
//   TODO: special case: note is first note of tie: goto to last note of tie
//---------------------------------------------------------

void InputState::moveToNextInputPos()
{
    Segment* s   = nextInputPos();
    m_lastSegment = m_segment;
    if (s) {
        m_segment = s;
    }
}

//---------------------------------------------------------
//   endOfScore
//---------------------------------------------------------

bool InputState::endOfScore() const
{
    return (m_lastSegment == m_segment) && !nextInputPos();
}
}
