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

#include "types/typesconv.h"
#include "layout/tlayout.h"
#include "iengravingfont.h"

#include "accidental.h"
#include "factory.h"
#include "score.h"
#include "staff.h"
#include "system.h"

#include "log.h"

using namespace mu;
using namespace mu::engraving;

namespace mu::engraving {
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
    TRACE_ITEM_DRAW;
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
    double x1 = 0;
    double x2 = pos2().x();
    double w   = x2 - x1;
    double mag = magS();
    IEngravingFontPtr f = score()->engravingFont();

    _symbols.clear();
    _symbols.push_back(start);
    double w1 = f->advance(start, mag);
    double w2 = f->advance(fill, mag);
    int n    = lrint((w - w1) / w2);
    for (int i = 0; i < n; ++i) {
        _symbols.push_back(fill);
    }
    RectF r(f->bbox(_symbols, mag));
    setbbox(r);
}

void TrillSegment::symbolLine(SymId start, SymId fill, SymId end)
{
    double x1 = 0;
    double x2 = pos2().x();
    double w   = x2 - x1;
    double mag = magS();
    IEngravingFontPtr f = score()->engravingFont();

    _symbols.clear();
    _symbols.push_back(start);
    double w1 = f->advance(start, mag);
    double w2 = f->advance(fill, mag);
    double w3 = f->advance(end, mag);
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
    LayoutContext ctx(score());
    v0::TLayout::layout(this, ctx);
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
//   scanElements
//---------------------------------------------------------

void TrillSegment::scanElements(void* data, void (* func)(void*, EngravingItem*), bool)
{
    func(data, this);
    if (isSingleType() || isBeginType()) {
        Accidental* a = trill()->accidental();
        if (a) {
            func(data, a);
        }
    }
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
    _trillType     = TrillType::TRILL_LINE;
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
    LayoutContext ctx(score());
    v0::TLayout::layout(this, ctx);
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
//   trillTypeName
//---------------------------------------------------------

String Trill::trillTypeUserName() const
{
    return TConv::translatedUserName(trillType());
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
        setTrillType(TrillType(val.toInt()));
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
//   accessibleInfo
//---------------------------------------------------------

String Trill::accessibleInfo() const
{
    return String(u"%1: %2").arg(EngravingItem::accessibleInfo(), trillTypeUserName());
}
}
