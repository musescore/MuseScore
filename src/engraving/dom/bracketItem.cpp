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

#include "bracketItem.h"

#include "property.h"
#include "score.h"
#include "staff.h"

using namespace mu;

namespace mu::engraving {
BracketItem::BracketItem(EngravingItem* parent)
    : EngravingItem(ElementType::BRACKET_ITEM, parent)
{
}

BracketItem::BracketItem(EngravingItem* parent, BracketType a, int b)
    : EngravingItem(ElementType::BRACKET_ITEM, parent), m_bracketType(a), m_bracketSpan(b)
{
}

EngravingItem* BracketItem::clone() const
{
    return new BracketItem(*this);
}

PropertyValue BracketItem::getProperty(Pid id) const
{
    switch (id) {
    case Pid::SYSTEM_BRACKET:
        return int(m_bracketType);
    case Pid::BRACKET_COLUMN:
        return m_column;
    case Pid::BRACKET_SPAN:
        return static_cast<int>(m_bracketSpan);
    case Pid::STAFF_LONG_NAME:
        return longName();
    case Pid::STAFF_SHORT_NAME:
        return shortName();
    case Pid::GROUP_BRACKET_SHOW_TEXT:
        return m_showText;
    case Pid::GROUP_BRACKET_SHOW_BRACKET:
        return m_showBracket;
    default:
        return EngravingItem::getProperty(id);
    }
}

bool BracketItem::setProperty(Pid id, const PropertyValue& v)
{
    switch (id) {
    case Pid::SYSTEM_BRACKET:
        staff()->setBracketType(column(), BracketType(v.toInt())); // change bracket type global
        break;
    case Pid::BRACKET_COLUMN:
        staff()->changeBracketColumn(column(), static_cast<size_t>(v.toInt()));
        break;
    case Pid::BRACKET_SPAN:
        m_bracketSpan = static_cast<size_t>(v.toInt());
        break;
    case Pid::VISIBLE:
        setVisible(v.toBool());
        break;
    case Pid::STAFF_LONG_NAME:
        m_name.setLongName(v.value<String>());
        break;
    case Pid::STAFF_SHORT_NAME:
        m_name.setShortName(v.value<String>());
        break;
    case Pid::GROUP_BRACKET_SHOW_TEXT:
        m_showText = v.toBool();
        break;
    case Pid::GROUP_BRACKET_SHOW_BRACKET:
        m_showBracket = v.toBool();
        break;
    default:
        return EngravingItem::setProperty(id, v);
    }
    score()->setLayoutAll();
    return true;
}

PropertyValue BracketItem::propertyDefault(Pid id) const
{
    switch (id) {
    case Pid::SYSTEM_BRACKET:
        return int(BracketType::NORMAL);
    case Pid::BRACKET_COLUMN:
        return size_t(0);
    case Pid::STAFF_LONG_NAME:
        return muse::mtrc("systemBrackets", "GROUP");
    case Pid::STAFF_SHORT_NAME:
        return muse::mtrc("systemBrackets", "GR.", "Short for GROUP");
    case Pid::GROUP_BRACKET_SHOW_TEXT:
    case Pid::GROUP_BRACKET_SHOW_BRACKET:
        return true;
    default:
        return EngravingItem::propertyDefault(id);
    }
}

bool BracketItem::intersects(const BracketItem* other) const
{
    staff_idx_t firstOfOther = other->staff()->idx();
    staff_idx_t lastOfOther = firstOfOther + other->bracketSpan() - 1;
    return intersects(firstOfOther, lastOfOther);
}

bool BracketItem::intersects(staff_idx_t first, staff_idx_t last) const
{
    staff_idx_t firstOfThis = staff()->idx();
    staff_idx_t lastOfThis = firstOfThis + bracketSpan() - 1;
    return firstOfThis <= last && lastOfThis >= first;
}
}
