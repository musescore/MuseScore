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

namespace mu::draw {
class Painter;
}

namespace mu::engraving {
class Segment;
class EngravingItem;
class Score;
class Measure;

//---------------------------------------------------------
//   ShapeElement
//---------------------------------------------------------

struct ShapeElement : public mu::RectF {
    const EngravingItem* toItem = nullptr;
    ShapeElement(const mu::RectF& f, const EngravingItem* p)
        : mu::RectF(f), toItem(p) {}
    ShapeElement(const mu::RectF& f)
        : mu::RectF(f) {}
#ifndef NDEBUG
    void dump() const;
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
    Shape(const mu::RectF& r, const EngravingItem* p) { add(r, p); }
    Shape(const mu::RectF& r) { add(r); }

    void add(const Shape& s) { insert(end(), s.begin(), s.end()); }
    void add(const mu::RectF& r, const EngravingItem* p) { push_back(ShapeElement(r, p)); }
    void add(const mu::RectF& r) { push_back(ShapeElement(r)); }

    void remove(const mu::RectF&);
    void remove(const Shape&);

    void addHorizontalSpacing(EngravingItem* item, qreal left, qreal right);

    void translate(const mu::PointF&);
    void translateX(qreal);
    void translateY(qreal);
    Shape translated(const mu::PointF&) const;

    qreal minHorizontalDistance(const Shape&, Score* score) const;
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

    void paint(mu::draw::Painter& painter) const;
#ifndef NDEBUG
    void dump(const char*) const;
#endif
};

//---------------------------------------------------------
//   intersects
//---------------------------------------------------------

inline static bool intersects(qreal a, qreal b, qreal c, qreal d, qreal verticalClearance)
{
    // return (a >= c && a < d) || (b >= c && b < d) || (a < c && b >= b);
    // return (std::max(a,b) > std::min(c,d)) && (std::min(a,b) < std::max(c,d));
    // if we can assume a <= b and c <= d
    if (a == b || c == d) {   // zero height
        return false;
    }
    return (b + verticalClearance > c) && (a < d + verticalClearance);
}
} // namespace Ms

#endif
