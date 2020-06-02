//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2020 MuseScore BVBA
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2
//  as published by the Free Software Foundation and appearing in
//  the file LICENCE.GPL
//=============================================================================

#include "mmrest.h"
#include "measure.h"
#include "score.h"
#include "undo.h"
#include "utils.h"

namespace Ms {
//---------------------------------------------------------
//   mmRestStyle
//---------------------------------------------------------

static const ElementStyle mmRestStyle {
    { Sid::mmRestNumberPos, Pid::MMREST_NUMBER_POS },
};

//---------------------------------------------------------
//    MMRest
//--------------------------------------------------------

MMRest::MMRest(Score* s)
    : Rest(s)
{
    _mmWidth = 0;
    if (score()) {
        initElementStyle(&mmRestStyle);
    }
}

MMRest::MMRest(const MMRest& r, bool link)
    : Rest(r, link)
{
    if (link) {
        score()->undo(new Link(this, const_cast<MMRest*>(&r)));
        setAutoplace(true);
    }
    _mmWidth = r._mmWidth;
}

//---------------------------------------------------------
//   MMRest::draw
//---------------------------------------------------------

void MMRest::draw(QPainter* painter) const
{
    if (shouldNotBeDrawn() || (track() % VOICES)) { //only on voice 1
        return;
    }

    qreal _spatium = spatium();
    painter->setPen(curColor());

    // draw number
    int n = measure()->mmRestCount();
    std::vector<SymId>&& s = toTimeSigString(QString("%1").arg(n));
    qreal y = _mmRestNumberPos * spatium() - staff()->height() * .5;
    qreal x = (_mmWidth - symBbox(s).width()) * .5;
    drawSymbols(s, painter, QPointF(x, y));

    // draw horizontal line
    qreal pw = _spatium * .7;
    QPen pen(painter->pen());
    pen.setWidthF(pw);
    painter->setPen(pen);
    qreal x1 = pw * .5;
    qreal x2 = _mmWidth - x1;
    if (_mmRestNumberPos > 0.7 && _mmRestNumberPos < 3.3) { // hack for when number encounters horizontal line
        qreal gapDistance = symBbox(s).width() * .5 + _spatium;
        qreal midpoint = (x1 + x2) * .5;
        painter->drawLine(QLineF(x1, 0.0, midpoint - gapDistance, 0.0));
        painter->drawLine(QLineF(midpoint + gapDistance, 0.0, x2, 0.0));
    } else {
        painter->drawLine(QLineF(x1, 0.0, x2, 0.0));
    }

    // draw vertical lines
    pen.setWidthF(_spatium * .2);
    painter->setPen(pen);
    painter->drawLine(QLineF(0.0, -_spatium, 0.0, _spatium));
    painter->drawLine(QLineF(_mmWidth, -_spatium, _mmWidth,  _spatium));
}

//---------------------------------------------------------
//   layout
//---------------------------------------------------------

void MMRest::layout()
{
    for (Element* e : el()) {
        e->layout();
    }
    _mmWidth = score()->styleP(Sid::minMMRestWidth) * mag();
    // setbbox(QRectF(0.0, -_spatium, _mmWidth, 2.0 * _spatium));
    return;
}

void MMRest::layout(qreal val)
{
    //      static const qreal verticalLineWidth = .2;

    qreal _spatium = spatium();
    _mmWidth       = val;
    //      qreal h        = _spatium * (2 + verticalLineWidth);
    //      qreal w        = _mmWidth + _spatium * verticalLineWidth * .5;
    //      bbox().setRect(-_spatium * verticalLineWidth * .5, -h * .5, w, h);
    bbox().setRect(0.0, -_spatium, _mmWidth, _spatium * 2);

    // text
    //      qreal y  = -_spatium * 2.5 - staff()->height() *.5;
    //      addbbox(QRectF(0, y, w, _spatium * 2));         // approximation
}

//---------------------------------------------------------
//   MMRest::write
//---------------------------------------------------------

void MMRest::write(XmlWriter& xml) const
{
    xml.stag("Rest");
    ChordRest::writeProperties(xml);
    el().write(xml);
    xml.etag();
}

//---------------------------------------------------------
//   propertyDefault
//---------------------------------------------------------

QVariant MMRest::propertyDefault(Pid propertyId) const
{
    switch (propertyId) {
        case Pid::MMREST_NUMBER_POS:
            return score()->styleV(Sid::mmRestNumberPos);
        default:
            return Rest::propertyDefault(propertyId);
    }
}

//---------------------------------------------------------
//   resetProperty
//---------------------------------------------------------

void MMRest::resetProperty(Pid propertyId)
{
    setProperty(propertyId, propertyDefault(propertyId));
    return;
}

//---------------------------------------------------------
//   getPropertyStyle
//---------------------------------------------------------

Sid MMRest::getPropertyStyle(Pid propertyId) const
{
    if (propertyId == Pid::MMREST_NUMBER_POS) {
        return Sid::mmRestNumberPos;
    }
    return Rest::getPropertyStyle(propertyId);
}

//---------------------------------------------------------
//   getProperty
//---------------------------------------------------------

QVariant MMRest::getProperty(Pid propertyId) const
{
    switch (propertyId) {
        case Pid::MMREST_NUMBER_POS:
            return _mmRestNumberPos;
        default:
            return Rest::getProperty(propertyId);
    }
}

//---------------------------------------------------------
//   setProperty
//---------------------------------------------------------

bool MMRest::setProperty(Pid propertyId, const QVariant& v)
{
    switch (propertyId) {
        case Pid::MMREST_NUMBER_POS:
            _mmRestNumberPos = v.toDouble();
            triggerLayout();
            break;
        default:
            return Rest::setProperty(propertyId, v);
    }
    return true;
}

//---------------------------------------------------------
//   shape
//---------------------------------------------------------

Shape MMRest::shape() const
{
    Shape shape;
    shape.add(Rest::shape());

    qreal _spatium = spatium();
    shape.add(QRectF(0.0, -_spatium, _mmWidth, 2.0 * _spatium));

    int n = measure()->mmRestCount();
    std::vector<SymId>&& s = toTimeSigString(QString("%1").arg(n));

    QRectF r = symBbox(s);
    qreal y = _mmRestNumberPos * spatium() - staff()->height() * .5;
    qreal x = .5 * (_mmWidth - r.width());

    r.translate(QPointF(x, y));
    shape.add(r);

    return shape;
}
}
