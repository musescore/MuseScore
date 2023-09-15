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

    bool numberVisible() const { return m_numberVisible; }

    PropertyValue propertyDefault(Pid) const override;
    bool setProperty(Pid, const PropertyValue&) override;
    PropertyValue getProperty(Pid) const override;

    Shape shape() const override;

    mu::RectF numberRect() const override;

    mu::PointF numberPosition(const mu::RectF& numberBbox) const;

    struct LayoutData : public Rest::LayoutData {
        int number = 0;                     // number of measures represented
        SymIdList numberSym;
        SymIdList restSyms;                 // stores symbols when using old-style rests
        double symsWidth = 0.0;             // width of symbols with spacing when using old-style

        bool isSetRestWidth() const { return m_restWidth.has_value(); }
        void setRestWidth(double v) { m_restWidth.set_value(v); }
        double restWidth() const { return m_restWidth.value(LD_ACCESS::CHECK); }

        void setNumberSym(int n) { numberSym = timeSigSymIdsFromString(String::number(n)); }

    private:
        ld_field<double> m_restWidth = { "restWidth", 0.0 };                   // width of multimeasure rest
    };
    DECLARE_LAYOUTDATA_METHODS(MMRest);

    //! --- DEPRECATED ---
    void setWidth(double width) override
    {
        UNREACHABLE;
        mutLayoutData()->setRestWidth(width);
    }

    double width(LD_ACCESS mode = LD_ACCESS::CHECK) const override
    {
        UNUSED(mode);
        UNREACHABLE;
        return layoutData()->restWidth();
    }

    //! ------------------

private:

    Sid getPropertyStyle(Pid) const override;

    double m_numberPos = 0.0;       // vertical position of number relative to staff
    bool m_numberVisible = false;   // show or hide number
};
} // namespace mu::engraving
#endif
