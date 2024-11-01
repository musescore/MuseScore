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

#include "engraving/dom/score.h"
#include "engraving/dom/measure.h"
#include "engraving/dom/staff.h"

namespace mu::instrumentsscene {
struct SystemObjectsGroup {
    mu::engraving::ElementType type = mu::engraving::ElementType::INVALID;
    mu::engraving::TranslatableString name;
    std::vector<mu::engraving::EngravingItem*> items;
};

inline std::vector<SystemObjectsGroup> collectSystemObjectGroups(const mu::engraving::Staff* systemObjectsStaff)
{
    if (!systemObjectsStaff) {
        return {};
    }

    const mu::engraving::Score* score = systemObjectsStaff->score();
    const mu::engraving::Measure* fm = score->firstMeasure();
    if (!fm) {
        return {};
    }

    std::vector<SystemObjectsGroup> result;

    for (const mu::engraving::Segment* seg = fm->first(mu::engraving::SegmentType::ChordRest); seg;
         seg = seg->next1(mu::engraving::SegmentType::ChordRest)) {
        for (mu::engraving::EngravingItem* annotation : seg->annotations()) {
            if (!annotation || annotation->staffIdx() != systemObjectsStaff->idx()) {
                continue;
            }

            auto it = std::find_if(result.begin(), result.end(), [annotation](const SystemObjectsGroup& group) {
                return group.type == annotation->type();
            });

            if (it != result.end()) {
                it->items.push_back(annotation);
            } else {
                result.emplace_back(SystemObjectsGroup { annotation->type(), annotation->typeUserName(), { annotation } });
            }
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
