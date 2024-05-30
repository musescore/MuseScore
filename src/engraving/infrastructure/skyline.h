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

#include <vector>

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

    void clear();
    double minDistance(const SkylineLine&) const;
    double max() const;
    bool valid() const;

    bool isNorth() const { return m_isNorth; }

    const std::vector<ShapeElement>& elements() const { return m_shape.elements(); }
    std::vector<ShapeElement>& elements() { return m_shape.elements(); }

private:
    const bool m_isNorth;
    Shape m_shape;

    double m_staffLinesTop = 0.0;
    double m_staffLinesBottom = 0.0;
    bool hasValidStaffLineEdges() const { return !(muse::RealIsNull(m_staffLinesBottom) && muse::RealIsNull(m_staffLinesTop)); }
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

    double minDistance(const Skyline&) const;

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
