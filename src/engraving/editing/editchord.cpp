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

#include "editchord.h"

#include "../dom/arpeggio.h"
#include "../dom/chord.h"
#include "../dom/chordrest.h"
#include "../dom/score.h"
#include "../dom/segment.h"
#include "../dom/tremolotwochord.h"

using namespace mu::engraving;

//---------------------------------------------------------
//   ChangeChordStaffMove
//---------------------------------------------------------

ChangeChordStaffMove::ChangeChordStaffMove(ChordRest* cr, int v)
    : chordRest(cr), staffMove(v)
{
}

void ChangeChordStaffMove::flip(EditData*)
{
    int v = chordRest->staffMove();
    staff_idx_t oldStaff = chordRest->vStaffIdx();

    chordRest->setStaffMove(staffMove);
    chordRest->checkStaffMoveValidity();
    chordRest->triggerLayout();
    if (chordRest->vStaffIdx() == oldStaff) {
        return;
    }

    for (EngravingObject* e : chordRest->linkList()) {
        ChordRest* cr = toChordRest(e);
        if (cr == chordRest) {
            continue;
        }
        cr->setStaffMove(staffMove);
        cr->checkStaffMoveValidity();
        cr->triggerLayout();
    }
    staffMove = v;
}

//---------------------------------------------------------
//   SwapCR
//---------------------------------------------------------

void SwapCR::flip(EditData*)
{
    Segment* s1 = cr1->segment();
    Segment* s2 = cr2->segment();
    track_idx_t track = cr1->track();

    if (cr1->isChord() && cr2->isChord() && toChord(cr1)->tremoloTwoChord()
        && (toChord(cr1)->tremoloTwoChord() == toChord(cr2)->tremoloTwoChord())) {
        TremoloTwoChord* t = toChord(cr1)->tremoloTwoChord();
        Chord* c1 = t->chord1();
        Chord* c2 = t->chord2();
        t->setParent(toChord(c2));
        t->setChords(toChord(c2), toChord(c1));
    }

    EngravingItem* cr = s1->element(track);
    s1->setElement(track, s2->element(track));
    s2->setElement(track, cr);
    cr1->score()->setLayout(s1->tick(), cr1->staffIdx(), cr1);
    cr1->score()->setLayout(s2->tick(), cr1->staffIdx(), cr1);
}

//---------------------------------------------------------
//   ChangeSpanArpeggio
//---------------------------------------------------------

void ChangeSpanArpeggio::flip(EditData*)
{
    Arpeggio* f_spanArp = m_chord->spanArpeggio();

    m_chord->setSpanArpeggio(m_spanArpeggio);
    m_spanArpeggio = f_spanArp;
}
