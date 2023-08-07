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

#ifndef __BEND_H__
#define __BEND_H__

#include "draw/types/font.h"

#include "engravingitem.h"
#include "types.h"

namespace mu::engraving {
class Factory;

//---------------------------------------------------------
//   @@ Bend
//---------------------------------------------------------

enum class BendType {
    BEND = 0,
    BEND_RELEASE,
    BEND_RELEASE_BEND,
    PREBEND,
    PREBEND_RELEASE,
    CUSTOM
};

class Bend final : public EngravingItem
{
    OBJECT_ALLOCATOR(engraving, Bend)
    DECLARE_CLASSOF(ElementType::BEND)

public:
    Bend* clone() const override { return new Bend(*this); }

    static const char* label[13];

    PitchValues& points() { return m_points; }
    const PitchValues& points() const { return m_points; }
    void addPoint(const PitchValue& pv) { m_points.push_back(pv); }
    void setPoints(const PitchValues& p) { m_points = p; }
    bool playBend() const { return m_playBend; }
    void setPlayBend(bool v) { m_playBend = v; }

    String fontFace() const { return m_fontFace; }
    void setFontFace(String f) { m_fontFace = f; }
    double fontSize() const { return m_fontSize; }
    void setFontSize(double s) { m_fontSize = s; }
    FontStyle fontStyle() const { return m_fontStyle; }
    void setFontStyle(FontStyle s) { m_fontStyle = s; }
    Millimetre lineWidth() const { return m_lineWidth; }
    void setLineWidth(Millimetre w) { m_lineWidth = w; }

    mu::draw::Font font(double) const;

    // property methods
    PropertyValue getProperty(Pid propertyId) const override;
    bool setProperty(Pid propertyId, const PropertyValue&) override;
    PropertyValue propertyDefault(Pid) const override;

    struct LayoutData {
        // cache
        PointF notePos;
        double noteWidth = 0.0;
        double noteHeight = 0.0;

        // out
        PointF pos;
        RectF bbox;
    };

    const LayoutData& layoutData() const { return m_layoutData; }
    void setLayoutData(const LayoutData& data);

    //! --- Old Interface ---
    void setNoteWidth(double v) { m_layoutData.noteWidth = v; }
    double noteWidth() const { return m_layoutData.noteWidth; }
    void setNoteHeight(double v) { m_layoutData.noteHeight = v; }
    double noteHeight() const { return m_layoutData.noteHeight; }
    void setNotePos(const PointF& v) { m_layoutData.notePos = v; }
    const PointF& notePos() const { return m_layoutData.notePos; }
    //! ---------------------

private:
    friend class Factory;

    Bend(Note* parent);

    BendType parseBendTypeFromCurve() const;
    void updatePointsByBendType(const BendType bendType);

    bool m_playBend = true;
    PitchValues m_points;

    String m_fontFace;
    double m_fontSize = 0.0;
    FontStyle m_fontStyle = FontStyle::Undefined;
    Millimetre m_lineWidth;

    LayoutData m_layoutData;
};
} // namespace mu::engraving
#endif
