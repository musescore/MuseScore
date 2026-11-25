/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2023 MuseScore Limited and others
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
#pragma once

#include "dom/articulation.h"
#include "dom/engravingitem.h"
#include "dom/spanner.h"

namespace mu::engraving {
class MeasureNumber;
}

namespace mu::engraving::rendering::score {
class Autoplace
{
public:
    static void autoplaceSegmentElement(const EngravingItem* item, EngravingItem::LayoutData* ldata, bool above, bool add);
    static void autoplaceMeasureElement(const EngravingItem* item, EngravingItem::LayoutData* ldata, bool above, bool add);
    static void autoplaceSegmentElement(const EngravingItem* item, EngravingItem::LayoutData* ldata, bool add = true)
    {
        autoplaceSegmentElement(item, ldata, item->placeAbove(), add);
    }

    static void autoplaceMeasureElement(const EngravingItem* item, EngravingItem::LayoutData* ldata, bool add = true)
    {
        autoplaceMeasureElement(item, ldata, item->placeAbove(), add);
    }

    static void autoplaceMeasureNumber(const MeasureNumber* item, EngravingItem::LayoutData* ldata);
    static void autoplaceSpannerSegment(const SpannerSegment* item, SpannerSegment::LayoutData* ldata);
    static bool itemsShouldIgnoreEachOther(const EngravingItem* itemToAutoplace, const EngravingItem* itemInSkyline);

private:
    enum class CollisionStrategy : unsigned char {
        MOVE_CLEAR,  // Move clear of everything in the skyline that overlaps horizontally
        FIT_IN_GAPS, // Move only as far as the nearest gap the item fits into
    };

    static void doAutoplaceMeasureElement(const EngravingItem* item, EngravingItem::LayoutData* ldata, bool above, bool add,
                                          CollisionStrategy strategy);

    static void doAutoplaceElement(const EngravingItem* item, EngravingItem::LayoutData* ldata, const System* system, bool above, bool add,
                                   double minSkylineHorizontalClearance, double spatium, CollisionStrategy strategy);

    static double distanceToMoveItem(const EngravingItem* item, const SkylineLine& staffSkyline, const Shape& shape, bool above,
                                     double minSkylineHorizontalClearance, double minDistance, CollisionStrategy strategy);

    // Returns the smallest move away from the staff that leaves an item in a gap between skyline elements
    static double distanceToNearestGap(const EngravingItem* item, const SkylineLine& skyline, const RectF& bbox, bool above,
                                       double minHorizontalClearance, double minDistance);
};
}
