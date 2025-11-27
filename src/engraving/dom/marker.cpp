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

#include "marker.h"

#include "dom/staff.h"
#include "dom/system.h"
#include "types/typesconv.h"

#include "measure.h"
#include "score.h"

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

//---------------------------------------------------------
//   Marker
//---------------------------------------------------------

Marker::Marker(EngravingItem* parent)
    : Marker(parent, TextStyleType::REPEAT_LEFT)
{
    resetProperty(Pid::MUSIC_SYMBOL_SIZE);
}

Marker::Marker(EngravingItem* parent, TextStyleType tid)
    : TextBase(ElementType::MARKER, parent, tid, ElementFlag::MOVABLE | ElementFlag::ON_STAFF | ElementFlag::SYSTEM)
{
    initElementStyle(&markerStyle);
    resetProperty(Pid::MUSIC_SYMBOL_SIZE);
    m_markerType = MarkerType::FINE;
}

//---------------------------------------------------------
//   setMarkerType
//---------------------------------------------------------

void Marker::setMarkerType(MarkerType t)
{
    m_markerType = t;
    const char* txt = 0;
    switch (t) {
    case MarkerType::SEGNO:
        txt = "<sym>segno</sym>";
        setLabel(u"segno");
        break;

    case MarkerType::VARSEGNO:
        txt = "<sym>segnoSerpent1</sym>";
        setLabel(u"varsegno");
        break;

    case MarkerType::CODA:
        txt = "<sym>coda</sym>";
        setLabel(u"codab");
        break;

    case MarkerType::VARCODA:
        txt = "<sym>codaSquare</sym>";
        setLabel(u"varcoda");
        break;

    case MarkerType::CODETTA:
        txt = "<sym>coda</sym><sym>coda</sym>";
        setLabel(u"codetta");
        break;

    case MarkerType::FINE:
        txt = "Fine";
        initTextStyleType(TextStyleType::REPEAT_RIGHT, true);
        setLabel(u"fine");
        break;

    case MarkerType::TOCODA:
        txt = "To Coda";
        initTextStyleType(TextStyleType::REPEAT_RIGHT, true);
        setLabel(u"coda");
        break;

    case MarkerType::TOCODASYM:
        txt = "To <font size=\"20\"/><sym>coda</sym>";
        initTextStyleType(TextStyleType::REPEAT_RIGHT, true);
        setLabel(u"coda");
        break;

    case MarkerType::DA_CODA:
        txt = "Da Coda";
        initTextStyleType(TextStyleType::REPEAT_RIGHT, true);
        setLabel(u"coda");
        break;

    case MarkerType::DA_DBLCODA:
        txt = "Da Doppia Coda";
        initTextStyleType(TextStyleType::REPEAT_RIGHT, true);
        setLabel(u"coda");
        break;

    case MarkerType::USER:
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
    return TConv::translatedUserName(m_markerType);
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
        return markerType();
    case Pid::MARKER_CENTER_ON_SYMBOL:
        return centerOnSymbol();
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
        setMarkerType(v.value<MarkerType>());
        break;
    case Pid::MARKER_CENTER_ON_SYMBOL:
        setCenterOnSymbol(v.toBool());
        break;
    default:
        if (!TextBase::setProperty(propertyId, v)) {
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

PropertyValue Marker::propertyDefault(Pid propertyId) const
{
    switch (propertyId) {
    case Pid::LABEL:
        return String();
    case Pid::MARKER_TYPE:
        return MarkerType::FINE;
    case Pid::PLACEMENT:
        return PlacementV::ABOVE;
    case Pid::MARKER_CENTER_ON_SYMBOL:
        return true;
    case Pid::MUSIC_SYMBOL_SIZE:
        return 18.0;
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
    if (markerType() == MarkerType::FINE) {
        seg = measure()->last();
        return seg->firstElementForNavigation(staffIdx());
    }
    Measure* prevMeasure = measure()->prevMeasureMM();
    if (prevMeasure) {
        seg = prevMeasure->last();
        return seg->firstElementForNavigation(staffIdx());
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
    return String(u"%1: %2").arg(EngravingItem::accessibleInfo(), markerTypeUserName());
}

std::vector<LineF> Marker::dragAnchorLines() const
{
    Measure* measure = parentItem() ? toMeasure(parentItem()) : nullptr;

    std::vector<LineF> lines(TextBase::dragAnchorLines());

    if (!measure || !isRightMarker()) {
        return lines;
    }

    for (LineF& l : lines) {
        l.setP1(l.p1() + PointF(measure->width(), 0.0));
    }

    return lines;
}

String Marker::symbolString() const
{
    // Returns the coda/segno symbol if present
    const static std::array REPEAT_SYMBOL_NAMES {
        String(u"<sym>coda</sym>"),
        String(u"<sym>codaSquare</sym>"),
        String(u"<sym>codaJapanes</sym>"),
        String(u"<sym>segno</sym>"),
        String(u"<sym>segnoSerpent1</sym>"),
        String(u"<sym>segnoSerpent2</sym>"),
        String(u"<sym>segnoJapanese</sym>"),
    };

    for (const String& sym : REPEAT_SYMBOL_NAMES) {
        if (xmlText().contains(sym)) {
            return sym;
        }
    }

    return String();
}

//---------------------------------------------------------
//   subtypeUserName
//---------------------------------------------------------

muse::TranslatableString Marker::subtypeUserName() const
{
    return TConv::userName(m_markerType);
}
}
