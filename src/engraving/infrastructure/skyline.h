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

#ifndef MU_ENGRAVING_SKYLINE_H
#define MU_ENGRAVING_SKYLINE_H

#include <cfloat>
#include <vector>
#include <map>

#include "draw/types/geometry.h"
#include "shape.h"

namespace muse::draw {
class Painter;
}

namespace mu::engraving {
class Segment;

//---------------------------------------------------------
//   SkylineLine
//---------------------------------------------------------

class SkylineLine
{
public:
    SkylineLine(bool n)
        : m_isNorth(n) {}

    void add(const ShapeElement& r);
    void add(const RectF& r, EngravingItem* item) { add(ShapeElement(r, item)); }
    void add(const Shape& s);

    template<typename Predicate>
    inline bool remove_if(Predicate p) { return m_shape.remove_if(p); }
    SkylineLine getFilteredCopy(std::function<bool(const ShapeElement&)> filterOut) const;

    void clear();
    // TODO: avoid passing down minHorizontalClearance (in future it must be done
    // item-by-item similar to what we do for horizontal spacing)
    double minDistance(const SkylineLine&, double minHorizontalClearance = 0.0) const;
    double minDistanceToShapeAbove(const Shape&, double minHorizontalClearance = 0.0) const;
    double minDistanceToShapeBelow(const Shape&, double minHorizontalClearance = 0.0) const;
    double verticalClearanceAbove(const Shape& shapeAbove) const;
    double verticalClaranceBelow(const Shape& shapeBelow) const;
    double max() const;
    bool valid() const;

    SkylineLine& translateY(double y);

    double top(double startX = -DBL_MAX, double endX = DBL_MAX);
    double bottom(double startX = -DBL_MAX, double endX = DBL_MAX);

    bool isNorth() const { return m_isNorth; }

    const std::vector<ShapeElement>& elements() const { return m_shape.elements(); }
    std::vector<ShapeElement>& elements() { return m_shape.elements(); }

private:
    double staffLinesTopAtX(double x) const;
    double staffLinesBottomAtX(double x) const;

private:
    const bool m_isNorth;
    Shape m_shape;

    struct StaffLineEdge {
        double top = 0.0;
        double bottom = 0.0;
        StaffLineEdge(double t, double b)
            : top(t), bottom(b) {}
    };

    std::map<double, StaffLineEdge> m_staffLineEdges;
    bool hasValidStaffLineEdges() const { return !m_staffLineEdges.empty(); }
};

//---------------------------------------------------------
//   Skyline
//---------------------------------------------------------

class Skyline
{
    SkylineLine _north;
    SkylineLine _south;

public:
    Skyline()
        : _north(true), _south(false) {}

    void clear();
    void add(const Shape& s);
    void add(const ShapeElement& r);
    void add(const RectF& r, EngravingItem* item) { add(ShapeElement(r, item)); }

    // TODO: avoid passing down minHorizontalClearance (in future it must be done
    // item-by-item similar to what we do for horizontal spacing)
    double minDistance(const Skyline&, double minHorizontalClearance = 0.0) const;

    SkylineLine& north() { return _north; }
    SkylineLine& south() { return _south; }
    const SkylineLine& north() const { return _north; }
    const SkylineLine& south() const { return _south; }

    void paint(muse::draw::Painter& painter, double lineWidth) const; // DEBUG only
};

void dump(const SkylineLine& slylineline, std::stringstream& ss);
std::string dump(const Skyline& skyline);
} // namespace mu::engraving

#endif
