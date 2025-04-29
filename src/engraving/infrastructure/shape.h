/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2023 MuseScore Limited
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

#include <functional>
#include <optional>

#include "draw/types/geometry.h"

#include "engraving/types/types.h"

namespace muse::draw {
class Painter;
}

namespace mu::engraving {
class EngravingItem;

//---------------------------------------------------------
//   ShapeElement
//---------------------------------------------------------

struct ShapeElement : public RectF {
public:

    ShapeElement(const RectF& f, const EngravingItem* p, bool ignoreForLayout)
        : RectF(f), m_item(p), m_ignoreForLayout(ignoreForLayout) {}
    ShapeElement(const RectF& f, const EngravingItem* p)
        : RectF(f), m_item(p) {}
    ShapeElement(const RectF& f)
        : RectF(f) {}

    const EngravingItem* item() const { return m_item; }
    void setItem(const EngravingItem* item) { m_item = item; }
    bool ignoreForLayout() const { return m_ignoreForLayout; }

private:
    const EngravingItem* m_item = nullptr;
    bool m_ignoreForLayout = false;
};

//---------------------------------------------------------
//   Shape
//---------------------------------------------------------

class Shape
{
public:

    enum class Type : unsigned char {
        Fixed,      // fixed size, like just bbox
//        FixedX,     // not implemented (reserved)
//        FixedY,     // not implemented (reserved)
        Composite   // composed of other shapes
    };

    Shape(Type t = Type::Fixed)
        : m_type(t) {}
    Shape(const RectF& r, const EngravingItem* p = nullptr, Type t = Type::Fixed)
        : m_type(t) { setBBox(r, p); }
    Shape(const std::vector<RectF>& rects, const EngravingItem* p = nullptr);

    Type type() const { return m_type; }
    bool isComposite() const { return m_type == Type::Composite; }

    size_t size() const { return m_elements.size(); }
    bool empty() const { return m_elements.empty(); }
    void clear() { m_elements.clear(); }

    bool equal(const Shape& sh) const
    {
        if (m_type != sh.m_type) {
            return false;
        }

        switch (m_type) {
        case Type::Fixed: return this->bbox() == sh.bbox();
        case Type::Composite: return this->m_elements == sh.m_elements;
        }

        return true;
    }

    // Fixed
    void setBBox(const RectF& r, const EngravingItem* p = nullptr);
    void addBBox(const RectF& r);

    // Composite
    void add(const Shape& s);
    void add(const ShapeElement& shapeEl);
    void add(const RectF& r, const EngravingItem* p, bool ignoreForLayout) { add(ShapeElement(r, p, ignoreForLayout)); }
    void add(const RectF& r, const EngravingItem* p) { add(ShapeElement(r, p)); }
    void add(const RectF& r) { add(ShapeElement(r)); }

    void remove(const RectF&);
    void remove(const Shape&);
    template<typename Predicate>
    inline bool remove_if(Predicate p)
    {
        size_t origSize = m_elements.size();
        m_elements.erase(std::remove_if(m_elements.begin(), m_elements.end(), p), m_elements.end());
        invalidateBBox();
        return origSize != m_elements.size();
    }

    // ---

    const std::vector<ShapeElement>& elements() const { return m_elements; }
    std::vector<ShapeElement>& elements() { return m_elements; }
    std::vector<RectF> toRects() const;

    std::optional<ShapeElement> find_if(const std::function<bool(const ShapeElement&)>& func) const;
    std::optional<ShapeElement> find_first(ElementType type) const;
    std::optional<ShapeElement> get_first() const;

    void removeInvisibles();
    void removeTypes(const std::set<ElementType>& types);

    void addHorizontalSpacing(EngravingItem* item, double left, double right);

    Shape& translate(const PointF&);
    void translateX(double);
    void translateY(double);
    Shape translated(const PointF&) const;
    Shape& scale(const SizeF&);
    Shape scaled(const SizeF&) const;
    Shape& adjust(double xp1, double yp1, double xp2, double yp2);
    Shape adjusted(double xp1, double yp1, double xp2, double yp2) const;
    Shape& pad(double p);
    Shape padded(double p) const;

    const RectF& bbox() const;
    double minVerticalDistance(const Shape&, double minHorizontalClearance = 0.0) const;
    double verticalClearance(const Shape&, double minHorizontalDistance = 0.0) const;
    double topDistance(const PointF&) const;
    double bottomDistance(const PointF&) const;
    double left() const;
    double right() const;
    double top() const;
    double bottom() const;
    double rightMostEdgeAtHeight(double yAbove, double yBelow) const;
    double leftMostEdgeAtHeight(double yAbove, double yBelow) const;
    double leftMostEdgeAtTop() const;

    bool contains(const PointF&) const;
    bool intersects(const RectF& rr) const;
    bool intersects(const Shape& other) const;
    bool clearsVertically(const Shape& a) const;

    void paint(muse::draw::Painter& painter) const;

private:

    void invalidateBBox();

    Type m_type = Type::Fixed;
    std::vector<ShapeElement> m_elements;
    mutable RectF m_bbox;   // cache
};

void dump(const ShapeElement& sh, std::stringstream& ss);
void dump(const Shape& sh, std::stringstream& ss);
std::string dump(const Shape& sh);

//---------------------------------------------------------
//   intersects
//---------------------------------------------------------

inline static bool intersects(double a, double b, double c, double d, double minClearance = 0.0)
{
    if (a == b || c == d) {
        return false;
    }
    return (b + minClearance > c) && (a < d + minClearance);
}
} // namespace mu::engraving

#endif
