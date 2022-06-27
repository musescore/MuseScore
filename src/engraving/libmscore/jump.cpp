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

#include "jump.h"

#include "translation.h"
#include "rw/xml.h"

#include "score.h"
#include "measure.h"

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
    { Jump::Type::DC,         "D.C.",         "start", "end",  "",      QT_TRANSLATE_NOOP("jumpType", "Da Capo") },
    { Jump::Type::DC_AL_FINE, "D.C. al Fine", "start", "fine", "",      QT_TRANSLATE_NOOP("jumpType", "Da Capo al Fine") },
    { Jump::Type::DC_AL_CODA, "D.C. al Coda", "start", "coda", "codab", QT_TRANSLATE_NOOP("jumpType", "Da Capo al Coda") },
    { Jump::Type::DS_AL_CODA, "D.S. al Coda", "segno", "coda", "codab", QT_TRANSLATE_NOOP("jumpType", "D.S. al Coda") },
    { Jump::Type::DS_AL_FINE, "D.S. al Fine", "segno", "fine", "",      QT_TRANSLATE_NOOP("jumpType", "D.S. al Fine") },
    { Jump::Type::DS,         "D.S.",         "segno", "end",  "",      QT_TRANSLATE_NOOP("jumpType", "D.S.") },

    { Jump::Type::DC_AL_DBLCODA,  "D.C. al Double Coda",   "start", "varcoda",  "codab",
      QT_TRANSLATE_NOOP("jumpType", "Da Capo al Double Coda") },
    { Jump::Type::DS_AL_DBLCODA,  "D.S. al Double Coda",   "segno", "varcoda",  "codab",
      QT_TRANSLATE_NOOP("jumpType", "Da Segno al Double Coda") },
    { Jump::Type::DSS,            "Dal Segno Segno",       "varsegno", "end",  "",
      QT_TRANSLATE_NOOP("jumpType", "Dal Segno Segno") },
    { Jump::Type::DSS_AL_CODA,    "D.S.S. al Coda",        "varsegno", "coda",  "codab",
      QT_TRANSLATE_NOOP("jumpType", "Dal Segno Segno al Coda") },
    { Jump::Type::DSS_AL_DBLCODA, "D.S.S. al Double Coda", "varsegno", "varcoda", "codab",
      QT_TRANSLATE_NOOP("jumpType", "Dal Segno Segno al Double Coda") },
    { Jump::Type::DSS_AL_FINE,    "D.S.S. al Fine",        "varsegno", "fine",  "",
      QT_TRANSLATE_NOOP("jumpType", "Dal Segno Segno al Fine") },
    { Jump::Type::DCODA,          "Da Coda",               "coda", "end",  "",
      QT_TRANSLATE_NOOP("jumpType", "Da Coda") },
    { Jump::Type::DDBLCODA,       "Da Double Coda",        "varcoda", "end",  "",
      QT_TRANSLATE_NOOP("jumpType", "Da Double Coda") }
};

//---------------------------------------------------------
//   Jump
//---------------------------------------------------------

Jump::Jump(Measure* parent)
    : TextBase(ElementType::JUMP, parent, TextStyleType::REPEAT_RIGHT, ElementFlag::MOVABLE | ElementFlag::SYSTEM | ElementFlag::ON_STAFF)
{
    initElementStyle(&jumpStyle);
    setLayoutToParentWidth(true);
    _playRepeats = false;
}

//---------------------------------------------------------
//   setJumpType
//---------------------------------------------------------

void Jump::setJumpType(Type t)
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

Jump::Type Jump::jumpType() const
{
    for (const JumpTypeTableItem& t : jumpTypeTable) {
        if (_jumpTo == t.jumpTo && _playUntil == t.playUntil && _continueAt == t.continueAt) {
            return t.type;
        }
    }
    return Type::USER;
}

String Jump::jumpTypeUserName() const
{
    size_t idx = static_cast<size_t>(jumpType());
    if (idx < jumpTypeTable.size()) {
        return mtrc("engraving", jumpTypeTable[idx].userText);
    }
    return mtrc("engraving", "Custom");
}

//---------------------------------------------------------
//   layout
//---------------------------------------------------------

void Jump::layout()
{
    TextBase::layout();
    autoplaceMeasureElement();
}

//---------------------------------------------------------
//   read
//---------------------------------------------------------

void Jump::read(XmlReader& e)
{
    while (e.readNextStartElement()) {
        const AsciiStringView tag(e.name());
        if (tag == "jumpTo") {
            _jumpTo = e.readText();
        } else if (tag == "playUntil") {
            _playUntil = e.readText();
        } else if (tag == "continueAt") {
            _continueAt = e.readText();
        } else if (tag == "playRepeats") {
            _playRepeats = e.readBool();
        } else if (!TextBase::readProperties(e)) {
            e.unknown();
        }
    }
}

//---------------------------------------------------------
//   write
//---------------------------------------------------------

void Jump::write(XmlWriter& xml) const
{
    xml.startElement(this);
    TextBase::writeProperties(xml);
    xml.tag("jumpTo", _jumpTo);
    xml.tag("playUntil", _playUntil);
    xml.tag("continueAt", _continueAt);
    writeProperty(xml, Pid::PLAY_REPEATS);
    xml.endElement();
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
        return String("");
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
    return seg->firstElement(staffIdx());
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
    return String("%1: %2").arg(EngravingItem::accessibleInfo(), this->jumpTypeUserName());
}
}
