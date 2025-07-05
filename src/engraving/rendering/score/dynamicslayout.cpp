/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2023 MuseScore Limited
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

#include "dynamicslayout.h"
#include "layoutcontext.h"
#include "tlayout.h"

#include "../dom/hairpin.h"
#include "../dom/staff.h"
#include "../dom/system.h"
#include "../types/typesconv.h"

using namespace mu::engraving::rendering::score;

void DynamicsLayout::layoutDynamic(Dynamic* item, TextBase::LayoutData* ldata, const LayoutConfiguration& conf)
{
    doLayoutDynamic(item, ldata, conf);

    // If the dynamic contains custom text, keep it aligned
    double customTextOffset = computeCustomTextOffset(item, ldata, conf);
    ldata->moveX(-customTextOffset);

    if (item->autoplace() && item->avoidBarLines()) {
        manageBarlineCollisions(item, ldata);
    }
}

void DynamicsLayout::doLayoutDynamic(Dynamic* item, Dynamic::LayoutData* ldata, const LayoutConfiguration& conf)
{
    ldata->disconnectSnappedItems();

    HairpinSegment* snapBeforeHairpinAcrossSysBreak = item->findSnapBeforeHairpinAcrossSystemBreak();
    if (snapBeforeHairpinAcrossSysBreak) {
        ldata->connectItemSnappedBefore(snapBeforeHairpinAcrossSysBreak);
    }

    const StaffType* stType = item->staffType();
    if (stType && stType->isHiddenElementOnTab(Sid::dynamicsShowTabCommon, Sid::dynamicsShowTabSimple)) {
        ldata->setIsSkipDraw(true);
        return;
    }
    ldata->setIsSkipDraw(false);

    item->setPlacementBasedOnVoiceAssignment(conf.styleV(Sid::dynamicsHairpinVoiceBasedPlacement).value<DirectionV>());

    TLayout::layoutBaseTextBase(item, ldata);

    const Segment* s = item->segment();
    if (!s || (!item->centerOnNotehead() && item->align().horizontal == AlignH::LEFT)) {
        return;
    }

    if (item->anchorToEndOfPrevious()) {
        layoutDynamicToEndOfPrevious(item, ldata);
        return;
    }

    bool centerOnNote = item->centerOnNotehead() || (!item->centerOnNotehead() && item->align().horizontal == AlignH::HCENTER);
    double noteHeadWidth = item->score()->noteHeadWidth();

    ldata->moveX(noteHeadWidth * (centerOnNote ? 0.5 : 1));

    if (!item->centerOnNotehead()) {
        return;
    }

    // Use Smufl optical center for dynamic if available
    SymId symId = TConv::symId(item->dynamicType());
    double opticalCenter = item->symSmuflAnchor(symId, SmuflAnchorId::opticalCenter).x();
    if (symId != SymId::noSym && opticalCenter) {
        double symWidth = item->symBbox(symId).width();
        double offset = symWidth / 2 - opticalCenter + item->symBbox(symId).left();
        double spatiumScaling = item->spatium() / conf.spatium();
        offset *= spatiumScaling;
        ldata->moveX(offset);
    }
}

double DynamicsLayout::computeCustomTextOffset(Dynamic* item, Dynamic::LayoutData* ldata, const LayoutConfiguration& conf)
{
    if (!item->centerOnNotehead() || item->dynamicType() == DynamicType::OTHER) {
        return 0.0;
    }

    String referenceString = String::fromUtf8(Dynamic::dynInfo(item->dynamicType()).text);
    if (item->xmlText() == referenceString) {
        return 0.0;
    }

    Dynamic referenceDynamic(*item);
    referenceDynamic.setXmlText(referenceString);
    doLayoutDynamic(&referenceDynamic, referenceDynamic.mutldata(), conf);

    TextFragment referenceFragment;
    if (!referenceDynamic.ldata()->blocks.empty()) {
        TextBlock referenceBlock = referenceDynamic.ldata()->blocks.front();
        if (!referenceBlock.fragments().empty()) {
            referenceFragment = referenceDynamic.ldata()->blocks.front().fragments().front();
        }
    }

    for (const TextBlock& block : ldata->blocks) {
        for (const TextFragment& fragment : block.fragments()) {
            if (fragment.text == referenceFragment.text) {
                return fragment.pos.x() - referenceFragment.pos.x();
            }
        }
    }

    return 0.0;
}

