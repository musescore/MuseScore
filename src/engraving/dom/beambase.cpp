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
#include "beambase.h"

using namespace mu::engraving;

BeamBase::BeamBase(const ElementType& type, EngravingItem* parent, ElementFlags flags)
    : EngravingItem(type, parent, flags)
{
}

BeamBase::BeamBase(const BeamBase& b)
    : EngravingItem(b)
{
    for (const BeamSegment* bs : b.m_beamSegments) {
        m_beamSegments.push_back(new BeamSegment(*bs));
    }
    _crossStaffMove = b._crossStaffMove;
    m_direction       = b.m_direction;
    m_up              = b.m_up;
    m_userModified[0] = b.m_userModified[0];
    m_userModified[1] = b.m_userModified[1];
}

void BeamBase::undoChangeProperty(Pid id, const PropertyValue& v, PropertyFlags ps)
{
    if (id == Pid::BEAM_CROSS_STAFF_MOVE) {
        if (!acceptCrossStaffMove(v.toInt())) {
            return;
        }
        undoResetProperty(Pid::USER_MODIFIED);
        undoResetProperty(Pid::BEAM_POS);
    }
    EngravingItem::undoChangeProperty(id, v, ps);
}

PropertyValue BeamBase::getProperty(Pid propertyId) const
{
    switch (propertyId) {
    case Pid::BEAM_CROSS_STAFF_MOVE:
        return crossStaffMove();
    case Pid::STEM_DIRECTION:
        return direction();
    case Pid::USER_MODIFIED:
        return userModified();
    default:
        return EngravingItem::getProperty(propertyId);
    }
}

bool BeamBase::setProperty(Pid propertyId, const PropertyValue& v)
{
    switch (propertyId) {
    case Pid::BEAM_CROSS_STAFF_MOVE:
        setCrossStaffMove(v.toInt());
        break;
    case Pid::STEM_DIRECTION:
        setDirection(v.value<DirectionV>());
        break;
    case Pid::USER_MODIFIED:
        setUserModified(v.toBool());
        break;
    default:
        if (!EngravingItem::setProperty(propertyId, v)) {
            return false;
        }
        break;
    }

    triggerLayout();
    setGenerated(false);
    return true;
}

PropertyValue BeamBase::propertyDefault(Pid propertyId) const
{
    switch (propertyId) {
    case Pid::BEAM_CROSS_STAFF_MOVE:
        return 0;
    case Pid::STEM_DIRECTION:
        return DirectionV::AUTO;
    case Pid::USER_MODIFIED:
        return false;
    default:
        return EngravingItem::propertyDefault(propertyId);
    }
}

int BeamBase::crossStaffIdx() const
{
    return defaultCrossStaffIdx() + _crossStaffMove;
}

int BeamBase::defaultCrossStaffIdx() const
{
    double average = 0.5 * (static_cast<double>(minCRMove()) + static_cast<double>(maxCRMove()));
    return std::ceil(average);
}

bool BeamBase::acceptCrossStaffMove(int move) const
{
    int newCrossStaffIdx = defaultCrossStaffIdx() + move;
    return newCrossStaffIdx >= minCRMove() && newCrossStaffIdx <= maxCRMove() + 1;
}

bool BeamBase::userModified() const
{
    int idx = directionIdx();
    return m_userModified[idx];
}

void BeamBase::setUserModified(bool val)
{
    int idx = directionIdx();
    m_userModified[idx] = val;
}

void BeamBase::clearBeamSegments()
{
    muse::DeleteAll(m_beamSegments);
    m_beamSegments.clear();
}
