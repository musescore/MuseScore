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

#include "measurenumber.h"

#include "measure.h"
#include "score.h"

using namespace mu;

namespace mu::engraving {
//---------------------------------------------------------
//   measureNumberStyle
//---------------------------------------------------------

static const ElementStyle measureNumberStyle {
    { Sid::measureNumberVPlacement, Pid::PLACEMENT },
    { Sid::measureNumberHPlacement, Pid::HPLACEMENT },
    { Sid::measureNumberMinDistance, Pid::MIN_DISTANCE }
};

//---------------------------------------------------------
//   MeasureNumber
//---------------------------------------------------------

MeasureNumber::MeasureNumber(Measure* parent, TextStyleType tid)
    : MeasureNumberBase(ElementType::MEASURE_NUMBER, parent, tid)
{
    initElementStyle(&measureNumberStyle);

    setHPlacement(style().styleV(Sid::measureNumberHPlacement).value<PlacementH>());
}

//---------------------------------------------------------
//   MeasureNumber
//     Copy constructor
//---------------------------------------------------------

MeasureNumber::MeasureNumber(const MeasureNumber& other)
    : MeasureNumberBase(other)
{
    initElementStyle(&measureNumberStyle);
}

//---------------------------------------------------------
//   propertyDefault
//---------------------------------------------------------

engraving::PropertyValue MeasureNumber::propertyDefault(Pid id) const
{
    switch (id) {
    case Pid::TEXT_STYLE:
        return TextStyleType::MEASURE_NUMBER;
    case Pid::PLACEMENT:
        return style().styleV(Sid::measureNumberVPlacement);
    case Pid::HPLACEMENT:
        return style().styleV(Sid::measureNumberHPlacement);
    default:
        return MeasureNumberBase::propertyDefault(id);
    }
}
} // namespace MS
