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

#include "organpedalmark.h"

#include "engraving/types/symnames.h"

#include "chord.h"
#include "measure.h"
#include "note.h"
#include "staff.h"

namespace mu::engraving {
static const ElementStyle organPedalMarkStyle {
    { Sid::organPedalMarkPlacement, Pid::PLACEMENT },
    { Sid::organPedalMarkMinDistance, Pid::MIN_DISTANCE },
};

//---------------------------------------------------------
//   OrganPedalMark
//---------------------------------------------------------

OrganPedalMark::OrganPedalMark(Note* parent)
    : TextBase(ElementType::ORGAN_PEDAL_MARK, parent, TextStyleType::ORGAN_PEDAL_MARK, ElementFlag::HAS_TAG)
{
    initElementStyle(&organPedalMarkStyle);
}

//---------------------------------------------------------
//   typeUserName
//---------------------------------------------------------

muse::TranslatableString OrganPedalMark::typeUserName() const
{
    return TranslatableString("engraving/sym", SymNames::translatedUserNameForSymId(symId()));
}

//---------------------------------------------------------
//   accessibleInfo
//---------------------------------------------------------

String OrganPedalMark::accessibleInfo() const
{
    return String(u"%1: %2").arg(EngravingItem::accessibleInfo(), SymNames::translatedUserNameForSymId(symId()));
}

//---------------------------------------------------------
//   propertyDefault
//---------------------------------------------------------

engraving::PropertyValue OrganPedalMark::propertyDefault(Pid id) const
{
    switch (id) {
    case Pid::TEXT_STYLE:
        return TextStyleType::ORGAN_PEDAL_MARK;
    default:
        return TextBase::propertyDefault(id);
    }
}
}
