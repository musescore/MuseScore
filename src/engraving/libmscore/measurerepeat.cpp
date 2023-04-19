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

#include "measurerepeat.h"

#include "draw/types/pen.h"
#include "rw/xml.h"

#include "translation.h"

#include "measure.h"
#include "score.h"
#include "staff.h"

#include "log.h"

using namespace mu;
using namespace mu::engraving;

namespace mu::engraving {
static const ElementStyle measureRepeatStyle {
    { Sid::measureRepeatNumberPos, Pid::MEASURE_REPEAT_NUMBER_POS },
};

//---------------------------------------------------------
//   MeasureRepeat
//---------------------------------------------------------

MeasureRepeat::MeasureRepeat(Segment* parent)
    : Rest(ElementType::MEASURE_REPEAT, parent), m_numMeasures(0), m_symId(SymId::noSym)
{
    // however many measures the group, the element itself is always exactly the duration of its containing measure
    setDurationType(DurationType::V_MEASURE);
    if (parent) {
        initElementStyle(&measureRepeatStyle);
    }
}

//---------------------------------------------------------
//   firstMeasureOfGroup
//---------------------------------------------------------

Measure* MeasureRepeat::firstMeasureOfGroup() const
{
    return measure()->firstOfMeasureRepeatGroup(staffIdx());
}

const Measure* MeasureRepeat::referringMeasure() const
{
    Measure* firstMeasureRepeat = firstMeasureOfGroup();

    if (!firstMeasureRepeat) {
        return nullptr;
    }

    return firstMeasureRepeat->prevMeasure();
}

//---------------------------------------------------------
//   draw
//---------------------------------------------------------

void MeasureRepeat::draw(mu::draw::Painter* painter) const
{
    TRACE_ITEM_DRAW;
    using namespace mu::draw;

    painter->setPen(curColor());
    drawSymbol(symId(), painter);

    if (track() != mu::nidx) { // in score rather than palette
        if (!m_numberSym.empty()) {
            PointF numberPos = numberPosition(symBbox(m_numberSym));
            drawSymbols(numberSym(), painter, numberPos);
        }

        if (score()->styleB(Sid::fourMeasureRepeatShowExtenders) && numMeasures() == 4) {
            // TODO: add style settings specific to measure repeats
            // for now, using thickness and margin same as mmrests
            double hBarThickness = score()->styleMM(Sid::mmRestHBarThickness);
            if (hBarThickness) { // don't draw at all if 0, QPainter interprets 0 pen width differently
                Pen pen(painter->pen());
                pen.setCapStyle(PenCapStyle::FlatCap);
                pen.setWidthF(hBarThickness);
                painter->setPen(pen);

                double twoMeasuresWidth = 2 * measure()->width();
                double margin = score()->styleMM(Sid::multiMeasureRestMargin);
                double xOffset = symBbox(symId()).width() * .5;
                double gapDistance = (symBbox(symId()).width() + spatium()) * .5;
                painter->drawLine(LineF(-twoMeasuresWidth + xOffset + margin, 0.0, xOffset - gapDistance, 0.0));
                painter->drawLine(LineF(xOffset + gapDistance, 0.0, twoMeasuresWidth + xOffset - margin, 0.0));
            }
        }
    }
}

//---------------------------------------------------------
//   layout
//---------------------------------------------------------

void MeasureRepeat::layout()
{
    for (EngravingItem* e : el()) {
        e->layout();
    }

    switch (numMeasures()) {
    case 1:
    {
        setSymId(SymId::repeat1Bar);
        if (score()->styleB(Sid::mrNumberSeries) && track() != mu::nidx) {
            int placeInSeries = 2; // "1" would be the measure actually being repeated
            staff_idx_t staffIdx = this->staffIdx();
            Measure* m = measure();
            while (m && m->isOneMeasureRepeat(staffIdx) && m->prevIsOneMeasureRepeat(staffIdx)) {
                placeInSeries++;
                m = m->prevMeasure();
            }
            if (placeInSeries % score()->styleI(Sid::mrNumberEveryXMeasures) == 0) {
                if (score()->styleB(Sid::mrNumberSeriesWithParentheses)) {
                    m_numberSym = timeSigSymIdsFromString(String(u"(%1)").arg(placeInSeries));
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

    RectF bbox = symBbox(symId());

    if (track() != mu::nidx) { // if this is in score rather than a palette cell
        // For unknown reasons, the symbol has some offset in almost all SMuFL fonts
        // We compensate for it, to make sure the symbol is visually centered around the staff line
        double offset = (-bbox.top() - bbox.bottom()) / 2.0;

        const StaffType* staffType = this->staffType();

        // Only need to set y position here; x position is handled in Measure::layoutMeasureElements()
        setPos(0, std::floor(staffType->middleLine() / 2.0) * staffType->lineDistance().val() * spatium() + offset);
    }

    setbbox(bbox);

    if (track() != mu::nidx && !m_numberSym.empty()) {
        addbbox(numberRect());
    }
}

//---------------------------------------------------------
//   numberRect
///   returns the measure repeat number's bounding rectangle
//---------------------------------------------------------

PointF MeasureRepeat::numberPosition(const mu::RectF& numberBbox) const
{
    double x = (symBbox(symId()).width() - numberBbox.width()) * .5;
    // -pos().y(): relative to topmost staff line
    // - 0.5 * r.height(): relative to the baseline of the number symbol
    // (rather than the center)
    double y = -pos().y() + m_numberPos * spatium() - 0.5 * numberBbox.height();

    return PointF(x, y);
}

RectF MeasureRepeat::numberRect() const
{
    RectF r = symBbox(m_numberSym);
    r.translate(numberPosition(r));
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
//   propertyDefault
//---------------------------------------------------------

PropertyValue MeasureRepeat::propertyDefault(Pid propertyId) const
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

PropertyValue MeasureRepeat::getProperty(Pid propertyId) const
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

bool MeasureRepeat::setProperty(Pid propertyId, const PropertyValue& v)
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

String MeasureRepeat::accessibleInfo() const
{
    return mtrc("engraving", "%1; Duration: %n measure(s)", nullptr, numMeasures()).arg(EngravingItem::accessibleInfo());
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
