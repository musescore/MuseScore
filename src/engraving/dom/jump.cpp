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

#include "jump.h"

#include "types/typesconv.h"

#include "measure.h"
#include "score.h"

#include "log.h"

using namespace mu;
using namespace mu::engraving;

namespace mu::engraving {
//---------------------------------------------------------
//   jumpStyle
//---------------------------------------------------------

static const ElementStyle jumpStyle {
    { Sid::repeatRightPlacement, Pid::PLACEMENT },
    { Sid::repeatMinDistance,    Pid::MIN_DISTANCE },
};

//---------------------------------------------------------
//   JumpTypeTable
//---------------------------------------------------------

const std::vector<JumpTypeTableItem> jumpTypeTable {
    { JumpType::DC,         "D.C.",         "start", "end",  "" },
    { JumpType::DC_AL_FINE, "D.C. al Fine", "start", "fine", "" },
    { JumpType::DC_AL_CODA, "D.C. al Coda", "start", "coda", "codab" },
    { JumpType::DS_AL_CODA, "D.S. al Coda", "segno", "coda", "codab" },
    { JumpType::DS_AL_FINE, "D.S. al Fine", "segno", "fine", "" },
    { JumpType::DS,         "D.S.",         "segno", "end",  "" },

    { JumpType::DC_AL_DBLCODA,  "D.C. al Doppia Coda",   "start",    "varcoda", "codab" },
    { JumpType::DS_AL_DBLCODA,  "D.S. al Doppia Coda",   "segno",    "varcoda", "codab" },
    { JumpType::DSS,            "Dal Doppio Segno",      "varsegno", "end",     "" },
    { JumpType::DSS_AL_CODA,    "D.D.S. al Coda",        "varsegno", "coda",    "codab" },
    { JumpType::DSS_AL_DBLCODA, "D.D.S. al Doppia Coda", "varsegno", "varcoda", "codab" },
    { JumpType::DSS_AL_FINE,    "D.D.S. al Fine",        "varsegno", "fine",    "" },
};

//---------------------------------------------------------
//   Jump
//---------------------------------------------------------

Jump::Jump(Measure* parent)
    : TextBase(ElementType::JUMP, parent, TextStyleType::REPEAT_RIGHT, ElementFlag::MOVABLE | ElementFlag::SYSTEM | ElementFlag::ON_STAFF)
{
    initElementStyle(&jumpStyle);
    setLayoutToParentWidth(true);
    m_playRepeats = false;
}

//---------------------------------------------------------
//   setJumpType
//---------------------------------------------------------

void Jump::setJumpType(JumpType t)
{
    for (const JumpTypeTableItem& p : jumpTypeTable) {
        if (p.type == t) {
            setXmlText(String::fromAscii(p.text.ascii()));
            setJumpTo(String::fromAscii(p.jumpTo.ascii()));
            setPlayUntil(String::fromAscii(p.playUntil.ascii()));
            setContinueAt(String::fromAscii(p.continueAt.ascii()));
            initTextStyleType(TextStyleType::REPEAT_RIGHT);
            break;
        }
    }
}

//---------------------------------------------------------
//   jumpType
//---------------------------------------------------------

JumpType Jump::jumpType() const
{
    for (const JumpTypeTableItem& t : jumpTypeTable) {
        if (m_jumpTo == t.jumpTo && m_playUntil == t.playUntil && m_continueAt == t.continueAt) {
            return t.type;
        }
    }
    return JumpType::USER;
}

String Jump::jumpTypeUserName() const
{
    return TConv::translatedUserName(jumpType());
}

//---------------------------------------------------------
//   undoSetJumpTo
//---------------------------------------------------------

void Jump::undoSetJumpTo(const String& s)
{
    undoChangeProperty(Pid::JUMP_TO, s);
}

//---------------------------------------------------------
//   undoSetPlayUntil
//---------------------------------------------------------

void Jump::undoSetPlayUntil(const String& s)
{
    undoChangeProperty(Pid::PLAY_UNTIL, s);
}

//---------------------------------------------------------
//   undoSetContinueAt
//---------------------------------------------------------

void Jump::undoSetContinueAt(const String& s)
{
    undoChangeProperty(Pid::CONTINUE_AT, s);
}

//---------------------------------------------------------
//   getProperty
//---------------------------------------------------------

PropertyValue Jump::getProperty(Pid propertyId) const
{
    switch (propertyId) {
    case Pid::JUMP_TO:
        return jumpTo();
    case Pid::PLAY_UNTIL:
        return playUntil();
    case Pid::CONTINUE_AT:
        return continueAt();
    case Pid::PLAY_REPEATS:
        return playRepeats();
    default:
        break;
    }
    return TextBase::getProperty(propertyId);
}

//---------------------------------------------------------
//   setProperty
//---------------------------------------------------------

bool Jump::setProperty(Pid propertyId, const PropertyValue& v)
{
    switch (propertyId) {
    case Pid::JUMP_TO:
        setJumpTo(v.value<String>());
        break;
    case Pid::PLAY_UNTIL:
        setPlayUntil(v.value<String>());
        break;
    case Pid::CONTINUE_AT:
        setContinueAt(v.value<String>());
        break;
    case Pid::PLAY_REPEATS:
        setPlayRepeats(v.toInt());
        break;
    default:
        if (!TextBase::setProperty(propertyId, v)) {
            return false;
        }
        break;
    }
    triggerLayout();
    score()->setPlaylistDirty();
    return true;
}

//---------------------------------------------------------
//   propertyDefault
//---------------------------------------------------------

PropertyValue Jump::propertyDefault(Pid propertyId) const
{
    switch (propertyId) {
    case Pid::JUMP_TO:
    case Pid::PLAY_UNTIL:
    case Pid::CONTINUE_AT:
        return String();
    case Pid::PLAY_REPEATS:
        return false;
    case Pid::PLACEMENT:
        return PlacementV::ABOVE;
    default:
        break;
    }
    return TextBase::propertyDefault(propertyId);
}

//---------------------------------------------------------
//   nextSegmentElement
//---------------------------------------------------------

EngravingItem* Jump::nextSegmentElement()
{
    Segment* seg = measure()->last();
    return seg->firstElementForNavigation(staffIdx());
}

//---------------------------------------------------------
//   prevSegmentElement
//---------------------------------------------------------

EngravingItem* Jump::prevSegmentElement()
{
    return nextSegmentElement();
}

//---------------------------------------------------------
//   accessibleInfo
//---------------------------------------------------------

String Jump::accessibleInfo() const
{
    return String(u"%1: %2").arg(EngravingItem::accessibleInfo(), this->jumpTypeUserName());
}

//---------------------------------------------------------
//   subtypeUserName
//---------------------------------------------------------

muse::TranslatableString Jump::subtypeUserName() const
{
    return TConv::userName(jumpType());
}
}
