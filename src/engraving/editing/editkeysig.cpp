/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2025 MuseScore Limited and others
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

#include "dom/factory.h"
#include "dom/instrument.h"
#include "dom/keysig.h"
#include "dom/measure.h"
#include "dom/part.h"
#include "dom/score.h"
#include "dom/segment.h"
#include "dom/staff.h"

#include "addremoveelement.h"
#include "transaction/transaction.h"
#include "transpose.h"

#include "log.h"

using namespace mu::engraving;

//---------------------------------------------------------
//   ChangeKeySig
//---------------------------------------------------------

ChangeKeySig::ChangeKeySig(KeySig* k, KeySigEvent newKeySig, bool sc, bool addEvtToStaff)
    : keysig(k), ks(newKeySig), showCourtesy(sc), evtInStaff(addEvtToStaff)
{}

void ChangeKeySig::flip()
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

//---------------------------------------------------------
//   EditKeySig
//---------------------------------------------------------

void EditKeySig::undoChangeKeySig(Transaction& tx, Score* score, Staff* ostaff, const Fraction& tick, KeySigEvent key)
{
    KeySig* lks = 0;
    bool needsUpdate = false;

    for (Staff* staff : ostaff->staffList()) {
        if (staff->isDrumStaff(tick)) {
            continue;
        }

        Score* staffScore = staff->score();
        Measure* measure = staffScore->tick2measure(tick);
        if (!measure) {
            LOGW("measure for tick %d not found!", tick.ticks());
            continue;
        }
        Segment* s   = measure->undoGetSegment(SegmentType::KeySig, tick);

        staff_idx_t staffIdx = staff->idx();
        track_idx_t track    = staffIdx * VOICES;
        KeySig* ks   = toKeySig(s->element(track));

        Interval interval = staff->part()->instrument(tick)->transpose();
        Interval oldStaffInterval = staff->transpose(tick);
        KeySigEvent nkey  = key;
        bool concertPitch = staffScore->style().styleB(Sid::concertPitch);

        if (interval.chromatic && !concertPitch && !nkey.isAtonal()) {
            interval.flip();
            nkey.setKey(Transpose::transposeKey(key.concertKey(), interval, staff->part()->preferSharpFlat()));
            interval.flip();
        }

        score->updateInstrumentChangeTranspositions(key, staff, tick);
        if (ks) {
            ks->undoChangeProperty(Pid::GENERATED, false);
            tx.push(new ChangeKeySig(ks, nkey, ks->showCourtesy()));
        } else {
            KeySig* nks = Factory::createKeySig(s);
            nks->setParent(s);
            nks->setTrack(track);
            nks->setKeySigEvent(nkey);
            staffScore->doUndoAddElement(nks);
            if (lks) {
                tx.push(new Link(lks, nks));
            } else {
                lks = nks;
            }
        }
        if (interval != staff->transpose(tick) || interval != oldStaffInterval) {
            needsUpdate = true;
        }
    }
    if (needsUpdate) {
        Fraction tickEnd = Fraction::fromTicks(ostaff->keyList()->nextKeyTick(tick.ticks()));
        Transpose::transpositionChanged(tx, score, ostaff->part(), ostaff->transpose(tick), tick, tickEnd);
    }
}
