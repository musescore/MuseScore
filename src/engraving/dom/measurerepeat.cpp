/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2021 MuseScore Limited
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

#include "translation.h"

#include "style/style.h"

#include "measure.h"
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
    : Rest(ElementType::MEASURE_REPEAT, parent), m_numMeasures(0)
{
    // however many measures the group, the element itself is always exactly the duration of its containing measure
    setDurationType(DurationType::V_MEASURE);
    if (parent) {
        initElementStyle(&measureRepeatStyle);
    }
}

void MeasureRepeat::setNumMeasures(int n)
{
    IF_ASSERT_FAILED(n <= MAX_NUM_MEASURES) {
        m_numMeasures = MAX_NUM_MEASURES;
        return;
    }

    m_numMeasures = n;
}

//---------------------------------------------------------
//   firstMeasureOfGroup
//---------------------------------------------------------

Measure* MeasureRepeat::firstMeasureOfGroup() const
{
    return measure()->firstOfMeasureRepeatGroup(staffIdx());
}

const Measure* MeasureRepeat::referringMeasure(const Measure* measure) const
{
    IF_ASSERT_FAILED(measure) {
        return nullptr;
    }

    const Measure* firstMeasure = firstMeasureOfGroup();
    if (!firstMeasure) {
        return nullptr;
    }

    const Measure* referringMeasure = firstMeasure->prevMeasure();

    int measuresBack = m_numMeasures - measure->measureRepeatCount(staffIdx());

    for (int i = 0; i < measuresBack && referringMeasure; ++i) {
        referringMeasure = referringMeasure->prevMeasure();
    }

    return referringMeasure;
}

//---------------------------------------------------------
//   numberRect
///   returns the measure repeat number's bounding rectangle
//---------------------------------------------------------

PointF MeasureRepeat::numberPosition(const RectF& numberBbox) const
{
    double x = (symBbox(ldata()->symId).width() - numberBbox.width()) * .5;
    // -pos().y(): relative to topmost staff line
    // - 0.5 * r.height(): relative to the baseline of the number symbol
    // (rather than the center)
    double staffTop = -pos().y();
    // Single line staff barlines extend above top of staff
    if (staffType() && staffType()->lines() == 1) {
        staffTop -= 2.0 * spatium();
    }
    double y = std::min(staffTop, -symBbox(ldata()->symId).height() / 2) + m_numberPos * spatium() - 0.5 * numberBbox.height();

    return PointF(x, y);
}

RectF MeasureRepeat::numberRect() const
{
    RectF r = symBbox(ldata()->numberSym);
    r.translate(numberPosition(r));
    return r;
}

//---------------------------------------------------------
//   propertyDefault
//---------------------------------------------------------

PropertyValue MeasureRepeat::propertyDefault(Pid propertyId) const
{
    switch (propertyId) {
    case Pid::MEASURE_REPEAT_NUMBER_POS:
        return style().styleV(Sid::measureRepeatNumberPos);
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
    return muse::mtrc("engraving", "%1; Duration: %n measure(s)", nullptr, numMeasures()).arg(EngravingItem::accessibleInfo());
}

//---------------------------------------------------------
//   subtypeUserName
//---------------------------------------------------------

muse::TranslatableString MeasureRepeat::subtypeUserName() const
{
    return muse::TranslatableString("engraving", "%n measure(s)", nullptr, numMeasures());
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
