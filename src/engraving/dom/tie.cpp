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
#include "tie.h"

#include <cmath>

#include "draw/types/transform.h"

#include "accidental.h"
#include "chord.h"
#include "hook.h"
#include "ledgerline.h"
#include "measure.h"
#include "mscoreview.h"
#include "note.h"
#include "notedot.h"
#include "score.h"
#include "staff.h"
#include "stafftype.h"
#include "stem.h"
#include "system.h"

#include "log.h"

using namespace mu;
using namespace mu::draw;

namespace mu::engraving {
Note* Tie::editStartNote;
Note* Tie::editEndNote;

TieSegment::TieSegment(System* parent)
    : SlurTieSegment(ElementType::TIE_SEGMENT, parent)
{
}

TieSegment::TieSegment(const TieSegment& s)
    : SlurTieSegment(s)
{
}

bool TieSegment::isEditAllowed(EditData& ed) const
{
    if (ed.key == Key_Home && !ed.modifiers) {
        return true;
    }

    return false;
}

//---------------------------------------------------------
//   edit
//    return true if event is accepted
//---------------------------------------------------------

bool TieSegment::edit(EditData& ed)
{
    if (!isEditAllowed(ed)) {
        return false;
    }

    SlurTie* sl = tie();

    if (ed.key == Key_Home && !ed.modifiers) {
        if (ed.hasCurrentGrip()) {
            ups(ed.curGrip).off = PointF();
            renderer()->layoutItem(sl);
            triggerLayout();
        }
        return true;
    }
    return false;
}

//---------------------------------------------------------
//   changeAnchor
//---------------------------------------------------------

void TieSegment::changeAnchor(EditData& ed, EngravingItem* element)
{
    if (ed.curGrip == Grip::START) {
        spanner()->setStartElement(element);
        Note* note = toNote(element);
        if (note->chord()->tick() <= tie()->endNote()->chord()->tick()) {
            tie()->startNote()->setTieFor(0);
            tie()->setStartNote(note);
            note->setTieFor(tie());
        }
    } else {
        spanner()->setEndElement(element);
        Note* note = toNote(element);
        // do not allow backward ties
        if (note->chord()->tick() >= tie()->startNote()->chord()->tick()) {
            tie()->endNote()->setTieBack(0);
            tie()->setEndNote(note);
            note->setTieBack(tie());
        }
    }

    const size_t segments  = spanner()->spannerSegments().size();
    ups(ed.curGrip).off = PointF();
    renderer()->layoutItem(spanner());
    if (spanner()->spannerSegments().size() != segments) {
        const std::vector<SpannerSegment*>& ss = spanner()->spannerSegments();

        TieSegment* newSegment = toTieSegment(ed.curGrip == Grip::END ? ss.back() : ss.front());
        score()->endCmd();
        score()->startCmd();
        ed.view()->changeEditElement(newSegment);
        triggerLayout();
    }
}

//---------------------------------------------------------
//   editDrag
//---------------------------------------------------------

void TieSegment::editDrag(EditData& ed)
{
    consolidateAdjustmentOffsetIntoUserOffset();
    Grip g = ed.curGrip;
    ups(g).off += ed.delta;

    if (g == Grip::START || g == Grip::END) {
        renderer()->computeBezier(this);
        //
        // move anchor for slurs/ties
        //
        if ((g == Grip::START && isSingleBeginType()) || (g == Grip::END && isSingleEndType())) {
            Spanner* spanner = tie();
            EngravingItem* e = ed.view()->elementNear(ed.pos);
            Note* note = (e && e->isNote()) ? toNote(e) : nullptr;
            if (note && ((g == Grip::END && note->tick() > tie()->tick()) || (g == Grip::START && note->tick() < tie()->tick2()))) {
                if (g == Grip::END) {
                    Tie* tie = toTie(spanner);
                    if (tie->startNote()->pitch() == note->pitch()
                        && tie->startNote()->chord()->tick() < note->chord()->tick()) {
                        ed.view()->setDropTarget(note);
                        if (note != tie->endNote()) {
                            changeAnchor(ed, note);
                            return;
                        }
                    }
                }
            } else {
                ed.view()->setDropTarget(0);
            }
        }
    } else if (g == Grip::BEZIER1 || g == Grip::BEZIER2) {
        renderer()->computeBezier(this);
    } else if (g == Grip::SHOULDER) {
        ups(g).off = PointF();
        ups(Grip::BEZIER1).off += ed.delta;
        ups(Grip::BEZIER2).off += ed.delta;
        renderer()->computeBezier(this);
    } else if (g == Grip::DRAG) {
        ups(Grip::DRAG).off = PointF();
        roffset() += ed.delta;
    }
}

void TieSegment::consolidateAdjustmentOffsetIntoUserOffset()
{
    for (size_t i = 0; i < m_adjustmentOffsets.size(); ++i) {
        Grip grip = static_cast<Grip>(i);
        PointF adjustOffset = m_adjustmentOffsets[i];
        if (!adjustOffset.isNull()) {
            ups(grip).p -= adjustOffset;
            ups(grip).off = adjustOffset;
        }
    }
    resetAdjustmentOffset();
}

//---------------------------------------------------------
//   isEdited
//---------------------------------------------------------

bool TieSegment::isEdited() const
{
    for (int i = 0; i < int(Grip::GRIPS); ++i) {
        if (!m_ups[i].off.isNull()) {
            return true;
        }
    }
    return false;
}

void TieSegment::addLineAttachPoints()
{
    // Add tie attach point to start and end note
    Note* startNote = tie()->startNote();
    Note* endNote = tie()->endNote();
    if (startNote) {
        startNote->addLineAttachPoint(ups(Grip::START).pos(), tie());
    }
    if (endNote) {
        endNote->addLineAttachPoint(ups(Grip::END).pos(), tie());
    }
}

//---------------------------------------------------------
//   Tie
//---------------------------------------------------------

Tie::Tie(EngravingItem* parent)
    : SlurTie(ElementType::TIE, parent)
{
    setAnchor(Anchor::NOTE);
}

//---------------------------------------------------------
//   calculateDirection
//---------------------------------------------------------

static int compareNotesPos(const Note* n1, const Note* n2)
{
    if (n1->line() != n2->line() && !(n1->staffType()->isTabStaff())) {
        return n2->line() - n1->line();
    } else if (n1->string() != n2->string()) {
        return n2->string() - n1->string();
    } else {
        return n1->pitch() - n2->pitch();
    }
}

void Tie::calculateDirection()
{
    Chord* c1   = startNote()->chord();
    Chord* c2   = endNote()->chord();
    Measure* m1 = c1->measure();
    Measure* m2 = c2->measure();

    if (m_slurDirection == DirectionV::AUTO) {
        std::vector<Note*> notes = c1->notes();
        size_t n = notes.size();
        StaffType* st = staff()->staffType(startNote() ? startNote()->tick() : Fraction(0, 1));
        bool simpleException = st && st->isSimpleTabStaff();
        // if there are multiple voices, the tie direction goes on stem side
        if (m1->hasVoices(c1->staffIdx(), c1->tick(), c1->actualTicks())) {
            m_up = simpleException ? isUpVoice(c1->voice()) : c1->up();
        } else if (m2->hasVoices(c2->staffIdx(), c2->tick(), c2->actualTicks())) {
            m_up = simpleException ? isUpVoice(c2->voice()) : c2->up();
        } else if (n == 1) {
            //
            // single note
            //
            if (c1->up() != c2->up()) {
                // if stem direction is mixed, always up
                m_up = true;
            } else {
                m_up = !c1->up();
            }
        } else {
            //
            // chords
            //
            // first, find pivot point in chord (below which all ties curve down and above which all ties curve up)
            Note* pivotPoint = nullptr;
            bool multiplePivots = false;
            for (size_t i = 0; i < n - 1; ++i) {
                if (!notes[i]->tieFor()) {
                    continue; // don't include notes that don't have ties
                }
                for (size_t j = i + 1; j < n; ++j) {
                    if (!notes[j]->tieFor()) {
                        continue;
                    }
                    int noteDiff = compareNotesPos(notes[i], notes[j]);
                    if (!multiplePivots && std::abs(noteDiff) <= 1) {
                        // TODO: Fix unison ties somehow--if noteDiff == 0 then we need to determine which of the unison is 'lower'
                        if (pivotPoint) {
                            multiplePivots = true;
                            pivotPoint = nullptr;
                        } else {
                            pivotPoint = noteDiff < 0 ? notes[i] : notes[j];
                        }
                    }
                }
            }
            if (!pivotPoint) {
                // if the pivot point was not found (either there are no unisons/seconds or there are more than one),
                // determine if this note is in the lower or upper half of this chord
                int notesAbove = 0, tiesAbove = 0;
                int notesBelow = 0, tiesBelow = 0;
                int unisonTies = 0;
                for (size_t i = 0; i < n; ++i) {
                    if (notes[i] == startNote()) {
                        // skip counting if this note is the current note or if this note doesn't have a tie
                        continue;
                    }
                    int noteDiff = compareNotesPos(startNote(), notes[i]);
                    if (noteDiff == 0) {  // unison
                        if (notes[i]->tieFor()) {
                            unisonTies++;
                        }
                    }
                    if (noteDiff < 0) { // the note is above startNote
                        notesAbove++;
                        if (notes[i]->tieFor()) {
                            tiesAbove++;
                        }
                    }
                    if (noteDiff > 0) { // the note is below startNote
                        notesBelow++;
                        if (notes[i]->tieFor()) {
                            tiesBelow++;
                        }
                    }
                }

                if (tiesAbove == 0 && tiesBelow == 0 && unisonTies == 0) {
                    // this is the only tie in the chord.
                    if (notesAbove == notesBelow) {
                        m_up = !c1->up();
                    } else {
                        m_up = (notesAbove < notesBelow);
                    }
                } else if (tiesAbove == tiesBelow) {
                    // this note is dead center, so its tie should go counter to the stem direction
                    m_up = !c1->up();
                } else {
                    m_up = (tiesAbove < tiesBelow);
                }
            } else if (pivotPoint == startNote()) {
                // the current note is the lower of the only second or unison in the chord; tie goes down.
                m_up = false;
            } else {
                // if lower than the pivot, tie goes down, otherwise up
                int noteDiff = compareNotesPos(startNote(), pivotPoint);
                m_up = (noteDiff >= 0);
            }
        }
    } else {
        m_up = m_slurDirection == DirectionV::UP ? true : false;
    }
}

void Tie::calculateIsInside()
{
    if (_tiePlacement != TiePlacement::AUTO) {
        setIsInside(_tiePlacement == TiePlacement::INSIDE);
        return;
    }

    const Note* startN = startNote();
    const Chord* startChord = startN ? startN->chord() : nullptr;
    const Note* endN = endNote();
    const Chord* endChord = endN ? endN->chord() : nullptr;

    if (!startChord || !endChord) {
        setIsInside(false);
        return;
    }

    const bool startIsSingleNote = startChord->notes().size() <= 1;
    const bool endIsSingleNote = endChord->notes().size() <= 1;

    if (startIsSingleNote && endIsSingleNote) {
        setIsInside(style().styleV(Sid::tiePlacementSingleNote).value<TiePlacement>() == TiePlacement::INSIDE);
    } else {
        setIsInside(style().styleV(Sid::tiePlacementChord).value<TiePlacement>() == TiePlacement::INSIDE);
    }
}

PropertyValue Tie::getProperty(Pid propertyId) const
{
    switch (propertyId) {
    case Pid::TIE_PLACEMENT:
        return tiePlacement();
    default:
        return SlurTie::getProperty(propertyId);
    }
}

PropertyValue Tie::propertyDefault(Pid id) const
{
    switch (id) {
    case Pid::TIE_PLACEMENT:
        return TiePlacement::AUTO;
    default:
        return SlurTie::propertyDefault(id);
    }
}

bool Tie::setProperty(Pid propertyId, const PropertyValue& v)
{
    switch (propertyId) {
    case Pid::TIE_PLACEMENT:
        setTiePlacement(v.value<TiePlacement>());
        break;
    default:
        return SlurTie::setProperty(propertyId, v);
    }
    triggerLayout();
    return true;
}

double Tie::scalingFactor() const
{
    const Note* startN = startNote();
    const Note* endN = endNote();

    if (!startN || !endN) {
        return 1.0;
    }

    if (startN->isGrace()) {
        return style().styleD(Sid::graceNoteMag);
    }

    return 0.5 * (startN->chord()->intrinsicMag() + endN->chord()->intrinsicMag());
}

//---------------------------------------------------------
//   setStartNote
//---------------------------------------------------------

void Tie::setStartNote(Note* note)
{
    setStartElement(note);
    setParent(note);
}

//---------------------------------------------------------
//   startNote
//---------------------------------------------------------

Note* Tie::startNote() const
{
    assert(!startElement() || startElement()->type() == ElementType::NOTE);
    return toNote(startElement());
}

//---------------------------------------------------------
//   endNote
//---------------------------------------------------------

Note* Tie::endNote() const
{
    return toNote(endElement());
}

bool Tie::isOuterTieOfChord(Grip startOrEnd) const
{
    if (m_isInside) {
        return false;
    }

    const bool start = startOrEnd == Grip::START;
    const Note* note = start ? startNote() : endNote();
    if (!note) {
        return false;
    }

    const Chord* chord = note->chord();

    return (note == chord->upNote() && up()) || (note == chord->downNote() && !up());
}

bool Tie::hasTiedSecondInside() const
{
    const Note* note = startNote();
    if (!note) {
        return false;
    }

    const Chord* chord = note->chord();
    const int line = note->line();
    const int secondInsideLine = up() ? line + 1 : line - 1;

    for (const Note* otherNote : chord->notes()) {
        if (otherNote->line() == secondInsideLine && otherNote->tieFor() && otherNote->tieFor()->up() == up()) {
            return true;
        }
    }

    return false;
}

bool Tie::isCrossStaff() const
{
    const Note* startN = startNote();
    const Note* endN = endNote();
    const Chord* startChord = startN ? startN->chord() : nullptr;
    const Chord* endChord = endN ? endN->chord() : nullptr;
    const staff_idx_t staff = staffIdx();

    return (startChord && (startChord->staffMove() != 0 || startChord->vStaffIdx() != staff))
           || (endChord && (endChord->staffMove() != 0 || endChord->vStaffIdx() != staff));
}
}
