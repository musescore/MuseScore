/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2025 MuseScore Limited and others
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

#include "editproperty.h"

#include "../dom/bracketitem.h"
#include "../dom/score.h"
#include "../dom/staff.h"
#include "../dom/textline.h"

#include "log.h"
#define LOG_UNDO() if (0) LOGD()

using namespace mu::engraving;

//---------------------------------------------------------
//   ChangeProperty::flip
//---------------------------------------------------------

void ChangeProperty::flip()
{
    LOG_UNDO() << element->typeName() << int(id) << "(" << propertyName(id) << ")" << element->getProperty(id) << "->" << property;

    PropertyValue v       = element->getProperty(id);
    PropertyFlags ps = element->propertyFlags(id);

    element->setProperty(id, property);
    element->setPropertyFlags(id, flags);

    property = v;
    flags = ps;
}

std::vector<EngravingObject*> ChangeProperty::objectItems() const
{
    return compoundObjects(element);
}

//---------------------------------------------------------
//   ChangeBracketProperty::flip
//---------------------------------------------------------

void ChangeBracketProperty::flip()
{
    if (!staff) {
        return;
    }

    element = staff->brackets()[level];
    ChangeProperty::flip();
    level = toBracketItem(element)->column();
}

//---------------------------------------------------------
//   ChangeTextLineProperty::flip
//---------------------------------------------------------

void ChangeTextLineProperty::flip()
{
    ChangeProperty::flip();
    if (element->isTextLine()) {
        toTextLine(element)->initStyle();
    }
}
