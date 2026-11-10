/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2026 MuseScore Limited and others
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

#include "boxline.h"

namespace mu::engraving {
//---------------------------------------------------------
//   BoxLineSegment
//---------------------------------------------------------
BoxLineSegment::BoxLineSegment(BoxLine* sp)
    : LineSegment(ElementType::BOXLINE_SEGMENT, sp, ElementFlag::MOVABLE | ElementFlag::ON_STAFF)
{}

//---------------------------------------------------------
//   BoxLine
//---------------------------------------------------------

BoxLine::BoxLine(DummyParentOr<EngravingItem> parent)
    : SLine(ElementType::BOXLINE, parent, ElementFlag::PLACE_ABOVE)
{}

BoxLine::BoxLine(const BoxLine& other)
    : SLine(other)
{
    m_height = other.m_height;
}

LineSegment* BoxLine::createLineSegment()
{
    BoxLineSegment* seg = new BoxLineSegment(this);
    seg->setTrack(track());
    seg->setColor(lineColor());
    return seg;
}

//---------------------------------------------------------
//   getProperty
//---------------------------------------------------------

PropertyValue BoxLine::getProperty(Pid propertyId) const
{
    switch (propertyId) {
    case Pid::BOXLINE_HEIGHT:
        return boxHeight();
    default:
        break;
    }
    return SLine::getProperty(propertyId);
}

//---------------------------------------------------------
//   setProperty
//---------------------------------------------------------

bool BoxLine::setProperty(Pid propertyId, const PropertyValue& val)
{
    switch (propertyId) {
    case Pid::BOXLINE_HEIGHT:
        setBoxHeight(val.value<Spatium>());
        break;
    default:
        return SLine::setProperty(propertyId, val);
    }
    triggerLayout();
    return true;
}

//---------------------------------------------------------
//   propertyDefault
//---------------------------------------------------------

PropertyValue BoxLine::propertyDefault(Pid propertyId) const
{
    switch (propertyId) {
    case Pid::BOXLINE_HEIGHT:
        return 2.0_sp;
    case Pid::PLACEMENT:
        return PlacementV::ABOVE;
    default:
        return SLine::propertyDefault(propertyId);
    }
}

PointF BoxLine::defaultPos() const
{
    const double gap = 1.0 * spatium();
    return PointF(0.0, placeAbove() ? -gap : gap);
}
}
