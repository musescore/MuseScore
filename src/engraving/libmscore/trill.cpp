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

#include "trill.h"

#include <cmath>

#include "translation.h"
#include "style/style.h"
#include "rw/xml.h"

#include "factory.h"
#include "system.h"
#include "measure.h"
#include "utils.h"
#include "score.h"
#include "scorefont.h"
#include "accidental.h"
#include "segment.h"
#include "staff.h"

using namespace mu;
using namespace mu::engraving;

namespace Ms {
//---------------------------------------------------------
//   trillTable
//    must be in sync with Trill::Type
//---------------------------------------------------------

const std::vector<TrillTableItem> trillTable = {
    { Trill::Type::TRILL_LINE,      "trill",      QT_TRANSLATE_NOOP("trillType", "Trill line") },
    { Trill::Type::UPPRALL_LINE,    "upprall",    QT_TRANSLATE_NOOP("trillType", "Upprall line") },
    { Trill::Type::DOWNPRALL_LINE,  "downprall",  QT_TRANSLATE_NOOP("trillType", "Downprall line") },
    { Trill::Type::PRALLPRALL_LINE, "prallprall", QT_TRANSLATE_NOOP("trillType", "Prallprall line") }
};

//---------------------------------------------------------
//   trillStyle
//---------------------------------------------------------

static const ElementStyle trillStyle {
    { Sid::trillPlacement, Pid::PLACEMENT },
    { Sid::trillPosAbove,  Pid::OFFSET },
};

TrillSegment::TrillSegment(Trill* sp, System* parent)
    : LineSegment(ElementType::TRILL_SEGMENT, sp, parent, ElementFlag::MOVABLE | ElementFlag::ON_STAFF)
{
}

TrillSegment::TrillSegment(System* parent)
    : LineSegment(ElementType::TRILL_SEGMENT, parent, ElementFlag::MOVABLE | ElementFlag::ON_STAFF)
{
}

//---------------------------------------------------------
//   draw
//---------------------------------------------------------

void TrillSegment::draw(mu::draw::Painter* painter) const
{
    TRACE_OBJ_DRAW;
    painter->setPen(spanner()->curColor());
    drawSymbols(_symbols, painter);
}

//---------------------------------------------------------
//   add
//---------------------------------------------------------

void TrillSegment::add(EngravingItem* e)
{
    e->setParent(this);
    if (e->type() == ElementType::ACCIDENTAL) {
        // accidental is part of trill
        trill()->setAccidental(toAccidental(e));
        e->added();
    }
}

//---------------------------------------------------------
//   remove
//---------------------------------------------------------

void TrillSegment::remove(EngravingItem* e)
{
    if (trill()->accidental() == e) {
        // accidental is part of trill
        trill()->setAccidental(nullptr);
        e->removed();
    }
}

//---------------------------------------------------------
//   symbolLine
//---------------------------------------------------------

void TrillSegment::symbolLine(SymId start, SymId fill)
{
    qreal x1 = 0;
    qreal x2 = pos2().x();
    qreal w   = x2 - x1;
    qreal mag = magS();
    ScoreFont* f = score()->scoreFont();

    _symbols.clear();
    _symbols.push_back(start);
    qreal w1 = f->advance(start, mag);
    qreal w2 = f->advance(fill, mag);
    int n    = lrint((w - w1) / w2);
    for (int i = 0; i < n; ++i) {
        _symbols.push_back(fill);
    }
    RectF r(f->bbox(_symbols, mag));
    setbbox(r);
}

void TrillSegment::symbolLine(SymId start, SymId fill, SymId end)
{
    qreal x1 = 0;
    qreal x2 = pos2().x();
    qreal w   = x2 - x1;
    qreal mag = magS();
    ScoreFont* f = score()->scoreFont();

    _symbols.clear();
    _symbols.push_back(start);
    qreal w1 = f->advance(start, mag);
    qreal w2 = f->advance(fill, mag);
    qreal w3 = f->advance(end, mag);
    int n    = lrint((w - w1 - w3) / w2);
    for (int i = 0; i < n; ++i) {
        _symbols.push_back(fill);
    }
    _symbols.push_back(end);
    RectF r(f->bbox(_symbols, mag));
    setbbox(r);
}

//---------------------------------------------------------
//   layout
//---------------------------------------------------------

void TrillSegment::layout()
{
    if (staff()) {
        setMag(staff()->staffMag(tick()));
    }
    if (spanner()->placeBelow()) {
        rypos() = staff() ? staff()->height() : 0.0;
    }

    if (isSingleType() || isBeginType()) {
        Accidental* a = trill()->accidental();
        if (a) {
            a->layout();
            a->setMag(a->mag() * .6);
            qreal _spatium = spatium();
            a->setPos(_spatium * 1.3, -2.2 * _spatium);
            a->setParent(this);
        }
        switch (trill()->trillType()) {
        case Trill::Type::TRILL_LINE:
            symbolLine(SymId::ornamentTrill, SymId::wiggleTrill);
            break;
        case Trill::Type::PRALLPRALL_LINE:
            symbolLine(SymId::wiggleTrill, SymId::wiggleTrill);
            break;
        case Trill::Type::UPPRALL_LINE:
            symbolLine(SymId::ornamentBottomLeftConcaveStroke,
                       SymId::ornamentZigZagLineNoRightEnd, SymId::ornamentZigZagLineWithRightEnd);
            break;
        case Trill::Type::DOWNPRALL_LINE:
            symbolLine(SymId::ornamentLeftVerticalStroke,
                       SymId::ornamentZigZagLineNoRightEnd, SymId::ornamentZigZagLineWithRightEnd);
            break;
        }
    } else {
        symbolLine(SymId::wiggleTrill, SymId::wiggleTrill);
    }
    if (isStyled(Pid::OFFSET)) {
        roffset() = trill()->propertyDefault(Pid::OFFSET).value<PointF>();
    }

    autoplaceSpannerSegment();
}

//---------------------------------------------------------
//   shape
//---------------------------------------------------------

Shape TrillSegment::shape() const
{
    return Shape(bbox());
}

//---------------------------------------------------------
//   acceptDrop
//---------------------------------------------------------

bool TrillSegment::acceptDrop(EditData& data) const
{
    if (data.dropElement->isAccidental()) {
        return true;
    }
    return false;
}

//---------------------------------------------------------
//   drop
//---------------------------------------------------------

EngravingItem* TrillSegment::drop(EditData& data)
{
    EngravingItem* e = data.dropElement;
    switch (e->type()) {
    case ElementType::ACCIDENTAL:
        e->setParent(trill());
        score()->undoAddElement(e);
        break;

    default:
        delete e;
        e = 0;
        break;
    }
    return e;
}

//---------------------------------------------------------
//   propertyDelegate
//---------------------------------------------------------

EngravingItem* TrillSegment::propertyDelegate(Pid pid)
{
    if (pid == Pid::TRILL_TYPE || pid == Pid::ORNAMENT_STYLE || pid == Pid::PLACEMENT || pid == Pid::PLAY) {
        return spanner();
    }
    return LineSegment::propertyDelegate(pid);
}

//---------------------------------------------------------
//   getPropertyStyle
//---------------------------------------------------------

Sid TrillSegment::getPropertyStyle(Pid pid) const
{
    if (pid == Pid::OFFSET) {
        return spanner()->placeAbove() ? Sid::trillPosAbove : Sid::trillPosBelow;
    }
    return LineSegment::getPropertyStyle(pid);
}

Sid Trill::getPropertyStyle(Pid pid) const
{
    if (pid == Pid::OFFSET) {
        return placeAbove() ? Sid::trillPosAbove : Sid::trillPosBelow;
    }
    return SLine::getPropertyStyle(pid);
}

//---------------------------------------------------------
//   Trill
//---------------------------------------------------------

Trill::Trill(EngravingItem* parent)
    : SLine(ElementType::TRILL, parent)
{
    _trillType     = Type::TRILL_LINE;
    _accidental    = 0;
    _ornamentStyle = OrnamentStyle::DEFAULT;
    setPlayArticulation(true);
    initElementStyle(&trillStyle);
}

Trill::Trill(const Trill& t)
    : SLine(t)
{
    _trillType = t._trillType;
    _accidental = t._accidental ? t._accidental->clone() : nullptr;
    _ornamentStyle = t._ornamentStyle;
    _playArticulation = t._playArticulation;
    initElementStyle(&trillStyle);
}

Trill::~Trill()
{
    delete _accidental;
}

//---------------------------------------------------------
//   add
//---------------------------------------------------------

void Trill::add(EngravingItem* e)
{
    if (e->type() == ElementType::ACCIDENTAL) {
        e->setParent(this);
        _accidental = toAccidental(e);
        e->added();
    } else {
        SLine::add(e);
    }
}

//---------------------------------------------------------
//   remove
//---------------------------------------------------------

void Trill::remove(EngravingItem* e)
{
    if (e == _accidental) {
        _accidental = nullptr;
        e->removed();
    }
}

//---------------------------------------------------------
//   layout
//---------------------------------------------------------

void Trill::layout()
{
    SLine::layout();
    if (score()->isPaletteScore()) {
        return;
    }
    if (spannerSegments().empty()) {
        return;
    }
    TrillSegment* ls = toTrillSegment(frontSegment());
    if (spannerSegments().empty()) {
        qDebug("Trill: no segments");
    }
    if (_accidental) {
        _accidental->setParent(ls);
    }
}

//---------------------------------------------------------
//   createLineSegment
//---------------------------------------------------------

static const ElementStyle trillSegmentStyle {
    { Sid::trillPosAbove, Pid::OFFSET },
    { Sid::trillMinDistance, Pid::MIN_DISTANCE },
};

LineSegment* Trill::createLineSegment(System* parent)
{
    TrillSegment* seg = new TrillSegment(this, parent);
    seg->setTrack(track());
    seg->setColor(color());
    seg->initElementStyle(&trillSegmentStyle);
    return seg;
}

//---------------------------------------------------------
//   Trill::write
//---------------------------------------------------------

void Trill::write(XmlWriter& xml) const
{
    if (!xml.canWrite(this)) {
        return;
    }
    xml.startObject(this);
    xml.tag("subtype", trillTypeName());
    writeProperty(xml, Pid::PLAY);
    writeProperty(xml, Pid::ORNAMENT_STYLE);
    writeProperty(xml, Pid::PLACEMENT);
    SLine::writeProperties(xml);
    if (_accidental) {
        _accidental->write(xml);
    }
    xml.endObject();
}

//---------------------------------------------------------
//   Trill::read
//---------------------------------------------------------

void Trill::read(XmlReader& e)
{
    eraseSpannerSegments();

    while (e.readNextStartElement()) {
        const QStringRef& tag(e.name());
        if (tag == "subtype") {
            setTrillType(e.readElementText());
        } else if (tag == "Accidental") {
            _accidental = Factory::createAccidental(this);
            _accidental->read(e);
            _accidental->setParent(this);
        } else if (tag == "ornamentStyle") {
            readProperty(e, Pid::ORNAMENT_STYLE);
        } else if (tag == "play") {
            setPlayArticulation(e.readBool());
        } else if (!SLine::readProperties(e)) {
            e.unknown();
        }
    }
}

//---------------------------------------------------------
//   setTrillType
//---------------------------------------------------------

void Trill::setTrillType(const QString& s)
{
    if (s == "0") {
        _trillType = Type::TRILL_LINE;
        return;
    }
    if (s == "pure") {
        _trillType = Type::PRALLPRALL_LINE;     // obsolete, compatibility only
        return;
    }
    for (TrillTableItem i : trillTable) {
        if (s.compare(i.name) == 0) {
            _trillType = i.type;
            return;
        }
    }
    qDebug("Trill::setSubtype: unknown <%s>", qPrintable(s));
}

//---------------------------------------------------------
//   type2name
//---------------------------------------------------------

QString Trill::type2name(Trill::Type t)
{
    for (TrillTableItem i : trillTable) {
        if (i.type == t) {
            return i.name;
        }
    }
    qDebug("unknown Trill subtype %d", int(t));
    return "?";
}

//---------------------------------------------------------
//   trillTypeName
//---------------------------------------------------------

QString Trill::trillTypeName() const
{
    return type2name(trillType());
}

//---------------------------------------------------------
//   trillTypeName
//---------------------------------------------------------

QString Trill::trillTypeUserName() const
{
    return qtrc("trillType", trillTable[static_cast<int>(trillType())].userName.toUtf8().constData());
}

//---------------------------------------------------------
//   getProperty
//---------------------------------------------------------

PropertyValue Trill::getProperty(Pid propertyId) const
{
    switch (propertyId) {
    case Pid::TRILL_TYPE:
        return int(trillType());
    case Pid::ORNAMENT_STYLE:
        return ornamentStyle();
    case Pid::PLAY:
        return bool(playArticulation());
    default:
        break;
    }
    return SLine::getProperty(propertyId);
}

//---------------------------------------------------------
//   setProperty
//---------------------------------------------------------

bool Trill::setProperty(Pid propertyId, const PropertyValue& val)
{
    switch (propertyId) {
    case Pid::TRILL_TYPE:
        setTrillType(Type(val.toInt()));
        break;
    case Pid::PLAY:
        setPlayArticulation(val.toBool());
        break;
    case Pid::ORNAMENT_STYLE:
        setOrnamentStyle(val.value<OrnamentStyle>());
        break;
    case Pid::COLOR:
        setColor(val.value<Color>());
        [[fallthrough]];
    default:
        if (!SLine::setProperty(propertyId, val)) {
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

PropertyValue Trill::propertyDefault(Pid propertyId) const
{
    switch (propertyId) {
    case Pid::TRILL_TYPE:
        return 0;
    case Pid::ORNAMENT_STYLE:
        return OrnamentStyle::DEFAULT;
    case Pid::PLAY:
        return true;
    case Pid::PLACEMENT:
        return score()->styleV(Sid::trillPlacement);

    default:
        return SLine::propertyDefault(propertyId);
    }
}

//---------------------------------------------------------
//   propertyId
//---------------------------------------------------------

Pid Trill::propertyId(const QStringRef& name) const
{
    if (name == "subtype") {
        return Pid::TRILL_TYPE;
    }
    return SLine::propertyId(name);
}

//---------------------------------------------------------
//   accessibleInfo
//---------------------------------------------------------

QString Trill::accessibleInfo() const
{
    return QString("%1: %2").arg(EngravingItem::accessibleInfo(), trillTypeUserName());
}
}
