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

#ifndef __TREMOLOBAR_H__
#define __TREMOLOBAR_H__

#include "element.h"
#include "pitchvalue.h"

namespace Ms {
//---------------------------------------------------------
//   @@ TremoloBar
//
//   @P userMag    qreal
//   @P lineWidth  qreal
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

class TremoloBar final : public Element
{
public:
    TremoloBar(Score* s);

    TremoloBar* clone() const override { return new TremoloBar(*this); }
    ElementType type() const override { return ElementType::TREMOLOBAR; }

    void layout() override;
    void draw(mu::draw::Painter*) const override;

    void write(XmlWriter&) const override;
    void read(XmlReader& e) override;

    QList<PitchValue>& points() { return m_points; }
    const QList<PitchValue>& points() const { return m_points; }
    void setPoints(const QList<PitchValue>& p) { m_points = p; }

    QVariant getProperty(Pid propertyId) const override;
    bool setProperty(Pid propertyId, const QVariant&) override;
    QVariant propertyDefault(Pid) const override;

    qreal userMag() const { return m_userMag; }
    void setUserMag(qreal m) { m_userMag = m; }

    void setLineWidth(Spatium v) { m_lw = v; }
    Spatium lineWidth() const { return m_lw; }

    bool play() const { return m_play; }
    void setPlay(bool val) { m_play = val; }

private:
    TremoloBarType parseTremoloBarTypeFromCurve(const QList<PitchValue>& curve) const;
    void updatePointsByTremoloBarType(const TremoloBarType type);

    Spatium m_lw;
    qreal m_userMag = 1.0;           // allowed 0.1 - 10.0
    bool m_play = true;

    QList<PitchValue> m_points;

    mu::PolygonF m_polygon;                    // computed by layout
};
}     // namespace Ms
#endif
