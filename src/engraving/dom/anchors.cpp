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

#include <climits>

#include "anchors.h"
#include "dom/utils.h"
#include "factory.h"
#include "figuredbass.h"
#include "fret.h"
#include "harmony.h"
#include "page.h"
#include "score.h"
#include "spanner.h"
#include "staff.h"
#include "system.h"
#include "textline.h"

#include "rendering/score/measurelayout.h"

using namespace mu::engraving::rendering::score;

namespace mu::engraving {
/***************************************
 * EditTimeTickAnchors
 * ************************************/

void EditTimeTickAnchors::updateAnchors(const EngravingItem* item)
{
    if (!item->allowTimeAnchor()) {
        item->score()->hideAnchors();
        return;
    }

    Fraction startTickMainRegion = item->isSpannerSegment() ? toSpannerSegment(item)->spanner()->tick() : item->tick();
    Fraction endTickMainRegion = item->isSpannerSegment() ? toSpannerSegment(item)->spanner()->tick2() : item->tick();

    Score* score = item->score();
    Measure* startMeasure = score->tick2measure(startTickMainRegion);
    Measure* endMeasure = score->tick2measure(endTickMainRegion);
    if (!startMeasure || !endMeasure) {
        return;
    }

    staff_idx_t staff = item->staffIdx();
    Measure* startOneBefore = startMeasure->prevMeasure();
    for (MeasureBase* mb = startOneBefore ? startOneBefore : startMeasure; mb && mb->tick() <= endMeasure->tick(); mb = mb->next()) {
        if (!mb->isMeasure()) {
            continue;
        }
        updateAnchors(toMeasure(mb), staff);
    }

    Fraction startTickExtendedRegion = startMeasure->tick();
    Fraction endTickExtendedRegion = endMeasure->endTick();
    voice_idx_t voiceIdx =  item->hasVoiceAssignmentProperties() && item->getProperty(Pid::VOICE_ASSIGNMENT).value<VoiceAssignment>()
                           != VoiceAssignment::CURRENT_VOICE_ONLY ? VOICES : item->voice();

    score->setShowAnchors(ShowAnchors(voiceIdx, staff, startTickMainRegion, endTickMainRegion, startTickExtendedRegion,
                                      endTickExtendedRegion));

    item->triggerLayout();
}

void EditTimeTickAnchors::updateAnchors(Measure* measure, staff_idx_t staffIdx, const std::set<Fraction>& additionalAnchorRelTicks)
{
    Fraction startTick = Fraction(0, 1);
    Fraction endTick = measure->ticks();

    Fraction timeSig = measure->timesig();
    Fraction halfDivision = Fraction(1, 2 * timeSig.denominator());

    std::set<Fraction> anchorTicks { additionalAnchorRelTicks };
    for (Fraction tick = startTick; tick <= endTick; tick += halfDivision) {
        anchorTicks.insert(tick);
    }
    for (Segment* seg = measure->first(Segment::CHORD_REST_OR_TIME_TICK_TYPE); seg;
         seg = seg->next(Segment::CHORD_REST_OR_TIME_TICK_TYPE)) {
        anchorTicks.insert(seg->rtick());
    }

    for (const Fraction& anchorTick : anchorTicks) {
        createTimeTickAnchor(measure, anchorTick, staffIdx);
    }

    updateLayout(measure);
}

TimeTickAnchor* EditTimeTickAnchors::createTimeTickAnchor(Measure* measure, Fraction relTick, staff_idx_t staffIdx)
{
    TimeTickAnchor* returnAnchor = nullptr;

    Staff* staff = measure->score()->staff(staffIdx);
    IF_ASSERT_FAILED(staff) {
        return nullptr;
    }

    for (EngravingObject* linkedObj : staff->linkList()) {
        IF_ASSERT_FAILED(linkedObj) {
            continue;
        }

        Staff* linkedStaff = toStaff(linkedObj);
        Measure* linkedMeasure = linkedObj == staff ? measure : linkedStaff->score()->tick2measureMM(measure->tick());
        if (!linkedMeasure) {
            continue;
        }

        Segment* segment = linkedMeasure->getSegmentR(SegmentType::TimeTick, relTick);
        track_idx_t track = staff2track(linkedStaff->idx());
        EngravingItem* element = segment->elementAt(track);
        TimeTickAnchor* anchor = element ? toTimeTickAnchor(element) : nullptr;
        if (!anchor) {
            anchor = Factory::createTimeTickAnchor(segment);
            anchor->setParent(segment);
            anchor->setTrack(track);
            segment->add(anchor);
        }
        if (linkedMeasure == measure) {
            returnAnchor = anchor;
        }
    }

    return returnAnchor;
}

void EditTimeTickAnchors::updateLayout(Measure* measure)
{
    measure->computeTicks();

    Score* score = measure->score();
    LayoutContext ctx(score);
    MeasureLayout::layoutTimeTickAnchors(measure, ctx);
}

void MoveElementAnchors::moveElementAnchors(EngravingItem* element, KeyboardKey key, KeyboardModifier mod)
{
    Segment* segment = element->parentItem() && element->parentItem()->isSegment() ? toSegment(element->parentItem()) : nullptr;
    if (!segment) {
        return;
    }

    bool altMod = mod & AltModifier;

    bool anchorToEndOfPrevious = element->getProperty(Pid::ANCHOR_TO_END_OF_PREVIOUS).toBool();
    bool changeAnchorType = altMod && canAnchorToEndOfPrevious(element);
    bool resetAnchorType = !altMod && anchorToEndOfPrevious;
    if (changeAnchorType || resetAnchorType) {
        element->undoChangeProperty(Pid::ANCHOR_TO_END_OF_PREVIOUS, !anchorToEndOfPrevious,
                                    element->propertyFlags(Pid::ANCHOR_TO_END_OF_PREVIOUS));

        bool needMoveSeg = (key == Key_Left && anchorToEndOfPrevious) || (key == Key_Right && !anchorToEndOfPrevious);
        if (!needMoveSeg) {
            checkMeasureBoundariesAndMoveIfNeed(element);
            return;
        }
    }

    moveSegment(element, /*forward*/ key == Key_Right);
    EditTimeTickAnchors::updateAnchors(element);
    checkMeasureBoundariesAndMoveIfNeed(element);
}

bool MoveElementAnchors::canAnchorToEndOfPrevious(const EngravingItem* element)
{
    switch (element->type()) {
    case ElementType::HARMONY:
    case ElementType::FRET_DIAGRAM:
    case ElementType::REHEARSAL_MARK:
        return false;
    default:
        return true;
    }
}

void MoveElementAnchors::checkMeasureBoundariesAndMoveIfNeed(EngravingItem* element)
{
    Segment* curSeg = toSegment(element->parent());
    Fraction curTick = curSeg->tick();
    Measure* curMeasure = curSeg->measure();
    Measure* prevMeasure = curMeasure->prevMeasure();
    bool anchorToEndOfPrevious = element->getProperty(Pid::ANCHOR_TO_END_OF_PREVIOUS).toBool();
    bool needMoveToNext = curTick == curMeasure->endTick() && !anchorToEndOfPrevious;
    bool needMoveToPrevious = curSeg->rtick().isZero() && anchorToEndOfPrevious && prevMeasure;

    if (!needMoveToPrevious && !needMoveToNext) {
        return;
    }

    Segment* newSeg = nullptr;
    if (needMoveToPrevious) {
        newSeg = prevMeasure->findSegment(SegmentType::TimeTick, curSeg->tick());
        if (!newSeg) {
            TimeTickAnchor* anchor = EditTimeTickAnchors::createTimeTickAnchor(prevMeasure, curTick - prevMeasure->tick(),
                                                                               element->staffIdx());
            EditTimeTickAnchors::updateLayout(prevMeasure);
            newSeg = anchor->segment();
        }
    } else {
        newSeg = curSeg->next1(SegmentType::ChordRest);
    }

    if (newSeg && newSeg->tick() == curTick) {
        moveSegment(element, newSeg, Fraction(0, 1));
    }
}

void MoveElementAnchors::moveElementAnchorsOnDrag(EngravingItem* element, EditData& ed)
{
    Segment* segment = element->explicitParent() && element->parent()->isSegment() ? toSegment(element->parent()) : nullptr;
    if (!segment) {
        return;
    }

    KeyboardModifiers km = ed.modifiers;
    if (km & (ShiftModifier | ControlModifier)) {
        return;
    }

    EditTimeTickAnchors::updateAnchors(element);

    Segment* newSeg = findNewAnchorableSegmentFromDrag(element, segment);

    if (newSeg && (newSeg != segment && !newSeg->measure()->isMMRest())) {
        PointF curOffset = element->offset();
        moveSegment(element, newSeg, newSeg->tick() - segment->tick());
        rebaseOffsetOnMoveSegment(element, curOffset, newSeg, segment);
    }
}

Segment* MoveElementAnchors::findNewAnchorableSegmentFromDrag(EngravingItem* element, Segment* curSeg)
{
    const System* system = curSeg->system();
    if (!system) {
        return nullptr;
    }

    const Measure* newMeasure = nullptr;
    double xRef = element->canvasX() - system->canvasX();
    for (MeasureBase* mb : system->measures()) {
        if (!mb->isMeasure()) {
            continue;
        }
        if (mb->x() + mb->width() > xRef) {
            newMeasure = toMeasure(mb);
            break;
        }
    }
    if (!newMeasure) {
        return nullptr;
    }

    Segment* newSeg = nullptr;
    dragPositionToSegment(element->canvasPos(), newMeasure, element->staffIdx(), &newSeg, 0.5, true);

    return newSeg;
}

Segment* MoveElementAnchors::findNewAnchorSegmentForLine(LineSegment* lineSegment, const EditData& ed, const Segment* curSeg)
{
    SLine* line = lineSegment->line();
    Score* score = lineSegment->score();
    if (!line->allowTimeAnchor()) {
        if (ed.key == Key_Left) {
            return curSeg->prev1WithElemsOnStaff(lineSegment->staffIdx());
        }
        if (ed.key == Key_Right) {
            Segment* lastCRSegInScore = score->lastSegment();
            while (lastCRSegInScore && !lastCRSegInScore->isChordRestType()) {
                lastCRSegInScore = lastCRSegInScore->prev1(SegmentType::ChordRest);
            }
            if (curSeg == lastCRSegInScore && curSeg == line->endSegment()) {
                // If we reach this point, it means that the line in question does not accept time anchors
                // and that the line's end segment is the last CR segment in the score. Trying to use
                // next1WithElemsOnStaff won't do anything from here, but the last segment in the score is
                // still a valid new anchor segment for this line (see also LineSegment::edit)...
                return score->lastSegment();
            }
            return curSeg->next1WithElemsOnStaff(lineSegment->staffIdx());
        }
    }

    if (ed.modifiers & ControlModifier) {
        if (ed.key == Key_Left) {
            Measure* measure = curSeg->rtick().isZero() ? curSeg->measure()->prevMeasure() : curSeg->measure();
            return measure ? measure->findFirstR(SegmentType::ChordRest, Fraction(0, 1)) : nullptr;
        }
        if (ed.key == Key_Right) {
            Measure* measure = curSeg->measure()->nextMeasure();
            return measure ? measure->findFirstR(SegmentType::ChordRest, Fraction(0, 1)) : nullptr;
        }
    }

    if (ed.key == Key_Left) {
        return findNewAnchorableSegment(curSeg, /*forward*/ false);
    }
    if (ed.key == Key_Right) {
        return findNewAnchorableSegment(curSeg, /*forward*/ true);
    }

    return nullptr;
}

void MoveElementAnchors::moveSegment(EngravingItem* element, bool forward)
{
    Segment* curSeg = toSegment(element->parentItem());
    Segment* newSeg = getNewSegment(element, curSeg, forward);

    if (newSeg) {
        moveSegment(element, newSeg, newSeg->tick() - curSeg->tick());
    }
}

Segment* MoveElementAnchors::getNewSegment(EngravingItem* element, Segment* curSeg, bool forward)
{
    switch (element->type()) {
    case ElementType::REHEARSAL_MARK:
    {
        Measure* measure = curSeg->measure();
        Measure* newMeasure = forward ? measure->nextMeasureMM() : measure->prevMeasureMM();
        return newMeasure ? newMeasure->first(SegmentType::ChordRest) : nullptr;
    }
    default:
        return findNewAnchorableSegment(curSeg, forward);
    }
}

Segment* MoveElementAnchors::findNewAnchorableSegment(const Segment* curSeg, bool forward)
{
    Segment* newSeg = forward ? curSeg->next1(Segment::CHORD_REST_OR_TIME_TICK_TYPE) : curSeg->prev1(Segment::CHORD_REST_OR_TIME_TICK_TYPE);

    // Continue until we get to a different tick than where we are
    while (newSeg && newSeg->tick() == curSeg->tick()) {
        newSeg = forward ? newSeg->next1(Segment::CHORD_REST_OR_TIME_TICK_TYPE) : newSeg->prev1(Segment::CHORD_REST_OR_TIME_TICK_TYPE);
    }
    if (!newSeg) {
        return nullptr;
    }

    // Always prefer ChordRest segment if one exists at the same tick
    if (newSeg->segmentType() != SegmentType::ChordRest) {
        Segment* newChordRestSeg = forward ? newSeg->next1(SegmentType::ChordRest) : newSeg->prev1(SegmentType::ChordRest);
        if (newChordRestSeg && newChordRestSeg->tick() == newSeg->tick()) {
            newSeg = newChordRestSeg;
        }
    }

    return newSeg;
}

void MoveElementAnchors::moveSegment(EngravingItem* element, Segment* newSeg, Fraction tickDiff)
{
    switch (element->type()) {
    case ElementType::FIGURED_BASS:
        doMoveSegment(toFiguredBass(element), newSeg, tickDiff);
        break;
    case ElementType::HARMONY:
    case ElementType::FRET_DIAGRAM:
        doMoveHarmonyOrFretDiagramSegment(element, newSeg, tickDiff);
        break;
    default:
        doMoveSegment(toEngravingItem(element), newSeg, tickDiff);
        break;
    }
}

void MoveElementAnchors::doMoveSegment(EngravingItem* element, Segment* newSeg, Fraction tickDiff)
{
    IF_ASSERT_FAILED(element->parent() && element->parent()->isSegment()) {
        return;
    }

    // NOTE: when creating mmRests, we clone text elements from the underlying measure onto the
    // mmRest measure. This creates lots of additional linked copies which are hard to manage
    // and can result in duplicates. Here we need to remove copies on mmRests because they are invalidated
    // when moving segments (and if needed will be recreated at the next layout). In future we need to
    // change approach and *move* elements onto the mmRests, not clone them. [M.S.]

    Score* score = element->score();
    Measure* parentMeasure = toMeasure(element->findAncestor(ElementType::MEASURE));
    bool parentMeasureIsMMRest = parentMeasure && parentMeasure->isMMRest();

    std::list<EngravingObject*> linkedElements = element->linkList();
    for (EngravingObject* linkedElement : linkedElements) {
        if (linkedElement == element) {
            continue;
        }
        Measure* linkedParentMeasure = toMeasure(toEngravingItem(linkedElement)->findAncestor(ElementType::MEASURE));
        bool linkedParentMeasureIsMMrest = linkedParentMeasure && linkedParentMeasure->isMMRest();
        if (linkedParentMeasureIsMMrest != parentMeasureIsMMRest) {
            for (EngravingObject* child : linkedElement->children()) {
                // e.g. fret diagram should also remove the harmony
                child->undoUnlink();
                score->undoRemoveElement(toEngravingItem(child));
            }
            linkedElement->undoUnlink();
            score->undoRemoveElement(static_cast<EngravingItem*>(linkedElement));
        }
    }

    score->undoChangeParent(element, newSeg, element->staffIdx());
    moveSnappedItems(element, newSeg, tickDiff);
}

void MoveElementAnchors::doMoveSegment(FiguredBass* element, Segment* newSeg, Fraction tickDiff)
{
    IF_ASSERT_FAILED(element->parent() && element->parent()->isSegment()) {
        return;
    }

    Segment* oldSeg = element->segment();

    doMoveSegment(toEngravingItem(element), newSeg, tickDiff);

    track_idx_t startTrack = staff2track(element->staffIdx());
    track_idx_t endTrack = startTrack + VOICES;

    // Shorten this if needed
    if (newSeg->tick() > oldSeg->tick()) {
        FiguredBass* nextFB = nullptr;
        Fraction endTick = newSeg->tick() + element->ticks();
        for (Segment* seg = newSeg->next1(Segment::CHORD_REST_OR_TIME_TICK_TYPE); seg && seg->tick() <= endTick;
             seg = seg->next1(Segment::CHORD_REST_OR_TIME_TICK_TYPE)) {
            nextFB = toFiguredBass(seg->findAnnotation(ElementType::FIGURED_BASS, startTrack, endTrack));
            if (nextFB) {
                break;
            }
        }
        if (nextFB) {
            element->setTicks(nextFB->tick() - newSeg->tick());
        }
    }

    // Shorten previous if needed
    if (newSeg->tick() < oldSeg->tick()) {
        FiguredBass* prevFB = nullptr;
        for (Segment* seg = newSeg->prev1(Segment::CHORD_REST_OR_TIME_TICK_TYPE); seg && seg->measure()->isAfterOrEqual(newSeg->measure());
             seg = seg->prev1(Segment::CHORD_REST_OR_TIME_TICK_TYPE)) {
            prevFB = (FiguredBass*)(seg->findAnnotation(ElementType::FIGURED_BASS, startTrack, endTrack));
            if (prevFB) {
                break;
            }
        }
        if (prevFB) {
            prevFB->setTicks(std::min(prevFB->ticks(), newSeg->tick() - prevFB->tick()));
        }
    }
}

void MoveElementAnchors::doMoveHarmonyOrFretDiagramSegment(EngravingItem* element, Segment* newSeg, Fraction tickDiff)
{
    for (EngravingObject* item : element->linkList()) {
        Score* score = item->score();
        Measure* measure = score->tick2measure(newSeg->tick());
        Segment* chordRestSeg = measure->undoGetSegment(SegmentType::ChordRest, newSeg->tick());
        if (item == element) {
            newSeg = chordRestSeg;
        }
    }

    Fraction oldTick = element->tick();

    doMoveSegment(toEngravingItem(element), newSeg, tickDiff);

    for (EngravingObject* item : element->linkList()) {
        Score* score = item->score();
        Segment* oldSegment = score->tick2segment(oldTick, true, SegmentType::ChordRest, false);
        if (oldSegment) {
            oldSegment->checkEmpty();
            if (oldSegment->empty()) {
                score->undoRemoveElement(oldSegment);
            }
        }
    }
}

void MoveElementAnchors::moveSnappedItems(EngravingItem* element, Segment* newSeg, Fraction tickDiff)
{
    if (EngravingItem* itemAfter = element->mutldata()->itemSnappedAfter()) {
        if (itemAfter->isTextBase() && itemAfter->parent() != newSeg) {
            element->score()->undoChangeParent(itemAfter, newSeg, itemAfter->staffIdx());
            moveSnappedItems(itemAfter, newSeg, tickDiff);
        } else if (itemAfter->isTextLineBaseSegment()) {
            TextLineBase* textLine = ((TextLineBaseSegment*)itemAfter)->textLineBase();
            if (textLine->tick() != newSeg->tick()) {
                textLine->undoMoveStart(tickDiff);
            }
        }
    }

    if (EngravingItem* itemBefore = element->mutldata()->itemSnappedBefore()) {
        if (itemBefore->isTextBase() && itemBefore->parent() != newSeg) {
            element->score()->undoChangeParent(itemBefore, newSeg, itemBefore->staffIdx());
            moveSnappedItems(itemBefore, newSeg, tickDiff);
        } else if (itemBefore->isTextLineBaseSegment()) {
            TextLineBase* textLine = ((TextLineBaseSegment*)itemBefore)->textLineBase();
            if (textLine->tick2() != newSeg->tick()) {
                textLine->undoMoveEnd(tickDiff);
            }
        }
    }
}

void MoveElementAnchors::rebaseOffsetOnMoveSegment(EngravingItem* element, const PointF& curOffset, Segment* newSeg, Segment* oldSeg)
{
    PointF offsetShift = newSeg->pagePos() - oldSeg->pagePos();
    element->setOffset(curOffset - offsetShift);

    if (EngravingItem* itemBefore = element->mutldata()->itemSnappedBefore()) {
        if (itemBefore->isTextBase()) {
            itemBefore->setOffset(itemBefore->offset() - offsetShift);
        }
    }

    if (EngravingItem* itemAfter = element->mutldata()->itemSnappedAfter()) {
        if (itemAfter->isTextBase()) {
            itemAfter->setOffset(itemAfter->offset() - offsetShift);
        }
    }
}

/********************************************
 * TimeTickAnchor
 * *****************************************/

TimeTickAnchor::TimeTickAnchor(Segment* parent)
    : EngravingItem(ElementType::TIME_TICK_ANCHOR, parent,
                    ElementFlag::ON_STAFF
                    | ElementFlag::NOT_SELECTABLE
                    | ElementFlag::GENERATED)
{
    setZ(-INT_MAX); // Make sure it is behind everything
}

TimeTickAnchor::DrawRegion TimeTickAnchor::drawRegion() const
{
    const ShowAnchors& showAnchors = score()->showAnchors();
    const staff_idx_t thisStaffIdx = staffIdx();
    const Fraction thisTick = segment()->tick();

    const bool trackOutOfRange = thisStaffIdx < showAnchors.staffIdx || thisStaffIdx >= showAnchors.endStaffIdx;
    const bool tickOutOfRange = thisTick < showAnchors.startTickExtendedRegion || thisTick >= showAnchors.endTickExtendedRegion;
    if (trackOutOfRange || tickOutOfRange) {
        return DrawRegion::OUT_OF_RANGE;
    }

    if (thisTick < showAnchors.startTickMainRegion || thisTick >= showAnchors.endTickMainRegion) {
        return DrawRegion::EXTENDED_REGION;
    }

    return DrawRegion::MAIN_REGION;
}

voice_idx_t TimeTickAnchor::voiceIdx() const
{
    return score()->showAnchors().voiceIdx;
}
} // namespace mu::engraving
