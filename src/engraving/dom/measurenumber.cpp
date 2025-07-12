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
    { Sid::measureNumberMinDistance, Pid::MIN_DISTANCE },
    { Sid::measureNumberTextStyle, Pid::TEXT_STYLE }
};

//---------------------------------------------------------
//   MeasureNumber
//---------------------------------------------------------

MeasureNumber::MeasureNumber(Measure* parent, TextStyleType tid)
    : MeasureNumberBase(ElementType::MEASURE_NUMBER, parent, tid)
{
    initElementStyle(&measureNumberStyle);
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
} // namespace MS
