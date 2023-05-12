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

#include "translation.h"

#include "layout/tlayout.h"

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
<<<<<<< HEAD
<<<<<<< HEAD
=======
//   layout
//---------------------------------------------------------

void MeasureRepeat::layout()
{
    UNREACHABLE;
    LayoutContext ctx(score());
    v0::TLayout::layout(this, ctx);
}

//---------------------------------------------------------
>>>>>>> 4f8a1b6dd0... [engraving] replaced item->layout() to TLayout::layout
=======
>>>>>>> 11610ff2b5... [engraving] removed item->layout method
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
