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
#include "types/typesconv.h"
#include "undo.h"
#include "utils.h"

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

void Tie::collectPossibleEndPoints()
{
    const MasterScore* master = masterScore();
    const Note* note = toNote(parentItem());
    const Chord* chord = note->chord();
    const Measure* measure = chord->measure();
    const MeasureBase* masterMeasureBase = master->measure(measure->index());
    const Measure* masterMeasure = masterMeasureBase && masterMeasureBase->isMeasure() ? toMeasure(masterMeasureBase) : nullptr;
    if (!tieEndPoints()) {
        return;
    }

    tieEndPoints()->clear();

    int endPointIdx = 0;

    Note* nextNote = searchTieNote(note);
    nextNote = nextNote ? nextNote : endNote();

    if (nextNote) {
        const bool hasTie = nextNote->tieBack();
        TieEndPoint endPoint = TieEndPoint(nextNote, u"next note", hasTie, endPointIdx, true);
        tieEndPoints()->add(endPoint);
        endPointIdx++;
    }

    if (!startNote()->followingJumpItem()) {
        return;
    }

    // Get following notes by taking repeats
    const RepeatList& repeatList = master->repeatList();
    for (auto it = repeatList.begin(); it != repeatList.end(); it++) {
        const RepeatSegment* rs = *it;
        const auto nextSegIt = std::next(it);
        if (!rs->containsMeasure(masterMeasure) || nextSegIt == repeatList.end()) {
            continue;
        }

        // Get next segment
        const RepeatSegment* nextSeg = *nextSegIt;
        const Measure* firstMasterMeasure = nextSeg->firstMeasure();
        const MeasureBase* firstMeasureBase = firstMasterMeasure ? score()->measure(firstMasterMeasure->index()) : nullptr;
        const Measure* firstMeasure = firstMeasureBase && firstMeasureBase->isMeasure() ? toMeasure(firstMeasureBase) : nullptr;
        const Segment* firstCrSeg = firstMeasure ? firstMeasure->first(SegmentType::ChordRest) : nullptr;
        if (!firstCrSeg) {
            continue;
        }

        Note* nextNote = searchTieNote(note, firstCrSeg);

        // EITHER add RepeatListElementType to repeat segment to RepeatSegment (less calculation)
        // OR get preceding repeat element from note method (get more info)
        if (nextNote) {
            bool hasIncomingTie = nextNote->tieBack();
            String jumpName = nextNote->precedingJumpItemName();
            TieEndPoint endPoint = TieEndPoint(nextNote, jumpName, hasIncomingTie, endPointIdx, false);
            tieEndPoints()->add(endPoint);
            endPointIdx++;
        }
    }
}

void Tie::addTiesToEndPoints()
{
    collectPossibleEndPoints();
    TieEndPointList* endPoints = tieEndPoints();
    if (!endPoints) {
        return;
    }

    for (TieEndPoint& endPoint : *endPoints) {
        if (endPoint.followingNote()) {
            endPoint.undoSetActive(true);
            continue;
        }
        endPoints->addTie(&endPoint);
    }
}

void Tie::removeTiesFromEndPoints()
{
    TieEndPointList* endPoints = tieEndPoints();
    if (!endPoints) {
        return;
    }
    for (TieEndPoint& endPoint : *endPoints) {
        if (endPoint.followingNote() || !endPoint.active()) {
            endPoint.undoSetActive(false);
            continue;
        }

        endPoints->removeTie(&endPoint);
    }
}

bool Tie::allEndPointsInactive() const
{
    if (endNote()) {
        return false;
    }
    if (!tieEndPoints()) {
        return true;
    }

    for (const TieEndPoint& endPoint : *tieEndPoints()) {
        if (endPoint.active()) {
            return false;
        }
    }

    return true;
}

TieEndPointList* Tie::tieEndPoints()
{
    return startNote() ? startNote()->tieEndPoints() : nullptr;
}

