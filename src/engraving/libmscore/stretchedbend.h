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

#include "engravingitem.h"
#include "draw/types/font.h"
#include "types.h"

namespace mu::engraving {
class Factory;

//---------------------------------------------------------
//   @@ StretchedBend
//---------------------------------------------------------

class StretchedBend final : public EngravingItem
{
    OBJECT_ALLOCATOR(engraving, StretchedBend)
    DECLARE_CLASSOF(ElementType::STRETCHED_BEND)

    M_PROPERTY(String,     fontFace,  setFontFace)
    M_PROPERTY(double,     fontSize,  setFontSize)
    M_PROPERTY(FontStyle,  fontStyle, setFontStyle)
    M_PROPERTY(Millimetre, lineWidth, setLineWidth)

public:
    StretchedBend* clone() const override { return new StretchedBend(*this); }

    mu::draw::Font font(double sp) const;

    void draw(mu::draw::Painter*) const override;

    void fillArrows(double width);
    void fillSegments();    // converting points from file to bend segments

    void fillStretchedSegments(bool untilNextSegment);
    mu::RectF calculateBoundingRect() const;

    double highestCoord() const;
    void updateHeights(double newHighestCoord);

    static void prepareBends(std::vector<StretchedBend*>& bends);

    void setPitchValues(const PitchValues& p) { m_pitchValues = p; }
    const PitchValues& pitchValues() const { return m_pitchValues; }

    void setNote(Note* note) { m_note = note; }
    Note* note() const { return m_note; }

    // property methods
    PropertyValue getProperty(Pid propertyId) const override;
    bool setProperty(Pid propertyId, const PropertyValue&) override;

private:

    friend class Factory;

    StretchedBend(Chord* parent);

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
        bool visible = true;
        bool needsHeightUpdate = false;
    };

    void fillDrawPoints(); // filling the points which specify how bend will be drawn
    void setupPainter(mu::draw::Painter* painter) const;
    double nextSegmentX() const;
    double bendHeight(int bendIdx) const;
    bool firstPointShouldBeSkipped() const;
    StretchedBend* backTiedStretchedBend() const;
    bool bendSegmentShouldBeHidden(StretchedBend* bendSegment) const;

    PitchValues m_pitchValues;
    std::vector<int> m_drawPoints;
    int m_maxDrawPointRead = 0;
    int m_maxDrawPointUpdated = 0;
    double m_highestCoord = 0;

    std::vector<BendSegment> m_bendSegments; // filled during note layout (when all coords are not known yet)
    mutable std::vector<BendSegment> m_bendSegmentsStretched; // filled during system layout (final coords used for drawing)

    struct Arrows
    {
        PolygonF up;
        PolygonF down;
        double width = 0;
    } m_arrows;

    Note* m_note = nullptr;
    Chord* m_chord = nullptr;
};
}     // namespace mu::engraving
#endif
