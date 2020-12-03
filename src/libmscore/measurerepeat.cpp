//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2002-2011 Werner Schweer
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2
//  as published by the Free Software Foundation and appearing in
//  the file LICENCE.GPL
//=============================================================================

#include "measurerepeat.h"
#include "barline.h"
#include "measure.h"
#include "mscore.h"
#include "score.h"
#include "staff.h"
#include "sym.h"
#include "system.h"

namespace Ms {
static const ElementStyle measureRepeatStyle {
    { Sid::measureRepeatNumberPos, Pid::MEASURE_REPEAT_NUMBER_POS },
};

//---------------------------------------------------------
//   MeasureRepeat
//---------------------------------------------------------

MeasureRepeat::MeasureRepeat(Score* score)
    : Rest(score), m_numMeasures(0), m_symId(SymId::noSym)
{
    // however many measures the group, the element itself is always exactly the duration of its containing measure
    setDurationType(TDuration::DurationType::V_MEASURE);
    if (score) {
        initElementStyle(&measureRepeatStyle);
    }
}

//---------------------------------------------------------
//   draw
//---------------------------------------------------------

void MeasureRepeat::draw(QPainter* painter) const
{
    painter->setPen(curColor());
    drawSymbol(symId(), painter);

    if (track() != -1) { // in score rather than palette
        qreal x = (symBbox(symId()).width() - symBbox(numberSym()).width()) * .5;
        qreal y = numberPos() * spatium() - staff()->height() * .5;
        drawSymbols(numberSym(), painter, QPointF(x, y));
        if (score()->styleB(Sid::fourMeasureRepeatShowExtenders) && numMeasures() == 4) {
            // TODO: add style settings specific to measure repeats
            // for now, using thickness and margin same as mmrests
            qreal hBarThickness = score()->styleP(Sid::mmRestHBarThickness);
            if (hBarThickness) { // don't draw at all if 0, QPainter interprets 0 pen width differently
                QPen pen(painter->pen());
                pen.setCapStyle(Qt::FlatCap);
                pen.setWidthF(hBarThickness);
                painter->setPen(pen);

                qreal twoMeasuresWidth = 2 * measure()->width();
                qreal margin = score()->styleP(Sid::multiMeasureRestMargin);
                qreal xOffset = symBbox(symId()).width() * .5;
                qreal gapDistance = (symBbox(symId()).width() + spatium()) * .5;
                painter->drawLine(QLineF(-twoMeasuresWidth + xOffset + margin, 0.0, xOffset - gapDistance, 0.0));
                painter->drawLine(QLineF(xOffset + gapDistance, 0.0, twoMeasuresWidth + xOffset - margin, 0.0));
            }
        }
    }
}

//---------------------------------------------------------
//   layout
//---------------------------------------------------------

void MeasureRepeat::layout()
{
    for (Element* e : el()) {
        e->layout();
    }

    switch (numMeasures()) {
    case 1:
    {
        setSymId(SymId::repeat1Bar);
        if (score()->styleB(Sid::mrNumberSeries) && track() != -1) {
            int placeInSeries = 2; // "1" would be the measure actually being repeated
            int staffIdx = staff()->idx();
            Measure* m = measure();
            while (m && m->isOneMeasureRepeat(staffIdx) && m->prevIsOneMeasureRepeat(staffIdx)) {
                placeInSeries++;
                m = m->prevMeasure();
            }
            if (placeInSeries % score()->styleI(Sid::mrNumberEveryXMeasures) == 0) {
                if (score()->styleB(Sid::mrNumberSeriesWithParentheses)) {
                    m_numberSym = toTimeSigString(QString("(%1)").arg(placeInSeries));
                } else {
                    setNumberSym(placeInSeries);
                }
            } else {
                m_numberSym.clear();
            }
        } else if (score()->styleB(Sid::oneMeasureRepeatShow1)) {
            setNumberSym(1);
        } else {
            m_numberSym.clear();
        }
        break;
    }
    case 2:
        setSymId(SymId::repeat2Bars);
        setNumberSym(numMeasures());
        break;
    case 4:
        setSymId(SymId::repeat4Bars);
        setNumberSym(numMeasures());
        break;
    default:
        setSymId(SymId::noSym); // should never happen
        m_numberSym.clear();
        break;
    }

    if (track() != -1) {    // in score rather than palette
        setPos(0, 2.0 * spatium() + 0.5 * styleP(Sid::staffLineWidth)); // xpos handled by Measure::stretchMeasure()
    }
    setbbox(symBbox(symId()));
    addbbox(numberRect());
}

//---------------------------------------------------------
//   numberRect
///   returns the measure repeat number's bounding rectangle
//---------------------------------------------------------

QRectF MeasureRepeat::numberRect() const
{
    if (track() == -1 || numberSym().empty()) { // don't display in palette
        return QRectF();
    }
    QRectF r = symBbox(numberSym());
    qreal x = (symBbox(symId()).width() - symBbox(numberSym()).width()) * .5;
    qreal y = numberPos() * spatium() - staff()->height() * .5;
    r.translate(QPointF(x, y));
    return r;
}

//---------------------------------------------------------
//   shape
//---------------------------------------------------------

Shape MeasureRepeat::shape() const
{
    Shape shape;
    shape.add(numberRect());
    shape.add(symBbox(symId()));
    return shape;
}

//---------------------------------------------------------
//   write
//---------------------------------------------------------

void MeasureRepeat::write(XmlWriter& xml) const
{
    xml.stag(this);
    writeProperty(xml, Pid::SUBTYPE);
    Rest::writeProperties(xml);
    el().write(xml);
    xml.etag();
}

//---------------------------------------------------------
//   read
//---------------------------------------------------------

void MeasureRepeat::read(XmlReader& e)
{
    while (e.readNextStartElement()) {
        const QStringRef& tag(e.name());
        if (tag == "subtype") {
            setNumMeasures(e.readInt());
        } else if (!Rest::readProperties(e)) {
            e.unknown();
        }
    }
}

//---------------------------------------------------------
//   propertyDefault
//---------------------------------------------------------

QVariant MeasureRepeat::propertyDefault(Pid propertyId) const
{
    switch (propertyId) {
    case Pid::MEASURE_REPEAT_NUMBER_POS:
        return score()->styleV(Sid::measureRepeatNumberPos);
    default:
        return Rest::propertyDefault(propertyId);
    }
}

//---------------------------------------------------------
//   getProperty
//---------------------------------------------------------

QVariant MeasureRepeat::getProperty(Pid propertyId) const
{
    switch (propertyId) {
    case Pid::SUBTYPE:
        return numMeasures();
    case Pid::MEASURE_REPEAT_NUMBER_POS:
        return numberPos();
    default:
        return Rest::getProperty(propertyId);
    }
}

//---------------------------------------------------------
//   setProperty
//---------------------------------------------------------

bool MeasureRepeat::setProperty(Pid propertyId, const QVariant& v)
{
    switch (propertyId) {
    case Pid::SUBTYPE:
        setNumMeasures(v.toInt());
        break;
    case Pid::MEASURE_REPEAT_NUMBER_POS:
        setNumberPos(v.toDouble());
        triggerLayout();
        break;
    default:
        return Rest::setProperty(propertyId, v);
    }
    return true;
}

//---------------------------------------------------------
//   ticks
//---------------------------------------------------------

Fraction MeasureRepeat::ticks() const
{
    if (measure()) {
        return measure()->stretchedLen(staff());
    }
    return Fraction(0, 1);
}

//---------------------------------------------------------
//   accessibleInfo
//---------------------------------------------------------

QString MeasureRepeat::accessibleInfo() const
{
    return QObject::tr("%1; Duration: %2 measure(s)").arg(Element::accessibleInfo()).arg(numMeasures());
}

//---------------------------------------------------------
//   getPropertyStyle
//---------------------------------------------------------

Sid MeasureRepeat::getPropertyStyle(Pid propertyId) const
{
    if (propertyId == Pid::MEASURE_REPEAT_NUMBER_POS) {
        return Sid::measureRepeatNumberPos;
    }
    return Rest::getPropertyStyle(propertyId);
}
}
