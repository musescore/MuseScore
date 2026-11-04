/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2026 MuseScore Limited and others
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 3 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <https://www.gnu.org/licenses/>.
 */

#include "editposition.h"

#include "../dom/engravingitem.h"
#include "../dom/score.h"

#include "../rendering/iscorerenderer.h"

using namespace mu::engraving;

void EditPosition::freezeItemsPositions(Transaction&, const std::vector<EngravingItem*>& items)
{
    if (items.empty()) {
        return;
    }
    Score* score = items.front()->score();

    std::unordered_map<EngravingItem*, PointF> frozenItems;
    for (EngravingItem* item : items) {
        if (item->generated() || !autoplaceAppliesToType(item->type()) || !item->autoplace()) {
            continue;
        }

        frozenItems.emplace(item, item->pos());

        PropertyFlags autoplacePf = item->propertyFlags(Pid::AUTOPLACE);
        if (autoplacePf == PropertyFlags::STYLED) {
            autoplacePf = PropertyFlags::UNSTYLED;
        }

        item->undoChangeProperty(Pid::AUTOPLACE, false, autoplacePf);

        item->triggerLayout();
    }

    if (frozenItems.empty()) {
        return;
    }

    score->update();

    for (auto& [item, oldPos] : frozenItems) {
        PropertyFlags offsetPf = item->propertyFlags(Pid::OFFSET);
        if (offsetPf == PropertyFlags::STYLED) {
            offsetPf = PropertyFlags::UNSTYLED;
        }

        item->undoChangeProperty(Pid::OFFSET, oldPos - item->ldata()->pos(), offsetPf);
    }
}
