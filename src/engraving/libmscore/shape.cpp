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

#include "shape.h"
#include "segment.h"
#include "chord.h"
#include "score.h"
#include "system.h"

#include "infrastructure/draw/painter.h"

#include "log.h"

using namespace mu;
using namespace mu::draw;

namespace mu::engraving {
//---------------------------------------------------------
//   addHorizontalSpacing
//    This methods creates "walls". They are represented by
//    rectangles of zero height, and it is assumed that rectangles
//    of zero height vertically collide with everything. Use this
//    method ONLY when you want to crate space that cannot tuck
//    above/below other elements of the staff.
//---------------------------------------------------------

void Shape::addHorizontalSpacing(EngravingItem* item, qreal leftEdge, qreal rightEdge)
{
    constexpr qreal eps = 100 * std::numeric_limits<qreal>::epsilon();
    if (leftEdge == rightEdge) { // HACK zero-width shapes collide with everything currently.
        rightEdge += eps;
    }
    add(RectF(leftEdge, 0, rightEdge - leftEdge, 0), item);
}

//---------------------------------------------------------
//   translate
//---------------------------------------------------------

void Shape::translate(const PointF& pt)
{
    for (RectF& r : *this) {
        r.translate(pt);
    }
}

void Shape::translateX(qreal xo)
{
    for (RectF& r : *this) {
        r.setLeft(r.left() + xo);
        r.setRight(r.right() + xo);
    }
}

void Shape::translateY(qreal yo)
{
    for (RectF& r : *this) {
        r.setTop(r.top() + yo);
        r.setBottom(r.bottom() + yo);
    }
}

//---------------------------------------------------------
//   translated
//---------------------------------------------------------

Shape Shape::translated(const PointF& pt) const
{
    Shape s;
    for (const ShapeElement& r : *this) {
        s.add(r.translated(pt), r.toItem);
    }
    return s;
}

//-------------------------------------------------------------------
//   minHorizontalDistance
//    a is located right of this shape.
//    Calculates the minimum horizontal distance between the two shapes
//    so they don’t touch.
//-------------------------------------------------------------------

qreal Shape::minHorizontalDistance(const Shape& a, Score* score) const
{
    qreal dist = -1000000.0;        // min real
    double verticalClearance = 0.2 * score->spatium();
    for (const ShapeElement& r2 : a) {
        const EngravingItem* item2 = r2.toItem;
        qreal by1 = r2.top();
        qreal by2 = r2.bottom();
        for (const ShapeElement& r1 : *this) {
            const EngravingItem* item1 = r1.toItem;
            qreal ay1 = r1.top();
            qreal ay2 = r1.bottom();
            bool intersection = mu::engraving::intersects(ay1, ay2, by1, by2, verticalClearance);
            double padding = 0;
            KerningType kerningType = KerningType::NON_KERNING;
            if (item1 && item2) {
                padding = item1->computePadding(item2);
                kerningType = item1->computeKerningType(item2);
            }
            if (intersection
                || (r1.width() == 0 || r2.width() == 0) // Temporary hack: shapes of zero-width are assumed to collide with everyghin
                || (!item1 && item2 && item2->isLyrics()) // Temporary hack: avoids collision with melisma line
                || kerningType == KerningType::NON_KERNING) {
                dist = qMax(dist, r1.right() - r2.left() + padding);
            }
            if (kerningType == KerningType::KERNING_UNTIL_ORIGIN) { //prepared for future user option, for now always false
                qreal origin = r1.left();
                dist = qMax(dist, origin - r2.left());
            }
        }
    }
    return dist;
}

//-------------------------------------------------------------------
//   minVerticalDistance
//    a is located below this shape.
//    Calculates the minimum distance between two shapes.
//-------------------------------------------------------------------

qreal Shape::minVerticalDistance(const Shape& a) const
{
    if (empty() || a.empty()) {
        return 0.0;
    }

    qreal dist = -1000000.0; // min real
    for (const RectF& r2 : a) {
        if (r2.height() <= 0.0) {
            continue;
        }
        qreal bx1 = r2.left();
        qreal bx2 = r2.right();
        for (const RectF& r1 : *this) {
            if (r1.height() <= 0.0) {
                continue;
            }
            qreal ax1 = r1.left();
            qreal ax2 = r1.right();
            if (mu::engraving::intersects(ax1, ax2, bx1, bx2, 0.0)) {
                dist = qMax(dist, r1.bottom() - r2.top());
            }
        }
    }
    return dist;
}

//---------------------------------------------------------
//   left
//    compute left border
//---------------------------------------------------------

qreal Shape::left() const
{
    qreal dist = 0.0;
    for (const RectF& r : *this) {
        if (r.height() != 0.0 && r.left() < dist) {
            // if (r.left() < dist)
            dist = r.left();
        }
    }
    return -dist;
}

//---------------------------------------------------------
//   right
//    compute right border
//---------------------------------------------------------

qreal Shape::right() const
{
    qreal dist = 0.0;
    for (const RectF& r : *this) {
        if (r.right() > dist) {
            dist = r.right();
        }
    }
    return dist;
}

/* NOTE: these top() and bottom() methods look very weird to me, as they
 * seem to return the opposite of what they say. Or it seems like the
 * rectangles are defined upside down, for some reason. Needs some
 * more understanding. [M.S.] */

//---------------------------------------------------------
//   top
//---------------------------------------------------------

qreal Shape::top() const
{
    qreal dist = 1000000.0;
    for (const RectF& r : *this) {
        if (r.top() < dist) {
            dist = r.top();
        }
    }
    return dist;
}

//---------------------------------------------------------
//   bottom
//---------------------------------------------------------

qreal Shape::bottom() const
{
    qreal dist = -1000000.0;
    for (const RectF& r : *this) {
        if (r.bottom() > dist) {
            dist = r.bottom();
        }
    }
    return dist;
}

//---------------------------------------------------------
//   topDistance
//    p is on top of shape
//    returns negative values if there is an overlap
//---------------------------------------------------------

qreal Shape::topDistance(const PointF& p) const
{
    qreal dist = 1000000.0;
    for (const RectF& r : *this) {
        if (p.x() >= r.left() && p.x() < r.right()) {
            dist = qMin(dist, r.top() - p.y());
        }
    }
    return dist;
}

//---------------------------------------------------------
//   bottomDistance
//    p is below the shape
//    returns negative values if there is an overlap
//---------------------------------------------------------

qreal Shape::bottomDistance(const PointF& p) const
{
    qreal dist = 1000000.0;
    for (const RectF& r : *this) {
        if (p.x() >= r.left() && p.x() < r.right()) {
            dist = qMin(dist, p.y() - r.bottom());
        }
    }
    return dist;
}

//---------------------------------------------------------
//   remove
//---------------------------------------------------------

void Shape::remove(const RectF& r)
{
    for (auto i = begin(); i != end(); ++i) {
        if (*i == r) {
            erase(i);
            return;
        }
    }

    ASSERT_X("Shape::remove: RectF not found in Shape");
}

void Shape::remove(const Shape& s)
{
    for (const RectF& r : s) {
        remove(r);
    }
}

//---------------------------------------------------------
//   contains
//---------------------------------------------------------

bool Shape::contains(const PointF& p) const
{
    for (const RectF& r : *this) {
        if (r.contains(p)) {
            return true;
        }
    }
    return false;
}

//---------------------------------------------------------
//   intersects
//---------------------------------------------------------

bool Shape::intersects(const RectF& rr) const
{
    for (const RectF& r : *this) {
        if (r.intersects(rr)) {
            return true;
        }
    }
    return false;
}

//---------------------------------------------------------
//   intersects
//---------------------------------------------------------

bool Shape::intersects(const Shape& other) const
{
    for (const RectF& r : other) {
        if (intersects(r)) {
            return true;
        }
    }
    return false;
}

void Shape::paint(Painter& painter) const
{
    for (const RectF& r : *this) {
        painter.drawRect(r);
    }
}

#ifndef NDEBUG
//---------------------------------------------------------
//   dump
//---------------------------------------------------------

void Shape::dump(const char* p) const
{
    LOGD("Shape dump: %p %s size %zu", this, p, size());
    for (const ShapeElement& r : *this) {
        r.dump();
    }
}

void ShapeElement::dump() const
{
    LOGD("   %s: %f %f %f %f", toItem ? toItem->typeName() : "", x(), y(), width(), height());
}

#endif
} // namespace mu::engraving
