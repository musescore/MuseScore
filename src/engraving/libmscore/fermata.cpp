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

#include "fermata.h"

#include "log.h"
#include "rw/xml.h"
#include "types/symnames.h"

#include "score.h"
#include "chordrest.h"
#include "system.h"
#include "measure.h"
#include "staff.h"
#include "stafftype.h"
#include "undo.h"
#include "page.h"
#include "barline.h"

using namespace mu;
using namespace mu::engraving;

namespace mu::engraving {
//---------------------------------------------------------
//   fermataStyle
//---------------------------------------------------------

static const ElementStyle fermataStyle {
    { Sid::fermataPosAbove, Pid::OFFSET },
    { Sid::fermataMinDistance, Pid::MIN_DISTANCE },
};

//---------------------------------------------------------
//   Fermata
//---------------------------------------------------------

Fermata::Fermata(EngravingItem* parent)
    : EngravingItem(ElementType::FERMATA, parent, ElementFlag::MOVABLE | ElementFlag::ON_STAFF)
{
    setPlacement(PlacementV::ABOVE);
    _symId         = SymId::noSym;
    _timeStretch   = 1.0;
    setPlay(true);
    initElementStyle(&fermataStyle);
}

//---------------------------------------------------------
//   read
//---------------------------------------------------------

void Fermata::read(XmlReader& e)
{
    while (e.readNextStartElement()) {
        if (!readProperties(e)) {
            e.unknown();
        }
    }
}

//---------------------------------------------------------
//   readProperties
//---------------------------------------------------------

bool Fermata::readProperties(XmlReader& e)
{
    const AsciiStringView tag(e.name());

    if (tag == "subtype") {
        AsciiStringView s = e.readAsciiText();
        SymId id = SymNames::symIdByName(s);
        setSymId(id);
    } else if (tag == "play") {
        setPlay(e.readBool());
    } else if (tag == "timeStretch") {
        _timeStretch = e.readDouble();
    } else if (tag == "offset") {
        if (score()->mscVersion() > 114) {
            EngravingItem::readProperties(e);
        } else {
            e.skipCurrentElement();       // ignore manual layout in older scores
        }
    } else if (EngravingItem::readProperties(e)) {
    } else {
        return false;
    }
    return true;
}

//---------------------------------------------------------
//   write
//---------------------------------------------------------

void Fermata::write(XmlWriter& xml) const
{
    if (!xml.context()->canWrite(this)) {
        LOGD("%s not written", typeName());
        return;
    }
    xml.startElement(this);
    xml.tag("subtype", SymNames::nameForSymId(_symId));
    writeProperty(xml, Pid::TIME_STRETCH);
    writeProperty(xml, Pid::PLAY);
    writeProperty(xml, Pid::MIN_DISTANCE);
    if (!isStyled(Pid::OFFSET)) {
        writeProperty(xml, Pid::OFFSET);
    }
    EngravingItem::writeProperties(xml);
    xml.endElement();
}

//---------------------------------------------------------
//   subtype
//---------------------------------------------------------

int Fermata::subtype() const
{
    QString s = SymNames::nameForSymId(_symId).toQLatin1String();
    if (s.endsWith("Below")) {
        return int(SymNames::symIdByName(s.left(s.size() - 5) + "Above"));
    } else {
        return int(_symId);
    }
}

//---------------------------------------------------------
//   typeUserName
//---------------------------------------------------------

String Fermata::typeUserName() const
{
    return SymNames::translatedUserNameForSymId(symId());
}

//---------------------------------------------------------
//   Symbol::draw
//---------------------------------------------------------

void Fermata::draw(mu::draw::Painter* painter) const
{
    TRACE_OBJ_DRAW;
    painter->setPen(curColor());
    drawSymbol(_symId, painter, PointF(-0.5 * width(), 0.0));
}

//---------------------------------------------------------
//   chordRest
//---------------------------------------------------------

ChordRest* Fermata::chordRest() const
{
    if (explicitParent() && explicitParent()->isChordRest()) {
        return toChordRest(explicitParent());
    }
    return 0;
}

//---------------------------------------------------------
//   measure
//---------------------------------------------------------

Measure* Fermata::measure() const
{
    Segment* s = segment();
    return toMeasure(s ? s->explicitParent() : 0);
}

//---------------------------------------------------------
//   system
//---------------------------------------------------------

System* Fermata::system() const
{
    Measure* m = measure();
    return toSystem(m ? m->explicitParent() : 0);
}

//---------------------------------------------------------
//   page
//---------------------------------------------------------

Page* Fermata::page() const
{
    System* s = system();
    return toPage(s ? s->explicitParent() : 0);
}

//---------------------------------------------------------
//   layout
//    height() and width() should return sensible
//    values when calling this method
//---------------------------------------------------------

void Fermata::layout()
{
    Segment* s = segment();
    setPos(PointF());
    if (!s) {            // for use in palette
        setOffset(0.0, 0.0);
        RectF b(symBbox(_symId));
        setbbox(b.translated(-0.5 * b.width(), 0.0));
        return;
    }

    if (isStyled(Pid::OFFSET)) {
        setOffset(propertyDefault(Pid::OFFSET).value<PointF>());
    }
    EngravingItem* e = s->element(track());
    if (e) {
        if (e->isChord()) {
            rxpos() += score()->noteHeadWidth() * staff()->staffMag(Fraction(0, 1)) * .5;
        } else {
            rxpos() += e->x() + e->width() * staff()->staffMag(Fraction(0, 1)) * .5;
        }
    }

    QString name = SymNames::nameForSymId(_symId).toQLatin1String();
    if (placeAbove()) {
        if (name.endsWith("Below")) {
            _symId = SymNames::symIdByName(name.left(name.size() - 5) + "Above");
        }
    } else {
        rypos() += staff()->height();
        if (name.endsWith("Above")) {
            _symId = SymNames::symIdByName(name.left(name.size() - 5) + "Below");
        }
    }
    RectF b(symBbox(_symId));
    setbbox(b.translated(-0.5 * b.width(), 0.0));
    autoplaceSegmentElement();
}

//---------------------------------------------------------
//   dragAnchorLines
//---------------------------------------------------------

std::vector<LineF> Fermata::dragAnchorLines() const
{
    std::vector<LineF> result;
    result.push_back(LineF(canvasPos(), parentItem()->canvasPos()));
    return result;
}

//---------------------------------------------------------
//   getProperty
//---------------------------------------------------------

PropertyValue Fermata::getProperty(Pid propertyId) const
{
    switch (propertyId) {
    case Pid::SYMBOL:
        return PropertyValue::fromValue(_symId);
    case Pid::TIME_STRETCH:
        return timeStretch();
    case Pid::PLAY:
        return play();
    default:
        return EngravingItem::getProperty(propertyId);
    }
}

//---------------------------------------------------------
//   setProperty
//---------------------------------------------------------

bool Fermata::setProperty(Pid propertyId, const PropertyValue& v)
{
    switch (propertyId) {
    case Pid::SYMBOL:
        setSymId(v.value<SymId>());
        break;
    case Pid::PLACEMENT: {
        PlacementV p = v.value<PlacementV>();
        if (p != placement()) {
            QString s = SymNames::nameForSymId(_symId).toQLatin1String();
            bool up = placeAbove();
            if (s.endsWith(up ? "Above" : "Below")) {
                QString s2 = s.left(s.size() - 5) + (up ? "Below" : "Above");
                _symId = SymNames::symIdByName(s2);
            }
            setPlacement(p);
        }
    }
    break;
    case Pid::PLAY:
        setPlay(v.toBool());
        break;
    case Pid::TIME_STRETCH:
        setTimeStretch(v.toDouble());
        score()->setUpTempoMap();
        break;
    default:
        return EngravingItem::setProperty(propertyId, v);
    }
    triggerLayout();
    return true;
}

//---------------------------------------------------------
//   propertyDefault
//---------------------------------------------------------

PropertyValue Fermata::propertyDefault(Pid propertyId) const
{
    switch (propertyId) {
    case Pid::PLACEMENT:
        return track() & 1 ? PlacementV::BELOW : PlacementV::ABOVE;
    case Pid::TIME_STRETCH:
        return 1.0;           // articulationList[int(articulationType())].timeStretch;
    case Pid::PLAY:
        return true;
    default:
        break;
    }
    return EngravingItem::propertyDefault(propertyId);
}

//---------------------------------------------------------
//   resetProperty
//---------------------------------------------------------

void Fermata::resetProperty(Pid id)
{
    switch (id) {
    case Pid::TIME_STRETCH:
        setProperty(id, propertyDefault(id));
        return;

    default:
        break;
    }
    EngravingItem::resetProperty(id);
}

//---------------------------------------------------------
//   getPropertyStyle
//---------------------------------------------------------

Sid Fermata::getPropertyStyle(Pid pid) const
{
    if (pid == Pid::OFFSET) {
        return placeAbove() ? Sid::fermataPosAbove : Sid::fermataPosBelow;
    }
    return EngravingObject::getPropertyStyle(pid);
}

//---------------------------------------------------------
//   mag
//---------------------------------------------------------

qreal Fermata::mag() const
{
    return staff() ? staff()->staffMag(tick()) * score()->styleD(Sid::articulationMag) : 1.0;
}

FermataType Fermata::fermataType() const
{
    static const std::unordered_map<SymId, FermataType> FERMATA_TYPES = {
        { SymId::fermataAbove, FermataType::Normal },
        { SymId::fermataBelow, FermataType::Normal },
        { SymId::fermataLongAbove, FermataType::Long },
        { SymId::fermataLongBelow, FermataType::Long },
        { SymId::fermataLongHenzeAbove, FermataType::LongHenze },
        { SymId::fermataLongHenzeBelow, FermataType::LongHenze },
        { SymId::fermataVeryLongAbove, FermataType::VeryLong },
        { SymId::fermataVeryLongBelow, FermataType::VeryLong },
        { SymId::fermataShortHenzeAbove, FermataType::ShortHenze },
        { SymId::fermataShortHenzeBelow, FermataType::ShortHenze },
        { SymId::fermataVeryShortAbove, FermataType::VeryShort },
        { SymId::fermataVeryShortBelow, FermataType::VeryShort },
        { SymId::fermataShortAbove, FermataType::Short },
        { SymId::fermataShortBelow, FermataType::Short },
    };

    auto search = FERMATA_TYPES.find(symId());

    if (search != FERMATA_TYPES.cend()) {
        return search->second;
    }

    return FermataType::Undefined;
}

//---------------------------------------------------------
//   accessibleInfo
//---------------------------------------------------------

String Fermata::accessibleInfo() const
{
    return String("%1: %2").arg(EngravingItem::accessibleInfo(), typeUserName());
}

void Fermata::added()
{
    IF_ASSERT_FAILED(score()) {
        return;
    }

    score()->setUpTempoMap();
}

void Fermata::removed()
{
    IF_ASSERT_FAILED(score()) {
        return;
    }

    score()->setUpTempoMap();
}
}
