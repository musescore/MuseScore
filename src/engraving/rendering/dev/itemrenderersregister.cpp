/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-CLA-applies
 *
 * MuseScore
 * Music Composition & Notation
 *
 * Copyright (C) 2023 MuseScore BVBA and others
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
#include "itemrenderersregister.h"

using namespace mu::engraving;
using namespace mu::engraving::rendering;
using namespace mu::engraving::rendering::dev;

ItemRenderersRegister* ItemRenderersRegister::instance()
{
    static ItemRenderersRegister r;
    return &r;
}

void ItemRenderersRegister::reg(ElementType type, std::shared_ptr<IItemRenderer> r)
{
    m_map.insert({ type, r });
}

const std::shared_ptr<IItemRenderer>& ItemRenderersRegister::renderer(ElementType type) const
{
    auto it = m_map.find(type);
    if (it == m_map.end()) {
        static std::shared_ptr<IItemRenderer> null;
        return null;
    }
    return it->second;
}
