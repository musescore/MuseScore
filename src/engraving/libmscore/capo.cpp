/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-CLA-applies
 *
 * MuseScore
 * Music Composition & Notation
 *
 * Copyright (C) 2023 MuseScore BVBA and others
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

#include "capo.h"

#include "segment.h"

using namespace mu::engraving;

static const ElementStyle capoStyle {
    { Sid::staffTextPlacement, Pid::PLACEMENT },
    { Sid::staffTextMinDistance, Pid::MIN_DISTANCE },
};

Capo::Capo(Segment* parent, TextStyleType textStyleType)
    : StaffTextBase(ElementType::CAPO, parent, textStyleType, ElementFlag::MOVABLE | ElementFlag::ON_STAFF)
{
    initElementStyle(&capoStyle);
}

Capo* Capo::clone() const
{
    return new Capo(*this);
}

PropertyValue Capo::getProperty(Pid id) const
{
    return StaffTextBase::getProperty(id);
}

PropertyValue Capo::propertyDefault(Pid id) const
{
    return StaffTextBase::propertyDefault(id);
}

bool Capo::setProperty(Pid propertyId, const PropertyValue& val)
{
    return StaffTextBase::setProperty(propertyId, val);
}
