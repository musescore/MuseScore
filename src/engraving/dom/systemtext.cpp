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

#include "systemtext.h"

using namespace mu;

namespace mu::engraving {
//---------------------------------------------------------
//   systemStyle
//---------------------------------------------------------

static const ElementStyle systemStyle {
    { Sid::systemTextPlacement,                Pid::PLACEMENT },
    { Sid::systemTextMinDistance,              Pid::MIN_DISTANCE },
};

//---------------------------------------------------------
//   SystemText
//---------------------------------------------------------

SystemText::SystemText(Segment* parent, TextStyleType tid, ElementType type)
    : StaffTextBase(type, parent, tid, ElementFlag::SYSTEM | ElementFlag::MOVABLE | ElementFlag::ON_STAFF)
{
    initElementStyle(&systemStyle);
}

//---------------------------------------------------------
//   isEditAllowed
//---------------------------------------------------------

bool SystemText::isEditAllowed(EditData& ed) const
{
    bool ctrlPressed  = ed.modifiers & ControlModifier;
    bool shiftPressed = ed.modifiers & ShiftModifier;
    bool altPressed = ed.modifiers & AltModifier;
    if (altPressed && !ctrlPressed && !shiftPressed && (ed.key == Key_Left || ed.key == Key_Right)) {
        return false;
    }

    return TextBase::isEditAllowed(ed);
}

//---------------------------------------------------------
//   propertyDefault
//---------------------------------------------------------

engraving::PropertyValue SystemText::propertyDefault(Pid id) const
{
    switch (id) {
    case Pid::TEXT_STYLE:
        return TextStyleType::SYSTEM;
    default:
        return TextBase::propertyDefault(id);
    }
}
} // namespace mu::engraving
