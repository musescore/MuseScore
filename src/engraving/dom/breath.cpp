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

#include "breath.h"

#include "types/symnames.h"

#include "measure.h"
#include "score.h"
#include "segment.h"
#include "staff.h"
#include "system.h"

#include "log.h"

using namespace mu;
using namespace mu::engraving;

namespace mu::engraving {
const std::vector<BreathType> Breath::BREATH_LIST {
    { SymId::breathMarkComma,      false, 0.0 },
    { SymId::breathMarkTick,       false, 0.0 },
    { SymId::breathMarkSalzedo,    false, 0.0 },
    { SymId::breathMarkUpbow,      false, 0.0 },
    { SymId::caesuraCurved,        true,  2.0 },
    { SymId::caesura,              true,  2.0 },
    { SymId::caesuraShort,         true,  2.0 },
    { SymId::caesuraThick,         true,  2.0 },
    { SymId::chantCaesura,         true,  2.0 },
    { SymId::caesuraSingleStroke,  true,  2.0 },
};

//---------------------------------------------------------
//   Breath
//---------------------------------------------------------

Breath::Breath(Segment* parent)
    : EngravingItem(ElementType::BREATH, parent, ElementFlag::MOVABLE)
{
    m_symId = SymId::breathMarkComma;
    m_pause = 0.0;
}

//---------------------------------------------------------
//   isCaesura
//---------------------------------------------------------

bool Breath::isCaesura() const
{
    for (const BreathType& bt : BREATH_LIST) {
        if (bt.id == m_symId) {
            return bt.isCaesura;
        }
    }
    return false;
}

//---------------------------------------------------------
//   mag
//---------------------------------------------------------

double Breath::mag() const
{
    return staff() ? staff()->staffMag(tick()) : 1.0;
}

//---------------------------------------------------------
//   pagePos
//---------------------------------------------------------

PointF Breath::pagePos() const
{
    if (explicitParent() == 0) {
        return pos();
    }
    System* system = segment()->measure()->system();
    double yp = y();
    if (system) {
        yp += system->staff(staffIdx())->y() + system->y();
    }
    return PointF(pageX(), yp);
}

//---------------------------------------------------------
//   getProperty
//---------------------------------------------------------

PropertyValue Breath::getProperty(Pid propertyId) const
{
    switch (propertyId) {
    case Pid::SYMBOL:
        return PropertyValue::fromValue(m_symId);
    case Pid::PAUSE:
        return m_pause;
    default:
        return EngravingItem::getProperty(propertyId);
    }
}

//---------------------------------------------------------
//   setProperty
//---------------------------------------------------------

bool Breath::setProperty(Pid propertyId, const PropertyValue& v)
{
    switch (propertyId) {
    case Pid::SYMBOL:
        setSymId(v.value<SymId>());
        break;
    case Pid::PAUSE:
        setPause(v.toDouble());
        score()->setUpTempoMapLater();
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

//---------------------------------------------------------
//   propertyDefault
//---------------------------------------------------------

PropertyValue Breath::propertyDefault(Pid id) const
{
    switch (id) {
    case Pid::PAUSE:
        return 0.0;
    case Pid::PLACEMENT:
        return track() & 1 ? PlacementV::BELOW : PlacementV::ABOVE;
    default:
        return EngravingItem::propertyDefault(id);
    }
}

//---------------------------------------------------------
//   nextSegmentElement
//---------------------------------------------------------

EngravingItem* Breath::nextSegmentElement()
{
    return segment()->firstInNextSegments(staffIdx());
}

//---------------------------------------------------------
//   prevSegmentElement
//---------------------------------------------------------

EngravingItem* Breath::prevSegmentElement()
{
    return segment()->lastInPrevSegments(staffIdx());
}

//---------------------------------------------------------
//   accessibleInfo
//---------------------------------------------------------

String Breath::accessibleInfo() const
{
    return SymNames::translatedUserNameForSymId(m_symId);
}

//---------------------------------------------------------
//   subtypeUserName
//---------------------------------------------------------

muse::TranslatableString Breath::subtypeUserName() const
{
    return SymNames::userNameForSymId(symId());
}

void Breath::added()
{
    IF_ASSERT_FAILED(score()) {
        return;
    }

    score()->setUpTempoMapLater();
}

void Breath::removed()
{
    IF_ASSERT_FAILED(score()) {
        return;
    }

    score()->setUpTempoMapLater();
}
}
