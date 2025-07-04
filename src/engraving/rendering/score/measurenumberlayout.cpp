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

#include "measurenumberlayout.h"
#include "measurelayout.h"
#include "tlayout.h"

#include "dom/measure.h"
#include "dom/score.h"
#include "dom/segment.h"
#include "dom/system.h"

using namespace mu::engraving;
using namespace mu::engraving::rendering::score;

void MeasureNumberLayout::layoutMeasureNumber(const MeasureNumber* item, MeasureNumber::LayoutData* ldata, const LayoutContext& ctx)
{
    layoutMeasureNumberBase(item, ldata);

    bool intervalType = !ctx.conf().styleB(Sid::measureNumberSystem);
    bool alignToBarline = ctx.conf().styleB(intervalType ? Sid::measureNumberAlignToBarlineInterval : Sid::measureNumberAlignToBarline);
    const RectF& itemBBox = ldata->bbox();
    PlacementH hPlacement = item->getProperty(Pid::HPLACEMENT).value<PlacementH>();

    if (alignToBarline) {
        ldata->setPosX(0);
    } else {
        Measure* measure = item->measure();
        Segment* firstCRSeg = measure->first(SegmentType::ChordRest);
        MeasureLayout::MeasureStartEndPos measStartEndPos = MeasureLayout::getMeasureStartEndPos(measure, firstCRSeg,
                                                                                                 item->staffIdx(), true, false, ctx);
        if (hPlacement == PlacementH::LEFT) {
            ldata->setPosX(measStartEndPos.x1);
        } else if (hPlacement == PlacementH::CENTER) {
            ldata->setPosX(0.5 * (measStartEndPos.x1 + measStartEndPos.x2));
        } else {
            ldata->setPosX(measStartEndPos.x2);
        }
    }

    if (hPlacement == PlacementH::LEFT) {
        ldata->moveX(-itemBBox.left());
    } else if (hPlacement == PlacementH::CENTER) {
        ldata->moveX(-0.5 * (itemBBox.right() + itemBBox.left()));
    } else {
        ldata->moveX(-itemBBox.right());
    }

    // Check collisions against barlines
    if (hPlacement == PlacementH::CENTER || !item->autoplace()) {
        return;
    }

    staff_idx_t staffIdx = item->staffIdx();
    staff_idx_t barlineStaff = muse::nidx;
    System* system = item->measure()->system();
    if (item->getProperty(Pid::PLACEMENT).value<PlacementV>() == PlacementV::ABOVE) {
        barlineStaff = system->prevVisibleStaff(staffIdx);
    } else {
        barlineStaff = staffIdx;
    }
    if (barlineStaff == muse::nidx) {
        return;
    }

    const double minBarLineDistance = 0.25 * item->spatium();
    Segment* barlineSeg = nullptr;
    Measure* measure = item->measure();
    if (alignToBarline || hPlacement == PlacementH::LEFT) {
        for (Segment* seg = measure->first(); seg && seg->tick() == measure->tick(); seg = seg->prev1enabled()) {
            if (seg->isType(SegmentType::BarLineType)) {
                barlineSeg = seg;
                break;
            }
        }
    } else {
        for (Segment* seg = measure->first(SegmentType::ChordRest); seg && seg->tick() <= measure->endTick(); seg = seg->next1enabled()) {
            if (seg->isType(SegmentType::BarLineType)) {
                barlineSeg = seg;
                break;
            }
        }
    }
    if (!barlineSeg || (barlineSeg->segmentType() != SegmentType::BeginBarLine && item->score()->staff(barlineStaff)->barLineSpan() < 1)) {
        return;
    }

    BarLine* barline = toBarLine(barlineSeg->elementAt(staff2track(barlineStaff)));
    if (!barline) {
        return;
    }

    double xCur = item->pageX();
    if (hPlacement == PlacementH::LEFT) {
        double xMin = barline->pageX() + barline->width() - itemBBox.left() + minBarLineDistance;
        if (xMin > xCur) {
            ldata->moveX(xMin - xCur);
        }
    } else {
        double xMax = barline->pageX() - itemBBox.right() - minBarLineDistance;
        if (xMax < xCur) {
            ldata->moveX(xMax - xCur);
        }
    }
}

void MeasureNumberLayout::layoutMMRestRange(const MMRestRange* item, MMRestRange::LayoutData* ldata, const LayoutContext& ctx)
{
    layoutMeasureNumberBase(item, ldata);

    const Measure* measure = item->measure();
    const Segment* crSeg = measure->first(SegmentType::ChordRest);
    const MeasureLayout::MeasureStartEndPos measureStartEnd
        = MeasureLayout::getMeasureStartEndPos(measure, crSeg, item->staffIdx(), true, false, ctx);

    const RectF& itemBBox = ldata->bbox();
    PlacementH hPlacement = item->getProperty(Pid::HPLACEMENT).value<PlacementH>();
    if (hPlacement == PlacementH::CENTER) {
        const double x1 = measureStartEnd.x1;
        const double x2 = measureStartEnd.x2;
        ldata->setPosX(0.5 * (x1 + x2) - 0.5 * (itemBBox.right() + itemBBox.left()));
    } else if (hPlacement == PlacementH::RIGHT) {
        ldata->setPosX(item->measure()->ldata()->bbox().width() - itemBBox.right());
    } else {
        ldata->setPosX(measureStartEnd.x1 - itemBBox.left());
    }
}

void MeasureNumberLayout::layoutMeasureNumberBase(const MeasureNumberBase* item, MeasureNumberBase::LayoutData* ldata)
{
    ldata->setPos(PointF());

    TLayout::layoutBaseTextBase1(item, ldata);

    if (item->placeBelow()) {
        double yoff = ldata->bbox().height();

        // If there is only one line, the barline spans outside the staff lines, so the default position is not correct.
        if (item->staff()->constStaffType(item->measure()->tick())->lines() == 1) {
            yoff += 2.0 * item->spatium();
        } else {
            yoff += item->staff()->staffHeight();
        }

        ldata->setPosY(yoff);
    } else {
        double yoff = 0.0;

        // If there is only one line, the barline spans outside the staff lines, so the default position is not correct.
        if (item->staff()->constStaffType(item->measure()->tick())->lines() == 1) {
            yoff -= 2.0 * item->spatium();
        }

        ldata->setPosY(yoff);
    }
}
