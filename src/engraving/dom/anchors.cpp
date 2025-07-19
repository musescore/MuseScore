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

void EditTimeTickAnchors::updateAnchors(Measure* measure, staff_idx_t staffIdx)
{
    Fraction startTick = Fraction(0, 1);
    Fraction endTick = measure->ticks();

    Fraction timeSig = measure->timesig();
    Fraction halfDivision = Fraction(1, 2 * timeSig.denominator());

    std::set<Fraction> anchorTicks;
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

    bool leftRightKey = key == Key_Left || key == Key_Right;
    bool altMod = mod & AltModifier;
    bool shiftMod = mod & ShiftModifier;

    bool changeAnchorType = shiftMod && altMod && leftRightKey && canAnchorToEndOfPrevious(element);
    bool anchorToEndOfPrevious = element->getProperty(Pid::ANCHOR_TO_END_OF_PREVIOUS).toBool();
    if (changeAnchorType) {
        element->undoChangeProperty(Pid::ANCHOR_TO_END_OF_PREVIOUS, !anchorToEndOfPrevious,
                                    element->propertyFlags(Pid::ANCHOR_TO_END_OF_PREVIOUS));

        bool doesntNeedMoveSeg = ((key == Key_Left && anchorToEndOfPrevious) || (key == Key_Right && !anchorToEndOfPrevious));
        if (doesntNeedMoveSeg) {
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
    bool needMoveToNext = curTick == curMeasure->endTick() && anchorToEndOfPrevious;
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

    staff_idx_t si = element->staffIdx();
    Segment* newSeg = nullptr;     // don't prefer any segment while dragging, just snap to the closest
    static constexpr double spacingFactor = 0.5;
    element->score()->dragPosition(element->canvasPos(), &si, &newSeg, spacingFactor, element->allowTimeAnchor());
    if (newSeg && (newSeg != segment || element->staffIdx() != si)) {
        PointF curOffset = element->offset();
        moveSegment(element, newSeg, newSeg->tick() - segment->tick());
        rebaseOffsetOnMoveSegment(element, curOffset, newSeg, segment);
    }
}

void MoveElementAnchors::moveSegment(EngravingItem* element, bool forward)
{
    if (canAnchorToEndOfPrevious(element)) {
        bool cachedAnchorToEndOfPrevious = element->getProperty(Pid::ANCHOR_TO_END_OF_PREVIOUS).toBool();
        element->undoResetProperty(Pid::ANCHOR_TO_END_OF_PREVIOUS);
        if (cachedAnchorToEndOfPrevious && forward) {
            return;
        }
    }

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
        return forward ? curSeg->next1ChordRestOrTimeTick() : curSeg->prev1ChordRestOrTimeTick();
    }
}

void MoveElementAnchors::moveSegment(EngravingItem* element, Segment* newSeg, Fraction tickDiff)
{
    switch (element->type()) {
    case ElementType::FIGURED_BASS:
        doMoveSegment(toFiguredBass(element), newSeg, tickDiff);
        break;
    case ElementType::HARMONY:
        doMoveSegment(toHarmony(element), newSeg, tickDiff);
        break;
    case ElementType::FRET_DIAGRAM:
        doMoveSegment(toFretDiagram(element), newSeg, tickDiff);
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
    std::list<EngravingObject*> linkedElements = element->linkList();
    for (EngravingObject* linkedElement : linkedElements) {
        if (linkedElement == element) {
            continue;
        }
        Segment* curParent = toSegment(linkedElement->parent());
        bool isOnMMRest = curParent->parent() && toMeasure(curParent->parent())->isMMRest();
        if (isOnMMRest) {
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

void MoveElementAnchors::doMoveSegment(Harmony* element, Segment* newSeg, Fraction tickDiff)
{
    if (newSeg->isTimeTickType()) {
        Measure* measure = newSeg->measure();
        Segment* chordRestSegAtSameTick = measure->undoGetSegment(SegmentType::ChordRest, newSeg->tick());
        newSeg = chordRestSegAtSameTick;
    }

    Segment* oldSegment = toSegment(element->parent());
    doMoveSegment(toEngravingItem(element), newSeg, tickDiff);

    oldSegment->checkEmpty();
    if (oldSegment->empty()) {
        element->score()->undoRemoveElement(oldSegment);
    }
}

void MoveElementAnchors::doMoveSegment(FretDiagram* element, Segment* newSeg, Fraction tickDiff)
{
    if (newSeg->isTimeTickType()) {
        Measure* measure = newSeg->measure();
        Segment* chordRestSegAtSameTick = measure->undoGetSegment(SegmentType::ChordRest, newSeg->tick());
        newSeg = chordRestSegAtSameTick;
    }

    Segment* oldSegment = toSegment(element->parent());
    doMoveSegment(toEngravingItem(element), newSeg, tickDiff);

    oldSegment->checkEmpty();
    if (oldSegment->empty()) {
        element->score()->undoRemoveElement(oldSegment);
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
