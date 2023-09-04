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

#ifndef __MEASURENUMBER_H__
#define __MEASURENUMBER_H__

#include "measurenumberbase.h"

namespace mu::engraving {
//---------------------------------------------------------
//   MeasureNumber
//---------------------------------------------------------

class MeasureNumber : public MeasureNumberBase
{
    OBJECT_ALLOCATOR(engraving, MeasureNumber)
    DECLARE_CLASSOF(ElementType::MEASURE_NUMBER)

public:
    MeasureNumber(Measure* parent = nullptr, TextStyleType tid = TextStyleType::MEASURE_NUMBER);
    MeasureNumber(const MeasureNumber& other);

    virtual MeasureNumber* clone() const override { return new MeasureNumber(*this); }

    PropertyValue propertyDefault(Pid id) const override;
};
} // namespace mu::engraving

#endif
