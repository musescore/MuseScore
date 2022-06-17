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

#ifndef __MMRESTRANGE_H__
#define __MMRESTRANGE_H__

#include "measurenumberbase.h"
#include "property.h"

namespace mu::engraving {
//---------------------------------------------------------
//   MMRestRange
//---------------------------------------------------------

class MMRestRange : public MeasureNumberBase
{
    /// Bracketing: [18-24], (18-24) or 18-24
    M_PROPERTY(MMRestRangeBracketType, bracketType, setBracketType)

public:
    MMRestRange(Measure* parent = nullptr);
    MMRestRange(const MMRestRange& other);

    MMRestRange* clone() const override { return new MMRestRange(*this); }

    PropertyValue getProperty(Pid id) const override;
    bool setProperty(Pid id, const PropertyValue& val) override;
    PropertyValue propertyDefault(Pid id) const override;

    bool readProperties(XmlReader&) override;

    void setXmlText(const String&) override;
};
} // namespace mu::engraving

#endif
