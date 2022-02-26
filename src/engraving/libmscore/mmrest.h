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

#ifndef __MMREST_H__
#define __MMREST_H__

#include "rest.h"

namespace Ms {
/// This class implements a multimeasure rest.
class MMRest final : public Rest
{
public:
    MMRest(Segment* s = 0);
    MMRest(const MMRest&, bool link = false);

    MMRest* clone() const override { return new MMRest(*this, false); }
    EngravingItem* linkedClone() override { return new MMRest(*this, true); }

    void draw(mu::draw::Painter*) const override;
    void layout() override;
    void setWidth(qreal width) override { m_width = width; }
    qreal width() const override { return m_width; }

    void write(XmlWriter&) const override;

    mu::engraving::PropertyValue propertyDefault(Pid) const override;
    bool setProperty(Pid, const mu::engraving::PropertyValue&) override;
    mu::engraving::PropertyValue getProperty(Pid) const override;

    Shape shape() const override;

private:
    Sid getPropertyStyle(Pid) const override;

    mu::PointF numberPosition(const mu::RectF& numberBbox) const;
    mu::RectF numberRect() const override;

    qreal m_width;        // width of multimeasure rest
    int m_number;         // number of measures represented
    SymIdList m_numberSym;
    qreal m_numberPos;    // vertical position of number relative to staff
    bool m_numberVisible; // show or hide number
    SymIdList m_restSyms; // stores symbols when using old-style rests
    qreal m_symsWidth;    // width of symbols with spacing when using old-style
};
}     // namespace Ms
#endif
