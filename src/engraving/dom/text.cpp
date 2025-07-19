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

#include "text.h"

#include "rw/rwregister.h"
#include "score.h"

#include "log.h"

using namespace mu;
using namespace mu::engraving;

namespace mu::engraving {
//---------------------------------------------------------
//   defaultStyle
//---------------------------------------------------------

static const ElementStyle defaultStyle {
    { Sid::defaultSystemFlag, Pid::SYSTEM_FLAG },
};

static bool styleIsSelectable(TextStyleType style)
{
    switch (style) {
    case TextStyleType::HEADER:
    case TextStyleType::FOOTER:
    case TextStyleType::COPYRIGHT:
    case TextStyleType::PAGE_NUMBER:
        return false;
    default: break;
    }
    return true;
}

//---------------------------------------------------------
//   Text
//---------------------------------------------------------

Text::Text(EngravingItem* parent, TextStyleType tid)
    : TextBase(ElementType::TEXT, parent, tid, styleIsSelectable(tid) ? ElementFlag::NOTHING : ElementFlag::NOT_SELECTABLE)
{
    initElementStyle(&defaultStyle);
}

//---------------------------------------------------------
//   propertyDefault
//---------------------------------------------------------

engraving::PropertyValue Text::propertyDefault(Pid id) const
{
    switch (id) {
    case Pid::TEXT_STYLE:
        return TextStyleType::DEFAULT;
    default:
        return TextBase::propertyDefault(id);
    }
}

PropertyValue Text::getProperty(Pid id) const
{
    switch (id) {
    case Pid::VOICE_ASSIGNMENT:
        if (hasVoiceAssignmentProperties()) {
            return parentItem()->getProperty(id);
        }
    // fallthrough
    default:
        return TextBase::getProperty(id);
    }
}

String Text::readXmlText(XmlReader& xml, Score* score)
{
    Text t(score->dummy());
    rw::RWRegister::reader()->readItem(&t, xml);
    return t.xmlText();
}

bool Text::hasVoiceAssignmentProperties() const
{
    const EngravingItem* parent = parentItem();
    if (parent && parent->isTextLineBaseSegment()) {
        return parent->hasVoiceAssignmentProperties();
    }
    return false;
}
}
