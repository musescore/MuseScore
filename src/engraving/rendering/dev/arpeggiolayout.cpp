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
#include "arpeggiolayout.h"

#include "dom/arpeggio.h"

#include "tlayout.h"

using namespace mu::engraving;
using namespace mu::engraving::rendering::dev;

//   layoutArpeggio2
//    called after layout of page

void ArpeggioLayout::layoutArpeggio2(Arpeggio* item, LayoutContext& ctx)
{
    if (!item || item->span() < 2) {
        return;
    }
    TLayout::layout(item, item->mutLayoutData(), ctx.conf(), /*includeCrossStaffHeight = */ true);
}

void ArpeggioLayout::layoutOnEditDrag(Arpeggio* item, LayoutContext& ctx)
{
    TLayout::layout(item, item->mutLayoutData(), ctx.conf());
}

void ArpeggioLayout::layoutOnEdit(Arpeggio* item, LayoutContext& ctx)
{
    Arpeggio::LayoutData* ldata = item->mutLayoutData();
    TLayout::layout(item, ldata, ctx.conf(), true);

    ldata->setPosX(-(ldata->bbox().width() + item->spatium() * .5));

    Fraction tick = item->tick();

    ctx.setLayout(tick, tick, item->staffIdx(), item->staffIdx() + item->span(), item);
}
