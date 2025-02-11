/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2021 MuseScore Limited
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

#ifndef MU_ENGRAVING_STRETCHED_BEND_H
#define MU_ENGRAVING_STRETCHED_BEND_H

#include "engravingitem.h"
#include "draw/types/font.h"
#include "types.h"

namespace mu::engraving {
class Factory;

//---------------------------------------------------------
//   @@ StretchedBend
//---------------------------------------------------------

/**********************************************************
 *    OBSOLETE CLASS
 *    Used to import GP bends before version 4.2. Now
 *    replaced by the GuitarBend class.
 *********************************************************/

class StretchedBend final : public EngravingItem
{
    OBJECT_ALLOCATOR(engraving, StretchedBend)
    DECLARE_CLASSOF(ElementType::STRETCHED_BEND)

    M_PROPERTY(String,     fontFace,  setFontFace)
    M_PROPERTY(double,     fontSize,  setFontSize)
    M_PROPERTY(FontStyle,  fontStyle, setFontStyle)
    M_PROPERTY(Spatium,    lineWidth, setLineWidth)

public:

    struct Arrows
    {
        PolygonF up;
        PolygonF down;
        double width = 0;
    };

    enum class BendSegmentType : signed char {
        NO_TYPE = -1,
        LINE_UP,
        CURVE_UP,
        CURVE_DOWN,
        LINE_STROKED
    };

    struct BendSegment {
        constexpr static int NO_TONE = -1;
        BendSegment();
        BendSegment(BendSegmentType bendType, int tone);
        void setupCoords(PointF src, PointF dest);
        PointF src;
        PointF dest;
        BendSegmentType type = BendSegmentType::NO_TYPE;
        int tone = NO_TONE;
        bool visible = true;
    };

    StretchedBend* clone() const override { return new StretchedBend(*this); }

    muse::draw::Font font(double sp) const;

    void fillArrows(double width);
    void fillSegments();    // converting points from file to bend segments

    void fillStretchedSegments(bool untilNextSegment);
    RectF calculateBoundingRect() const;

    static std::vector<Note*> notesWithStretchedBend(Chord* chord);
    static void prepareBends(std::vector<StretchedBend*>& bends);
    void adjustBendInChord();

    void setPitchValues(const PitchValues& p) { m_pitchValues = p; }
    const PitchValues& pitchValues() const { return m_pitchValues; }

    void setNote(Note* note) { m_note = note; }
    Note* note() const { return m_note; }

    static String toneToLabel(int tone);
    const Arrows& arrows() const { return m_arrows; }
    const std::vector<BendSegment>& bendSegmentsStretched() const { return m_bendSegmentsStretched; }
    static PainterPath bendCurveFromPoints(const PointF& p1, const PointF& p2);
    static int textFlags();

    // property methods
    PropertyValue getProperty(Pid propertyId) const override;
    bool setProperty(Pid propertyId, const PropertyValue&) override;

private:

    friend class Factory;

    StretchedBend(Chord* parent);

    void addSegment(std::vector<BendSegment>& bendSegments, BendSegmentType type, int tone) const;
    // creating bend segments with the information about their types
    void createBendSegments();
    double nextSegmentX() const;
    double bendHeight(int bendIdx) const;
    bool firstPointShouldBeSkipped() const;
    StretchedBend* backTiedStretchedBend() const;
    static bool equalBendTypes(const StretchedBend* bend1, const StretchedBend* bend2);

    PitchValues m_pitchValues;
    std::vector<BendSegment> m_bendSegments; // filled during note layout (when all coords are not known yet)
    std::vector<BendSegment> m_bendSegmentsStretched; // filled during system layout (final coords used for drawing)

    Arrows m_arrows;
    Note* m_note = nullptr;
    bool m_needsHeightAdjust = false;
    Note* m_noteToAdjust = nullptr;
};
}     // namespace mu::engraving
#endif