const TieEndPointList* Tie::tieEndPoints() const
{
    return startNote() ? startNote()->tieEndPoints() : nullptr;
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

//---------------------------------------------------------
//   PartialTieEndPoint
//---------------------------------------------------------

TieEndPoint::TieEndPoint(Note* note, String jumpName, bool active, int idx, bool followingNote)
    : m_note(note), m_jumpName(jumpName), m_active(active), m_followingNote(followingNote)
{
    m_id = u"endPoint" + String::fromStdString(std::to_string(idx));
}

Tie* TieEndPoint::endTie() const
{
    return m_note ? m_note->tieBack() : nullptr;
}

bool TieEndPoint::followingNote() const
{
    return m_followingNote;
}

void TieEndPoint::undoSetActive(bool v)
{
    Score* score = m_note ? m_note->score() : nullptr;
    if (!score || m_active == v) {
        return;
    }
    score->undo(new ChangeTieEndPointActive(this, v));
}

const String TieEndPoint::menuTitle() const
{
    const Measure* measure = m_note->findMeasure();
    const int measureNo = measure ? measure->no() + 1 : 0;
    const TranslatableString tieTo("engraving", "Tie to ");
    const String title = tieTo.str + m_jumpName + u" " + muse::mtrc("engraving", "(m. %1)").arg(measureNo);

    return title;
}

//---------------------------------------------------------
//   PartialTieEndPointList
//---------------------------------------------------------

void TieEndPointList::add(TieEndPoint& item)
{
    item.setEndPointList(this);
    m_endPoints.push_back(item);
}

void TieEndPointList::setStartTie(Tie* startTie)
{
    m_startTie = startTie;
}

void TieEndPointList::toggleEndPoint(const String& id)
{
    TieEndPoint* end = nullptr;

    for (TieEndPoint& endPoint : m_endPoints) {
        if (endPoint.id() != id) {
            continue;
        }

        end = &endPoint;
        break;
    }

    if (!end) {
        LOGE() << "No partial tie end point found with id: " << id;
        return;
    }

    Score* score = end->note() ? end->note()->score() : nullptr;

    score->startCmd(TranslatableString("engraving", "Toggle partial tie"));
    const bool checked = end->active();
    if (checked) {
        removeTie(end);
    } else {
        addTie(end);
    }
    score->endCmd();
}

void TieEndPointList::addTie(TieEndPoint* endPoint)
{
    Note* note = endPoint->note();
    Score* score = note ? note->score() : nullptr;
    Tie* _startTie = startTie();
    if (!_startTie || !score) {
        return;
    }

    if (endPoint->followingNote()) {
        // Remove partial tie and add full tie
        if (!_startTie->isPartialTie() || !toPartialTie(_startTie)->isOutgoing()) {
            return;
        }
        endPoint->undoSetActive(true);
        m_startTie = Tie::changeTieType(_startTie, note);
        return;
    }

    endPoint->undoSetActive(true);

    // Check if there is already a tie.  If so, add partial tie info to it
    Tie* tieBack = note->tieBack();
    if (tieBack && !tieBack->isPartialTie()) {
        tieBack->setEndPoint(endPoint);
        return;
    }
    // Otherwise create incoming partial tie on note
    PartialTie* pt = Factory::createPartialTie(note);
    pt->setParent(note);
    pt->setEndNote(note);
    pt->setEndPoint(endPoint);
    score->undoAddElement(pt);
}

void TieEndPointList::removeTie(TieEndPoint* endPoint)
{
    Note* note = endPoint->note();
    Score* score = note ? note->score() : nullptr;
    Tie* _startTie = startTie();
    if (!_startTie || !score) {
        return;
    }

    if (endPoint->followingNote()) {
        // Remove full tie and add partial tie
        if (_startTie->isPartialTie()) {
            return;
        }
        endPoint->undoSetActive(false);

        m_startTie = Tie::changeTieType(_startTie);
        return;
    }

    endPoint->undoSetActive(false);

    // Check if there is a full tie. If so, remove partial tie info from it
    Tie* tieBack = note->tieBack();
    if (tieBack && !tieBack->isPartialTie()) {
        tieBack->setEndPoint(nullptr);
        return;
    }
    // Otherwise remove incoming partial tie on note
    PartialTie* pt = note->incomingPartialTie();
    if (!pt) {
        return;
    }
    score->undoRemoveElement(pt);
}

Tie* Tie::changeTieType(Tie* oldTie, Note* endNote)
{
    // Replaces oldTie with an outgoing partial tie if no endNote is specified.  Otherwise replaces oldTie with a regular tie
    Note* startNote = oldTie->startNote();
    bool addPartialTie = !endNote;
    Score* score = startNote ? startNote->score() : nullptr;
    if (!score) {
        return nullptr;
    }

    TranslatableString undoCmd = addPartialTie ? TranslatableString("engraving", "Replace full tie with partial tie") : TranslatableString(
        "engraving", "Replace partial tie with full tie");
    Tie* newTie = addPartialTie ? Factory::createPartialTie(score->dummy()->note()) : Factory::createTie(score->dummy()->note());

    score->undoRemoveElement(oldTie);

    newTie->setParent(startNote);
    newTie->setStartNote(startNote);
    startNote->setTieFor(newTie);
    if (!addPartialTie) {
        newTie->setEndNote(endNote);
        endNote->setTieBack(newTie);
    }

    newTie->setTick(startNote->tick());
    newTie->setTrack(startNote->track());

    newTie->setStyleType(oldTie->styleType());
    newTie->setTiePlacement(oldTie->tiePlacement());
    newTie->setSlurDirection(oldTie->slurDirection());

    newTie->setVisible(oldTie->visible());
    newTie->setOffset(oldTie->offset());

    score->undoAddElement(newTie);

    score->endCmd();

    return newTie;
}

Tie* Tie::startTie() const
{
    return startTieEndPoints() ? startTieEndPoints()->startTie() : nullptr;
}

TieEndPoint* Tie::endPoint() const
{
    return m_endPoint;
}

void Tie::setEndPoint(TieEndPoint* endPoint)
{
    m_endPoint = endPoint;
}

void Tie::updateStartTieOnRemoval()
{
    if (!endPoint() || !startTie() || !startTieEndPoints()) {
        return;
    }
    endPoint()->undoSetActive(false);
    Tie* _startTie = startTie();
    if (startTieEndPoints()->size() <= 1 || _startTie->allEndPointsInactive()) {
        score()->undoRemoveElement(_startTie);
    }
}

TieEndPointList* Tie::startTieEndPoints() const
{
    return m_endPoint ? m_endPoint->endPointList() : nullptr;
}
}
