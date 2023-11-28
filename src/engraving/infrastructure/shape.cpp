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

#include "shape.h"

#include "draw/painter.h"

#include "dom/engravingitem.h"

#include "log.h"

using namespace mu;
using namespace mu::draw;
using namespace mu::engraving;

//---------------------------------------------------------
//   addHorizontalSpacing
//    This methods creates "walls". They are represented by
//    rectangles of zero height, and it is assumed that rectangles
//    of zero height vertically collide with everything. Use this
//    method ONLY when you want to create space that cannot tuck
//    above/below other elements of the staff.
//---------------------------------------------------------

void Shape::addHorizontalSpacing(EngravingItem* item, double leftEdge, double rightEdge)
{
    constexpr double eps = 100 * std::numeric_limits<double>::epsilon();
    if (leftEdge == rightEdge) { // HACK zero-width shapes collide with everything currently.
        rightEdge += eps;
    }
    add(RectF(leftEdge, 0, rightEdge - leftEdge, 0), item);
}

//---------------------------------------------------------
//   translate
//---------------------------------------------------------

Shape& Shape::translate(const PointF& pt)
{
    for (RectF& r : m_elements) {
        r.translate(pt);
    }
    invalidateBBox();
    return *this;
}

void Shape::translateX(double xo)
{
    for (RectF& r : m_elements) {
        r.setLeft(r.left() + xo);
        r.setRight(r.right() + xo);
    }
    invalidateBBox();
}

void Shape::translateY(double yo)
{
    for (RectF& r : m_elements) {
        r.setTop(r.top() + yo);
        r.setBottom(r.bottom() + yo);
    }
    invalidateBBox();
}

//---------------------------------------------------------
//   translated
//---------------------------------------------------------

Shape Shape::translated(const PointF& pt) const
{
    Shape s;
    for (const ShapeElement& r : m_elements) {
        s.add(r.translated(pt), r.item());
    }
    return s;
}

void Shape::invalidateBBox()
{
    m_bbox = RectF();
}

const RectF& Shape::bbox() const
{
    if (m_elements.size() == 0) {
        static const RectF _dummy;
        return _dummy;
    } else if (m_elements.size() == 1) {
        return m_elements.at(0);
    } else {
        if (m_bbox.isNull()) {
            for (const ShapeElement& e : m_elements) {
                m_bbox.unite(e);
            }
        }
        return m_bbox;
    }
}

std::optional<ShapeElement> Shape::find_if(const std::function<bool(const ShapeElement&)>& func) const
{
    auto it = std::find_if(m_elements.begin(), m_elements.end(), func);
    if (it == m_elements.end()) {
        return std::nullopt;
    }
    return std::make_optional(*it);
}

std::optional<ShapeElement> Shape::find_first(ElementType type) const
{
    for (const ShapeElement& i : m_elements) {
        if (i.item() && i.item()->type() == type) {
            return std::make_optional(i);
        }
    }

    return std::nullopt;
}

std::optional<ShapeElement> Shape::get_first() const
{
    if (m_elements.empty()) {
        return std::nullopt;
    }
    return std::make_optional(m_elements.at(0));
}

//-------------------------------------------------------------------
//   minVerticalDistance
//    a is located below this shape.
//    Calculates the minimum distance between two shapes.
//-------------------------------------------------------------------

double Shape::minVerticalDistance(const Shape& a) const
{
    if (empty() || a.empty()) {
        return 0.0;
    }

    double dist = -1000000.0; // min real
    for (const RectF& r2 : a.m_elements) {
        if (r2.height() <= 0.0) {
            continue;
        }
        double bx1 = r2.left();
        double bx2 = r2.right();
        for (const RectF& r1 : m_elements) {
            if (r1.height() <= 0.0) {
                continue;
            }
            double ax1 = r1.left();
            double ax2 = r1.right();
            if (mu::engraving::intersects(ax1, ax2, bx1, bx2, 0.0)) {
                dist = std::max(dist, r1.bottom() - r2.top());
            }
        }
    }
    return dist;
}

//-------------------------------------------------------------------
//   verticalClearance
//    a is located below this shape.
//    Claculates the amount of clearance between the two shapes.
//    If there is an overlap, returns a negative value corresponging
//    to the amount of overlap.
//-------------------------------------------------------------------

double Shape::verticalClearance(const Shape& a, double minHorizontalDistance) const
{
    if (empty() || a.empty()) {
        return 0.0;
    }

    double dist = 1000000.0; // max real
    for (const RectF& r2 : a.m_elements) {
        if (r2.height() <= 0.0) {
            continue;
        }
        double bx1 = r2.left() - minHorizontalDistance;
        double bx2 = r2.right() + minHorizontalDistance;
        for (const RectF& r1 : m_elements) {
            if (r1.height() <= 0.0) {
                continue;
            }
            double ax1 = r1.left();
            double ax2 = r1.right();
            if (mu::engraving::intersects(ax1, ax2, bx1, bx2, 0.0)) {
                dist = std::min(dist, r2.top() - r1.bottom());
            }
        }
    }
    return dist;
}

//----------------------------------------------------------------
// clearsVertically()
// a is located below this shape
// returns true if, within the horizontal width of both shapes,
// all parts of this shape are above all parts of a
//----------------------------------------------------------------
bool Shape::clearsVertically(const Shape& a) const
{
    for (const RectF& r1 : a.m_elements) {
        for (const RectF& r2 : m_elements) {
            if (mu::engraving::intersects(r1.left(), r1.right(), r2.left(), r2.right(), 0.0)) {
                if (std::min(r1.top(), r1.bottom()) <= std::max(r2.top(), r2.bottom())) {
                    return false;
                }
            }
        }
    }
    return true;
}

