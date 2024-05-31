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
#include "score.h"
#include "spanner.h"
#include "system.h"
#include "page.h"

#include "rendering/dev/measurelayout.h"

using namespace mu::engraving::rendering::dev;

namespace mu::engraving {
/***************************************
 * EditTimeTickAnchors
 * ************************************/

void EditTimeTickAnchors::updateAnchors(const EngravingItem* item, Fraction absTick, track_idx_t track)
{
    if (!item->allowTimeAnchor()) {
        item->score()->hideAnchors();
        return;
    }

    const Score* score = item->score();
    Measure* measure = score->tick2measure(absTick);
    if (!measure) {
        return;
    }

    Fraction measureTick = measure->tick();
    Measure* prevMeasure = measureTick == absTick ? measure->prevMeasure() : nullptr;

    staff_idx_t staff = track2staff(track);
    if (prevMeasure) {
        updateAnchors(prevMeasure, staff);
    }
    updateAnchors(measure, staff);
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
    for (Segment* seg = measure->first(SegmentType::ChordRest); seg; seg = seg->next(SegmentType::ChordRest)) {
        anchorTicks.insert(seg->rtick());
    }

    for (const Fraction& anchorTick : anchorTicks) {
        createTimeTickAnchor(measure, anchorTick, staffIdx);
    }
    measure->computeTicks();

    Score* score = measure->score();

    LayoutContext ctx(score);
    MeasureLayout::layoutTimeTickAnchors(measure, ctx);

    score->updateShowAnchors(staffIdx, measure->tick(), measure->endTick());
}

TimeTickAnchor* EditTimeTickAnchors::createTimeTickAnchor(Measure* measure, Fraction relTick, staff_idx_t staffIdx)
{
    Segment* segment = measure->getSegmentR(SegmentType::TimeTick, relTick);

    track_idx_t track = staff2track(staffIdx);
    EngravingItem* element = segment->elementAt(track);
    TimeTickAnchor* anchor = element ? toTimeTickAnchor(element) : nullptr;
    if (!anchor) {
        anchor = Factory::createTimeTickAnchor(segment);
        anchor->setParent(segment);
        anchor->setTrack(track);
        segment->add(anchor);
        if (System* system = measure->system()) {
            if (Page* page = system->page()) {
                page->invalidateBspTree();
            }
        }
    }

    return anchor;
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

bool TimeTickAnchor::isDraw() const
{
    const ShowAnchors& showAnchors = score()->showAnchors();
    Fraction thisTick = segment()->tick();

    return showAnchors.staffIdx == staffIdx() && showAnchors.startTick <= thisTick && showAnchors.endTick > thisTick;
}
} // namespace mu::engraving
