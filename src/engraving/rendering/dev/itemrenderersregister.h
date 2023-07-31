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
#ifndef MU_ENGRAVING_ITEMRENDERERSREGISTER_H
#define MU_ENGRAVING_ITEMRENDERERSREGISTER_H

#include <unordered_map>

#include "types/types.h"
#include "iitemrenderer.h"

namespace mu::engraving::rendering::dev {
class ItemRenderersRegister
{
public:

    static ItemRenderersRegister* instance();

    void reg(ElementType type, std::shared_ptr<IItemRenderer> r);
    const std::shared_ptr<IItemRenderer>& renderer(ElementType type) const;

private:
    ItemRenderersRegister() = default;

    std::unordered_map<ElementType, std::shared_ptr<IItemRenderer> > m_map;
};
}

#endif // MU_ENGRAVING_ITEMRENDERERSREGISTER_H
