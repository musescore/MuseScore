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

#include "xml.h"
#include "icon.h"
#include "property.h"

using namespace mu;

namespace Ms {
Icon::Icon(Score* score)
    : Element(score)
{
}

Icon* Icon::clone() const
{
    return new Icon(*this);
}

ElementType Icon::type() const
{
    return ElementType::ICON;
}

IconType Icon::iconType() const
{
    return _iconType;
}

RectF Icon::boundingBox() const
{
    return RectF(0, 0, _extent, _extent);
}

void Icon::setAction(const std::string& actionCode, char16_t icon)
{
    _actionCode = actionCode;
    _icon = icon;
}

const std::string& Icon::actionCode() const
{
    return _actionCode;
}

void Icon::setIconType(IconType val)
{
    _iconType = val;
}

void Icon::setExtent(int extent)
{
    _extent = extent;
}

//---------------------------------------------------------
//   write
//---------------------------------------------------------

void Icon::write(XmlWriter& xml) const
{
    xml.stag(this);
    xml.tag("subtype", int(_iconType));
    if (!_actionCode.empty()) {
        xml.tag("action", QString::fromStdString(_actionCode));
    }
    xml.etag();
}

//---------------------------------------------------------
//   read
//---------------------------------------------------------

void Icon::read(XmlReader& e)
{
    while (e.readNextStartElement()) {
        const QStringRef& tag(e.name());
        if (tag == "action") {
            _actionCode = e.readElementText().toStdString();
        } else if (tag == "subtype") {
            _iconType = IconType(e.readInt());
        } else {
            e.unknown();
        }
    }
}

//---------------------------------------------------------
//   layout
//---------------------------------------------------------

void Icon::layout()
{
    setbbox(boundingBox());
}

//---------------------------------------------------------
//   draw
//---------------------------------------------------------

void Icon::draw(mu::draw::Painter* painter) const
{
    TRACE_OBJ_DRAW;
    painter->drawText(boundingBox(), Qt::AlignCenter, QChar(_icon));
}

//---------------------------------------------------------
//   Icon::getProperty
//---------------------------------------------------------

QVariant Icon::getProperty(Pid pid) const
{
    switch (pid) {
    case Pid::ACTION:
        return QString::fromStdString(actionCode());
    default:
        break;
    }
    return Element::getProperty(pid);
}

//---------------------------------------------------------
//   Icon::setProperty
//---------------------------------------------------------

bool Icon::setProperty(Pid pid, const QVariant& v)
{
    switch (pid) {
    case Pid::ACTION:
        _actionCode = v.toString().toStdString();
        triggerLayout();
        break;
    default:
        return Element::setProperty(pid, v);
    }
    return true;
}
}
