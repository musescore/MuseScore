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

#include "infrastructure/draw/painter.h"

using namespace mu;
using namespace mu::draw;

namespace Ms {
//---------------------------------------------------------
//   addHorizontalSpacing
//    Currently implemented by adding rectangles of zero
//    height to the Y position corresponding to the type.
//    This is a simple solution but has its drawbacks too.
//---------------------------------------------------------

void Shape::addHorizontalSpacing(HorizontalSpacingType type, qreal leftEdge, qreal rightEdge)
{
    constexpr qreal eps = 100 * std::numeric_limits<qreal>::epsilon();
    const qreal y = eps * int(type);
    if (leftEdge == rightEdge) { // HACK zero-width shapes collide with everything currently.
        rightEdge += eps;
    }
    add(RectF(leftEdge, y, rightEdge - leftEdge, 0));
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
#ifndef NDEBUG
        s.add(r.translated(pt), r.text);
#else
        s.add(r.translated(pt));
#endif
    }
    return s;
}

//-------------------------------------------------------------------
//   minHorizontalDistance
//    a is located right of this shape.
//    Calculates the minimum horizontal distance between the two shapes
//    so they don’t touch.
//-------------------------------------------------------------------

qreal Shape::minHorizontalDistance(const Shape& a) const
{
    qreal dist = -1000000.0;        // min real
    for (const RectF& r2 : a) {
        qreal by1 = r2.top();
        qreal by2 = r2.bottom();
        for (const RectF& r1 : *this) {
            qreal ay1 = r1.top();
            qreal ay2 = r1.bottom();
            if (Ms::intersects(ay1, ay2, by1, by2)
                || ((r1.height() == 0.0) && (r2.height() == 0.0) && (ay1 == by1))
                || ((r1.width() == 0.0) || (r2.width() == 0.0))) {
                dist = qMax(dist, r1.right() - r2.left());
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
    qreal dist = -1000000.0;        // min real
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
            if (Ms::intersects(ax1, ax2, bx1, bx2)) {
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
    // qWarning("Shape::remove: RectF not found in Shape");
    qFatal("Shape::remove: RectF not found in Shape");
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
    qDebug("Shape dump: %p %s size %zu", this, p, size());
    for (const ShapeElement& r : *this) {
        r.dump();
    }
}

void ShapeElement::dump() const
{
    qDebug("   %s: %f %f %f %f", text ? text : "", x(), y(), width(), height());
}

//---------------------------------------------------------
//   add
//---------------------------------------------------------
void Shape::add(const RectF& r, const char* t)
{
    push_back(ShapeElement(r, t));
}

#endif

#ifdef DEBUG_SHAPES
//---------------------------------------------------------
//   testShapes
//---------------------------------------------------------

void testShapes()
{
    printf("======test shapes======\n");

    //=======================
    //    minDistance()
    //=======================
    Shape a;
    Shape b;

    a.add(RectF(-10, -10, 20, 20));
    qreal d = a.minHorizontalDistance(b);             // b is empty
    printf("      minHDistance (0.0): %f", d);
    if (d != 0.0) {
        printf("   =====error");
    }
    printf("\n");

    b.add(RectF(0, 0, 10, 10));
    d = a.minHorizontalDistance(b);
    printf("      minHDistance (10.0): %f", d);
    if (d != 10.0) {
        printf("   =====error");
    }
    printf("\n");

    d = a.minVerticalDistance(b);
    printf("      minVDistance (10.0): %f", d);
    if (d != 10.0) {
        printf("   =====error");
    }
    printf("\n");
}

#endif // DEBUG_SHAPES
} // namespace Ms
