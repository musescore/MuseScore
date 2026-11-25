/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2026 MuseScore Limited and others
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

#include "notationautomationutils.h"

#include <algorithm>

#include "engraving/dom/measure.h"
#include "engraving/dom/segment.h"
#include "engraving/dom/system.h"

#include "log.h"

using namespace mu::engraving;

namespace mu::notation {
muse::RectF regionCanvasRect(const System* system, staff_idx_t staffIdx, const Measure* startMeasure, const Measure* endMeasure)
{
    const SysStaff* sysStaff = system ? system->staff(staffIdx) : nullptr;
    if (!sysStaff || !startMeasure || !endMeasure) {
        return muse::RectF();
    }
    const muse::RectF staffCanvasRect = sysStaff->bbox().translated(system->canvasPos());
    const double left = startMeasure->canvasX();
    const double right = endMeasure->canvasX() + endMeasure->width();
    return muse::RectF(left, staffCanvasRect.y(), right - left, staffCanvasRect.height());
}

// A point on a region's end tick is at the start of the next measure (so it belongs
// to whatever follows the region) unless the region ends the score...
int regionLastTick(const Measure* endMeasure)
{
    const int endTick = endMeasure->endTick().ticks();
    return endMeasure->nextMeasureMM() ? endTick - 1 : endTick;
}

int tickFromXInPolyline(const PolylineKey& key, const PolylineData& data, qreal x)
{
    const Measure* startMeasure = data.startMeasure;
    const Measure* endMeasure = data.endMeasure;
    IF_ASSERT_FAILED(key.isValid() && startMeasure && endMeasure) {
        return -1;
    }

    const muse::RectF regionRect = regionCanvasRect(key.system, key.staffIdx, startMeasure, endMeasure);
    IF_ASSERT_FAILED(regionRect.isValid()) {
        return -1;
    }

    const double pointCanvasX = regionRect.x() + x * regionRect.width();
    const mu::engraving::SegmentType type = mu::engraving::SegmentType::Duration | mu::engraving::SegmentType::BarLineTypes;

    const Segment* lastSegInRegion = endMeasure->last(type);
    IF_ASSERT_FAILED(lastSegInRegion) {
        return -1;
    }

    const Segment* prevSeg = nullptr;
    const Segment* nextSeg = nullptr;
    for (const Segment* seg = startMeasure->first(type); seg; seg = seg->next1(type)) {
        if (seg->canvasX() <= pointCanvasX) {
            prevSeg = seg;
        } else {
            nextSeg = seg;
            break;
        }
        if (seg == lastSegInRegion) {
            break;
        }
    }

    // The end barline's tick (when there's a measure after it) is already in the next region...
    const int lastTick = regionLastTick(endMeasure);

    if (!prevSeg) {
        // The point is before the first segment, so snap to the next segment's tick (or none)...
        return nextSeg ? std::min(nextSeg->tick().ticks(), lastTick) : -1;
    }

    // No next segment - use prevSeg's own end as a virtual next point
    const double nextCanvasX = nextSeg ? nextSeg->canvasX() : prevSeg->canvasX() + prevSeg->width();
    const int nextTick = nextSeg ? nextSeg->tick().ticks() : prevSeg->tick().ticks() + prevSeg->ticks().ticks();
    const double canvasSpan = nextCanvasX - prevSeg->canvasX();
    const double ratio = canvasSpan > 0.0 ? (pointCanvasX - prevSeg->canvasX()) / canvasSpan : 0.0;

    // Interpolate between prevSeg's tick and the next tick (but don't go past the last tick that belongs to this region)...
    const int prevTick = prevSeg->tick().ticks();
    const int interpolatedTick = prevTick + static_cast<int>(ratio * (nextTick - prevTick));
    return std::min(interpolatedTick, lastTick);
}
}
