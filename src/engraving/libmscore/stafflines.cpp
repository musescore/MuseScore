/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-CLA-applies
 *
 * MuseScore
 * Music Composition & Notation
 *
 * Copyright (C) 2021 MuseScore BVBA and others
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

#include "layout/tlayout.h"

#include "measure.h"
#include "score.h"
#include "staff.h"
#include "stafftype.h"
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

//---------------------------------------------------------
<<<<<<< HEAD
=======
//   layout
//---------------------------------------------------------

void StaffLines::layout()
{
    UNREACHABLE;
    LayoutContext ctx(score());
    v0::TLayout::layout(this, ctx);
}

//---------------------------------------------------------
>>>>>>> 4f8a1b6dd0... [engraving] replaced item->layout() to TLayout::layout
//   layoutForWidth
//---------------------------------------------------------

void StaffLines::layoutForWidth(double w)
{
    LayoutContext ctx(score());
    v0::TLayout::layoutForWidth(this, w, ctx);
}

//---------------------------------------------------------
//   layoutPartialWidth
///   Layout staff lines for the specified width only, aligned
///   to the left or right of the measure
//---------------------------------------------------------

void StaffLines::layoutPartialWidth(double w, double wPartial, bool alignRight)
{
    const Staff* s = staff();
    double _spatium = spatium();
    wPartial *= spatium();
    double dist     = _spatium;
    setPos(PointF(0.0, 0.0));
    int _lines;
    if (s) {
        setMag(s->staffMag(measure()->tick()));
        setColor(s->color(measure()->tick()));
        const StaffType* st = s->staffType(measure()->tick());
        dist         *= st->lineDistance().val();
        _lines        = st->lines();
        setPosY(st->yoffset().val() * _spatium);
    } else {
        _lines = 5;
        setColor(engravingConfiguration()->defaultColor());
    }
    m_lw       = score()->styleS(Sid::staffLineWidth).val() * _spatium;
    double x1 = pos().x();
    double x2 = x1 + w;
    double y  = pos().y();
    bbox().setRect(x1, -m_lw * .5 + y, w, (_lines - 1) * dist + m_lw);

    if (_lines == 1) {
        double extraSize = _spatium;
        bbox().adjust(0, -extraSize, 0, extraSize);
    }

    m_lines.clear();
    for (int i = 0; i < _lines; ++i) {
        if (alignRight) {
            m_lines.push_back(LineF(x2 - wPartial, y, x2, y));
        } else {
            m_lines.push_back(LineF(x1, y, x1 + wPartial, y));
        }
        y += dist;
    }
}

RectF StaffLines::hitBBox() const
{
    double clickablePadding = spatium();
    if (m_lines.size() <= 1) {
        return bbox().adjusted(0.0, -clickablePadding, 0.0, clickablePadding);
    }
    return bbox();
}

Shape StaffLines::hitShape() const
{
    return Shape(hitBBox(), this);
}

//---------------------------------------------------------
//   draw
//---------------------------------------------------------

void StaffLines::draw(mu::draw::Painter* painter) const
{
    TRACE_ITEM_DRAW;
    using namespace mu::draw;
    painter->setPen(Pen(curColor(), m_lw, PenStyle::SolidLine, PenCapStyle::FlatCap));
    painter->drawLines(m_lines);
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
    return system->staff(staffIdx())->y() + ipos().y();
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
