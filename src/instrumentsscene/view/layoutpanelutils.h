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
#include "engraving/types/typesconv.h"

namespace mu::instrumentsscene {
struct SystemObjectsGroup {
    mu::engraving::ElementType type = mu::engraving::ElementType::INVALID;
    std::vector<mu::engraving::EngravingItem*> items;
    mu::engraving::Staff* staff = nullptr;
};

using SystemObjectGroups = std::vector<SystemObjectsGroup>;
using SystemObjectGroupsByStaff = std::map<const mu::engraving::Staff*, SystemObjectGroups>;

inline SystemObjectGroupsByStaff collectSystemObjectGroups(const std::vector<mu::engraving::Staff*>& staves)
{
    if (staves.empty()) {
        return {};
    }

    const std::vector<engraving::EngravingItem*> systemObjects = engraving::collectSystemObjects(staves.front()->score(), staves);
    SystemObjectGroupsByStaff result;

    for (engraving::EngravingItem* obj : systemObjects) {
        SystemObjectGroups& groups = result[obj->staff()];

        auto it = std::find_if(groups.begin(), groups.end(), [obj](const SystemObjectsGroup& group) {
            return group.type == obj->type();
        });

        if (it != groups.end()) {
            it->items.push_back(obj);
        } else {
            groups.emplace_back(SystemObjectsGroup { obj->type(), { obj }, obj->staff() });
        }
    }

    for (mu::engraving::Staff* staff : staves) {
        SystemObjectGroups& systemObjectGroups = result[staff];
        systemObjectGroups.push_back(SystemObjectsGroup { mu::engraving::ElementType::MEASURE_NUMBER, {}, staff });
    }

    return result;
}

inline SystemObjectGroups collectSystemObjectGroups(const mu::engraving::Staff* systemObjectsStaff)
{
    const std::vector<mu::engraving::Staff*> staves {
        const_cast<mu::engraving::Staff*>(systemObjectsStaff)
    };

    SystemObjectGroupsByStaff systemObjects = collectSystemObjectGroups(staves);
    return systemObjects[systemObjectsStaff];
}

inline bool isSystemObjectsGroupVisible(const SystemObjectsGroup& group)
{
    if (group.type == mu::engraving::ElementType::MEASURE_NUMBER) {
        return group.staff && group.staff->shouldShowMeasureNumbers();
    }

    for (const mu::engraving::EngravingItem* item : group.items) {
        if (item->visible()) {
            return true;
        }
    }

    return false;
}

inline muse::String translatedSystemObjectsGroupName(const SystemObjectsGroup& group)
{
    const muse::TranslatableString& name = mu::engraving::TConv::userName(group.type);
    const int n = static_cast<int>(group.items.size());

    return name.translated(n);
}

inline muse::String translatedSystemObjectsGroupCapitalizedName(const SystemObjectsGroup& group)
{
    const muse::TranslatableString& name = mu::engraving::TConv::capitalizedUserName(group.type);
    const int n = static_cast<int>(group.items.size());

    return name.translated(n);
}
}
