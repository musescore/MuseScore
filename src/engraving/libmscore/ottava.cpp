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

#include "ottava.h"

#include "style/style.h"
#include "rw/xml.h"

#include "system.h"
#include "measure.h"
#include "utils.h"
#include "score.h"
#include "text.h"
#include "staff.h"
#include "segment.h"
#include "musescoreCore.h"

using namespace mu;
using namespace mu::engraving;

namespace Ms {
//---------------------------------------------------------
//   ottavaStyle
//---------------------------------------------------------

static const ElementStyle ottavaStyle {
    { Sid::ottavaNumbersOnly,                  Pid::NUMBERS_ONLY },
    { Sid::ottava8VAPlacement,                 Pid::PLACEMENT },
    { Sid::ottava8VAText,                      Pid::BEGIN_TEXT },
    { Sid::ottava8VAContinueText,              Pid::CONTINUE_TEXT },
    { Sid::ottavaHookAbove,                    Pid::END_HOOK_HEIGHT },
    { Sid::ottavaFontFace,                     Pid::BEGIN_FONT_FACE },
    { Sid::ottavaFontFace,                     Pid::CONTINUE_FONT_FACE },
    { Sid::ottavaFontFace,                     Pid::END_FONT_FACE },
    { Sid::ottavaFontSize,                     Pid::BEGIN_FONT_SIZE },
    { Sid::ottavaFontSize,                     Pid::CONTINUE_FONT_SIZE },
    { Sid::ottavaFontSize,                     Pid::END_FONT_SIZE },
    { Sid::ottavaFontStyle,                    Pid::BEGIN_FONT_STYLE },
    { Sid::ottavaFontStyle,                    Pid::CONTINUE_FONT_STYLE },
    { Sid::ottavaFontStyle,                    Pid::END_FONT_STYLE },
    { Sid::ottavaTextAlign,                    Pid::BEGIN_TEXT_ALIGN },
    { Sid::ottavaTextAlign,                    Pid::CONTINUE_TEXT_ALIGN },
    { Sid::ottavaTextAlign,                    Pid::END_TEXT_ALIGN },
    { Sid::ottavaLineWidth,                    Pid::LINE_WIDTH },
    { Sid::ottavaLineStyle,                    Pid::LINE_STYLE },
    { Sid::ottavaPosAbove,                     Pid::OFFSET },
};

OttavaSegment::OttavaSegment(Ottava* sp, System* parent)
    : TextLineBaseSegment(ElementType::OTTAVA_SEGMENT, sp, parent, ElementFlag::MOVABLE | ElementFlag::ON_STAFF)
{
}

//---------------------------------------------------------
//   layout
//---------------------------------------------------------

void OttavaSegment::layout()
{
    TextLineBaseSegment::layout();
    autoplaceSpannerSegment();
}

//---------------------------------------------------------
//   propertyDelegate
//---------------------------------------------------------

EngravingItem* OttavaSegment::propertyDelegate(Pid pid)
{
    if (pid == Pid::OTTAVA_TYPE || pid == Pid::NUMBERS_ONLY) {
        return spanner();
    }
    return TextLineBaseSegment::propertyDelegate(pid);
}

//---------------------------------------------------------
//   setOttavaType
//---------------------------------------------------------

void Ottava::setOttavaType(OttavaType val)
{
    if (_ottavaType == val) {
        return;
    }
    _ottavaType = val;
    styleChanged();
}

//---------------------------------------------------------
//   setNumbersOnly
//---------------------------------------------------------

void Ottava::setNumbersOnly(bool val)
{
    _numbersOnly = val;
}

//---------------------------------------------------------
//   setPlacement
//---------------------------------------------------------

void Ottava::setPlacement(PlacementV p)
{
    TextLineBase::setPlacement(p);
}

//---------------------------------------------------------
//   undoChangeProperty
//---------------------------------------------------------

void OttavaSegment::undoChangeProperty(Pid id, const PropertyValue& v, PropertyFlags ps)
{
    if (id == Pid::OTTAVA_TYPE || id == Pid::NUMBERS_ONLY) {
        EngravingObject::undoChangeProperty(id, v, ps);
    } else {
        EngravingObject::undoChangeProperty(id, v, ps);
    }
}

void Ottava::undoChangeProperty(Pid id, const PropertyValue& v, PropertyFlags ps)
{
    if (id == Pid::OTTAVA_TYPE || id == Pid::NUMBERS_ONLY) {
        TextLineBase::undoChangeProperty(id, v, ps);
        styleChanged();       // these properties may change style settings
    } else {
        TextLineBase::undoChangeProperty(id, v, ps);
    }
}

//---------------------------------------------------------
//   getPropertyStyle
//---------------------------------------------------------

Sid OttavaSegment::getPropertyStyle(Pid pid) const
{
    switch (pid) {
    case Pid::OFFSET:
        return spanner()->placeAbove() ? Sid::ottavaPosAbove : Sid::ottavaPosBelow;
    default:
        return TextLineBaseSegment::getPropertyStyle(pid);
    }
}

Sid Ottava::getPropertyStyle(Pid pid) const
{
    Q_ASSERT(int(OttavaType::OTTAVA_22MB) - int(OttavaType::OTTAVA_8VA) == 5);

    static const std::vector<Sid> ss = {
        Sid::ottava8VAPlacement,
        Sid::ottava8VAnoText,
        Sid::ottava8VAnoContinueText,
        Sid::ottava8VBPlacement,
        Sid::ottava8VBnoText,
        Sid::ottava8VBnoContinueText,
        Sid::ottava15MAPlacement,
        Sid::ottava15MAnoText,
        Sid::ottava15MAnoContinueText,
        Sid::ottava15MBPlacement,
        Sid::ottava15MBnoText,
        Sid::ottava15MBnoContinueText,
        Sid::ottava22MAPlacement,
        Sid::ottava22MAnoText,
        Sid::ottava22MAnoContinueText,
        Sid::ottava22MBPlacement,
        Sid::ottava22MBnoText,
        Sid::ottava22MBnoContinueText,

        Sid::ottava8VAPlacement,
        Sid::ottava8VAText,
        Sid::ottava8VAContinueText,
        Sid::ottava8VBPlacement,
        Sid::ottava8VBText,
        Sid::ottava8VBContinueText,
        Sid::ottava15MAPlacement,
        Sid::ottava15MAText,
        Sid::ottava15MAContinueText,
        Sid::ottava15MBPlacement,
        Sid::ottava15MBText,
        Sid::ottava15MBContinueText,
        Sid::ottava22MAPlacement,
        Sid::ottava22MAText,
        Sid::ottava22MAContinueText,
        Sid::ottava22MBPlacement,
        Sid::ottava22MBText,
        Sid::ottava22MBContinueText,
    };

    size_t idx = size_t(_ottavaType) * 3 + (_numbersOnly ? 0 : ss.size() / 2);
    switch (pid) {
    case Pid::OFFSET:
        return placeAbove() ? Sid::ottavaPosAbove : Sid::ottavaPosBelow;
    case Pid::PLACEMENT:
        return ss[idx];
    case Pid::BEGIN_TEXT:
        return ss[idx + 1];               // BEGIN_TEXT
    case Pid::CONTINUE_TEXT:
        return ss[idx + 2];               // CONTINUE_TEXT
    case Pid::END_HOOK_HEIGHT:
        if (isStyled(Pid::PLACEMENT)) {
            return score()->styleI(ss[idx]) == int(PlacementV::ABOVE) ? Sid::ottavaHookAbove : Sid::ottavaHookBelow;
        } else {
            return placeAbove() ? Sid::ottavaHookAbove : Sid::ottavaHookBelow;
        }
    default:
        return TextLineBase::getPropertyStyle(pid);
    }
}

//---------------------------------------------------------
//   Ottava
//---------------------------------------------------------

Ottava::Ottava(EngravingItem* parent)
    : TextLineBase(ElementType::OTTAVA, parent, ElementFlag::ON_STAFF | ElementFlag::MOVABLE)
{
    _ottavaType  = OttavaType::OTTAVA_8VA;
    _numbersOnly = false;
    setBeginTextPlace(TextPlace::LEFT);
    setContinueTextPlace(TextPlace::LEFT);
    setEndHookType(HookType::HOOK_90);
    setLineVisible(true);
    setBeginHookHeight(Spatium(.0));
    setEndText("");

    initElementStyle(&ottavaStyle);
}

Ottava::Ottava(const Ottava& o)
    : TextLineBase(o)
{
    setOttavaType(o._ottavaType);
    _numbersOnly = o._numbersOnly;
}

//---------------------------------------------------------
//   pitchShift
//---------------------------------------------------------

int Ottava::pitchShift() const
{
    return ottavaDefault[int(_ottavaType)].shift;
}

//---------------------------------------------------------
//   createLineSegment
//---------------------------------------------------------

static const ElementStyle ottavaSegmentStyle {
    { Sid::ottavaPosAbove, Pid::OFFSET },
    { Sid::ottavaMinDistance, Pid::MIN_DISTANCE },
};

LineSegment* Ottava::createLineSegment(System* parent)
{
    OttavaSegment* os = new OttavaSegment(this, parent);
    os->setTrack(track());
    os->initElementStyle(&ottavaSegmentStyle);
    return os;
}

//---------------------------------------------------------
//   write
//---------------------------------------------------------

void Ottava::write(XmlWriter& xml) const
{
    if (!xml.canWrite(this)) {
        return;
    }
    xml.startObject(this);
    writeProperty(xml, Pid::OTTAVA_TYPE);
    writeProperty(xml, Pid::PLACEMENT);
    writeProperty(xml, Pid::NUMBERS_ONLY);
//      for (const StyledProperty& spp : *styledProperties())
//            writeProperty(xml, spp.pid);
    TextLineBase::writeProperties(xml);
    xml.endObject();
}

//---------------------------------------------------------
//   read
//---------------------------------------------------------

void Ottava::read(XmlReader& e)
{
    eraseSpannerSegments();
    if (score()->mscVersion() < 301) {
        e.addSpanner(e.intAttribute("id", -1), this);
    }
    while (e.readNextStartElement()) {
        readProperties(e);
    }
    if (_ottavaType != OttavaType::OTTAVA_8VA || _numbersOnly != propertyDefault(Pid::NUMBERS_ONLY).toBool()) {
        styleChanged();
    }
}

//---------------------------------------------------------
//   readProperties
//---------------------------------------------------------

bool Ottava::readProperties(XmlReader& e)
{
    const QStringRef& tag(e.name());
    if (tag == "subtype") {
        QString s = e.readElementText();
        bool ok;
        int idx = s.toInt(&ok);
        if (!ok) {
            _ottavaType = OttavaType::OTTAVA_8VA;
            for (OttavaDefault d : ottavaDefault) {
                if (s == d.name) {
                    _ottavaType = d.type;
                    break;
                }
            }
        } else if (score()->mscVersion() <= 114) {
            //subtype are now in a different order...
            if (idx == 1) {
                idx = 2;
            } else if (idx == 2) {
                idx = 1;
            }
            _ottavaType = OttavaType(idx);
        } else {
            _ottavaType = OttavaType(idx);
        }
    } else if (readStyledProperty(e, tag)) {
        return true;
    } else if (!TextLineBase::readProperties(e)) {
        e.unknown();
        return false;
    }
    return true;
}

//---------------------------------------------------------
//   getProperty
//---------------------------------------------------------

PropertyValue Ottava::getProperty(Pid propertyId) const
{
    switch (propertyId) {
    case Pid::OTTAVA_TYPE:
        return int(ottavaType());

    case Pid::NUMBERS_ONLY:
        return _numbersOnly;

    case Pid::END_TEXT_PLACE:                         // HACK
        return TextPlace::LEFT;

    default:
        break;
    }
    return TextLineBase::getProperty(propertyId);
}

//---------------------------------------------------------
//   setProperty
//---------------------------------------------------------

bool Ottava::setProperty(Pid propertyId, const PropertyValue& val)
{
    switch (propertyId) {
    case Pid::OTTAVA_TYPE:
        setOttavaType(OttavaType(val.toInt()));
        break;

    case Pid::NUMBERS_ONLY:
        _numbersOnly = val.toBool();
        break;

    case Pid::SPANNER_TICKS:
        setTicks(val.value<Fraction>());
        staff()->updateOttava();
        break;

    case Pid::SPANNER_TICK:
        setTick(val.value<Fraction>());
        staff()->updateOttava();
        break;

    default:
        if (!TextLineBase::setProperty(propertyId, val)) {
            return false;
        }
        break;
    }
    triggerLayout();
    return true;
}

//---------------------------------------------------------
//   propertyDefault
//---------------------------------------------------------

PropertyValue Ottava::propertyDefault(Pid pid) const
{
    switch (pid) {
    case Pid::OTTAVA_TYPE:
        return PropertyValue();
    case Pid::END_HOOK_TYPE:
        return HookType::HOOK_90;
    case Pid::LINE_VISIBLE:
        return true;
    case Pid::BEGIN_TEXT_OFFSET:
    case Pid::CONTINUE_TEXT_OFFSET:
    case Pid::END_TEXT_OFFSET:
        return PropertyValue::fromValue(PointF());
    case Pid::BEGIN_TEXT_PLACE:
    case Pid::CONTINUE_TEXT_PLACE:
    case Pid::END_TEXT_PLACE:
        return TextPlace::LEFT;
    case Pid::BEGIN_HOOK_TYPE:
        return HookType::NONE;
    case Pid::BEGIN_HOOK_HEIGHT:
        return Spatium(.0);
    case Pid::END_TEXT:
        return QString("");
    case Pid::PLACEMENT:
        return styleValue(Pid::PLACEMENT, getPropertyStyle(Pid::PLACEMENT));

    default:
        return TextLineBase::propertyDefault(pid);
    }
}

//---------------------------------------------------------
//   Ottava::propertyId
//---------------------------------------------------------

Pid Ottava::propertyId(const QStringRef& name) const
{
    if (name == propertyName(Pid::OTTAVA_TYPE)) {
        return Pid::OTTAVA_TYPE;
    }
    return TextLineBase::propertyId(name);
}

//---------------------------------------------------------
//   accessibleInfo
//---------------------------------------------------------

QString Ottava::accessibleInfo() const
{
    return QString("%1: %2").arg(EngravingItem::accessibleInfo(), ottavaDefault[static_cast<int>(ottavaType())].name);
}

//---------------------------------------------------------
//   ottavaTypeName
//---------------------------------------------------------

const char* Ottava::ottavaTypeName(OttavaType type)
{
    return ottavaDefault[int(type)].name;
}
}
