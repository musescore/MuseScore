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

#ifndef MU_ENGRAVING_SHAPE_H
#define MU_ENGRAVING_SHAPE_H

#include "global/allocator.h"
#include "draw/types/geometry.h"

namespace mu::draw {
class Painter;
}

namespace mu::engraving {
class EngravingItem;
class Score;

//---------------------------------------------------------
//   ShapeElement
//---------------------------------------------------------

struct ShapeElement : public mu::RectF {
public:
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

class Shape
{
public:

    Shape() {}
    Shape(const mu::RectF& r, const EngravingItem* p = nullptr) { add(r, p); }

    void add(const Shape& s);
    void add(const mu::RectF& r, const EngravingItem* p);
    void add(const mu::RectF& r) { m_elements.push_back(ShapeElement(r)); }

    void remove(const mu::RectF&);
    void remove(const Shape&);
    template<typename Predicate>
    inline bool remove_if(Predicate p)
    {
        size_t origSize = m_elements.size();
        m_elements.erase(std::remove_if(m_elements.begin(), m_elements.end(), p), m_elements.end());
        return origSize != m_elements.size();
    }

    void removeInvisibles();

    void addHorizontalSpacing(EngravingItem* item, double left, double right);

    Shape& translate(const mu::PointF&);
    void translateX(double);
    void translateY(double);
    Shape translated(const mu::PointF&) const;

    double minHorizontalDistance(const Shape&) const;
    double minVerticalDistance(const Shape&) const;
    double verticalClearance(const Shape&) const;
    double topDistance(const mu::PointF&) const;
    double bottomDistance(const mu::PointF&) const;
    double left() const;
    double right() const;
    double top() const;
    double bottom() const;
    double rightMostEdgeAtHeight(double yAbove, double yBelow) const;
    double leftMostEdgeAtHeight(double yAbove, double yBelow) const;

    const std::vector<ShapeElement>& elements() const { return m_elements; }
    size_t size() const { return m_elements.size(); }
    bool empty() const { return m_elements.empty(); }
    void clear() { m_elements.clear(); }

    bool contains(const mu::PointF&) const;
    bool intersects(const mu::RectF& rr) const;
    bool intersects(const Shape&) const;
    bool clearsVertically(const Shape& a) const;

    void setSqueezeFactor(double v) { m_squeezeFactor = v; }

    void paint(mu::draw::Painter& painter) const;
#ifndef NDEBUG
    void dump(const char*) const;
#endif

private:
    std::vector<ShapeElement> m_elements;
    double m_spatium = 0.0;
    double m_squeezeFactor = 1.0;
};

//---------------------------------------------------------
//   intersects
//---------------------------------------------------------

inline static bool intersects(double a, double b, double c, double d, double verticalClearance)
{
    // return (a >= c && a < d) || (b >= c && b < d) || (a < c && b >= b);
    // return (std::max(a,b) > std::min(c,d)) && (std::min(a,b) < std::max(c,d));
    // if we can assume a <= b and c <= d
    if (a == b || c == d) {   // zero height
        return false;
    }
    return (b + verticalClearance > c) && (a < d + verticalClearance);
}
} // namespace mu::engraving

#endif
