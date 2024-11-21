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

#include "fingering.h"

#include "translation.h"

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
//   fingeringStyle
//---------------------------------------------------------

static const ElementStyle fingeringStyle {
    { Sid::fingeringPlacement, Pid::PLACEMENT },
    { Sid::fingeringMinDistance, Pid::MIN_DISTANCE },
};

//---------------------------------------------------------
//   Fingering
//      EngravingItem(Score* = 0, ElementFlags = ElementFlag::NOTHING);
//---------------------------------------------------------

Fingering::Fingering(Note* parent, TextStyleType tid, ElementFlags ef)
    : TextBase(ElementType::FINGERING, parent, tid, ef)
{
    setPlacement(PlacementV::ABOVE);
    initElementStyle(&fingeringStyle);
}

Fingering::Fingering(Note* parent, ElementFlags ef)
    : Fingering(parent, TextStyleType::FINGERING, ef)
{
}

//---------------------------------------------------------
//   layoutType
//---------------------------------------------------------

ElementType Fingering::layoutType() const
{
    switch (textStyleType()) {
    case TextStyleType::FINGERING:
    case TextStyleType::RH_GUITAR_FINGERING:
    case TextStyleType::STRING_NUMBER:
        return ElementType::CHORD;
    default:
        return ElementType::NOTE;
    }
}

//---------------------------------------------------------
//   calculatePlacement
//---------------------------------------------------------

PlacementV Fingering::calculatePlacement() const
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

bool Fingering::isEditAllowed(EditData& ed) const
{
    if (isTextNavigationKey(ed.key, ed.modifiers)) {
        return false;
    }

    if (ed.key == Key_Left) {
        return cursor()->column() != 0 || cursor()->hasSelection();
    }

    if (ed.key == Key_Right) {
        bool cursorInLastColumn = cursor()->column() == cursor()->curLine().columns();
        return !cursorInLastColumn || cursor()->hasSelection();
    }

    return TextBase::isEditAllowed(ed);
}

//---------------------------------------------------------
//   accessibleInfo
//---------------------------------------------------------

String Fingering::accessibleInfo() const
{
    String rez = EngravingItem::accessibleInfo();
    if (textStyleType() == TextStyleType::STRING_NUMBER) {
        rez += u' ' + muse::mtrc("engraving", "String number");
    }
    return String(u"%1: %2").arg(rez, plainText());
}

bool Fingering::isOnCrossBeamSide() const
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
//   propertyDefault
//---------------------------------------------------------

engraving::PropertyValue Fingering::propertyDefault(Pid id) const
{
    switch (id) {
    case Pid::PLACEMENT:
        return calculatePlacement();
    case Pid::TEXT_STYLE:
        return TextStyleType::FINGERING;
    default:
        return TextBase::propertyDefault(id);
    }
}
}
