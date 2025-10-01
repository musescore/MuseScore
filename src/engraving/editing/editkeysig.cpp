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

#include "editkeysig.h"

#include "dom/keysig.h"
#include "dom/score.h"
#include "dom/segment.h"
#include "dom/staff.h"

using namespace mu::engraving;

//---------------------------------------------------------
//   ChangeKeySig
//---------------------------------------------------------

ChangeKeySig::ChangeKeySig(KeySig* k, KeySigEvent newKeySig, bool sc, bool addEvtToStaff)
    : keysig(k), ks(newKeySig), showCourtesy(sc), evtInStaff(addEvtToStaff)
{}

void ChangeKeySig::flip(EditData*)
{
    Segment* segment = keysig->segment();
    Fraction tick = segment->tick();
    Staff* staff = keysig->staff();

    const bool curEvtInStaff = (staff->currentKeyTick(tick) == tick);
    KeySigEvent curKey = keysig->keySigEvent();
    const bool curShowCourtesy = keysig->showCourtesy();

    keysig->setKeySigEvent(ks);
    keysig->setShowCourtesy(showCourtesy);

    // Add/remove the corresponding key events, if appropriate.
    if (evtInStaff) {
        staff->setKey(tick, ks);     // replace
    } else if (curEvtInStaff) {
        staff->removeKey(tick);     // if nothing to add instead, just remove.
    }
    // If no keysig event corresponds to the key signature then this keysig
    // is probably generated. Otherwise it is probably added manually.
    // Set segment flags according to this, layout will change it if needed.
    segment->setEnabled(evtInStaff);
    segment->setHeader(!evtInStaff && segment->rtick() == Fraction(0, 1));

    showCourtesy = curShowCourtesy;
    ks           = curKey;
    evtInStaff   = curEvtInStaff;
    keysig->triggerLayout();
    keysig->score()->setLayout(keysig->staff()->nextKeyTick(tick), keysig->staffIdx());
}
