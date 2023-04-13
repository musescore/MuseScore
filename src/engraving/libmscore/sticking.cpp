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

#include "sticking.h"

#include "rw/xml.h"

#include "segment.h"

#include "log.h"

using namespace mu;

namespace mu::engraving {
//---------------------------------------------------------
//   stickingStyle
//---------------------------------------------------------

static const ElementStyle stickingStyle {
    { Sid::stickingPlacement, Pid::PLACEMENT },
    { Sid::stickingMinDistance, Pid::MIN_DISTANCE },
};

//---------------------------------------------------------
//   Sticking
//---------------------------------------------------------

Sticking::Sticking(Segment* parent)
    : TextBase(ElementType::STICKING, parent, TextStyleType::STICKING, ElementFlag::MOVABLE | ElementFlag::ON_STAFF)
{
    initElementStyle(&stickingStyle);
}

//---------------------------------------------------------
//   write
//---------------------------------------------------------

void Sticking::write(XmlWriter& xml) const
{
    UNREACHABLE;
    if (!xml.context()->canWrite(this)) {
        return;
    }
    xml.startElement(this);
    TextBase::writeProperties(xml);
    xml.endElement();
}

bool Sticking::isEditAllowed(EditData& ed) const
{
    if (isTextNavigationKey(ed.key, ed.modifiers)) {
        return false;
    }

    return TextBase::isEditAllowed(ed);
}

//---------------------------------------------------------
//   layout
//---------------------------------------------------------

void Sticking::layout()
{
    TextBase::layout();
    autoplaceSegmentElement();
}

//---------------------------------------------------------
//   propertyDefault
//---------------------------------------------------------

engraving::PropertyValue Sticking::propertyDefault(Pid id) const
{
    switch (id) {
    case Pid::TEXT_STYLE:
        return TextStyleType::STICKING;
    default:
        return TextBase::propertyDefault(id);
    }
}
}
