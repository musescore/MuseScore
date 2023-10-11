/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-CLA-applies
 *
 * MuseScore
 * Music Composition & Notation
 *
 * Copyright (C) 2023 MuseScore BVBA and others
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
#include "shapeutils.h"

#include "dom/engravingitem.h"

//-------------------------------------------------------------------
//   minHorizontalDistance
//    a is located right of this shape.
//    Calculates the minimum horizontal distance between the two shapes
//    so they don’t touch.
//-------------------------------------------------------------------

double mu::engraving::rendering::dev::minHorizontalDistance(const Shape& f, const Shape& s, double spatium, double squeezeFactor)
{
    double dist = -1000000.0;        // min real
    double absoluteMinPadding = 0.1 * spatium * squeezeFactor;
    double verticalClearance = 0.2 * spatium * squeezeFactor;
    for (const ShapeElement& r2 : s.elements()) {
        if (r2.isNull()) {
            continue;
        }
        const EngravingItem* item2 = r2.toItem;
        double by1 = r2.top();
        double by2 = r2.bottom();
        for (const ShapeElement& r1 : f.elements()) {
            if (r1.isNull()) {
                continue;
            }
            const EngravingItem* item1 = r1.toItem;
            double ay1 = r1.top();
            double ay2 = r1.bottom();
            bool intersection = mu::engraving::intersects(ay1, ay2, by1, by2, verticalClearance);
            double padding = 0;
            KerningType kerningType = KerningType::NON_KERNING;
            if (item1 && item2) {
                padding = EngravingItem::renderer()->computePadding(item1, item2);
                padding *= squeezeFactor;
                padding = std::max(padding, absoluteMinPadding);
                kerningType = EngravingItem::renderer()->computeKerning(item1, item2);
            }
            if ((intersection && kerningType != KerningType::ALLOW_COLLISION)
                || (r1.width() == 0 || r2.width() == 0)  // Temporary hack: shapes of zero-width are assumed to collide with everyghin
                || (!item1 && item2 && item2->isLyrics())  // Temporary hack: avoids collision with melisma line
                || kerningType == KerningType::NON_KERNING) {
                dist = std::max(dist, r1.right() - r2.left() + padding);
            }
            if (kerningType == KerningType::KERNING_UNTIL_ORIGIN) { //prepared for future user option, for now always false
                double origin = r1.left();
                dist = std::max(dist, origin - r2.left());
            }
        }
    }
    return dist;
}
