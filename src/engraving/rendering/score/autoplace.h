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
#ifndef MU_ENGRAVING_AUTOPLACE_DEV_H
#define MU_ENGRAVING_AUTOPLACE_DEV_H

#include "dom/articulation.h"
#include "dom/engravingitem.h"
#include "dom/spanner.h"

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

    static void autoplaceSpannerSegment(const SpannerSegment* item, SpannerSegment::LayoutData* ldata, double spatium);

    static double rebaseOffset(const EngravingItem* item, EngravingItem::LayoutData* ldata, bool nox = true);
    static bool rebaseMinDistance(const EngravingItem* item, EngravingItem::LayoutData* ldata, double& md, double& yd, double sp,
                                  double rebase, bool above, bool fix);

    static void setOffsetChanged(const EngravingItem* item, EngravingItem::LayoutData* ldata, bool v, bool absolute = true,
                                 const PointF& diff = PointF());

    static void doAutoplace(const Articulation* item, Articulation::LayoutData* ldata);

private:
    static bool itemsShouldIgnoreEachOther(const EngravingItem* itemToAutoplace, const EngravingItem* itemInSkyline);
};
}

#endif // MU_ENGRAVING_AUTOPLACE_DEV_H
