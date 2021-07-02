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

#include "mmrest.h"
#include "measure.h"
#include "score.h"
#include "symid.h"
#include "undo.h"
#include "utils.h"
#include "draw/pen.h"

using namespace mu;

namespace Ms {
static const ElementStyle mmRestStyle {
    { Sid::mmRestNumberPos, Pid::MMREST_NUMBER_POS },
};

//---------------------------------------------------------
//    MMRest
//--------------------------------------------------------

MMRest::MMRest(Score* s)
    : Rest(s)
{
    m_width = 0;
    m_symsWidth = 0;
    m_numberVisible = true;
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
    m_width = r.m_width;
    m_symsWidth = r.m_symsWidth;
    m_numberVisible = r.m_numberVisible;
}

//---------------------------------------------------------
//   MMRest::draw
//---------------------------------------------------------

void MMRest::draw(mu::draw::Painter* painter) const
{
    TRACE_OBJ_DRAW;
    if (shouldNotBeDrawn() || (track() % VOICES)) {     //only on voice 1
        return;
    }

    qreal _spatium = spatium();

    // draw number
    painter->setPen(curColor());
    std::vector<SymId>&& numberSym = toTimeSigString(QString("%1").arg(m_number));
    RectF numberBox = symBbox(numberSym);
    qreal x = (m_width - numberBox.width()) * .5;
    qreal y = m_numberPos * spatium() - staff()->height() * .5;
    if (m_numberVisible) {
        drawSymbols(numberSym, painter, PointF(x, y));
    }

    if (score()->styleB(Sid::oldStyleMultiMeasureRests)
        && m_number <= score()->styleI(Sid::mmRestOldStyleMaxMeasures)) {
        // draw rest symbols
        x = (m_width - m_symsWidth) * 0.5;
        qreal spacing = score()->styleP(Sid::mmRestOldStyleSpacing);
        for (SymId sym : m_restSyms) {
            y = (sym == SymId::restWhole ? -spatium() : 0);
            drawSymbol(sym, painter, PointF(x, y));
            x += symBbox(sym).width() + spacing;
        }
    } else {
        mu::draw::Pen pen(painter->pen());
        pen.setCapStyle(mu::draw::PenCapStyle::FlatCap);

        // draw horizontal line
        qreal hBarThickness = score()->styleP(Sid::mmRestHBarThickness);
        if (hBarThickness) { // don't draw at all if 0, QPainter interprets 0 pen width differently
            pen.setWidthF(hBarThickness);
            painter->setPen(pen);
            qreal halfHBarThickness = hBarThickness * .5;
            if (score()->styleB(Sid::mmRestNumberMaskHBar) // avoid painting line through number
                && (y + (numberBox.height() * .5)) > -halfHBarThickness
                && (y - (numberBox.height() * .5)) < halfHBarThickness) {
                qreal gapDistance = (numberBox.width() + _spatium) * .5;
                qreal midpoint = m_width * .5;
                painter->drawLine(LineF(0.0, 0.0, midpoint - gapDistance, 0.0));
                painter->drawLine(LineF(midpoint + gapDistance, 0.0, m_width, 0.0));
            } else {
                painter->drawLine(LineF(0.0, 0.0, m_width, 0.0));
            }
        }

        // draw vertical lines
        qreal vStrokeThickness = score()->styleP(Sid::mmRestHBarVStrokeThickness);
        if (vStrokeThickness) { // don't draw at all if 0, QPainter interprets 0 pen width differently
            pen.setWidthF(vStrokeThickness);
            painter->setPen(pen);
            qreal halfVStrokeHeight = score()->styleP(Sid::mmRestHBarVStrokeHeight) * .5;
            painter->drawLine(LineF(0.0, -halfVStrokeHeight, 0.0, halfVStrokeHeight));
            painter->drawLine(LineF(m_width, -halfVStrokeHeight, m_width, halfVStrokeHeight));
        }
    }
}

//---------------------------------------------------------
//   layout
//---------------------------------------------------------

void MMRest::layout()
{
    m_number = measure()->mmRestCount();

    for (Element* e : el()) {
        e->layout();
    }

    if (score()->styleB(Sid::oldStyleMultiMeasureRests)) {
        m_restSyms.clear();
        m_symsWidth = 0;

        int remaining = m_number;
        qreal spacing = score()->styleP(Sid::mmRestOldStyleSpacing);
        SymId sym;

        while (remaining > 0) {
            if (remaining >= 4) {
                sym = SymId::restLonga;
                remaining -= 4;
            } else if (remaining >= 2) {
                sym = SymId::restDoubleWhole;
                remaining -= 2;
            } else {
                sym = SymId::restWhole;
                remaining -= 1;
            }
            m_restSyms.push_back(sym);
            m_symsWidth += symBbox(sym).width();
            if (remaining > 0) { // do not add spacing after last symbol
                m_symsWidth += spacing;
            }
        }
        qreal symHeight = symBbox(m_restSyms[0]).height();
        setbbox(RectF((m_width - m_symsWidth) * .5, -spatium(), m_symsWidth, symHeight));
    } else { // H-bar
        qreal vStrokeHeight = score()->styleP(Sid::mmRestHBarVStrokeHeight);
        setbbox(RectF(0.0, -(vStrokeHeight * .5), m_width, vStrokeHeight));
    }
    if (m_numberVisible) {
        addbbox(numberRect());
    }
    return;
}

//---------------------------------------------------------
//   numberRect
///   returns the mmrest number's bounding rectangle
//---------------------------------------------------------

RectF MMRest::numberRect() const
{
    std::vector<SymId>&& s = toTimeSigString(QString("%1").arg(m_number));

    RectF r = symBbox(s);
    qreal x = (m_width - r.width()) * .5;
    qreal y = m_numberPos * spatium() - staff()->height() * .5;

    r.translate(PointF(x, y));
    return r;
}

//---------------------------------------------------------
//   MMRest::write
//---------------------------------------------------------

void MMRest::write(XmlWriter& xml) const
{
    xml.stag("Rest"); // for compatibility, see also Measure::readVoice()
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
    case Pid::MMREST_NUMBER_VISIBLE:
        return true;
    default:
        return Rest::propertyDefault(propertyId);
    }
}

//---------------------------------------------------------
//   getProperty
//---------------------------------------------------------

QVariant MMRest::getProperty(Pid propertyId) const
{
    switch (propertyId) {
    case Pid::MMREST_NUMBER_POS:
        return m_numberPos;
    case Pid::MMREST_NUMBER_VISIBLE:
        return m_numberVisible;
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
        m_numberPos = v.toDouble();
        triggerLayout();
        break;
    case Pid::MMREST_NUMBER_VISIBLE:
        m_numberVisible = v.toBool();
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
    qreal vStrokeHeight = score()->styleP(Sid::mmRestHBarVStrokeHeight);
    shape.add(RectF(0.0, -(vStrokeHeight * .5), m_width, vStrokeHeight));
    if (m_numberVisible) {
        shape.add(numberRect());
    }
    return shape;
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
}
