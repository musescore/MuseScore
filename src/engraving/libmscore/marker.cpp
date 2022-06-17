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

#include "marker.h"

#include "translation.h"
#include "rw/xml.h"

#include "score.h"
#include "measure.h"

#include "log.h"

using namespace mu;
using namespace mu::engraving;

namespace mu::engraving {
//---------------------------------------------------------
//   markerStyle
//---------------------------------------------------------

static const ElementStyle markerStyle {
    { Sid::repeatLeftPlacement, Pid::PLACEMENT },
    { Sid::repeatMinDistance,   Pid::MIN_DISTANCE },
};

//must be in sync with Marker::Type enum
const std::vector<MarkerTypeItem> markerTypeTable = {
    { Marker::Type::SEGNO, QT_TRANSLATE_NOOP("markerType", "Segno") },
    { Marker::Type::VARSEGNO, QT_TRANSLATE_NOOP("markerType", "Segno variation") },
    { Marker::Type::CODA, QT_TRANSLATE_NOOP("markerType", "Coda") },
    { Marker::Type::VARCODA, QT_TRANSLATE_NOOP("markerType", "Varied coda") },
    { Marker::Type::CODETTA, QT_TRANSLATE_NOOP("markerType", "Codetta") },
    { Marker::Type::FINE, QT_TRANSLATE_NOOP("markerType", "Fine") },
    { Marker::Type::TOCODA, QT_TRANSLATE_NOOP("markerType", "To coda") },
    { Marker::Type::TOCODASYM, QT_TRANSLATE_NOOP("markerType", "To coda (symbol)") },
    { Marker::Type::USER, QT_TRANSLATE_NOOP("markerType", "Custom") }
};

//---------------------------------------------------------
//   Marker
//---------------------------------------------------------

Marker::Marker(EngravingItem* parent)
    : Marker(parent, TextStyleType::REPEAT_LEFT)
{
}

Marker::Marker(EngravingItem* parent, TextStyleType tid)
    : TextBase(ElementType::MARKER, parent, tid, ElementFlag::MOVABLE | ElementFlag::ON_STAFF | ElementFlag::SYSTEM)
{
    initElementStyle(&markerStyle);
    _markerType = Type::FINE;
    setLayoutToParentWidth(true);
}

//---------------------------------------------------------
//   setMarkerType
//---------------------------------------------------------

void Marker::setMarkerType(Type t)
{
    _markerType = t;
    const char* txt = 0;
    switch (t) {
    case Type::SEGNO:
        txt = "<sym>segno</sym>";
        setLabel(u"segno");
        break;

    case Type::VARSEGNO:
        txt = "<sym>segnoSerpent1</sym>";
        setLabel(u"varsegno");
        break;

    case Type::CODA:
        txt = "<sym>coda</sym>";
        setLabel(u"codab");
        break;

    case Type::VARCODA:
        txt = "<sym>codaSquare</sym>";
        setLabel(u"varcoda");
        break;

    case Type::CODETTA:
        txt = "<sym>coda</sym><sym>coda</sym>";
        setLabel(u"codetta");
        break;

    case Type::FINE:
        txt = "Fine";
        initTextStyleType(TextStyleType::REPEAT_RIGHT, true);
        setLabel(u"fine");
        break;

    case Type::TOCODA:
        txt = "To Coda";
        initTextStyleType(TextStyleType::REPEAT_RIGHT, true);
        setLabel(u"coda");
        break;

    case Type::TOCODASYM:
        txt = "To <font size=\"20\"/><sym>coda</sym>";
        initTextStyleType(TextStyleType::REPEAT_RIGHT, true);
        setLabel(u"coda");
        break;

    case Type::USER:
        break;

    default:
        LOGD("unknown marker type %d", int(t));
        break;
    }
    if (empty() && txt) {
        setXmlText(String::fromAscii(txt));
    }
}

//---------------------------------------------------------
//   markerTypeUserName
//---------------------------------------------------------

String Marker::markerTypeUserName() const
{
    return mtrc("engraving", markerTypeTable[static_cast<int>(_markerType)].name);
}

//---------------------------------------------------------
//   styleChanged
//---------------------------------------------------------

void Marker::styleChanged()
{
    setMarkerType(_markerType);
    TextBase::styleChanged();
}

//---------------------------------------------------------
//   markerType
//---------------------------------------------------------

Marker::Type Marker::markerType(const String& s) const
{
    if (s == "segno") {
        return Type::SEGNO;
    } else if (s == "varsegno") {
        return Type::VARSEGNO;
    } else if (s == "codab") {
        return Type::CODA;
    } else if (s == "varcoda") {
        return Type::VARCODA;
    } else if (s == "codetta") {
        return Type::CODETTA;
    } else if (s == "fine") {
        return Type::FINE;
    } else if (s == "coda") {
        return Type::TOCODA;
    } else {
        return Type::USER;
    }
}

//---------------------------------------------------------
//   layout
//---------------------------------------------------------

void Marker::layout()
{
    TextBase::layout();

    // although normally laid out to parent (measure) width,
    // force to center over barline if left-aligned

    if (!score()->isPaletteScore() && layoutToParentWidth() && align() == AlignH::LEFT) {
        rxpos() -= width() * 0.5;
    }

    autoplaceMeasureElement();
}

//---------------------------------------------------------
//   read
//---------------------------------------------------------

void Marker::read(XmlReader& e)
{
    Type mt = Type::SEGNO;

    while (e.readNextStartElement()) {
        const AsciiStringView tag(e.name());
        if (tag == "label") {
            String s(e.readText());
            setLabel(s);
            mt = markerType(s);
        } else if (!TextBase::readProperties(e)) {
            e.unknown();
        }
    }
    setMarkerType(mt);
}

//---------------------------------------------------------
//   write
//---------------------------------------------------------

void Marker::write(XmlWriter& xml) const
{
    xml.startElement(this);
    TextBase::writeProperties(xml);
    xml.tag("label", _label);
    xml.endElement();
}

//---------------------------------------------------------
//   undoSetLabel
//---------------------------------------------------------

void Marker::undoSetLabel(const String& s)
{
    undoChangeProperty(Pid::LABEL, s);
}

//---------------------------------------------------------
//   undoSetMarkerType
//---------------------------------------------------------

void Marker::undoSetMarkerType(Type t)
{
    undoChangeProperty(Pid::MARKER_TYPE, int(t));
}

//---------------------------------------------------------
//   getProperty
//---------------------------------------------------------

PropertyValue Marker::getProperty(Pid propertyId) const
{
    switch (propertyId) {
    case Pid::LABEL:
        return label();
    case Pid::MARKER_TYPE:
        return int(markerType());
    default:
        break;
    }
    return TextBase::getProperty(propertyId);
}

//---------------------------------------------------------
//   setProperty
//---------------------------------------------------------

bool Marker::setProperty(Pid propertyId, const PropertyValue& v)
{
    switch (propertyId) {
    case Pid::LABEL:
        setLabel(v.value<String>());
        break;
    case Pid::MARKER_TYPE:
        setMarkerType(Type(v.toInt()));
        break;
    default:
        if (!TextBase::setProperty(propertyId, v)) {
            return false;
        }
        break;
    }
    triggerLayoutAll();
    return true;
}

//---------------------------------------------------------
//   propertyDefault
//---------------------------------------------------------

PropertyValue Marker::propertyDefault(Pid propertyId) const
{
    switch (propertyId) {
    case Pid::LABEL:
        return String();
    case Pid::MARKER_TYPE:
        return int(Type::FINE);
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

EngravingItem* Marker::nextSegmentElement()
{
    Segment* seg;
    if (markerType() == Marker::Type::FINE) {
        seg = measure()->last();
        return seg->firstElement(staffIdx());
    }
    Measure* prevMeasure = measure()->prevMeasureMM();
    if (prevMeasure) {
        seg = prevMeasure->last();
        return seg->firstElement(staffIdx());
    }
    return EngravingItem::nextSegmentElement();
}

//---------------------------------------------------------
//   prevSegmentElement
//---------------------------------------------------------

EngravingItem* Marker::prevSegmentElement()
{
    //it's the same barline
    return nextSegmentElement();
}

//---------------------------------------------------------
//   accessibleInfo
//---------------------------------------------------------

String Marker::accessibleInfo() const
{
    return String("%1: %2").arg(EngravingItem::accessibleInfo(), markerTypeUserName());
}
}
