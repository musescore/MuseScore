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

#ifndef MU_ENGRAVING_TREMOLOBAR_H
#define MU_ENGRAVING_TREMOLOBAR_H

#include "engravingitem.h"

namespace mu::engraving {
//---------------------------------------------------------
//   @@ TremoloBar
//
//   @P userMag    double
//   @P lineWidth  double
//   @P play       bool         play tremolo bar
//---------------------------------------------------------

enum class TremoloBarType {
    DIP = 0,
    DIVE,
    RELEASE_UP,
    INVERTED_DIP,
    RETURN,
    RELEASE_DOWN,
    CUSTOM
};

class TremoloBar final : public EngravingItem
{
    OBJECT_ALLOCATOR(engraving, TremoloBar)
    DECLARE_CLASSOF(ElementType::TREMOLOBAR)

public:

    TremoloBar* clone() const override { return new TremoloBar(*this); }

    PitchValues& points() { return m_points; }
    const PitchValues& points() const { return m_points; }
    void setPoints(const PitchValues& p) { m_points = p; }

    PropertyValue getProperty(Pid propertyId) const override;
    bool setProperty(Pid propertyId, const PropertyValue&) override;
    PropertyValue propertyDefault(Pid) const override;

    double userMag() const { return m_userMag; }
    void setUserMag(double m) { m_userMag = m; }

    void setLineWidth(Spatium v) { m_lw = v; }
    Spatium lineWidth() const { return m_lw; }

    bool play() const { return m_play; }
    void setPlay(bool val) { m_play = val; }

    struct LayoutData : public EngravingItem::LayoutData {
        PolygonF polygon;
    };
    DECLARE_LAYOUTDATA_METHODS(TremoloBar)

private:

    friend class Factory;
    TremoloBar(EngravingItem* parent);

    TremoloBarType parseTremoloBarTypeFromCurve(const PitchValues& curve) const;
    void updatePointsByTremoloBarType(const TremoloBarType type);

    Spatium m_lw;
    double m_userMag = 1.0;           // allowed 0.1 - 10.0
    bool m_play = true;

    PitchValues m_points;
};
} // namespace mu::engraving
#endif
