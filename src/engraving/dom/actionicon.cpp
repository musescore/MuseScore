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

#include "actionicon.h"

#include "property.h"

using namespace mu;
using namespace muse::draw;
using namespace mu::engraving;

ActionIcon::ActionIcon(EngravingItem* parent)
    : EngravingItem(ElementType::ACTION_ICON, parent)
{
    m_iconFont = Font(configuration()->iconsFontFamily(), Font::Type::Icon);
    m_iconFont.setPointSizeF(DEFAULT_FONT_SIZE);
}

ActionIcon* ActionIcon::clone() const
{
    return new ActionIcon(*this);
}

ActionIconType ActionIcon::actionType() const
{
    return m_actionType;
}

void ActionIcon::setActionType(ActionIconType val)
{
    m_actionType = val;
}

const std::string& ActionIcon::actionCode() const
{
    return m_actionCode;
}

void ActionIcon::setAction(const std::string& actionCode, char16_t icon)
{
    m_actionCode = actionCode;
    m_icon = icon;
}

double ActionIcon::fontSize() const
{
    return m_iconFont.pointSizeF();
}

void ActionIcon::setFontSize(double size)
{
    m_iconFont.setPointSizeF(size);
}

engraving::PropertyValue ActionIcon::getProperty(Pid pid) const
{
    switch (pid) {
    case Pid::ACTION:
        return String::fromStdString(actionCode());
    default:
        break;
    }
    return EngravingItem::getProperty(pid);
}

bool ActionIcon::setProperty(Pid pid, const PropertyValue& v)
{
    switch (pid) {
    case Pid::ACTION:
        m_actionCode = v.value<String>().toStdString();
        triggerLayout();
        break;
    default:
        return EngravingItem::setProperty(pid, v);
    }
    return true;
}
