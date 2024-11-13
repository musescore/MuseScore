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
#include "utils.h"

#include "log.h"

using namespace mu;
using namespace muse::draw;

namespace mu::engraving {
Note* Tie::editStartNote;
Note* Tie::editEndNote;

TieSegment::TieSegment(System* parent)
    : SlurTieSegment(ElementType::TIE_SEGMENT, parent)
{
}

TieSegment::TieSegment(const ElementType& type, System* parent)
    : SlurTieSegment(type, parent)
{
}

TieSegment::TieSegment(const TieSegment& s)
    : SlurTieSegment(s)
{
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
        score()->startCmd(TranslatableString("undoableAction", "Change tie anchor"));
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

double TieSegment::minShoulderHeight() const
{
    return style().styleMM(Sid::tieMinShoulderHeight);
}

double TieSegment::maxShoulderHeight() const
{
    return style().styleMM(Sid::tieMaxShoulderHeight);
}

double TieSegment::endWidth() const
{
    return style().styleMM(Sid::tieEndWidth);
}

double TieSegment::midWidth() const
{
    return style().styleMM(Sid::tieMidWidth);
}

double TieSegment::dottedWidth() const
{
    return style().styleMM(Sid::tieDottedWidth);
}

//---------------------------------------------------------
//   Tie
//---------------------------------------------------------

Tie::Tie(const ElementType& type, EngravingItem* parent)
    : SlurTie(type, parent)
{
    setAnchor(Anchor::NOTE);
}

Tie::Tie(EngravingItem* parent)
    : SlurTie(ElementType::TIE, parent)
{
    setAnchor(Anchor::NOTE);
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
    const bool hasBothNotes = startNote() && endNote();

    const Note* primaryNote = startNote() ? startNote() : endNote();
    const Note* secondaryNote = hasBothNotes ? endNote() : nullptr;

    if (!primaryNote) {
        return 1.0;
    }

    if (primaryNote->isGrace()) {
        return style().styleD(Sid::graceNoteMag);
    }

    if (hasBothNotes) {
        return 0.5 * (primaryNote->chord()->intrinsicMag() + secondaryNote->chord()->intrinsicMag());
    }

    return primaryNote->chord()->intrinsicMag();
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
