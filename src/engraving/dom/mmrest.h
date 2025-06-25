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

#ifndef MU_ENGRAVING_MMREST_H
#define MU_ENGRAVING_MMREST_H

#include "rest.h"

#include "utils.h"

namespace mu::engraving {

enum class MMRestRangeBracketType : unsigned char {
    ///.\{
    BRACKETS, PARENTHESES, NONE
    ///\}
};

class MMRest final : public Rest
{
    OBJECT_ALLOCATOR(engraving, MMRest)
    DECLARE_CLASSOF(ElementType::MMREST)

public:
    MMRest(Segment* s = 0);
    MMRest(const MMRest&, bool link = false);

    MMRest* clone() const override { return new MMRest(*this, false); }
    EngravingItem* linkedClone() override { return new MMRest(*this, true); }

    bool shouldShowNumber() const;

    PropertyValue propertyDefault(Pid) const override;
    bool setProperty(Pid, const PropertyValue&) override;
    PropertyValue getProperty(Pid) const override;

    RectF numberRect() const override;
    PointF numberPos() const;

    void setNumberOffset(double y) { m_numberOffset = y; }
    double numberOffset() const { return m_numberOffset; }

    double yNumberPos() const;

    bool isOldStyle() const;

    struct LayoutData : public Rest::LayoutData {
        int number = 0;                     // number of measures represented
        double yNumberPos = 0.0;
        SymIdList numberSym;
        SymIdList restSyms;                 // stores symbols when using old-style rests
        double symsWidth = 0.0;             // width of symbols with spacing when using old-style

        ld_field<double> restWidth = { "[MMRest] restWidth", 0.0 }; // width of multimeasure rest

        void setNumberSym(int n) { numberSym = timeSigSymIdsFromString(String::number(n)); }
    };
    DECLARE_LAYOUTDATA_METHODS(MMRest)

private:

    double m_numberOffset = 0.0;   // vertical position of number relative to staff
    bool m_numberVisible = false;   // show or hide number
};
} // namespace mu::engraving
#endif
