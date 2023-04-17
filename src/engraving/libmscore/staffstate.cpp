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

#include "staffstate.h"

#include "draw/types/pen.h"
#include "rw/xml.h"

#include "part.h"
#include "score.h"

#include "log.h"

using namespace mu;
using namespace mu::draw;

namespace mu::engraving {
//---------------------------------------------------------
//   StaffState
//---------------------------------------------------------

StaffState::StaffState(EngravingItem* parent)
    : EngravingItem(ElementType::STAFF_STATE, parent)
{
    _staffStateType = StaffStateType::INSTRUMENT;
    _instrument = new Instrument;
}

StaffState::StaffState(const StaffState& ss)
    : EngravingItem(ss)
{
    _instrument = new Instrument(*ss._instrument);
}

StaffState::~StaffState()
{
    delete _instrument;
}

//---------------------------------------------------------
//   write
//---------------------------------------------------------

void StaffState::write(XmlWriter& xml) const
{
    xml.startElement(this);
    xml.tag("subtype", int(_staffStateType));
    if (staffStateType() == StaffStateType::INSTRUMENT) {
        _instrument->write(xml, nullptr);
    }
    EngravingItem::writeProperties(xml);
    xml.endElement();
}

//---------------------------------------------------------
//   draw
//---------------------------------------------------------

void StaffState::draw(mu::draw::Painter* painter) const
{
    TRACE_ITEM_DRAW;
    if (score()->printing() || !score()->showUnprintable()) {
        return;
    }
    Pen pen(selected() ? engravingConfiguration()->selectionColor() : engravingConfiguration()->formattingMarksColor(),
            lw, PenStyle::SolidLine, PenCapStyle::RoundCap, PenJoinStyle::RoundJoin);
    painter->setPen(pen);
    painter->setBrush(BrushStyle::NoBrush);
    painter->drawPath(path);
}

//---------------------------------------------------------
//   layout
//---------------------------------------------------------

void StaffState::layout()
{
    double _spatium = spatium();
    path      = PainterPath();
    lw        = _spatium * 0.3;
    double h  = _spatium * 4;
    double w  = _spatium * 2.5;
//      double w1 = w * .6;

    switch (staffStateType()) {
    case StaffStateType::INSTRUMENT:
        path.lineTo(w, 0.0);
        path.lineTo(w, h);
        path.lineTo(0.0, h);
        path.lineTo(0.0, 0.0);
        path.moveTo(w * .5, h - _spatium * .5);
        path.lineTo(w * .5, _spatium * 2);
        path.moveTo(w * .5, _spatium * .8);
        path.lineTo(w * .5, _spatium * 1.0);
        break;

    case StaffStateType::TYPE:
        path.lineTo(w, 0.0);
        path.lineTo(w, h);
        path.lineTo(0.0, h);
        path.lineTo(0.0, 0.0);
        break;

    case StaffStateType::VISIBLE:
        path.lineTo(w, 0.0);
        path.lineTo(w, h);
        path.lineTo(0.0, h);
        path.lineTo(0.0, 0.0);
        break;

    case StaffStateType::INVISIBLE:
        path.lineTo(w, 0.0);
        path.lineTo(w, h);
        path.lineTo(0.0, h);
        path.lineTo(0.0, 0.0);
        break;

    default:
        LOGD("unknown layout break symbol");
        break;
    }
    RectF bb(0, 0, w, h);
    bb.adjust(-lw, -lw, lw, lw);
    setbbox(bb);
    setPos(0.0, _spatium * -6.0);
}

//---------------------------------------------------------
//   setStaffStateType
//---------------------------------------------------------

void StaffState::setStaffStateType(const String& s)
{
    if (s == "instrument") {
        setStaffStateType(StaffStateType::INSTRUMENT);
    } else if (s == "type") {
        setStaffStateType(StaffStateType::TYPE);
    } else if (s == "visible") {
        setStaffStateType(StaffStateType::VISIBLE);
    } else if (s == "invisible") {
        setStaffStateType(StaffStateType::INVISIBLE);
    }
}

//---------------------------------------------------------
//   staffStateTypeName
//---------------------------------------------------------

String StaffState::staffStateTypeName() const
{
    switch (staffStateType()) {
    case StaffStateType::INSTRUMENT:
        return u"instrument";
    case StaffStateType::TYPE:
        return u"type";
    case StaffStateType::VISIBLE:
        return u"visible";
    case StaffStateType::INVISIBLE:
        return u"invisible";
    default:
        return u"??";
    }
}

//---------------------------------------------------------
//   acceptDrop
//---------------------------------------------------------

bool StaffState::acceptDrop(EditData&) const
{
    return false;
}

//---------------------------------------------------------
//   drop
//---------------------------------------------------------

EngravingItem* StaffState::drop(EditData& data)
{
    EngravingItem* e = data.dropElement;
    score()->undoChangeElement(this, e);
    return e;
}
}
