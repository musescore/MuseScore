//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2019 Werner Schweer and others
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
//=============================================================================

#include "sticking.h"
#include "xml.h"

namespace Ms {
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

Sticking::Sticking(Score* s)
    : TextBase(s, Tid::STICKING, ElementFlag::MOVABLE | ElementFlag::ON_STAFF)
{
    initElementStyle(&stickingStyle);
}

//---------------------------------------------------------
//   write
//---------------------------------------------------------

void Sticking::write(XmlWriter& xml) const
{
    if (!xml.canWrite(this)) {
        return;
    }
    xml.stag(this);
    TextBase::writeProperties(xml);
    xml.etag();
}

//---------------------------------------------------------
//   read
//---------------------------------------------------------

void Sticking::read(XmlReader& e)
{
    while (e.readNextStartElement()) {
        if (!TextBase::readProperties(e)) {
            e.unknown();
        }
    }
    styleChanged();
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

QVariant Sticking::propertyDefault(Pid id) const
{
    switch (id) {
    case Pid::SUB_STYLE:
        return int(Tid::STICKING);
    default:
        return TextBase::propertyDefault(id);
    }
}
}
