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

#include <functional>
#include <optional>
#include <memory>

#include "draw/types/geometry.h"

#include "engraving/types/types.h"

namespace mu::draw {
class Painter;
}

namespace mu::engraving {
class EngravingItem;

//---------------------------------------------------------
//   ShapeElement
//---------------------------------------------------------

struct ShapeElement : public mu::RectF {
public:

    ShapeElement(const mu::RectF& f, const EngravingItem* p, bool ignoreForLayout)
        : mu::RectF(f), m_item(p), m_ignoreForLayout(ignoreForLayout) {}
    ShapeElement(const mu::RectF& f, const EngravingItem* p)
        : mu::RectF(f), m_item(p) {}
    ShapeElement(const mu::RectF& f)
        : mu::RectF(f) {}

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

    enum class Type {
        Fixed,      // fixed size, like just bbox
//        FixedX,     // not implemented (reserved)
//        FixedY,     // not implemented (reserved)
        Composite   // composed of other shapes
    };

    Shape(Type t = Type::Fixed);
    Shape(const mu::RectF& r, const EngravingItem* p = nullptr, Type t = Type::Fixed);
    Shape(const std::vector<mu::RectF>& rects, const EngravingItem* p = nullptr);

    Shape(const Shape& sh) {
        s_stat.copies++;
        m_data = sh.m_data;
    }

    Shape& operator=(const Shape& sh) {
        s_stat.copies++;
        m_data = sh.m_data;
        return *this;
    }

    Type type() const { return m_data->type; }
    bool isComposite() const { return m_data->type == Type::Composite; }

    size_t size() const { return m_data->elements.size(); }
    bool empty() const { return m_data->elements.empty(); }
    void clear();

    bool equal(const Shape& sh) const
    {
        if (m_data->type != sh.m_data->type) {
            return false;
        }

        switch (m_data->type) {
        case Type::Fixed: return this->bbox() == sh.bbox();
        case Type::Composite: return this->m_data->elements == sh.m_data->elements;
        }

        return true;
    }

    // Fixed
    void setBBox(const mu::RectF& r, const EngravingItem* p = nullptr);
    void addBBox(const mu::RectF& r);

    // Composite
    void add(const Shape& s);
    void add(const ShapeElement& shapeEl);
    void add(const mu::RectF& r, const EngravingItem* p, bool ignoreForLayout) { add(ShapeElement(r, p, ignoreForLayout)); }
    void add(const mu::RectF& r, const EngravingItem* p) { add(ShapeElement(r, p)); }
    void add(const mu::RectF& r) { add(ShapeElement(r)); }

    void remove(const mu::RectF&);
    void remove(const Shape&);
    template<typename Predicate>
    inline bool remove_if(Predicate p)
    {
        detach();
        size_t origSize = m_data->elements.size();
        m_data->elements.erase(std::remove_if(m_data->elements.begin(), m_data->elements.end(), p), m_data->elements.end());
        invalidateBBox();
        return origSize != m_data->elements.size();
    }

    // ---

    const std::vector<ShapeElement>& elements() const { return m_data->elements; }
    std::vector<ShapeElement>& elements()
    {
        detach();
        return m_data->elements;
    }

    std::optional<ShapeElement> find_if(const std::function<bool(const ShapeElement&)>& func) const;
    std::optional<ShapeElement> find_first(ElementType type) const;
    std::optional<ShapeElement> get_first() const;

    void removeInvisibles();
    void removeTypes(const std::set<ElementType>& types);

    void addHorizontalSpacing(EngravingItem* item, double left, double right);

    Shape& translate(const mu::PointF&);
    void translateX(double);
    void translateY(double);
    Shape translated(const mu::PointF&) const;
    Shape& scale(const mu::SizeF&);
    Shape scaled(const mu::SizeF&) const;

    const mu::RectF& bbox() const;
    double minVerticalDistance(const Shape&) const;
    double verticalClearance(const Shape&, double minHorizontalDistance = 0) const;
    double topDistance(const mu::PointF&) const;
    double bottomDistance(const mu::PointF&) const;
    double left() const;
    double right() const;
    double top() const;
    double bottom() const;
    double rightMostEdgeAtHeight(double yAbove, double yBelow) const;
    double leftMostEdgeAtHeight(double yAbove, double yBelow) const;

    bool contains(const mu::PointF&) const;
    bool intersects(const mu::RectF& rr) const;
    bool intersects(const Shape&) const;
    bool clearsVertically(const Shape& a) const;

    void paint(mu::draw::Painter& painter) const;

    struct Stat {

        struct Counter {
            int count = 0;
        };

        std::map<int, Counter> stat;
        int copies = 0;

        void clear() {
            copies = 0;
            stat.clear();
        }

        void add(int use_count) {
            stat[use_count].count++;
        }

        void print();
    };

    static Stat s_stat;

private:

    void invalidateBBox();

    void detach();

    struct Data {
        Type type = Type::Fixed;
        std::vector<ShapeElement> elements;
        mutable RectF bbox;   // cache
    };

    std::shared_ptr<Data> m_data;
};

void dump(const ShapeElement& sh, std::stringstream& ss);
void dump(const Shape& sh, std::stringstream& ss);
std::string dump(const Shape& sh);

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
