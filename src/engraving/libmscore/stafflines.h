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

#ifndef MU_LIBMSCORE_STAFFLINES_H
#define MU_LIBMSCORE_STAFFLINES_H

#include <vector>

#include "engravingitem.h"

namespace mu::engraving {
//-------------------------------------------------------------------
//   @@ StaffLines
///    The StaffLines class is the graphic representation of a staff,
///    it draws the horizontal staff lines.
//-------------------------------------------------------------------

class StaffLines final : public EngravingItem
{
    OBJECT_ALLOCATOR(engraving, StaffLines)

public:

    StaffLines* clone() const override { return new StaffLines(*this); }

    void draw(mu::draw::Painter*) const override;
    mu::PointF pagePos() const override;      ///< position in page coordinates
    mu::PointF canvasPos() const override;    ///< position in page coordinates

    void scanElements(void* data, void (* func)(void*, EngravingItem*), bool all=true) override;

    const std::vector<mu::LineF>& lines() { return m_lines; }
    void setLines(const std::vector<mu::LineF>& l) { m_lines = l; }

    Measure* measure() const { return (Measure*)explicitParent(); }
    double y1() const;

    double lw() const { return m_lw; }
    void setLw(double w) { m_lw = w; }

    RectF hitBBox() const override;
    Shape hitShape() const override;

private:
    friend class Factory;
    StaffLines(Measure* parent);

    double m_lw = 0.0;
    std::vector<mu::LineF> m_lines;
};
}

#endif // MU_LIBMSCORE_STAFFLINES_H
