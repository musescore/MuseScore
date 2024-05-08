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

#include "stafftypechange.h"

#include "score.h"
#include "measure.h"
#include "system.h"
#include "staff.h"

#include "log.h"

using namespace mu;
using namespace mu::engraving;

namespace mu::engraving {
//---------------------------------------------------------
//   StaffTypeChange
//---------------------------------------------------------

StaffTypeChange::StaffTypeChange(MeasureBase* parent)
    : EngravingItem(ElementType::STAFFTYPE_CHANGE, parent, ElementFlag::HAS_TAG)
{
    m_lw = spatium() * 0.3;
}

StaffTypeChange::StaffTypeChange(const StaffTypeChange& lb)
    : EngravingItem(lb)
{
    m_lw = lb.m_lw;
    m_ownsStaffType = lb.m_ownsStaffType;
    if (lb.m_ownsStaffType && lb.m_staffType) {
        m_staffType = new StaffType(*lb.m_staffType);
    } else {
        m_staffType = lb.m_staffType;
    }
}

StaffTypeChange::~StaffTypeChange()
{
    if (m_staffType && m_ownsStaffType) {
        delete m_staffType;
    }
}

void StaffTypeChange::setStaffType(StaffType* st, bool owned)
{
    if (m_staffType && m_ownsStaffType) {
        delete m_staffType;
    }

    m_staffType = st;
    m_ownsStaffType = owned && (st != nullptr);
}

//---------------------------------------------------------
//   spatiumChanged
//---------------------------------------------------------

void StaffTypeChange::spatiumChanged(double, double)
{
    m_lw = spatium() * 0.3;
}

//---------------------------------------------------------
//   getProperty
//---------------------------------------------------------

PropertyValue StaffTypeChange::getProperty(Pid propertyId) const
{
    switch (propertyId) {
    case Pid::STEP_OFFSET:
        return m_staffType->stepOffset();
    case Pid::STAFF_LINES:
        return m_staffType->lines();
    case Pid::LINE_DISTANCE:
        return m_staffType->lineDistance();
    case Pid::STAFF_SHOW_BARLINES:
        return m_staffType->showBarlines();
    case Pid::STAFF_SHOW_LEDGERLINES:
        return m_staffType->showLedgerLines();
    case Pid::STAFF_STEMLESS:
        return m_staffType->stemless();
    case Pid::HEAD_SCHEME:
        return int(m_staffType->noteHeadScheme());
    case Pid::STAFF_GEN_CLEF:
        return m_staffType->genClef();
    case Pid::STAFF_GEN_TIMESIG:
        return m_staffType->genTimesig();
    case Pid::STAFF_GEN_KEYSIG:
        return m_staffType->genKeysig();
    case Pid::MAG:
        return m_staffType->userMag();
    case Pid::SMALL:
        return m_staffType->isSmall();
    case Pid::STAFF_INVISIBLE:
        return m_staffType->invisible();
    case Pid::STAFF_COLOR:
        return PropertyValue::fromValue(m_staffType->color());
    case Pid::STAFF_YOFFSET:
        return m_staffType->yoffset();
    default:
        return EngravingItem::getProperty(propertyId);
    }
}

//---------------------------------------------------------
//   setProperty
//---------------------------------------------------------

bool StaffTypeChange::setProperty(Pid propertyId, const PropertyValue& v)
{
    switch (propertyId) {
    case Pid::STEP_OFFSET:
        m_staffType->setStepOffset(v.toInt());
        break;
    case Pid::STAFF_LINES:
        m_staffType->setLines(v.toInt());
        break;
    case Pid::LINE_DISTANCE:
        m_staffType->setLineDistance(v.value<Spatium>());
        break;
    case Pid::STAFF_SHOW_BARLINES:
        m_staffType->setShowBarlines(v.toBool());
        break;
    case Pid::STAFF_SHOW_LEDGERLINES:
        m_staffType->setShowLedgerLines(v.toBool());
        break;
    case Pid::STAFF_STEMLESS:
        m_staffType->setStemless(v.toBool());
        break;
    case Pid::HEAD_SCHEME:
        m_staffType->setNoteHeadScheme(v.value<NoteHeadScheme>());
        break;
    case Pid::STAFF_GEN_CLEF:
        m_staffType->setGenClef(v.toBool());
        break;
    case Pid::STAFF_GEN_TIMESIG:
        m_staffType->setGenTimesig(v.toBool());
        break;
    case Pid::STAFF_GEN_KEYSIG:
        m_staffType->setGenKeysig(v.toBool());
        break;
    case Pid::MAG: {
        double _spatium = spatium();
        m_staffType->setUserMag(v.toDouble());
        Staff* _staff = staff();
        if (_staff) {
            _staff->setLocalSpatium(_spatium, spatium(), tick());
        }
    }
    break;
    case Pid::SMALL: {
        double _spatium = spatium();
        m_staffType->setSmall(v.toBool());
        Staff* _staff = staff();
        if (_staff) {
            _staff->setLocalSpatium(_spatium, spatium(), tick());
        }
    }
    break;
    case Pid::STAFF_INVISIBLE:
        m_staffType->setInvisible(v.toBool());
        break;
    case Pid::STAFF_COLOR:
        m_staffType->setColor(v.value<Color>());
        break;
    case Pid::STAFF_YOFFSET:
        m_staffType->setYoffset(v.value<Spatium>());
        break;
    default:
        if (!EngravingItem::setProperty(propertyId, v)) {
            return false;
        }
        break;
    }
    if (explicitParent()) {
        staff()->staffTypeListChanged(measure()->tick());
    }
    return true;
}

//---------------------------------------------------------
//   propertyDefault
//---------------------------------------------------------

PropertyValue StaffTypeChange::propertyDefault(Pid id) const
{
    switch (id) {
    case Pid::STEP_OFFSET:
        return 0;
    case Pid::STAFF_LINES:
        return 5;
    case Pid::LINE_DISTANCE:
        return Spatium(1.0);
    case Pid::STAFF_SHOW_BARLINES:
        return true;
    case Pid::STAFF_SHOW_LEDGERLINES:
        return true;
    case Pid::STAFF_STEMLESS:
        return false;
    case Pid::HEAD_SCHEME:
        return NoteHeadScheme::HEAD_NORMAL;
    case Pid::STAFF_GEN_CLEF:
        return true;
    case Pid::STAFF_GEN_TIMESIG:
        return true;
    case Pid::STAFF_GEN_KEYSIG:
        return true;
    case Pid::MAG:
        return 1.0;
    case Pid::SMALL:
        return false;
    case Pid::STAFF_INVISIBLE:
        return false;
    case Pid::STAFF_COLOR:
        return PropertyValue::fromValue(configuration()->defaultColor());
    case Pid::STAFF_YOFFSET:
        return Spatium(0.0);
    default:
        return EngravingItem::propertyDefault(id);
    }
}
}
