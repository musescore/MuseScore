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

#include "actionicon.h"
#include "rw/xml.h"
#include "property.h"

#include "log.h"

using namespace mu;
using namespace mu::draw;

namespace Ms {
ActionIcon::ActionIcon(EngravingItem* score)
    : EngravingItem(ElementType::ACTION_ICON, score)
{
    m_iconFont = Font(QString::fromStdString(engravingConfiguration()->iconsFontFamily()));
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

qreal ActionIcon::extent() const
{
    return m_extent;
}

void ActionIcon::setExtent(qreal extent)
{
    m_extent = extent;
    m_iconFont.setPointSizeF(m_extent / 2);
}

void ActionIcon::write(XmlWriter& xml) const
{
    xml.startObject(this);
    xml.tag("subtype", int(m_actionType));
    if (!m_actionCode.empty()) {
        xml.tag("action", QString::fromStdString(m_actionCode));
    }
    xml.endObject();
}

void ActionIcon::read(XmlReader& e)
{
    while (e.readNextStartElement()) {
        const QStringRef& tag(e.name());
        if (tag == "action") {
            m_actionCode = e.readElementText().toStdString();
        } else if (tag == "subtype") {
            m_actionType = static_cast<ActionIconType>(e.readInt());
        } else {
            e.unknown();
        }
    }
}

RectF ActionIcon::boundingBox() const
{
    return RectF(0, 0, m_extent, m_extent);
}

void ActionIcon::layout()
{
    setbbox(boundingBox());
}

void ActionIcon::draw(Painter* painter) const
{
    TRACE_OBJ_DRAW;
    painter->setFont(m_iconFont);
    painter->drawText(boundingBox(), Qt::AlignCenter, QChar(m_icon));
}

engraving::PropertyValue ActionIcon::getProperty(Pid pid) const
{
    switch (pid) {
    case Pid::ACTION:
        return QString::fromStdString(actionCode());
    default:
        break;
    }
    return EngravingItem::getProperty(pid);
}

bool ActionIcon::setProperty(Pid pid, const engraving::PropertyValue& v)
{
    switch (pid) {
    case Pid::ACTION:
        m_actionCode = v.toString().toStdString();
        triggerLayout();
        break;
    default:
        return EngravingItem::setProperty(pid, v);
    }
    return true;
}
}
