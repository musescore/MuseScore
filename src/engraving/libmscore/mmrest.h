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

#include "utils.h"

namespace mu::engraving {
/// This class implements a multimeasure rest.
class MMRest final : public Rest
{
    OBJECT_ALLOCATOR(engraving, MMRest)
    DECLARE_CLASSOF(ElementType::MMREST)

public:
    MMRest(Segment* s = 0);
    MMRest(const MMRest&, bool link = false);

    MMRest* clone() const override { return new MMRest(*this, false); }
    EngravingItem* linkedClone() override { return new MMRest(*this, true); }

    void draw(mu::draw::Painter*) const override;

    void setWidth(double width) override { m_width = width; }
    double width() const override { return m_width; }

    int number() const { return m_number; }
    void setNumber(int n) { m_number = n; }

    void setNumberSym(int n) { m_numberSym = timeSigSymIdsFromString(String::number(n)); }

    const SymIdList& restSyms() const { return m_restSyms; }
    void setRestSyms(const SymIdList& syms) { m_restSyms = syms; }

    double symsWidth() const { return m_symsWidth; }
    void setSymsWidth(double w) { m_symsWidth = w; }

    bool numberVisible() const { return m_numberVisible; }

    PropertyValue propertyDefault(Pid) const override;
    bool setProperty(Pid, const PropertyValue&) override;
    PropertyValue getProperty(Pid) const override;

    Shape shape() const override;

    mu::RectF numberRect() const override;

private:

    Sid getPropertyStyle(Pid) const override;

    mu::PointF numberPosition(const mu::RectF& numberBbox) const;

    double m_width = 0.0;           // width of multimeasure rest
    int m_number = 0;               // number of measures represented
    SymIdList m_numberSym;
    double m_numberPos = 0.0;       // vertical position of number relative to staff
    bool m_numberVisible = false;   // show or hide number
    SymIdList m_restSyms;           // stores symbols when using old-style rests
    double m_symsWidth = 0.0;       // width of symbols with spacing when using old-style
};
} // namespace mu::engraving
#endif
