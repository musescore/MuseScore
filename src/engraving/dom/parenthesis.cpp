/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2024 MuseScore Limited
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

#include "parenthesis.h"

#include "segment.h"
#include "types/typesconv.h"

using namespace mu::engraving;

Parenthesis::Parenthesis(Segment* parent)
    : EngravingItem(ElementType::PARENTHESIS, parent, ElementFlag::ON_STAFF | ElementFlag::GENERATED)
{
}

Parenthesis::Parenthesis(const Parenthesis& p)
    : EngravingItem(p)
{
    _direction = p._direction;
}

PropertyValue Parenthesis::getProperty(Pid pid) const
{
    switch (pid) {
    case Pid::HORIZONTAL_DIRECTION:
        return direction();
    default:
        return EngravingItem::getProperty(pid);
    }
}

bool Parenthesis::setProperty(Pid pid, const PropertyValue& v)
{
    switch (pid) {
    case Pid::HORIZONTAL_DIRECTION:
        setDirection(v.value<DirectionH>());
        break;
    default:
        return EngravingItem::setProperty(pid, v);
    }

    return true;
}

PropertyValue Parenthesis::propertyDefault(Pid pid) const
{
    switch (pid) {
    case Pid::HORIZONTAL_DIRECTION:
        return DirectionH::LEFT;
    default:
        return EngravingItem::propertyDefault(pid);
    }
}

String Parenthesis::accessibleInfo() const
{
    return String(u"%1: %2").arg(EngravingItem::accessibleInfo(), TConv::translatedUserName(direction()));
}
