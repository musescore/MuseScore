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
#include "barline.h"
#include "chord.h"
#include "factory.h"
#include "hook.h"
#include "ledgerline.h"
#include "masterscore.h"
#include "measure.h"
#include "mscoreview.h"
#include "note.h"
#include "notedot.h"
#include "part.h"
#include "partialtie.h"
#include "repeatlist.h"
#include "score.h"
#include "staff.h"
#include "stafftype.h"
#include "stem.h"
#include "system.h"
#include "tiejumppointlist.h"
#include "undo.h"
#include "utils.h"
#include "volta.h"

#include "log.h"

using namespace mu;
using namespace muse::draw;

namespace mu::engraving {
//---------------------------------------------------------
//   TieSegment
//---------------------------------------------------------

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
        if (isPartialTieSegment()) {
            return;
        }
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

TieJumpPointList* Tie::startTieJumpPoints() const
{
    return m_jumpPoint ? m_jumpPoint->jumpPointList() : nullptr;
}

void Tie::updatePossibleJumpPoints()
{
    if (!tieJumpPoints()) {
        return;
    }

    tieJumpPoints()->clear();

    const Note* note = toNote(parentItem());
    const Chord* chord = note ? note->chord() : nullptr;
    const Measure* measure = chord ? chord->measure() : nullptr;
    if (!measure) {
        return;
    }

    const Segment* segment = chord ? chord->segment() : nullptr;

    // Check ties starting in this measure and ending in another
    // If they cross a repeat, add jump points
    const bool hasFollowingJumpItem = chord->hasFollowingJumpItem();

    if (!hasFollowingJumpItem) {
        const Note* tieEndNote = endNote();
        const Chord* endChord = tieEndNote ? tieEndNote->chord() : nullptr;
        if (!endChord) {
            return;
        }
        const Segment* endNoteSegment = endChord ? endChord->segment() : nullptr;
        const ChordRest* finalCROfMeasure = measure->lastChordRest(track());
        const bool finalCRHasFollowingJump = finalCROfMeasure ? finalCROfMeasure->hasFollowingJumpItem() : false;
        const bool segsAreAdjacent = segmentsAreAdjacentInRepeatStructure(segment, endNoteSegment);
        const bool segsAreInDifferentRepeatSegments = segmentsAreInDifferentRepeatSegments(segment, endNoteSegment);

        if (!(finalCRHasFollowingJump && segsAreAdjacent) || !segsAreInDifferentRepeatSegments) {
            return;
        }
    }

    int jumpPointIdx = 0;

    Note* nextNote = searchTieNote(note);
    nextNote = nextNote ? nextNote : endNote();

    if (nextNote) {
        const bool hasTie = nextNote->tieBack();
        TieJumpPoint* jumpPoint = new TieJumpPoint(nextNote, hasTie, jumpPointIdx, true);
        tieJumpPoints()->add(jumpPoint);
        jumpPointIdx++;
    }

    for (Measure* jumpMeasure : findFollowingRepeatMeasures(measure)) {
        const Segment* firstCrSeg = jumpMeasure ? jumpMeasure->first(SegmentType::ChordRest) : nullptr;
        if (!firstCrSeg) {
            continue;
        }

        nextNote = searchTieNote(note, firstCrSeg, false);

        if (nextNote) {
            bool hasIncomingTie = nextNote->tieBack();
            TieJumpPoint* jumpPoint = new TieJumpPoint(nextNote, hasIncomingTie, jumpPointIdx, false);
            tieJumpPoints()->add(jumpPoint);
            jumpPointIdx++;
        }
    }

    if (jumpPointIdx < 2 && !isPartialTie()) {
        tieJumpPoints()->clear();
    }
}

void Tie::addTiesToJumpPoints()
{
    updatePossibleJumpPoints();
    TieJumpPointList* jumpPoints = tieJumpPoints();
    if (!jumpPoints) {
        return;
    }

    for (TieJumpPoint* jumpPoint : *jumpPoints) {
        if (jumpPoint->followingNote()) {
            jumpPoint->undoSetActive(true);
            continue;
        }
        jumpPoints->undoAddTieToScore(jumpPoint);
    }

    // Update jump points for linked ties
    for (EngravingObject* linkedTie : linkList()) {
        if (!linkedTie || !linkedTie->isTie() || linkedTie == this) {
            continue;
        }
        toTie(linkedTie)->updatePossibleJumpPoints();
    }
}

void Tie::undoRemoveTiesFromJumpPoints()
{
    TieJumpPointList* jumpPoints = tieJumpPoints();
    if (!jumpPoints) {
        return;
    }
    for (TieJumpPoint* jumpPoint : *jumpPoints) {
        if (jumpPoint->followingNote() || !jumpPoint->active()) {
            jumpPoint->undoSetActive(false);
            continue;
        }

        jumpPoints->undoRemoveTieFromScore(jumpPoint);
    }
}

bool Tie::allJumpPointsInactive() const
{
    if (endNote()) {
        return false;
    }
    if (!tieJumpPoints()) {
        return true;
    }

    for (const TieJumpPoint* jumpPoint : *tieJumpPoints()) {
        if (jumpPoint->active()) {
            return false;
        }
    }

    return true;
}

TieJumpPointList* Tie::tieJumpPoints()
{
    return startNote() ? startNote()->tieJumpPoints() : nullptr;
}

const TieJumpPointList* Tie::tieJumpPoints() const
{
    return startNote() ? startNote()->tieJumpPoints() : nullptr;
}

Tie::Tie(EngravingItem* parent)
    : SlurTie(ElementType::TIE, parent)
{
    setAnchor(Anchor::NOTE);
}

Tie::Tie(const Tie& t)
    : SlurTie(t)
{
    m_isInside = t.m_isInside;
    m_tiePlacement = t.m_tiePlacement;
    // Jump points must be recalculated for this tie
    m_jumpPoint = nullptr;
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

void Tie::setStartNote(Note* note)
{
    setStartElement(note);
    setParent(note);
}

Note* Tie::startNote() const
{
    assert(!startElement() || startElement()->type() == ElementType::NOTE);
    return toNote(startElement());
}

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

void Tie::changeTieType(Tie* oldTie, Note* endNote)
{
    // Replaces oldTie with an outgoing partial tie if no endNote is specified.  Otherwise replaces oldTie with a regular tie
    Note* startNote = oldTie->startNote();
    bool addPartialTie = !endNote;
    Score* score = startNote ? startNote->score() : nullptr;
    if (!score) {
        return;
    }

    TranslatableString undoCmd = addPartialTie ? TranslatableString("engraving", "Replace full tie with partial tie")
                                 : TranslatableString("engraving", "Replace partial tie with full tie");
    Tie* newTie = addPartialTie ? Factory::createPartialTie(score->dummy()->note()) : Factory::createTie(score->dummy()->note());

    score->undoRemoveElement(oldTie);

    newTie->setParent(startNote);
    newTie->setStartNote(startNote);
    newTie->setTick(startNote->tick());
    newTie->setTrack(startNote->track());
    startNote->setTieFor(newTie);
    if (!addPartialTie) {
        newTie->setEndNote(endNote);
        endNote->setTieBack(newTie);
    }

    newTie->setStyleType(oldTie->styleType());
    newTie->setTiePlacement(oldTie->tiePlacement());
    newTie->setSlurDirection(oldTie->slurDirection());

    newTie->setVisible(oldTie->visible());
    newTie->setOffset(oldTie->offset());

    score->undoAddElement(newTie);

    score->endCmd();
}

void Tie::updateStartTieOnRemoval()
{
    if (!jumpPoint() || !startTie() || !startTieJumpPoints()) {
        return;
    }
    jumpPoint()->undoSetActive(false);
    Tie* _startTie = startTie();
    if (startTieJumpPoints()->size() <= 1 || _startTie->allJumpPointsInactive()) {
        score()->undoRemoveElement(_startTie);
    }
}

Tie* Tie::startTie() const
{
    return startTieJumpPoints() ? startTieJumpPoints()->startTie() : nullptr;
}
}
