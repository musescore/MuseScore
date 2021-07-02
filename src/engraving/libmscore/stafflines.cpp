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
#include "system.h"
#include "measure.h"
#include "score.h"
#include "stafftype.h"
#include "staff.h"

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

namespace Ms {
//---------------------------------------------------------
//   StaffLines
//---------------------------------------------------------

StaffLines::StaffLines(Score* s)
    : Element(s)
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
    Element* e = parent();
    while (e) {
        if (e->type() == ElementType::PAGE) {
            p += e->pos();
            break;
        }
        e = e->parent();
    }
    return p;
}

//---------------------------------------------------------
//   layout
//---------------------------------------------------------

void StaffLines::layout()
{
    layoutForWidth(measure()->width());
}

//---------------------------------------------------------
//   layoutForWidth
//---------------------------------------------------------

void StaffLines::layoutForWidth(qreal w)
{
    const Staff* s = staff();
    qreal _spatium = spatium();
    qreal dist     = _spatium;
    setPos(PointF(0.0, 0.0));
    int _lines;
    if (s) {
        setMag(s->staffMag(measure()->tick()));
        setVisible(!s->invisible(measure()->tick()));
        setColor(s->color(measure()->tick()));
        const StaffType* st = s->staffType(measure()->tick());
        dist         *= st->lineDistance().val();
        _lines        = st->lines();
        rypos()       = st->yoffset().val() * _spatium;
//            if (_lines == 1)
//                  rypos() = 2 * _spatium;
    } else {
        _lines = 5;
        setColor(MScore::defaultColor);
    }
    lw       = score()->styleS(Sid::staffLineWidth).val() * _spatium;
    qreal x1 = pos().x();
    qreal x2 = x1 + w;
    qreal y  = pos().y();
    bbox().setRect(x1, -lw * .5 + y, w, (_lines - 1) * dist + lw);

    if (_lines == 1) {
        qreal extraSize = _spatium;
        bbox().adjust(0, -extraSize, 0, extraSize);
    } else if (_lines == 0) {
        bbox().adjust(0, -2 * dist, 0, 2 * dist);
    }

    lines.clear();
    for (int i = 0; i < _lines; ++i) {
        lines.push_back(LineF(x1, y, x2, y));
        y += dist;
    }
}

//---------------------------------------------------------
//   layoutPartialWidth
///   Layout staff lines for the specified width only, aligned
///   to the left or right of the measure
//---------------------------------------------------------

void StaffLines::layoutPartialWidth(qreal w, qreal wPartial, bool alignRight)
{
    const Staff* s = staff();
    qreal _spatium = spatium();
    wPartial *= spatium();
    qreal dist     = _spatium;
    setPos(PointF(0.0, 0.0));
    int _lines;
    if (s) {
        setMag(s->staffMag(measure()->tick()));
        setColor(s->color(measure()->tick()));
        const StaffType* st = s->staffType(measure()->tick());
        dist         *= st->lineDistance().val();
        _lines        = st->lines();
        rypos()       = st->yoffset().val() * _spatium;
    } else {
        _lines = 5;
        setColor(MScore::defaultColor);
    }
    lw       = score()->styleS(Sid::staffLineWidth).val() * _spatium;
    qreal x1 = pos().x();
    qreal x2 = x1 + w;
    qreal y  = pos().y();
    bbox().setRect(x1, -lw * .5 + y, w, (_lines - 1) * dist + lw);

    if (_lines == 1) {
        qreal extraSize = _spatium;
        bbox().adjust(0, -extraSize, 0, extraSize);
    }

    lines.clear();
    for (int i = 0; i < _lines; ++i) {
        if (alignRight) {
            lines.push_back(LineF(x2 - wPartial, y, x2, y));
        } else {
            lines.push_back(LineF(x1, y, x1 + wPartial, y));
        }
        y += dist;
    }
}

//---------------------------------------------------------
//   draw
//---------------------------------------------------------

void StaffLines::draw(mu::draw::Painter* painter) const
{
    TRACE_OBJ_DRAW;
    using namespace mu::draw;
    painter->setPen(Pen(curColor(), lw, PenStyle::SolidLine, PenCapStyle::FlatCap));
    painter->drawLines(lines);
}

//---------------------------------------------------------
//   y1
//---------------------------------------------------------

qreal StaffLines::y1() const
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

void StaffLines::scanElements(void* data, void (* func)(void*, Element*), bool all)
{
    if (all || (measure()->visible(staffIdx()) && score()->staff(staffIdx())->show())) {
        func(data, this);
    }
}
}
