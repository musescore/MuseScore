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

#include "stafflines.h"

#include "measure.h"
#include "score.h"
#include "staff.h"
#include "system.h"

using namespace mu;

// Anatomy of StaffLines:
//
//    step         - The possible vertical positions of a note are counted as steps.
//                   The top staff line is step position zero.
//    lines        - number of visible staff lines
//    lineDistance - The distance between lines, measured in step units. A standard five line
//                   staff has a line distance of two steps.
//    stepDistance - The distance between steps measured in scaled spatium/2 units. A standard five
//                   line staff has a step distance of 0.5 which results in a line distance of
//                   one spatium. The spatium unit is scaled by staff size.
//    yoffset      - vertical offset to align with other staves of different height
//    stepOffset   - This value changes the staff line step numbering.
//

namespace mu::engraving {
//---------------------------------------------------------
//   StaffLines
//---------------------------------------------------------

StaffLines::StaffLines(Measure* parent)
    : EngravingItem(ElementType::STAFF_LINES, parent)
{
    setSelectable(false);
}

//---------------------------------------------------------
//   pagePos
//---------------------------------------------------------

PointF StaffLines::pagePos() const
{
    System* system = measure()->system();
    return PointF(measure()->x() + system->x(), system->staff(staffIdx())->y() + system->y());
}

//---------------------------------------------------------
//   canvasPos
//---------------------------------------------------------

PointF StaffLines::canvasPos() const
{
    PointF p(pagePos());
    EngravingItem* e = parentItem();
    while (e) {
        if (e->type() == ElementType::PAGE) {
            p += e->pos();
            break;
        }
        e = e->parentItem();
    }
    return p;
}

RectF StaffLines::hitBBox() const
{
    double clickablePadding = spatium();
    if (m_lines.size() <= 1) {
        return ldata()->bbox().adjusted(0.0, -clickablePadding, 0.0, clickablePadding);
    }
    return ldata()->bbox();
}

Shape StaffLines::hitShape() const
{
    return Shape(hitBBox(), this);
}

//---------------------------------------------------------
//   y1
//---------------------------------------------------------

double StaffLines::y1() const
{
    System* system = measure()->system();
/*      if (system == 0 || staffIdx() >= system->staves()->size())
            return 0.0;
      */
    return system->staff(staffIdx())->y() + ldata()->pos().y();
}

//---------------------------------------------------------
//   scanElements
//---------------------------------------------------------

void StaffLines::scanElements(void* data, void (* func)(void*, EngravingItem*), bool all)
{
    if (all || (measure()->visible(staffIdx()) && score()->staff(staffIdx())->show())) {
        func(data, this);
    }
}
}
