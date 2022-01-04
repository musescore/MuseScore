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

#ifndef __SHAPE_H__
#define __SHAPE_H__

#include "infrastructure/draw/geometry.h"

namespace Ms {
#ifndef NDEBUG
// #define DEBUG_SHAPES    // enable shape debugging
#endif

class Segment;

//---------------------------------------------------------
//   ShapeElement
//---------------------------------------------------------

struct ShapeElement : public mu::RectF {
#ifndef NDEBUG
    const char* text;
    void dump() const;
    ShapeElement(const mu::RectF& f, const char* t = 0)
        : mu::RectF(f), text(t) {}
#else
    ShapeElement(const mu::RectF& f)
        : mu::RectF(f) {}
#endif
};

//---------------------------------------------------------
//   Shape
//---------------------------------------------------------

class Shape : public std::vector<ShapeElement>
{
// class Shape : std::vector<ShapeElement> {
public:
    enum HorizontalSpacingType {
        SPACING_GENERAL = 0,
        SPACING_LYRICS,
        SPACING_HARMONY,
    };

    Shape() {}
#ifndef NDEBUG
    Shape(const mu::RectF& r, const char* s = 0) { add(r, s); }
#else
    Shape(const mu::RectF& r) { add(r); }
#endif
    void add(const Shape& s) { insert(end(), s.begin(), s.end()); }
#ifndef NDEBUG
    void add(const mu::RectF& r, const char* t = 0);
#else
    void add(const mu::RectF& r) { push_back(r); }
#endif
    void remove(const mu::RectF&);
    void remove(const Shape&);

    void addHorizontalSpacing(HorizontalSpacingType type, qreal left, qreal right);

    void translate(const mu::PointF&);
    void translateX(qreal);
    void translateY(qreal);
    Shape translated(const mu::PointF&) const;

    qreal minHorizontalDistance(const Shape&) const;
    qreal minVerticalDistance(const Shape&) const;
    qreal topDistance(const mu::PointF&) const;
    qreal bottomDistance(const mu::PointF&) const;
    qreal left() const;
    qreal right() const;
    qreal top() const;
    qreal bottom() const;

    size_t size() const { return std::vector<ShapeElement>::size(); }
    bool empty() const { return std::vector<ShapeElement>::empty(); }
    void clear() { std::vector<ShapeElement>::clear(); }

    bool contains(const mu::PointF&) const;
    bool intersects(const mu::RectF& rr) const;
    bool intersects(const Shape&) const;

#ifndef NDEBUG
    void dump(const char*) const;
#endif
};

//---------------------------------------------------------
//   intersects
//---------------------------------------------------------

inline static bool intersects(qreal a, qreal b, qreal c, qreal d)
{
    // return (a >= c && a < d) || (b >= c && b < d) || (a < c && b >= b);
    // return (std::max(a,b) > std::min(c,d)) && (std::min(a,b) < std::max(c,d));
    // if we can assume a <= b and c <= d
    if (a == b || c == d) {   // zero height
        return false;
    }
    return (b > c) && (a < d);
}

#ifdef DEBUG_SHAPES
extern void testShapes();
#endif
} // namespace Ms

#endif
