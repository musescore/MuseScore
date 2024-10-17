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

#include "mmrest.h"

#include "score.h"
#include "system.h"
#include "undo.h"

#include "log.h"

using namespace mu;
using namespace mu::engraving;

namespace mu::engraving {
//---------------------------------------------------------
//    MMRest
//--------------------------------------------------------

MMRest::MMRest(Segment* s)
    : Rest(ElementType::MMREST, s)
{
    m_numberVisible = true;
    setPlacement(PlacementV::ABOVE);
}

MMRest::MMRest(const MMRest& r, bool link)
    : Rest(r, link)
{
    if (link) {
        score()->undo(new Link(this, const_cast<MMRest*>(&r)));
        setAutoplace(true);
    }
    m_numberVisible = r.m_numberVisible;
}

bool MMRest::shouldShowNumber() const
{
    bool shouldShow = isOldStyle() && ldata()->number == 1
                      ? m_numberVisible && style().styleB(Sid::singleMeasureMMRestShowNumber)
                      : m_numberVisible;

    const Part* itemPart = part();
    const System* system = measure()->system();
    bool isTopStaffOfPartAndCenteringIsActive = itemPart->nstaves() > 1 && staff() == itemPart->staves().front()
                                                && (system && system->nextVisibleStaff(staffIdx()) != muse::nidx)
                                                && style().styleB(Sid::mmRestBetweenStaves);

    return shouldShow && !isTopStaffOfPartAndCenteringIsActive;
}

bool MMRest::isOldStyle() const
{
    int number = ldata()->number;
    return (style().styleB(Sid::oldStyleMultiMeasureRests) && number <= style().styleI(Sid::mmRestOldStyleMaxMeasures))
           || (number == 1 && style().styleB(Sid::singleMeasureMMRestUseNormalRest));
}

RectF MMRest::numberRect() const
{
    return symBbox(ldata()->numberSym);
}

PointF MMRest::numberPos() const
{
    RectF numBBox = numberRect();
    double x = 0.5 * (ldata()->restWidth - numBBox.width());
    double y = -pos().y() + yNumberPos() - 0.5 * numBBox.height();
    return PointF(x, y);
}

double MMRest::yNumberPos() const
{
    return ldata()->yNumberPos + spatium() * m_numberOffset;
}

//---------------------------------------------------------
//   propertyDefault
//---------------------------------------------------------

PropertyValue MMRest::propertyDefault(Pid propertyId) const
{
    switch (propertyId) {
    case Pid::MMREST_NUMBER_OFFSET:
        return 0.0;
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
    case Pid::MMREST_NUMBER_OFFSET:
        return m_numberOffset;
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
    case Pid::MMREST_NUMBER_OFFSET:
        m_numberOffset = v.toDouble();
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
}
