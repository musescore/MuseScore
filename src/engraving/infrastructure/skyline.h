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

#ifndef MU_ENGRAVING_SKYLINE_H
#define MU_ENGRAVING_SKYLINE_H

#include <vector>

#include "draw/types/geometry.h"
#include "shape.h"

namespace mu::draw {
class Painter;
}

namespace mu::engraving {
class Segment;

//---------------------------------------------------------
//   SkylineSegment
//---------------------------------------------------------

struct SkylineSegment {
    double x;
    double y;
    double w;

    SkylineSegment(double _x, double _y, double _w)
        : x(_x), y(_y), w(_w) {}
};

//---------------------------------------------------------
//   SkylineLine
//---------------------------------------------------------

class SkylineLine
{
    const bool north;
    std::vector<SkylineSegment> seg;
    typedef std::vector<SkylineSegment>::iterator SegIter;
    typedef std::vector<SkylineSegment>::const_iterator SegConstIter;

    SegIter insert(SegIter i, double x, double y, double w);
    void append(double x, double y, double w);
    SegIter find(double x);
    SegConstIter find(double x) const;

public:
    SkylineLine(bool n)
        : north(n) {}
    void add(const Shape& s);
    void add(const ShapeElement& r);
    void add(double x, double y, double w);
    void add(const RectF& r) { add(ShapeElement(r)); }

    void clear() { seg.clear(); }
    void paint(mu::draw::Painter& painter) const;
    void dump() const;
    double minDistance(const SkylineLine&) const;
    double max() const;
    bool valid() const;
    bool valid(const SkylineSegment& s) const;
    bool isNorth() const { return north; }

    SegIter begin() { return seg.begin(); }
    SegConstIter begin() const { return seg.begin(); }
    SegIter end() { return seg.end(); }
    SegConstIter end() const { return seg.end(); }
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
    void add(const RectF& r) { add(ShapeElement(r)); }

    double minDistance(const Skyline&) const;

    SkylineLine& north() { return _north; }
    SkylineLine& south() { return _south; }
    const SkylineLine& north() const { return _north; }
    const SkylineLine& south() const { return _south; }

    void paint(mu::draw::Painter& painter, double lineWidth) const;
    void dump(const char*, bool north = false) const;
};
} // namespace mu::engraving

#endif
