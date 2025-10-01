/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2025 MuseScore Limited
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

#include "editclef.h"

#include "../dom/clef.h"
#include "../dom/note.h"
#include "../dom/score.h"
#include "../dom/segment.h"
#include "../dom/staff.h"

using namespace mu::engraving;

// compute line position of noteheads after clef change
static void updateNoteLines(Segment* segment, track_idx_t track)
{
    Staff* staff = segment->score()->staff(track / VOICES);
    if (staff->isDrumStaff(segment->tick()) || staff->isTabStaff(segment->tick())) {
        return;
    }
    for (Segment* s = segment->next1(); s; s = s->next1()) {
        if ((s->segmentType() & (SegmentType::Clef | SegmentType::HeaderClef)) && s->element(track) && !s->element(track)->generated()) {
            break;
        }
        if (!s->isChordRestType()) {
            continue;
        }
        for (track_idx_t t = track; t < track + VOICES; ++t) {
            EngravingItem* e = s->element(t);
            if (e && e->isChord()) {
                Chord* chord = toChord(e);
                for (Note* n : chord->notes()) {
                    n->updateLine();
                }
                chord->sortNotes();
                for (Chord* gc : chord->graceNotes()) {
                    for (Note* gn : gc->notes()) {
                        gn->updateLine();
                    }
                    gc->sortNotes();
                }
            }
        }
    }
}

//---------------------------------------------------------
//   ChangeClefType
//---------------------------------------------------------

ChangeClefType::ChangeClefType(Clef* c, ClefType cl, ClefType tc)
{
    clef            = c;
    concertClef     = cl;
    transposingClef = tc;
}

void ChangeClefType::flip(EditData*)
{
    ClefType ocl = clef->concertClef();
    ClefType otc = clef->transposingClef();

    clef->setConcertClef(concertClef);
    clef->setTransposingClef(transposingClef);

    clef->staff()->setClef(clef);
    Segment* segment = clef->segment();
    updateNoteLines(segment, clef->track());
    clef->triggerLayoutAll(); // TODO: reduce layout to clef range

    concertClef     = ocl;
    transposingClef = otc;
}
