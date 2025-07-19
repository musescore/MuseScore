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
#include "passresetlayoutdata.h"

#include "dom/score.h"

#include "layoutcontext.h"

using namespace mu::engraving;
using namespace mu::engraving::rendering::score;

static void resetLayoutData(EngravingItem* item)
{
    if (item->ldata()) {
        item->mutldata()->reset();
    }

    for (EngravingItem* ch : item->childrenItems()) {
        resetLayoutData(ch);
    }
}

void PassResetLayoutData::doRun(Score* score, LayoutContext& ctx)
{
    if (ctx.state().isLayoutAll()) {
        resetLayoutData(score->rootItem());
    } else {
        MeasureBase* m = ctx.mutState().nextMeasure();
        while (m && m->tick() <= ctx.state().endTick()) {
            resetLayoutData(m);
            m = m->next();
        }
    }
}