//---------------------------------------------------------
//   left
//    compute left border
//---------------------------------------------------------

double Shape::left() const
{
    double dist = 10000.0;
    for (const ShapeElement& r : m_elements) {
        if (r.height() != 0.0 && !(r.item() && r.item()->isTextBase()) && r.left() < dist) {
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

double Shape::right() const
{
    double dist = -10000.0;
    for (const RectF& r : m_elements) {
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

double Shape::top() const
{
    double dist = 1000000.0;
    for (const RectF& r : m_elements) {
        if (r.top() < dist) {
            dist = r.top();
        }
    }
    return dist;
}

//---------------------------------------------------------
//   bottom
//---------------------------------------------------------

double Shape::bottom() const
{
    double dist = -1000000.0;
    for (const RectF& r : m_elements) {
        if (r.bottom() > dist) {
            dist = r.bottom();
        }
    }
    return dist;
}

double Shape::rightMostEdgeAtHeight(double yAbove, double yBelow) const
{
    double edge = -100000.0;
    for (const ShapeElement& sh : m_elements) {
        if (sh.bottom() > yAbove && sh.top() < yBelow) {
            edge = std::max(edge, sh.right());
        }
    }

    return edge;
}

double Shape::leftMostEdgeAtHeight(double yAbove, double yBelow) const
{
    double edge = 100000.0;
    for (const ShapeElement& sh : m_elements) {
        if (sh.bottom() > yAbove && sh.top() < yBelow) {
            edge = std::min(edge, sh.left());
        }
    }

    return edge;
}

//---------------------------------------------------------
//   topDistance
//    p is on top of shape
//    returns negative values if there is an overlap
//---------------------------------------------------------

double Shape::topDistance(const PointF& p) const
{
    double dist = 1000000.0;
    for (const RectF& r : m_elements) {
        if (p.x() >= r.left() && p.x() < r.right()) {
            dist = std::min(dist, r.top() - p.y());
        }
    }
    return dist;
}

//---------------------------------------------------------
//   bottomDistance
//    p is below the shape
//    returns negative values if there is an overlap
//---------------------------------------------------------

double Shape::bottomDistance(const PointF& p) const
{
    double dist = 1000000.0;
    for (const RectF& r : m_elements) {
        if (p.x() >= r.left() && p.x() < r.right()) {
            dist = std::min(dist, p.y() - r.bottom());
        }
    }
    return dist;
}

void Shape::setBBox(const mu::RectF& r, const EngravingItem* p)
{
    IF_ASSERT_FAILED(type() == Type::Fixed) {
        return;
    }

    if (m_elements.empty()) {
        m_elements.push_back(ShapeElement(r, p));
    } else {
        m_elements[0] = ShapeElement(r, p);
    }
}

void Shape::addBBox(const mu::RectF& r)
{
//    IF_ASSERT_FAILED(type() == Type::Fixed) {
//        return;
//    }

    if (m_elements.empty()) {
        m_elements.push_back(mu::RectF());
    }

    m_elements[0].unite(r);
}

//---------------------------------------------------------
//   add
//---------------------------------------------------------

void Shape::add(const Shape& s)
{
    m_type = Type::Composite;
    m_elements.insert(m_elements.end(), s.m_elements.begin(), s.m_elements.end());
    invalidateBBox();
}

void Shape::add(const RectF& r, const EngravingItem* p)
{
    m_type = Type::Composite;
    m_elements.push_back(ShapeElement(r, p));
    invalidateBBox();
}

void Shape::add(const mu::RectF& r)
{
    m_type = Type::Composite;
    m_elements.push_back(ShapeElement(r));
    invalidateBBox();
}

//---------------------------------------------------------
//   remove
//---------------------------------------------------------

void Shape::remove(const RectF& r)
{
    for (auto i = m_elements.begin(); i != m_elements.end(); ++i) {
        if (*i == r) {
            m_elements.erase(i);
            return;
        }
    }

    ASSERT_X("Shape::remove: RectF not found in Shape");

    invalidateBBox();
}

void Shape::remove(const Shape& s)
{
    for (const RectF& r : s.m_elements) {
        remove(r);
    }

    invalidateBBox();
}

void Shape::removeInvisibles()
{
    mu::remove_if(m_elements, [](ShapeElement& shapeElement) {
        return !shapeElement.item() || !shapeElement.item()->visible();
    });
    invalidateBBox();
}

//---------------------------------------------------------
//   contains
//---------------------------------------------------------

bool Shape::contains(const PointF& p) const
{
    for (const RectF& r : m_elements) {
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
    for (const RectF& r : m_elements) {
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
    for (const RectF& r : other.m_elements) {
        if (intersects(r)) {
            return true;
        }
    }
    return false;
}

void Shape::paint(Painter& painter) const
{
    for (const RectF& r : m_elements) {
        painter.drawRect(r);
    }
}

void mu::engraving::dump(const ShapeElement& se, std::stringstream& ss)
{
    const mu::RectF* r = &se;
    ss << "item: " << (se.item() ? se.item()->typeName() : "no") << " rect: ";
    mu::dump(*r, ss);
}

void mu::engraving::dump(const Shape& sh, std::stringstream& ss)
{
    ss << "Shape size: " << sh.size() << "\n";
    for (const ShapeElement& r : sh.elements()) {
        ss << "  ";
        dump(r, ss);
        ss << "\n";
    }
}

std::string mu::engraving::dump(const Shape& sh)
{
    std::stringstream ss;
    dump(sh, ss);
    return ss.str();
}
