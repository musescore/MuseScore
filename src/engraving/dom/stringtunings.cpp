/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-CLA-applies
 *
 * MuseScore
 * Music Composition & Notation
 *
 * Copyright (C) 2022 MuseScore BVBA and others
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

#include "stringtunings.h"

#include "types/typesconv.h"

#include "part.h"
#include "score.h"
#include "segment.h"

using namespace mu;
using namespace mu::engraving;

// STYLE
static const ElementStyle stringTuningsStyle {
        { Sid::staffTextPlacement, Pid::PLACEMENT },
        { Sid::staffTextMinDistance, Pid::MIN_DISTANCE },
};

StringTunings::StringTunings(Segment* parent, TextStyleType textStyleType)
    : StaffTextBase(ElementType::STRING_TUNINGS, parent, textStyleType, ElementFlag::MOVABLE | ElementFlag::ON_STAFF)
{
    initElementStyle(&stringTuningsStyle);
}

StringTunings::StringTunings(const StringTunings& s)
    : StaffTextBase(s)
{

}

StringTunings* StringTunings::clone() const
{
    return new StringTunings(*this);
}

PropertyValue StringTunings::getProperty(Pid id) const
{
    return StaffTextBase::getProperty(id);
}

PropertyValue StringTunings::propertyDefault(Pid id) const
{
    return StaffTextBase::propertyDefault(id);
}

bool StringTunings::setProperty(Pid propertyId, const PropertyValue& val)
{
    return StaffTextBase::setProperty(propertyId, val);
}

StringData* StringTunings::stringData() const
{
    return m_stringData;
}

void StringTunings::setStringData(StringData* stringData)
{
    m_stringData = stringData;
}

void StringTunings::updateText()
{
}
