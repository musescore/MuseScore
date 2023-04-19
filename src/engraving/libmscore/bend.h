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

class Bend : public EngravingItem // TODO: bring back "final" keyword
{
    OBJECT_ALLOCATOR(engraving, Bend)
    DECLARE_CLASSOF(ElementType::BEND)

    M_PROPERTY(String,     fontFace,  setFontFace)
    M_PROPERTY(double,      fontSize,  setFontSize)
    M_PROPERTY(FontStyle,  fontStyle, setFontStyle)
    M_PROPERTY(Millimetre, lineWidth, setLineWidth)

public:
    Bend* clone() const override { return new Bend(*this); }

    void layout() override;
    void draw(mu::draw::Painter*) const override;
    void write(XmlWriter&) const override;
    PitchValues& points() { return m_points; }
    const PitchValues& points() const { return m_points; }
    void addPoint(const PitchValue& pv) { m_points.push_back(pv); }
    void setPoints(const PitchValues& p) { m_points = p; }
    bool playBend() const { return m_playBend; }
    void setPlayBend(bool v) { m_playBend = v; }

    // property methods
    PropertyValue getProperty(Pid propertyId) const override;
    bool setProperty(Pid propertyId, const PropertyValue&) override;
    PropertyValue propertyDefault(Pid) const override;

protected: /// TODO: bring back "private" keyword after removing StretchedBend class
    friend class Factory;
    Bend(Note* parent, ElementType type = ElementType::BEND);

    mu::draw::Font font(double) const;
    BendType parseBendTypeFromCurve() const;
    void updatePointsByBendType(const BendType bendType);

    bool m_playBend = true;
    PitchValues m_points;

    mu::PointF m_notePos;
    double m_noteWidth = 0;
    double m_noteHeight = 0;
};
} // namespace mu::engraving
#endif
