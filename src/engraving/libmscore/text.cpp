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

#include "text.h"
#include "io/xml.h"
#include "score.h"

using namespace mu;

namespace Ms {
//---------------------------------------------------------
//   defaultStyle
//---------------------------------------------------------

static const ElementStyle defaultStyle {
    { Sid::defaultSystemFlag, Pid::SYSTEM_FLAG },
};

//---------------------------------------------------------
//   Text
//---------------------------------------------------------

Text::Text(Score* s, Tid tid)
    : TextBase(ElementType::TEXT, s, tid)
{
    initElementStyle(&defaultStyle);
}

//---------------------------------------------------------
//   read
//---------------------------------------------------------

void Text::read(XmlReader& e)
{
    while (e.readNextStartElement()) {
        const QStringRef& tag(e.name());
        if (tag == "style") {
            QString sn = e.readElementText();
            if (sn == "Tuplet") {              // ugly hack for compatibility
                continue;
            }
            Tid s = textStyleFromName(sn);
            initTid(s);
        } else if (!readProperties(e)) {
            e.unknown();
        }
    }
}

//---------------------------------------------------------
//   propertyDefault
//---------------------------------------------------------

QVariant Text::propertyDefault(Pid id) const
{
    switch (id) {
    case Pid::SUB_STYLE:
        return int(Tid::DEFAULT);
    default:
        return TextBase::propertyDefault(id);
    }
}
}
