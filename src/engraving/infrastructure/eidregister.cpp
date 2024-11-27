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

#include "eidregister.h"
#include "log.h"

using namespace mu::engraving;

void EIDRegister::init(uint32_t val)
{
    m_lastID = val;
}

EID EIDRegister::newEID(ElementType type)
{
    return EID(type, ++m_lastID);
}

void EIDRegister::registerItemEID(EID eid, EngravingObject* item)
{
    bool inserted = m_register.emplace(eid.toUint64(), item).second;
#ifdef NDEBUG
    UNUSED(inserted);
#else
    assert(inserted);
#endif
}

EngravingObject* EIDRegister::itemFromEID(EID eid)
{
    auto iter = m_register.find(eid.toUint64());
    assert(iter != m_register.end());
    return (*iter).second;
}
