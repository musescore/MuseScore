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

#include "iengravingfont.h"

#include "accidental.h"
#include "chord.h"
#include "factory.h"
#include "note.h"
#include "ornament.h"
#include "score.h"
#include "staff.h"
#include "system.h"
#include "undo.h"

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
    double x1 = 0.0;
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
    double x1 = 0.0;
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
//   shape
//---------------------------------------------------------

Shape TrillSegment::shape() const
{
    IEngravingFontPtr font = score()->engravingFont();
    Shape s = font->shape(_symbols, _mag);
    Accidental* accidental = trill()->accidental();
    if (accidental && accidental->visible() && isSingleBeginType()) {
        s.add(accidental->shape().translate(accidental->pos()));
    }
    return s;
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
        Chord* cueNoteChord = trill()->cueNoteChord();
        if (cueNoteChord) {
            cueNoteChord->scanElements(data, func);
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
    _ornament = nullptr;
    _accidental = nullptr;
    _cueNoteChord = nullptr;
    _ornamentStyle = OrnamentStyle::DEFAULT;
    setPlayArticulation(true);
    initElementStyle(&trillStyle);
}

Trill::Trill(const Trill& t)
    : SLine(t)
{
    _trillType = t._trillType;
    _ornament = t._ornament ? t._ornament->clone() : nullptr;
    _ornamentStyle = t._ornamentStyle;
    _playArticulation = t._playArticulation;
    initElementStyle(&trillStyle);
}

EngravingItem* Trill::linkedClone()
{
    Trill* linkedTrill = clone();
    Ornament* linkedOrnament = toOrnament(_ornament->linkedClone());
    linkedTrill->setOrnament(linkedOrnament);
    linkedTrill->setAutoplace(true);
    score()->undo(new Link(linkedTrill, this));
    return linkedTrill;
}

Trill::~Trill()
{
    delete _ornament;
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

void Trill::setTrack(track_idx_t n)
{
    EngravingItem::setTrack(n);

    for (SpannerSegment* ss : spannerSegments()) {
        ss->setTrack(n);
    }

    if (_ornament) {
        _ornament->setTrack(n);
    }
}

void Trill::setTrillType(TrillType tt)
{
    _trillType = tt;
    if (!_ornament) {
        // ornament parent will be explicitely set at layout stage
        _ornament = Factory::createOrnament((ChordRest*)score()->dummy()->chord());
    }
    _ornament->setTrack(track());
    _ornament->setSymId(Ornament::fromTrillType(tt));
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
