/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-CLA-applies
 *
 * MuseScore
 * Music Composition & Notation
 *
 * Copyright (C) 2021 MuseScore BVBA and others
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

#include "libmscore/arpeggio.h"
#include "libmscore/chord.h"
#include "libmscore/note.h"
#include "libmscore/segment.h"

#include "tlayout.h"

using namespace mu::engraving;
using namespace mu::engraving::v0;

//   layoutArpeggio2
//    called after layout of page

void ArpeggioLayout::layoutArpeggio2(Arpeggio* item, LayoutContext& ctx)
{
    if (!item || item->span() < 2) {
        return;
    }
    computeHeight(item, /*includeCrossStaffHeight = */ true);
    TLayout::layout(item, ctx);
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
