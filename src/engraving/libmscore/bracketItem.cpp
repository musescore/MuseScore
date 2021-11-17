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
#include "bracketItem.h"
#include "property.h"
#include "staff.h"

using namespace mu;

namespace Ms {
BracketItem::BracketItem(EngravingItem* parent)
    : EngravingItem(ElementType::BRACKET_ITEM, parent)
{
}

BracketItem::BracketItem(EngravingItem* parent, BracketType a, int b)
    : EngravingItem(ElementType::BRACKET_ITEM, parent), _bracketType(a), _bracketSpan(b)
{
}

EngravingItem* BracketItem::clone() const
{
    return new BracketItem(*this);
}

mu::engraving::PropertyValue BracketItem::getProperty(Pid id) const
{
    switch (id) {
    case Pid::SYSTEM_BRACKET:
        return int(_bracketType);
    case Pid::BRACKET_COLUMN:
        return _column;
    case Pid::BRACKET_SPAN:
        return _bracketSpan;
    default:
        return EngravingItem::getProperty(id);
    }
}

bool BracketItem::setProperty(Pid id, const engraving::PropertyValue& v)
{
    switch (id) {
    case Pid::SYSTEM_BRACKET:
        staff()->setBracketType(column(), BracketType(v.toInt())); // change bracket type global
        break;
    case Pid::BRACKET_COLUMN:
        staff()->changeBracketColumn(column(), v.toInt());
        break;
    case Pid::BRACKET_SPAN:
        _bracketSpan = v.toInt();
        break;
    default:
        return EngravingItem::setProperty(id, v);
    }
    score()->setLayoutAll();
    return true;
}

engraving::PropertyValue BracketItem::propertyDefault(Pid id) const
{
    switch (id) {
    case Pid::SYSTEM_BRACKET:
        return int(BracketType::NORMAL);
    case Pid::BRACKET_COLUMN:
        return 0;
    default:
        return EngravingItem::propertyDefault(id);
    }
}
}
