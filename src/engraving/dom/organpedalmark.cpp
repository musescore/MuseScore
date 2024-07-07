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

#include "translation.h"
#include "engraving/types/symnames.h"

#include "beam.h"
#include "chord.h"
#include "measure.h"
#include "note.h"
#include "part.h"
#include "skyline.h"
#include "staff.h"
#include "stem.h"
#include "system.h"

using namespace mu;

namespace mu::engraving {
//---------------------------------------------------------
//   organPedalMarkStyle
//---------------------------------------------------------

static const ElementStyle organPedalMarkStyle {
    { Sid::organPedalMarkPlacement, Pid::PLACEMENT },
    { Sid::organPedalMarkMinDistance, Pid::MIN_DISTANCE },
};

//---------------------------------------------------------
//   OrganPedalMark
//---------------------------------------------------------

OrganPedalMark::OrganPedalMark(Note* parent, TextStyleType tid, ElementFlags ef)
    : TextBase(ElementType::ORGAN_PEDAL_MARK, parent, tid, ef)
{
    m_symId = SymId::noSym;

    setPlacement(PlacementV::BELOW);
    initElementStyle(&organPedalMarkStyle);
}

OrganPedalMark::OrganPedalMark(Note* parent, ElementFlags ef)
    : OrganPedalMark(parent, TextStyleType::ORGAN_PEDAL_MARK, ef)
{
}

//---------------------------------------------------------
//   layoutType
//---------------------------------------------------------

ElementType OrganPedalMark::layoutType() const
{
    switch (textStyleType()) {
    case TextStyleType::ORGAN_PEDAL_MARK:
        return ElementType::CHORD;
    default:
        return ElementType::NOTE;
    }
}

//---------------------------------------------------------
//   calculatePlacement
//---------------------------------------------------------

PlacementV OrganPedalMark::calculatePlacement() const
{
    Note* n = note();
    if (!n) {
        return PlacementV::ABOVE;
    }
    Chord* chord = n->chord();
    Staff* staff = chord->staff();
    Part* part   = staff->part();
    size_t nstaves  = part->nstaves();
    bool voices  = chord->measure()->hasVoices(staff->idx(), chord->tick(), chord->actualTicks());
    bool below   = voices ? !chord->up() : (nstaves > 1) && (staff->rstaff() == nstaves - 1);
    return below ? PlacementV::BELOW : PlacementV::ABOVE;
}

bool OrganPedalMark::isEditAllowed(EditData& ed) const
{
    if (isTextNavigationKey(ed.key, ed.modifiers)) {
        return false;
    }

    return TextBase::isEditAllowed(ed);
}

bool OrganPedalMark::isOnCrossBeamSide() const
{
    Chord* chord = note() ? note()->chord() : nullptr;
    if (!chord) {
        return false;
    }
    return layoutType() == ElementType::CHORD
           && chord->beam() && (chord->beam()->cross() || chord->staffMove() != 0)
           && placeAbove() == chord->up();
}

//---------------------------------------------------------
//   setSymId
//---------------------------------------------------------

void OrganPedalMark::setSymId(SymId id)
{
    m_symId  = id;
}

//---------------------------------------------------------
//   typeUserName
//---------------------------------------------------------

muse::TranslatableString OrganPedalMark::typeUserName() const
{
    return TranslatableString("engraving/sym", SymNames::userNameForSymId(symId()));
}

String OrganPedalMark::translatedTypeUserName() const
{
    return SymNames::translatedUserNameForSymId(symId());
}

//---------------------------------------------------------
//   accessibleInfo
//---------------------------------------------------------

String OrganPedalMark::accessibleInfo() const
{
    return String(u"%1: %2").arg(EngravingItem::accessibleInfo(), translatedTypeUserName());
}

//---------------------------------------------------------
//   propertyDefault
//---------------------------------------------------------

engraving::PropertyValue OrganPedalMark::propertyDefault(Pid id) const
{
    switch (id) {
    case Pid::PLACEMENT:
        return calculatePlacement();
    case Pid::TEXT_STYLE:
        return TextStyleType::ORGAN_PEDAL_MARK;
    default:
        return TextBase::propertyDefault(id);
    }
}
}
