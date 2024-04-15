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
#include "eid.h"

using namespace mu::engraving;

EID::EID(ElementType type, uint32_t id)
    : m_type(type), m_id(id)
{
}

struct _Data {
    uint16_t type = 0;
    uint16_t reserved = 0;
    uint32_t id = 0;
};

union _Pack
{
    uint64_t val;
    _Data data;
};

uint64_t EID::toUint64() const
{
    _Pack pack = { 0 };
    pack.data.type = static_cast<uint16_t>(m_type);
    pack.data.id = m_id;
    return pack.val;
}

EID EID::fromUint64(uint64_t v)
{
    _Pack pack = { 0 };
    pack.val = v;
    return EID(static_cast<ElementType>(pack.data.type), pack.data.id);
}

std::string EID::toStdString() const
{
    return std::to_string(toUint64());
}

EID EID::fromStdString(const std::string& s)
{
    uint64_t v = std::stoull(s);
    return fromUint64(v);
}

EID EID::fromStdString(const std::string_view& s)
{
    uint64_t v = std::stoull(s.data(), nullptr, 10);
    return fromUint64(v);
}
