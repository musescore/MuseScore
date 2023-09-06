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
#include "passlayoutindependeditems.h"

#include "dom/score.h"

#include "layoutcontext.h"
#include "tlayout.h"

using namespace mu::engraving;
using namespace mu::engraving::rendering::dev;

void PassLayoutIndependedItems::doRun(Score* score, LayoutContext& ctx)
{
    RootItem* rootItem = score->rootItem();
    scan(rootItem, ctx);
}

void PassLayoutIndependedItems::scan(EngravingItem* item, LayoutContext& ctx)
{
    if (isItemIndepended(item->type())) {
        TLayout::layoutItem(item, ctx);
    }

    for (EngravingItem* ch : item->childrenItems()) {
        scan(ch, ctx);
    }
}

bool PassLayoutIndependedItems::isItemIndepended(ElementType type) const
{
    switch (type) {
    case ElementType::ACCIDENTAL: return true;
    default:
        break;
    }
    return false;
}
