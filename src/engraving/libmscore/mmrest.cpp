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

#include "draw/pen.h"
#include "rw/xml.h"
#include "types/symnames.h"

#include "measure.h"
#include "score.h"
#include "undo.h"
#include "utils.h"

using namespace mu;
using namespace mu::engraving;

namespace Ms {
static const ElementStyle mmRestStyle {
    { Sid::mmRestNumberPos, Pid::MMREST_NUMBER_POS },
};

//---------------------------------------------------------
//    MMRest
//--------------------------------------------------------

MMRest::MMRest(Segment* s)
    : Rest(ElementType::MMREST, s)
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
    RectF numberBox = symBbox(m_numberSym);
    PointF numberPos = numberPosition(numberBox);
    if (m_numberVisible) {
        drawSymbols(m_numberSym, painter, numberPos);
    }

    numberBox.translate(numberPos);

    if (score()->styleB(Sid::oldStyleMultiMeasureRests)
        && m_number <= score()->styleI(Sid::mmRestOldStyleMaxMeasures)) {
        // draw rest symbols
        qreal x = (m_width - m_symsWidth) * 0.5;
        qreal spacing = score()->styleMM(Sid::mmRestOldStyleSpacing);
        for (SymId sym : m_restSyms) {
            qreal y = (sym == SymId::restWhole ? -spatium() : 0);
            drawSymbol(sym, painter, PointF(x, y));
            x += symBbox(sym).width() + spacing;
        }
    } else {
        mu::draw::Pen pen(painter->pen());
        pen.setCapStyle(mu::draw::PenCapStyle::FlatCap);

        // draw horizontal line
        qreal hBarThickness = score()->styleMM(Sid::mmRestHBarThickness);
        if (hBarThickness) { // don't draw at all if 0, QPainter interprets 0 pen width differently
            pen.setWidthF(hBarThickness);
            painter->setPen(pen);
            qreal halfHBarThickness = hBarThickness * .5;
            if (m_numberVisible // avoid painting line through number
                && score()->styleB(Sid::mmRestNumberMaskHBar)
                && numberBox.bottom() >= -halfHBarThickness
                && numberBox.top() <= halfHBarThickness) {
                qreal gapDistance = (numberBox.width() + _spatium) * .5;
                qreal midpoint = m_width * .5;
                painter->drawLine(LineF(0.0, 0.0, midpoint - gapDistance, 0.0));
                painter->drawLine(LineF(midpoint + gapDistance, 0.0, m_width, 0.0));
            } else {
                painter->drawLine(LineF(0.0, 0.0, m_width, 0.0));
            }
        }

        // draw vertical lines
        qreal vStrokeThickness = score()->styleMM(Sid::mmRestHBarVStrokeThickness);
        if (vStrokeThickness) { // don't draw at all if 0, QPainter interprets 0 pen width differently
            pen.setWidthF(vStrokeThickness);
            painter->setPen(pen);
            qreal halfVStrokeHeight = score()->styleMM(Sid::mmRestHBarVStrokeHeight) * .5;
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
    m_numberSym = timeSigSymIdsFromString(QString("%1").arg(m_number));

    for (EngravingItem* e : el()) {
        e->layout();
    }

    if (score()->styleB(Sid::oldStyleMultiMeasureRests)) {
        m_restSyms.clear();
        m_symsWidth = 0;

        int remaining = m_number;
        qreal spacing = score()->styleMM(Sid::mmRestOldStyleSpacing);
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
        qreal vStrokeHeight = score()->styleMM(Sid::mmRestHBarVStrokeHeight);
        setbbox(RectF(0.0, -(vStrokeHeight * .5), m_width, vStrokeHeight));
    }

    // Only need to set y position here; x position is handled in Measure::layoutMeasureElements()
    const StaffType* staffType = this->staffType();
    setPos(0, std::floor(staffType->middleLine() / 2.0) * staffType->lineDistance().val() * spatium());

    if (m_numberVisible) {
        addbbox(numberRect());
    }
}

//---------------------------------------------------------
//   numberRect
///   returns the mmrest number's bounding rectangle
//---------------------------------------------------------

PointF MMRest::numberPosition(const mu::RectF& numberBbox) const
{
    qreal x = (m_width - numberBbox.width()) * .5;
    // -pos().y(): relative to topmost staff line
    // - 0.5 * r.height(): relative to the baseline of the number symbol
    // (rather than the center)
    qreal y = -pos().y() + m_numberPos * spatium() - 0.5 * numberBbox.height();

    return PointF(x, y);
}

RectF MMRest::numberRect() const
{
    RectF r = symBbox(m_numberSym);
    r.translate(numberPosition(r));
    return r;
}

//---------------------------------------------------------
//   MMRest::write
//---------------------------------------------------------

void MMRest::write(XmlWriter& xml) const
{
    xml.startObject("Rest"); // for compatibility, see also Measure::readVoice()
    ChordRest::writeProperties(xml);
    el().write(xml);
    xml.endObject();
}

//---------------------------------------------------------
//   propertyDefault
//---------------------------------------------------------

PropertyValue MMRest::propertyDefault(Pid propertyId) const
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

PropertyValue MMRest::getProperty(Pid propertyId) const
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

bool MMRest::setProperty(Pid propertyId, const PropertyValue& v)
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
    qreal vStrokeHeight = score()->styleMM(Sid::mmRestHBarVStrokeHeight);
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
