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

#include "mmrestrange.h"

#include "measure.h"
#include "score.h"

#include "log.h"

using namespace mu;
using namespace mu::engraving;

namespace mu::engraving {
//---------------------------------------------------------
//   mmRestRangeStyle
//---------------------------------------------------------

static const ElementStyle mmRestRangeStyle {
    { Sid::mmRestRangeBracketType, Pid::MMREST_RANGE_BRACKET_TYPE },
    { Sid::mmRestRangeVPlacement,  Pid::PLACEMENT },
    { Sid::mmRestRangeHPlacement,  Pid::HPLACEMENT }
};

MMRestRange::MMRestRange(Measure* parent)
    : MeasureNumberBase(ElementType::MMREST_RANGE, parent, TextStyleType::MMREST_RANGE)
{
    initElementStyle(&mmRestRangeStyle);
}

//---------------------------------------------------------
//   MMRestRange
///   Copy constructor
//---------------------------------------------------------

MMRestRange::MMRestRange(const MMRestRange& other)
    : MeasureNumberBase(other)
{
    initElementStyle(&mmRestRangeStyle);
}

PropertyValue MMRestRange::getProperty(Pid id) const
{
    switch (id) {
    case Pid::MMREST_RANGE_BRACKET_TYPE:
        return int(bracketType());
    default:
        return MeasureNumberBase::getProperty(id);
    }
}

bool MMRestRange::setProperty(Pid id, const PropertyValue& val)
{
    switch (id) {
    case Pid::MMREST_RANGE_BRACKET_TYPE:
        setBracketType(MMRestRangeBracketType(val.toInt()));
        setLayoutInvalid();
        triggerLayout();
        return true;
    default:
        return MeasureNumberBase::setProperty(id, val);
    }
}

PropertyValue MMRestRange::propertyDefault(Pid id) const
{
    switch (id) {
    case Pid::TEXT_STYLE:
        return TextStyleType::MMREST_RANGE;
    case Pid::PLACEMENT:
        return score()->styleV(Sid::mmRestRangeVPlacement);
    case Pid::HPLACEMENT:
        return score()->styleV(Sid::mmRestRangeHPlacement);
    default:
        return MeasureNumberBase::propertyDefault(id);
    }
}

//---------------------------------------------------------
//   setXmlText
///   This is reimplemented from TextBase::setXmlText to take care of the brackets
//---------------------------------------------------------

void MMRestRange::setXmlText(const String& s)
{
    switch (bracketType()) {
    case MMRestRangeBracketType::BRACKETS:
        TextBase::setXmlText(u"[" + s + u"]");
        break;
    case MMRestRangeBracketType::PARENTHESES:
        TextBase::setXmlText(u"(" + s + u")");
        break;
    case MMRestRangeBracketType::NONE:
    default:
        TextBase::setXmlText(s);
        break;
    }
}
} // namespace MS
