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

#include "staffstate.h"

#include "draw/types/pen.h"

#include "part.h"
#include "score.h"

#include "log.h"

using namespace mu;
using namespace muse::draw;

namespace mu::engraving {
//---------------------------------------------------------
//   StaffState
//---------------------------------------------------------

StaffState::StaffState(EngravingItem* parent)
    : EngravingItem(ElementType::STAFF_STATE, parent)
{
    m_staffStateType = StaffStateType::INSTRUMENT;
    m_instrument = new Instrument;
}

StaffState::StaffState(const StaffState& ss)
    : EngravingItem(ss)
{
    m_instrument = new Instrument(*ss.m_instrument);
}

StaffState::~StaffState()
{
    delete m_instrument;
}

//---------------------------------------------------------
//   setStaffStateType
//---------------------------------------------------------

void StaffState::setStaffStateType(const String& s)
{
    if (s == "instrument") {
        setStaffStateType(StaffStateType::INSTRUMENT);
    } else if (s == "type") {
        setStaffStateType(StaffStateType::TYPE);
    } else if (s == "visible") {
        setStaffStateType(StaffStateType::VISIBLE);
    } else if (s == "invisible") {
        setStaffStateType(StaffStateType::INVISIBLE);
    }
}

//---------------------------------------------------------
//   staffStateTypeName
//---------------------------------------------------------

String StaffState::staffStateTypeName() const
{
    switch (staffStateType()) {
    case StaffStateType::INSTRUMENT:
        return u"instrument";
    case StaffStateType::TYPE:
        return u"type";
    case StaffStateType::VISIBLE:
        return u"visible";
    case StaffStateType::INVISIBLE:
        return u"invisible";
    default:
        return u"??";
    }
}

//---------------------------------------------------------
//   acceptDrop
//---------------------------------------------------------

bool StaffState::acceptDrop(EditData&) const
{
    return false;
}

//---------------------------------------------------------
//   drop
//---------------------------------------------------------

EngravingItem* StaffState::drop(EditData& data)
{
    EngravingItem* e = data.dropElement;
    score()->undoChangeElement(this, e);
    return e;
}
}
