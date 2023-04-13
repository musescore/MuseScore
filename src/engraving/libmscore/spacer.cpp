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

#include "draw/types/pen.h"
#include "rw/xml.h"

#include "measure.h"
#include "score.h"

#include "log.h"

using namespace mu;
using namespace mu::draw;
using namespace mu::engraving;

namespace mu::engraving {
//---------------------------------------------------------
//   LayoutBreak
//---------------------------------------------------------

Spacer::Spacer(Measure* parent)
    : EngravingItem(ElementType::SPACER, parent)
{
    _spacerType = SpacerType::UP;
    _gap = 0.0;
}

Spacer::Spacer(const Spacer& s)
    : EngravingItem(s)
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
    TRACE_ITEM_DRAW;
    if (score()->printing() || !score()->showUnprintable()) {
        return;
    }
    Pen pen(selected() ? engravingConfiguration()->selectionColor() : engravingConfiguration()->formattingMarksColor(),
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
    double _spatium = spatium();

    path    = PainterPath();
    double w = _spatium;
    double b = w * .5;
    double h = explicitParent() ? _gap : std::min(_gap.val(), spatium() * 4.0);      // limit length for palette

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
    double lw = _spatium * 0.4;
    RectF bb(0, 0, w, h);
    bb.adjust(-lw, -lw, lw, lw);
    setbbox(bb);
}

//---------------------------------------------------------
//   setGap
//---------------------------------------------------------

void Spacer::setGap(Millimetre sp)
{
    _gap = sp;
    layout0();
}

//---------------------------------------------------------
//   spatiumChanged
//---------------------------------------------------------

void Spacer::spatiumChanged(double ov, double nv)
{
    _gap = (_gap / ov) * nv;
    layout0();
}

//---------------------------------------------------------
//   startEditDrag
//---------------------------------------------------------

void Spacer::startEditDrag(EditData& ed)
{
    ElementEditDataPtr eed = ed.getData(this);
    eed->pushProperty(Pid::SPACE);
}

//---------------------------------------------------------
//   editDrag
//---------------------------------------------------------

void Spacer::editDrag(EditData& ed)
{
    double s = ed.delta.y();

    switch (spacerType()) {
    case SpacerType::DOWN:
    case SpacerType::FIXED:
        _gap += s;
        break;
    case SpacerType::UP:
        _gap -= s;
        break;
    }
    if (_gap.val() < spatium() * 2.0) {
        _gap = Millimetre(spatium() * 2);
    }
    layout0();
    triggerLayout();
}

//---------------------------------------------------------
//   gripsPositions
//---------------------------------------------------------

std::vector<mu::PointF> Spacer::gripsPositions(const EditData&) const
{
    double _spatium = spatium();
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
    UNREACHABLE;
    xml.startElement(this);
    xml.tag("subtype", int(_spacerType));
    EngravingItem::writeProperties(xml);
    xml.tag("space", _gap.val() / spatium());
    xml.endElement();
}

//---------------------------------------------------------
//   getProperty
//---------------------------------------------------------

PropertyValue Spacer::getProperty(Pid propertyId) const
{
    switch (propertyId) {
    case Pid::SPACE:
        return gap();
    default:
        return EngravingItem::getProperty(propertyId);
    }
}

//---------------------------------------------------------
//   setProperty
//---------------------------------------------------------

bool Spacer::setProperty(Pid propertyId, const PropertyValue& v)
{
    switch (propertyId) {
    case Pid::SPACE:
        setGap(v.value<Millimetre>());
        break;
    default:
        if (!EngravingItem::setProperty(propertyId, v)) {
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

PropertyValue Spacer::propertyDefault(Pid id) const
{
    switch (id) {
    case Pid::SPACE:
        return Millimetre(0.0);
    default:
        return EngravingItem::propertyDefault(id);
    }
}
}
