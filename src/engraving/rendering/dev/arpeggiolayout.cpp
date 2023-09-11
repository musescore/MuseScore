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
#include "dom/chord.h"
#include "dom/note.h"
#include "dom/segment.h"
#include "dom/staff.h"

#include "tlayout.h"

using namespace mu::engraving;
using namespace mu::engraving::rendering::dev;

void ArpeggioLayout::layout(const Arpeggio* item, const LayoutContext& ctx, Arpeggio::LayoutData* ldata)
{
}

//   layoutArpeggio2
//    called after layout of page

void ArpeggioLayout::layoutArpeggio2(Arpeggio* item, LayoutContext& ctx)
{
    if (!item || item->span() < 2) {
        return;
    }
    computeHeight(item, /*includeCrossStaffHeight = */ true);
    TLayout::layout(item, item->mutLayoutData(), ctx.conf());
}

void ArpeggioLayout::computeHeight(Arpeggio* item, bool includeCrossStaffHeight)
{
    Chord* topChord = item->chord();
    if (!topChord) {
        return;
    }
    double y = topChord->upNote()->pagePos().y() - topChord->upNote()->headHeight() * .5;

    Note* bottomNote = topChord->downNote();
    if (includeCrossStaffHeight) {
        track_idx_t bottomTrack = item->track() + (item->span() - 1) * VOICES;
        EngravingItem* element = topChord->segment()->element(bottomTrack);
        Chord* bottomChord = (element && element->isChord()) ? toChord(element) : topChord;
        bottomNote = bottomChord->downNote();
    }

    double h = bottomNote->pagePos().y() + bottomNote->headHeight() * .5 - y;
    item->setHeight(h);
}

void ArpeggioLayout::layoutOnEditDrag(Arpeggio* item, LayoutContext& ctx)
{
    ArpeggioLayout::layout(item, ctx, item->mutLayoutData());
}

void ArpeggioLayout::layoutOnEdit(Arpeggio* item, LayoutContext& ctx)
{
    Arpeggio::LayoutData* ldata = item->mutLayoutData();
    ArpeggioLayout::layout(item, ctx, ldata);

    Chord* c = item->chord();
    ldata->setPosX(-(item->width() + item->spatium() * .5));

    layoutArpeggio2(c->arpeggio(), ctx);
    Fraction _tick = item->tick();

    ctx.setLayout(_tick, _tick, item->staffIdx(), item->staffIdx() + item->span(), item);
}
