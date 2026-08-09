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
    { Sid::organPedalMarkPosition, Pid::POSITION },
    { Sid::organPedalMarkMinDistance, Pid::MIN_DISTANCE },
};

static const QList<SymIdList> POPUP_SET_DEFAULT {
    {
        SymId::keyboardPedalToe2,
        SymId::keyboardPedalHeel1,
        SymId::keyboardPedalHeel3,
        SymId::keyboardPedalToe1,
        SymId::keyboardPedalHeel2,
    },
    {
        SymId::keyboardPedalToeToHeel,
        SymId::keyboardPedalHeelToToe,
        SymId::keyboardPedalHeelToe,
    }
};

static const QList<SymIdList> POPUP_SET_CONCISE {
    {
        SymId::keyboardPedalToe2,
        SymId::keyboardPedalHeel1,
        SymId::keyboardPedalHeel3,
    },
    {
        SymId::keyboardPedalToeToHeel,
        SymId::keyboardPedalHeelToToe,
        SymId::keyboardPedalHeelToe,
    }
};

//---------------------------------------------------------
//   OrganPedalMark
//---------------------------------------------------------

OrganPedalMark::OrganPedalMark(Note* parent)
    : TextBase(ElementType::ORGAN_PEDAL_MARK, parent, TextStyleType::ORGAN_PEDAL_MARK, ElementFlag::ON_STAFF)
{
    initElementStyle(&organPedalMarkStyle);
    setPedalMark(engravingConfiguration()->organPedalMarksDefaultPedalMark());
}

//---------------------------------------------------------
//   propertyDefault
//---------------------------------------------------------

PropertyValue OrganPedalMark::propertyDefault(Pid id) const
{
    switch (id) {
    case Pid::TEXT_STYLE:
        return TextStyleType::ORGAN_PEDAL_MARK;
    default:
        return TextBase::propertyDefault(id);
    }
}

//---------------------------------------------------------
//   typeUserName
//---------------------------------------------------------

muse::TranslatableString OrganPedalMark::typeUserName() const
{
    return TranslatableString("engraving", "Organ pedal mark");
}

//---------------------------------------------------------
//   subtypeUserName
//---------------------------------------------------------

muse::TranslatableString OrganPedalMark::subtypeUserName() const
{
    return SymNames::userNameForSymId(symId());
}

//---------------------------------------------------------
//   accessibleInfo
//---------------------------------------------------------

String OrganPedalMark::accessibleInfo() const
{
    return String(u"%1: %2").arg(EngravingItem::accessibleInfo(), translatedSubtypeUserName());
}

//---------------------------------------------------------
//   getPedalMarksSet
//---------------------------------------------------------

QList<SymIdList> OrganPedalMark::getPedalMarksPopupSet() const
{
    switch(engravingConfiguration()->organPedalMarksPopupSet()) {
    case OrganPedalMarksPopupSet::CONCISE:
        return POPUP_SET_CONCISE;
    default:
        return POPUP_SET_DEFAULT;
    }
}

//---------------------------------------------------------
//   setPedalMark
//---------------------------------------------------------

void OrganPedalMark::setPedalMark(engraving::SymId pedalMarkSymId) {
    setSymId(pedalMarkSymId);

    const String pedalMarkName = String::fromAscii(SymNames::nameForSymId(pedalMarkSymId).ascii());

    undoChangeProperty(Pid::TEXT, u"<sym>" + pedalMarkName + u"</sym>");
}

//---------------------------------------------------------
//   changePedalMark
//---------------------------------------------------------

void OrganPedalMark::changePedalMark(int popupPageIndex, int pageElementIndex)
{
    setPedalMark(getPedalMarksPopupSet()[popupPageIndex][pageElementIndex]);
}
}