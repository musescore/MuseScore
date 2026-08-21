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

#include "segmentcanvasinterpolation.h"

#include "engraving/dom/segment.h"
#include "engraving/dom/system.h"

using namespace mu::notation;
using namespace mu::engraving;

std::optional<int> mu::notation::tickFromCanvasX(const System* system, double canvasX)
{
    IF_ASSERT_FAILED(system) {
        return std::nullopt;
    }

    const SegmentType type = SegmentType::Duration | SegmentType::BarLineTypes;

    const Segment* prevSeg = nullptr;
    const Segment* nextSeg = nullptr;
    for (const Segment* seg = system->firstMeasure() ? system->firstMeasure()->first(type) : nullptr;
         seg && seg->system() == system; seg = seg->next1(type)) {
        if (seg->canvasX() <= canvasX) {
            prevSeg = seg;
        } else {
            nextSeg = seg;
            break;
        }
    }

    if (!prevSeg) {
        return nextSeg ? std::make_optional(nextSeg->tick().ticks()) : std::nullopt;
    }

    const double nextCanvasX = nextSeg ? nextSeg->canvasX() : prevSeg->canvasX() + prevSeg->width();
    const int nextTick = nextSeg ? nextSeg->tick().ticks() : prevSeg->tick().ticks() + prevSeg->ticks().ticks();
    const double canvasSpan = nextCanvasX - prevSeg->canvasX();
    const double ratio = canvasSpan > 0.0 ? (canvasX - prevSeg->canvasX()) / canvasSpan : 0.0;

    return prevSeg->tick().ticks() + static_cast<int>(ratio * (nextTick - prevSeg->tick().ticks()));
}

std::optional<double> mu::notation::canvasXFromTick(const System* system, int tick)
{
    IF_ASSERT_FAILED(system) {
        return std::nullopt;
    }

    const SegmentType type = SegmentType::Duration | SegmentType::BarLineTypes;

    const Segment* prevSeg = nullptr;
    const Segment* nextSeg = nullptr;
    for (const Segment* seg = system->firstMeasure() ? system->firstMeasure()->first(type) : nullptr;
         seg && seg->system() == system; seg = seg->next1(type)) {
        if (seg->tick().ticks() <= tick) {
            prevSeg = seg;
        } else {
            nextSeg = seg;
            break;
        }
    }

    if (!prevSeg) {
        return nextSeg ? std::make_optional(nextSeg->canvasX()) : std::nullopt;
    }

    const int nextTick = nextSeg ? nextSeg->tick().ticks() : prevSeg->tick().ticks() + prevSeg->ticks().ticks();
    const double nextCanvasX = nextSeg ? nextSeg->canvasX() : prevSeg->canvasX() + prevSeg->width();
    const int tickSpan = nextTick - prevSeg->tick().ticks();
    const double ratio = tickSpan > 0 ? static_cast<double>(tick - prevSeg->tick().ticks()) / tickSpan : 0.0;

    return prevSeg->canvasX() + ratio * (nextCanvasX - prevSeg->canvasX());
}
