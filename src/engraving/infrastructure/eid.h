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
#ifndef MU_ENGRAVING_EID_H
#define MU_ENGRAVING_EID_H

#include <cstdint>
#include <string>
#include <string_view>

#include "types/types.h"

namespace mu::engraving {
class EID
{
public:
    EID(ElementType type = ElementType::INVALID, uint32_t id = 0);

    bool isValid() const { return m_type != ElementType::INVALID && m_id != 0; }

    ElementType type() const { return m_type; }
    uint32_t id() const { return m_id; }

    inline bool operator ==(const EID& other) const { return m_type == other.m_type && m_id == other.m_id; }
    inline bool operator !=(const EID& other) const { return !this->operator ==(other); }

    uint64_t toUint64() const;
    static EID fromUint64(uint64_t v);

    std::string toStdString() const;
    static EID fromStdString(const std::string& v);
    static EID fromStdString(const std::string_view& v);

private:

    ElementType m_type = ElementType::INVALID;
    uint32_t m_id = 0;
};
}

#endif // MU_ENGRAVING_EID_H
