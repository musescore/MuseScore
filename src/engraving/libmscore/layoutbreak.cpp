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

#include "layoutbreak.h"
#include "score.h"
#include "mscore.h"
#include "xml.h"

using namespace mu;
using namespace mu::draw;

namespace Ms {
//---------------------------------------------------------
//   sectionBreakStyle
//---------------------------------------------------------

static const ElementStyle sectionBreakStyle {
    { Sid::SectionPause, Pid::PAUSE }
};

//---------------------------------------------------------
//   LayoutBreak
//---------------------------------------------------------

LayoutBreak::LayoutBreak(Score* score)
    : Element(score, ElementFlag::SYSTEM | ElementFlag::HAS_TAG)
{
    _pause = 0.;
    _startWithLongNames = false;
    _startWithMeasureOne = false;
    _firstSystemIdentation = false;
    _layoutBreakType = Type(propertyDefault(Pid::LAYOUT_BREAK).toInt());

    initElementStyle(&sectionBreakStyle);

    resetProperty(Pid::PAUSE);
    resetProperty(Pid::START_WITH_LONG_NAMES);
    resetProperty(Pid::START_WITH_MEASURE_ONE);
    resetProperty(Pid::FIRST_SYSTEM_INDENTATION);
    lw = spatium() * 0.3;
}

LayoutBreak::LayoutBreak(const LayoutBreak& lb)
    : Element(lb)
{
    _layoutBreakType       = lb._layoutBreakType;
    lw                     = lb.lw;
    _pause                 = lb._pause;
    _startWithLongNames    = lb._startWithLongNames;
    _startWithMeasureOne   = lb._startWithMeasureOne;
    _firstSystemIdentation = lb._firstSystemIdentation;
    layout0();
}

//---------------------------------------------------------
//   write
//---------------------------------------------------------

void LayoutBreak::write(XmlWriter& xml) const
{
    xml.stag(this);
    Element::writeProperties(xml);

    for (auto id :
         { Pid::LAYOUT_BREAK, Pid::PAUSE, Pid::START_WITH_LONG_NAMES, Pid::START_WITH_MEASURE_ONE, Pid::FIRST_SYSTEM_INDENTATION }) {
        writeProperty(xml, id);
    }

    xml.etag();
}

//---------------------------------------------------------
//   read
//---------------------------------------------------------

void LayoutBreak::read(XmlReader& e)
{
    while (e.readNextStartElement()) {
        const QStringRef& tag(e.name());
        if (tag == "subtype") {
            readProperty(e, Pid::LAYOUT_BREAK);
        } else if (tag == "pause") {
            readProperty(e, Pid::PAUSE);
        } else if (tag == "startWithLongNames") {
            readProperty(e, Pid::START_WITH_LONG_NAMES);
        } else if (tag == "startWithMeasureOne") {
            readProperty(e, Pid::START_WITH_MEASURE_ONE);
        } else if (tag == "firstSystemIdentation") {
            readProperty(e, Pid::FIRST_SYSTEM_INDENTATION);
        } else if (!Element::readProperties(e)) {
            e.unknown();
        }
    }
    layout0();
}

//---------------------------------------------------------
//   draw
//---------------------------------------------------------

void LayoutBreak::draw(mu::draw::Painter* painter) const
{
    TRACE_OBJ_DRAW;
    using namespace mu::draw;
    if (score()->printing() || !score()->showUnprintable()) {
        return;
    }

    Pen pen;
    pen.setColor(selected() ? MScore::selectColor[0] : MScore::layoutBreakColor);
    pen.setWidthF(lw / 2);
    pen.setJoinStyle(PenJoinStyle::MiterJoin);
    pen.setCapStyle(PenCapStyle::SquareCap);
    pen.setDashPattern({ 1, 3 });

    painter->setPen(pen);
    painter->setBrush(BrushStyle::NoBrush);
    painter->drawRect(m_iconBorderRect);

    pen.setWidthF(lw);
    pen.setStyle(PenStyle::SolidLine);

    painter->setPen(pen);
    painter->drawPath(m_iconPath);
}

//---------------------------------------------------------
//   layout0
//---------------------------------------------------------

void LayoutBreak::layout0()
{
    qreal _spatium = spatium();
    qreal w = _spatium * 2.5;
    qreal h = w;

    m_iconBorderRect = RectF(0.0, 0.0, w, h);
    m_iconPath = PainterPath();

    switch (layoutBreakType()) {
    case Type::LINE:
        m_iconPath.moveTo(w * .8, h * .3);
        m_iconPath.lineTo(w * .8, h * .6);
        m_iconPath.lineTo(w * .3, h * .6);

        m_iconPath.moveTo(w * .4, h * .5);
        m_iconPath.lineTo(w * .25, h * .6);
        m_iconPath.lineTo(w * .4, h * .7);
        m_iconPath.lineTo(w * .4, h * .5);
        break;

    case Type::PAGE:
        m_iconPath.moveTo(w * .25, h * .2);
        m_iconPath.lineTo(w * .60, h * .2);
        m_iconPath.lineTo(w * .75, h * .35);
        m_iconPath.lineTo(w * .75, h * .8);
        m_iconPath.lineTo(w * .25, h * .8);
        m_iconPath.lineTo(w * .25, h * .2);

        m_iconPath.moveTo(w * .55, h * .21); // 0.01 to avoid overlap
        m_iconPath.lineTo(w * .55, h * .40);
        m_iconPath.lineTo(w * .74, h * .40);
        break;

    case Type::SECTION:
        m_iconPath.moveTo(w * .25, h * .2);
        m_iconPath.lineTo(w * .75, h * .2);
        m_iconPath.lineTo(w * .75, h * .8);
        m_iconPath.lineTo(w * .25, h * .8);

        m_iconPath.moveTo(w * .55, h * .21); // 0.01 to avoid overlap
        m_iconPath.lineTo(w * .55, h * .79);
        break;

    case Type::NOBREAK:
        m_iconPath.moveTo(w * .1,  h * .5);
        m_iconPath.lineTo(w * .9,  h * .5);

        m_iconPath.moveTo(w * .7, h * .3);
        m_iconPath.lineTo(w * .5, h * .5);
        m_iconPath.lineTo(w * .7, h * .7);
        m_iconPath.lineTo(w * .7, h * .3);

        m_iconPath.moveTo(w * .3,  h * .3);
        m_iconPath.lineTo(w * .5,  h * .5);
        m_iconPath.lineTo(w * .3,  h * .7);
        m_iconPath.lineTo(w * .3,  h * .3);
        break;

    default:
        qDebug("unknown layout break symbol");
        break;
    }

    setbbox(m_iconBorderRect.adjusted(-lw, -lw, lw, lw));
}

//---------------------------------------------------------
//   setLayoutBreakType
//---------------------------------------------------------

void LayoutBreak::setLayoutBreakType(Type val)
{
    _layoutBreakType = val;
    layout0();
}

//---------------------------------------------------------
//   spatiumChanged
//---------------------------------------------------------

void LayoutBreak::spatiumChanged(qreal, qreal)
{
    lw = spatium() * 0.3;
    layout0();
}

//---------------------------------------------------------
//   acceptDrop
//---------------------------------------------------------

bool LayoutBreak::acceptDrop(EditData& data) const
{
    return data.dropElement->type() == ElementType::LAYOUT_BREAK
           && toLayoutBreak(data.dropElement)->layoutBreakType() != layoutBreakType();
}

//---------------------------------------------------------
//   drop
//---------------------------------------------------------

Element* LayoutBreak::drop(EditData& data)
{
    Element* e = data.dropElement;
    score()->undoChangeElement(this, e);
    return e;
}

//---------------------------------------------------------
//   getProperty
//---------------------------------------------------------

QVariant LayoutBreak::getProperty(Pid propertyId) const
{
    switch (propertyId) {
    case Pid::LAYOUT_BREAK:
        return int(_layoutBreakType);
    case Pid::PAUSE:
        return _pause;
    case Pid::START_WITH_LONG_NAMES:
        return _startWithLongNames;
    case Pid::START_WITH_MEASURE_ONE:
        return _startWithMeasureOne;
    case Pid::FIRST_SYSTEM_INDENTATION:
        return _firstSystemIdentation;
    default:
        return Element::getProperty(propertyId);
    }
}

//---------------------------------------------------------
//   setProperty
//---------------------------------------------------------

bool LayoutBreak::setProperty(Pid propertyId, const QVariant& v)
{
    switch (propertyId) {
    case Pid::LAYOUT_BREAK:
        setLayoutBreakType(Type(v.toInt()));
        break;
    case Pid::PAUSE:
        setPause(v.toDouble());
        break;
    case Pid::START_WITH_LONG_NAMES:
        setStartWithLongNames(v.toBool());
        break;
    case Pid::START_WITH_MEASURE_ONE:
        setStartWithMeasureOne(v.toBool());
        break;
    case Pid::FIRST_SYSTEM_INDENTATION:
        setFirstSystemIdentation(v.toBool());
        break;
    default:
        if (!Element::setProperty(propertyId, v)) {
            return false;
        }
        break;
    }
    triggerLayoutAll();
    setGenerated(false);
    return true;
}

//---------------------------------------------------------
//   propertyDefault
//---------------------------------------------------------

QVariant LayoutBreak::propertyDefault(Pid id) const
{
    switch (id) {
    case Pid::LAYOUT_BREAK:
        return QVariant();           // LAYOUT_BREAK_LINE;
    case Pid::PAUSE:
        return score()->styleD(Sid::SectionPause);
    case Pid::START_WITH_LONG_NAMES:
        return true;
    case Pid::START_WITH_MEASURE_ONE:
        return true;
    case Pid::FIRST_SYSTEM_INDENTATION:
        return true;
    default:
        return Element::propertyDefault(id);
    }
}

//---------------------------------------------------------
//   propertyId
//---------------------------------------------------------

Pid LayoutBreak::propertyId(const QStringRef& name) const
{
    if (name == propertyName(Pid::LAYOUT_BREAK)) {
        return Pid::LAYOUT_BREAK;
    }
    return Element::propertyId(name);
}
}
