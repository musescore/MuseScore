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
#pragma once

#include <cstdint>
#include <unordered_map>

#include "eid.h"
#include "../types/types.h"

namespace mu::engraving {
class EngravingObject;

class EIDRegister
{
public:
    EIDRegister() = default;

    EID newEIDForItem(const EngravingObject* item);
    void registerItemEID(const EID& eid, const EngravingObject* item);

    EngravingObject* itemFromEID(const EID& eid) const;
    EID EIDFromItem(const EngravingObject* item) const;

private:
    EIDRegister(const EIDRegister&) = delete;

    std::unordered_map<EID, EngravingObject*> m_eidToItem;
    std::unordered_map<EngravingObject*, EID> m_itemToEid;

    uint64_t m_maxValTestMode = 0;
};
}
