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

#include "logger.h"
#include "eidregister.h"
#include "log.h"

#include "dom/mscore.h"

using namespace mu::engraving;

EID EIDRegister::newEIDForItem(const EngravingObject* item)
{
    EID eid = MScore::testMode ? EID::newUniqueTestMode(m_maxValTestMode) : EID::newUnique();
    registerItemEID(eid, const_cast<EngravingObject*>(item));
    return eid;
}

void EIDRegister::registerItemEID(const EID& eid, const EngravingObject* item)
{
    IF_ASSERT_FAILED(eid.isValid() && item) {
        return;
    }

    bool inserted = m_eidToItem.emplace(eid, const_cast<EngravingObject*>(item)).second;
    assert(inserted);

    inserted = m_itemToEid.emplace(const_cast<EngravingObject*>(item), eid).second;
    assert(inserted);

    if (MScore::testMode) {
        EID::updateMaxValTestMode(eid, m_maxValTestMode);
    }

#ifdef NDEBUG
    UNUSED(inserted);
#endif
}

EngravingObject* EIDRegister::itemFromEID(const EID& eid) const
{
    auto iter = m_eidToItem.find(eid);
    IF_ASSERT_FAILED(iter != m_eidToItem.end()) {
        return nullptr;
    }
    return iter->second;
}

EID EIDRegister::EIDFromItem(const EngravingObject* item) const
{
    auto iter = m_itemToEid.find(const_cast<EngravingObject*>(item));
    return iter == m_itemToEid.end() ? EID::invalid() : iter->second;
}
