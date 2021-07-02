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
#include "score.h"
#include "instrtemplate.h"
#include "segment.h"
#include "staff.h"
#include "part.h"
#include "mscore.h"
#include "xml.h"
#include "draw/pen.h"

using namespace mu;
using namespace mu::draw;

namespace Ms {
//---------------------------------------------------------
//   StaffState
//---------------------------------------------------------

StaffState::StaffState(Score* score)
    : Element(score)
{
    _staffStateType = StaffStateType::INSTRUMENT;
    _instrument = new Instrument;
}

StaffState::StaffState(const StaffState& ss)
    : Element(ss)
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
    xml.stag(this);
    xml.tag("subtype", int(_staffStateType));
    if (staffStateType() == StaffStateType::INSTRUMENT) {
        _instrument->write(xml, nullptr);
    }
    Element::writeProperties(xml);
    xml.etag();
}

//---------------------------------------------------------
//   read
//---------------------------------------------------------

void StaffState::read(XmlReader& e)
{
    while (e.readNextStartElement()) {
        const QStringRef& tag(e.name());
        if (tag == "subtype") {
            _staffStateType = StaffStateType(e.readInt());
        } else if (tag == "Instrument") {
            _instrument->read(e, nullptr);
        } else if (!Element::readProperties(e)) {
            e.unknown();
        }
    }
}

//---------------------------------------------------------
//   draw
//---------------------------------------------------------

void StaffState::draw(mu::draw::Painter* painter) const
{
    TRACE_OBJ_DRAW;
    if (score()->printing() || !score()->showUnprintable()) {
        return;
    }
    Pen pen(selected() ? MScore::selectColor[0] : MScore::layoutBreakColor,
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
    qreal _spatium = spatium();
    path      = PainterPath();
    lw        = _spatium * 0.3;
    qreal h  = _spatium * 4;
    qreal w  = _spatium * 2.5;
//      qreal w1 = w * .6;

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
        qDebug("unknown layout break symbol");
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

void StaffState::setStaffStateType(const QString& s)
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

QString StaffState::staffStateTypeName() const
{
    switch (staffStateType()) {
    case StaffStateType::INSTRUMENT:
        return "instrument";
    case StaffStateType::TYPE:
        return "type";
    case StaffStateType::VISIBLE:
        return "visible";
    case StaffStateType::INVISIBLE:
        return "invisible";
    default:
        return "??";
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

Element* StaffState::drop(EditData& data)
{
    Element* e = data.dropElement;
    score()->undoChangeElement(this, e);
    return e;
}
}
