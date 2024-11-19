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
#ifndef MU_ENGRAVING_GETEID_H
#define MU_ENGRAVING_GETEID_H

#include <cstdint>
#include <map>

#include "eid.h"
#include "../types/types.h"

namespace mu::engraving {
class EngravingObject;

class EIDRegister
{
public:
    EIDRegister() = default;

    void init(uint32_t val);
    uint32_t lastID() const { return m_lastID; }

    EID newEID(ElementType type);

    void registerItemEID(EID eid, EngravingObject* item);
    EngravingObject* itemFromEID(EID eid);

private:
    EIDRegister(const EIDRegister&) = delete;

    uint32_t m_lastID = 0;
    std::map<uint64_t, EngravingObject*> m_register;
};
}

#endif // MU_ENGRAVING_GETEID_H
