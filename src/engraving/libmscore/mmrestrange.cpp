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

#include "score.h"
#include "mmrestrange.h"
#include "measure.h"

using namespace mu;

namespace Ms {
//---------------------------------------------------------
//   mmRestRangeStyle
//---------------------------------------------------------

static const ElementStyle mmRestRangeStyle {
    { Sid::mmRestRangeBracketType, Pid::MMREST_RANGE_BRACKET_TYPE },
    { Sid::mmRestRangeVPlacement,  Pid::PLACEMENT },
    { Sid::mmRestRangeHPlacement,  Pid::HPLACEMENT }
};

MMRestRange::MMRestRange(Score* s)
    : MeasureNumberBase(s, Tid::MMREST_RANGE)
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

QVariant MMRestRange::getProperty(Pid id) const
{
    switch (id) {
    case Pid::MMREST_RANGE_BRACKET_TYPE:
        return int(bracketType());
    default:
        return MeasureNumberBase::getProperty(id);
    }
}

bool MMRestRange::setProperty(Pid id, const QVariant& val)
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

QVariant MMRestRange::propertyDefault(Pid id) const
{
    switch (id) {
    case Pid::SUB_STYLE:
        return int(Tid::MMREST_RANGE);
    case Pid::PLACEMENT:
        return score()->styleV(Sid::mmRestRangeVPlacement);
    case Pid::HPLACEMENT:
        return score()->styleV(Sid::mmRestRangeHPlacement);
    default:
        return MeasureNumberBase::propertyDefault(id);
    }
}

bool MMRestRange::readProperties(XmlReader& xml)
{
    if (readProperty(xml.name(), xml, Pid::MMREST_RANGE_BRACKET_TYPE)) {
        return true;
    } else {
        return MeasureNumberBase::readProperties(xml);
    }
}

//---------------------------------------------------------
//   setXmlText
///   This is reimplemented from TextBase::setXmlText to take care of the brackets
//---------------------------------------------------------

void MMRestRange::setXmlText(const QString& s)
{
    switch (bracketType()) {
    case MMRestRangeBracketType::BRACKETS:
        TextBase::setXmlText("[" + s + "]");
        break;
    case MMRestRangeBracketType::PARENTHESES:
        TextBase::setXmlText("(" + s + ")");
        break;
    case MMRestRangeBracketType::NONE:
    default:
        TextBase::setXmlText(s);
        break;
    }
}
} // namespace MS
