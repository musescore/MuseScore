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

#ifndef __STRETCHED_BEND_H__
#define __STRETCHED_BEND_H__

#include "bend.h"

namespace mu::engraving {
class Factory;

//---------------------------------------------------------
//   @@ StretchedBend
//---------------------------------------------------------

class StretchedBend final : public Bend
{
public:
    StretchedBend* clone() const override { return new StretchedBend(*this); }

    void layout() override;
    void draw(mu::draw::Painter*) const override;

    static void prepareBends(std::vector<StretchedBend*>& bends);

private:
    friend class mu::engraving::Factory;

    StretchedBend(Note* parent);

    void fillSegments(); // converting points from file to bend segments
    void stretchSegments(); // stretching until end of chord duration
    void glueNeighbor(); // fixing the double appearance of some bends

    void layoutDraw(const bool layoutMode, mu::draw::Painter* painter = nullptr) const; /// loop for both layout and draw logic
    void preLayout();
    void postLayout();

    void setupPainter(mu::draw::Painter* painter) const;
    void fillArrows();
    double nextSegmentX() const;
    double bendHeight(int bendIdx) const;

    bool m_reduntant = false; // marks that the bend was 'glued' to neighbour and is now unnecessary

    enum class BendSegmentType {
        NO_TYPE = -1,
        LINE_UP,
        CURVE_UP,
        CURVE_DOWN,
        LINE_STROKED
    };

    struct BendSegment {
        PointF src;
        PointF dest;
        BendSegmentType type = BendSegmentType::NO_TYPE;
        int tone = -1;
    };

    std::vector<BendSegment> m_bendSegments;

    PolygonF m_arrowUp;
    PolygonF m_arrowDown;
    double m_spatium = 0;
    double m_bendArrowWidth = 0;
    mutable RectF m_boundingRect;
    bool m_releasedToInitial = false;
};
}     // namespace mu::engraving
#endif
