/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2024 MuseScore Limited
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

#include "engraving/dom/staff.h"
#include "engraving/dom/utils.h"

namespace mu::instrumentsscene {
struct SystemObjectsGroup {
    mu::engraving::ElementType type = mu::engraving::ElementType::INVALID;
    mu::engraving::TranslatableString name;
    std::vector<mu::engraving::EngravingItem*> items;
};

inline std::vector<SystemObjectsGroup> collectSystemObjectGroups(const mu::engraving::Staff* systemObjectsStaff)
{
    std::vector<engraving::EngravingItem*> systemObjects = engraving::collectSystemObjects(systemObjectsStaff->score(),
                                                                                           { systemObjectsStaff->idx() });
    std::vector<SystemObjectsGroup> result;

    for (engraving::EngravingItem* obj : systemObjects) {
        auto it = std::find_if(result.begin(), result.end(), [obj](const SystemObjectsGroup& group) {
            return group.type == obj->type();
        });

        if (it != result.end()) {
            it->items.push_back(obj);
        } else {
            result.emplace_back(SystemObjectsGroup { obj->type(), obj->typeUserName(), { obj } });
        }
    }

    return result;
}

inline bool isSystemObjectsGroupVisible(const SystemObjectsGroup& group)
{
    for (const mu::engraving::EngravingItem* item : group.items) {
        if (item->visible()) {
            return true;
        }
    }

    return false;
}
}
