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

#include "spacer.h"
#include "score.h"
#include "staff.h"
#include "mscore.h"
#include "xml.h"
#include "measure.h"
#include "draw/pen.h"

using namespace mu;
using namespace mu::draw;

namespace Ms {
//---------------------------------------------------------
//   LayoutBreak
//---------------------------------------------------------

Spacer::Spacer(Score* score)
    : Element(score)
{
    _spacerType = SpacerType::UP;
    _gap = 0.0;
}

Spacer::Spacer(const Spacer& s)
    : Element(s)
{
    _gap        = s._gap;
    path        = s.path;
    _spacerType = s._spacerType;
}

//---------------------------------------------------------
//   draw
//---------------------------------------------------------

void Spacer::draw(mu::draw::Painter* painter) const
{
    TRACE_OBJ_DRAW;
    if (score()->printing() || !score()->showUnprintable()) {
        return;
    }
    Pen pen(selected() ? MScore::selectColor[0] : MScore::layoutBreakColor,
            spatium() * 0.3);
    painter->setPen(pen);
    painter->setBrush(BrushStyle::NoBrush);
    painter->drawPath(path);
}

//---------------------------------------------------------
//   layout0
//---------------------------------------------------------

void Spacer::layout0()
{
    qreal _spatium = spatium();

    path    = PainterPath();
    qreal w = _spatium;
    qreal b = w * .5;
    qreal h = parent() ? _gap : qMin(_gap, spatium() * 4.0);      // limit length for palette

    switch (spacerType()) {
    case SpacerType::DOWN:
        path.lineTo(w, 0.0);
        path.moveTo(b, 0.0);
        path.lineTo(b, h);
        path.lineTo(0.0, h - b);
        path.moveTo(b, h);
        path.lineTo(w, h - b);
        break;
    case SpacerType::UP:
        path.moveTo(b, 0.0);
        path.lineTo(0.0, b);
        path.moveTo(b, 0.0);
        path.lineTo(w, b);
        path.moveTo(b, 0.0);
        path.lineTo(b, h);
        path.moveTo(0.0, h);
        path.lineTo(w, h);
        break;
    case SpacerType::FIXED:
        path.lineTo(w, 0.0);
        path.moveTo(b, 0.0);
        path.lineTo(b, h);
        path.moveTo(0.0, h);
        path.lineTo(w, h);
        break;
    }
    qreal lw = _spatium * 0.4;
    RectF bb(0, 0, w, h);
    bb.adjust(-lw, -lw, lw, lw);
    setbbox(bb);
}

//---------------------------------------------------------
//   setGap
//---------------------------------------------------------

void Spacer::setGap(qreal sp)
{
    _gap = sp;
    layout0();
}

//---------------------------------------------------------
//   spatiumChanged
//---------------------------------------------------------

void Spacer::spatiumChanged(qreal ov, qreal nv)
{
    _gap = (_gap / ov) * nv;
    layout0();
}

//---------------------------------------------------------
//   startEditDrag
//---------------------------------------------------------

void Spacer::startEditDrag(EditData& ed)
{
    ElementEditData* eed = ed.getData(this);
    eed->pushProperty(Pid::SPACE);
}

//---------------------------------------------------------
//   editDrag
//---------------------------------------------------------

void Spacer::editDrag(EditData& ed)
{
    qreal s = ed.delta.y();

    switch (spacerType()) {
    case SpacerType::DOWN:
    case SpacerType::FIXED:
        _gap += s;
        break;
    case SpacerType::UP:
        _gap -= s;
        break;
    }
    if (_gap < spatium() * 2.0) {
        _gap = spatium() * 2;
    }
    layout0();
    triggerLayout();
}

//---------------------------------------------------------
//   gripsPositions
//---------------------------------------------------------

std::vector<mu::PointF> Spacer::gripsPositions(const EditData&) const
{
    qreal _spatium = spatium();
    PointF p;
    switch (spacerType()) {
    case SpacerType::DOWN:
    case SpacerType::FIXED:
        p = PointF(_spatium * .5, _gap);
        break;
    case SpacerType::UP:
        p = PointF(_spatium * .5, 0.0);
        break;
    }
    return { pagePos() + p };
}

//---------------------------------------------------------
//   write
//---------------------------------------------------------

void Spacer::write(XmlWriter& xml) const
{
    xml.stag(this);
    xml.tag("subtype", int(_spacerType));
    Element::writeProperties(xml);
    xml.tag("space", _gap / spatium());
    xml.etag();
}

//---------------------------------------------------------
//   read
//---------------------------------------------------------

void Spacer::read(XmlReader& e)
{
    while (e.readNextStartElement()) {
        const QStringRef& tag(e.name());
        if (tag == "subtype") {
            _spacerType = SpacerType(e.readInt());
        } else if (tag == "space") {
            _gap = e.readDouble() * spatium();
        } else if (!Element::readProperties(e)) {
            e.unknown();
        }
    }
    layout0();
}

//---------------------------------------------------------
//   getProperty
//---------------------------------------------------------

QVariant Spacer::getProperty(Pid propertyId) const
{
    switch (propertyId) {
    case Pid::SPACE:
        return gap();
    default:
        return Element::getProperty(propertyId);
    }
}

//---------------------------------------------------------
//   setProperty
//---------------------------------------------------------

bool Spacer::setProperty(Pid propertyId, const QVariant& v)
{
    switch (propertyId) {
    case Pid::SPACE:
        setGap(v.toDouble());
        break;
    default:
        if (!Element::setProperty(propertyId, v)) {
            return false;
        }
        break;
    }
    layout0();
    triggerLayout();
    setGenerated(false);
    return true;
}

//---------------------------------------------------------
//   propertyDefault
//---------------------------------------------------------

QVariant Spacer::propertyDefault(Pid id) const
{
    switch (id) {
    case Pid::SPACE:
        return QVariant(0.0);
    default:
        return Element::propertyDefault(id);
    }
}

//---------------------------------------------------------
//   scanElements
//---------------------------------------------------------

void Spacer::scanElements(void* data, void (* func)(void*, Element*), bool all)
{
    if (all || (measure()->visible(staffIdx()) && score()->staff(staffIdx())->show())) {
        func(data, this);
    }
}
}
