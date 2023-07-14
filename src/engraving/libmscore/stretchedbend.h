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
    OBJECT_ALLOCATOR(engraving, StretchedBend)
    DECLARE_CLASSOF(ElementType::STRETCHED_BEND)

public:
    StretchedBend* clone() const override { return new StretchedBend(*this); }

    void draw(mu::draw::Painter*) const override;

    bool stretchedMode() const { return m_stretchedMode; }
    void setStretchedMode(bool val) { m_stretchedMode = val; }

    void fillArrows(double width);
    void fillSegments();    // converting points from file to bend segments
    void stretchSegments(); // stretching until end of chord duration

    mu::RectF calculateBoundingRect() const;

    static void prepareBends(std::vector<StretchedBend*>& bends);

private:

    friend class Factory;

    StretchedBend(Note* parent);

    void fillDrawPoints(); // filling the points which specify how bend will be drawn

    void setupPainter(mu::draw::Painter* painter) const;

    double nextSegmentX() const;
    double bendHeight(int bendIdx) const;
    bool firstPointShouldBeSkipped() const;

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

    bool m_stretchedMode = false; // layout with fixed size or stretched to next segment

    std::vector<int> m_drawPoints;
    std::vector<BendSegment> m_bendSegments;

    struct Arrows
    {
        PolygonF up;
        PolygonF down;
        double width = 0;
    } m_arrows;
};
}     // namespace mu::engraving
#endif
