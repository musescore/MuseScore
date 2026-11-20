/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2026 MuseScore Limited
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

#include "editdynamic.h"

#include "../dom/dynamic.h"
#include "../dom/score.h"

#include "edithairpin.h"

using namespace mu::engraving;

void EditDynamic::increaseDecreaseSelectedDynamicsValues(Transaction& tx, Score* score, int delta)
{
    const auto inRange = [](DynamicType type) {
        return type >= DynamicType::PPPPP && type <= DynamicType::FFFFF;
    };

    for (EngravingItem* item : score->selection().elements()) {
        if (!item->isDynamic()) {
            continue;
        }
        Dynamic* dynamic = toDynamic(item);
        const DynamicType newType = static_cast<DynamicType>(static_cast<int>(dynamic->dynamicType()) + delta);
        if (!inRange(newType)) {
            continue;
        }
        dynamic->undoChangeProperty(Pid::DYNAMIC_TYPE, newType);
        dynamic->undoChangeProperty(Pid::TEXT, Dynamic::dynamicText(newType));
        EditHairpin::autoFlipHairpinsType(tx, dynamic);
    }
}
