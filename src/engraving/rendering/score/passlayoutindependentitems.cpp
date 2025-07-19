/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2023 MuseScore Limited
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
#include "passlayoutindependentitems.h"

#include "dom/score.h"

#include "tlayout.h"

using namespace mu::engraving;
using namespace mu::engraving::rendering::score;

void PassLayoutIndependentItems::doRun(Score* score, LayoutContext& ctx)
{
    RootItem* rootItem = score->rootItem();
    scan(rootItem, ctx);
}

void PassLayoutIndependentItems::scan(EngravingItem* item, LayoutContext& ctx)
{
    //! NOTE These items are independent
    switch (item->type()) {
    case ElementType::ACCIDENTAL:
    case ElementType::ACTION_ICON:
    case ElementType::AMBITUS:
    case ElementType::BREATH:
    case ElementType::CLEF:
    case ElementType::DEAD_SLAPPED:
    case ElementType::HARMONY:
    case ElementType::HOOK:
    case ElementType::INSTRUMENT_NAME:
    case ElementType::KEYSIG:
    case ElementType::LAYOUT_BREAK:
    case ElementType::NOTE:
    case ElementType::NOTEDOT:
    case ElementType::STAFF_STATE:
    case ElementType::STAFFTYPE_CHANGE:
    case ElementType::STEM:
    case ElementType::SYMBOL:
    case ElementType::FSYMBOL:
    case ElementType::SYSTEM_DIVIDER:
    case ElementType::TIMESIG:
    case ElementType::TREMOLOBAR:
        TLayout::layoutItem(item, ctx);
    default:
        break;
    }

    for (EngravingItem* ch : item->childrenItems()) {
        if (ch->isType(ElementType::DUMMY)) {
            continue;
        }
        scan(ch, ctx);
    }
}