void DynamicsLayout::layoutDynamicToEndOfPrevious(const Dynamic* item, TextBase::LayoutData* ldata)
{
    Segment* curSegment = item->segment();
    Segment* leftMostSegment = curSegment;
    Segment* prevSeg = curSegment;
    while (true) {
        prevSeg = prevSeg->prev1enabled();
        if (!prevSeg || prevSeg->tick() != curSegment->tick()) {
            break;
        }
        if (prevSeg->isActive() && prevSeg->hasElements(item->staffIdx())) {
            leftMostSegment = prevSeg;
            break;
        }
    }

    double xDiff = curSegment->x() + curSegment->measure()->x() - (leftMostSegment->x() + leftMostSegment->measure()->x());
    ldata->setPosX(-xDiff - ldata->bbox().right() - 0.50 * item->spatium());
}

void DynamicsLayout::manageBarlineCollisions(const Dynamic* item, TextBase::LayoutData* ldata)
{
    if (item->score()->nstaves() <= 1 || item->anchorToEndOfPrevious() || !item->isStyled(Pid::OFFSET)) {
        return;
    }

    Segment* thisSegment = item->segment();
    if (!thisSegment) {
        return;
    }

    System* system = thisSegment->measure()->system();
    if (!system) {
        return;
    }

    staff_idx_t staffIdx = item->staffIdx();
    staff_idx_t barLineStaff = muse::nidx;
    if (item->placeAbove()) {
        // need to find the barline from the staff above
        // taking into account there could be invisible staves
        if (staffIdx == 0) {
            return;
        }
        for (int staffIndex = static_cast<int>(staffIdx) - 1; staffIndex >= 0; --staffIndex) {
            if (system->staff(staffIndex)->show()) {
                barLineStaff = staffIndex;
                break;
            }
        }
    } else {
        barLineStaff = staffIdx;
    }

    if (barLineStaff == muse::nidx) {
        return;
    }

    if (item->score()->staff(barLineStaff)->barLineSpan() < 1) {
        return; // Barline doesn't extend through staves
    }

    const double minBarLineDistance = 0.25 * item->spatium();

    RectF referenceBBox = ldata->bbox();

    // Check barlines to the right
    Segment* rightBarLineSegment = nullptr;
    for (Segment* segment = thisSegment; segment && segment->measure()->system() == system; segment = segment->next1enabled()) {
        if (segment->segmentType() & SegmentType::BarLineType) {
            rightBarLineSegment = segment;
            break;
        }
    }

    if (rightBarLineSegment) {
        EngravingItem* e = rightBarLineSegment->elementAt(barLineStaff * VOICES);
        if (e) {
            double rightMargin = e->ldata()->bbox().translated(e->pagePos()).left()
                                 - referenceBBox.translated(item->pagePos() - item->offset()).right()
                                 - minBarLineDistance;
            if (rightMargin < 0) {
                ldata->moveX(rightMargin);
            }
        }
    }

    // Check barlines to the left
    Segment* leftBarLineSegment = nullptr;
    for (Segment* segment = thisSegment; segment && segment->measure()->system() == system; segment = segment->prev1enabled()) {
        if (segment->segmentType() & SegmentType::BarLineType) {
            leftBarLineSegment = segment;
            break;
        }
    }
    if (leftBarLineSegment) {
        EngravingItem* e = leftBarLineSegment->elementAt(barLineStaff * VOICES);
        if (e) {
            double leftMargin = referenceBBox.translated(item->pagePos() - item->offset()).left()
                                - e->ldata()->bbox().translated(e->pagePos()).right()
                                - minBarLineDistance;
            if (leftMargin < 0) {
                ldata->moveX(-leftMargin);
            }
        }
    }
}
