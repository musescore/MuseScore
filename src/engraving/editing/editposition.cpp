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

    // Some items are laid out relative to the positions (including offsets) of other items
    // We can't know whether elements affect each other's position without performing another layout
    // Setting one item's offset can therefore move another item's layout position, so lay out and update offsets
    // until they stop changing. A chain of N dependent items needs roughly N + 1 passes.
    std::unordered_map<EngravingItem*, PointF> originalOffsets;
    for (const auto& [item, oldPos] : frozenItems) {
        originalOffsets.emplace(item, item->offset());
    }

    constexpr size_t MAX_PASSES = 20;
    bool converged = false;
    for (size_t pass = 0; pass < MAX_PASSES && !converged; ++pass) {
        score->update();

        converged = true;
        for (const auto& [item, oldPos] : frozenItems) {
            const PointF newOffset = oldPos - item->ldata()->pos();
            const PointF& currentOffset = item->offset();
            if (newOffset == currentOffset) {
                continue;
            }

            converged = false;
            item->setOffset(newOffset);
            item->triggerLayout();
        }
    }

    if (!converged) {
        LOGW() << "Frozen item offsets did not converge after " << MAX_PASSES << " layout passes";
    }

    // Make actual undoable change. Using setOffset above avoids having N + 1 commands for affected elements in the transaction
    for (const auto& [item, originalOffset] : originalOffsets) {
        const PointF frozenOffset = item->offset();
        // Restore the original value so undoChangeProperty registers the change for undo
        item->setOffset(originalOffset);

        PropertyFlags offsetPf = item->propertyFlags(Pid::OFFSET);
        if (offsetPf == PropertyFlags::STYLED) {
            offsetPf = PropertyFlags::UNSTYLED;
        }

        item->undoChangeProperty(Pid::OFFSET, frozenOffset, offsetPf);
    }
}
