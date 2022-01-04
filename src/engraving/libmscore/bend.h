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

#include "infrastructure/draw/font.h"
#include "style/style.h"

#include "engravingitem.h"
#include "property.h"

namespace mu::engraving {
class Factory;
}

namespace Ms {
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
    M_PROPERTY(QString,    fontFace,  setFontFace)
    M_PROPERTY(qreal,      fontSize,  setFontSize)
    M_PROPERTY(FontStyle,  fontStyle, setFontStyle)
    M_PROPERTY(Millimetre, lineWidth, setLineWidth)

public:
    Bend* clone() const override { return new Bend(*this); }

    void layout() override;
    void draw(mu::draw::Painter*) const override;
    void write(XmlWriter&) const override;
    void read(XmlReader& e) override;
    PitchValues& points() { return m_points; }
    const PitchValues& points() const { return m_points; }
    void setPoints(const PitchValues& p) { m_points = p; }
    bool playBend() const { return m_playBend; }
    void setPlayBend(bool v) { m_playBend = v; }

    // property methods
    mu::engraving::PropertyValue getProperty(Pid propertyId) const override;
    bool setProperty(Pid propertyId, const mu::engraving::PropertyValue&) override;
    mu::engraving::PropertyValue propertyDefault(Pid) const override;

private:
    friend class mu::engraving::Factory;
    Bend(Note* parent);

    mu::draw::Font font(qreal) const;
    BendType parseBendTypeFromCurve() const;
    void updatePointsByBendType(const BendType bendType);

    bool m_playBend = true;
    PitchValues m_points;

    mu::PointF m_notePos;
    qreal m_noteWidth;
};
}     // namespace Ms
#endif
