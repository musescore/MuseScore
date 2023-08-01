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

#include "draw/types/pen.h"

#include "measure.h"
#include "score.h"
#include "undo.h"
#include "utils.h"

#include "log.h"

using namespace mu;
using namespace mu::engraving;

namespace mu::engraving {
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

void MMRest::draw(mu::draw::Painter*) const
{
    UNREACHABLE;
}

//---------------------------------------------------------
//   numberRect
///   returns the mmrest number's bounding rectangle
//---------------------------------------------------------

PointF MMRest::numberPosition(const mu::RectF& numberBbox) const
{
    double x = (m_width - numberBbox.width()) * .5;
    // -pos().y(): relative to topmost staff line
    // - 0.5 * r.height(): relative to the baseline of the number symbol
    // (rather than the center)
    double y = -pos().y() + m_numberPos * spatium() - 0.5 * numberBbox.height();

    return PointF(x, y);
}

RectF MMRest::numberRect() const
{
    RectF r = symBbox(m_numberSym);
    r.translate(numberPosition(r));
    return r;
}

//---------------------------------------------------------
//   propertyDefault
//---------------------------------------------------------

PropertyValue MMRest::propertyDefault(Pid propertyId) const
{
    switch (propertyId) {
    case Pid::MMREST_NUMBER_POS:
        return style().styleV(Sid::mmRestNumberPos);
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
    double vStrokeHeight = style().styleMM(Sid::mmRestHBarVStrokeHeight);
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
