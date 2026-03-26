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

#include "editinstrumentchange.h"

#include "../dom/instrchange.h"
#include "../dom/instrument.h"
#include "../dom/masterscore.h"
#include "../dom/part.h"
#include "../dom/score.h"
#include "../dom/segment.h"
#include "../dom/staff.h"

using namespace mu::engraving;

//---------------------------------------------------------
//   ChangeInstrument
//---------------------------------------------------------

void ChangeInstrument::flip(EditData*)
{
    Instrument curInstrument = *is->instrument();

    // Modify the shared instrument in-place. Since IC is non-owning,
    // its pointer is the same as Part's InstrumentList entry, so both
    // are updated by this single call. No need to call part->setInstrument
    // which would create a redundant copy and orphan the shared pointer.
    is->setInstrument(instrument);

    // Only rebuild MIDI mapping from master score to avoid race conditions
    // when excerpts are processed in parallel (e.g., during save)
    if (is->score()->isMaster()) {
        is->masterScore()->rebuildMidiMapping();
        is->masterScore()->updateChannel();
    }
    is->score()->setInstrumentsChanged(true);
    is->triggerLayoutAll();

    instrument = std::move(curInstrument);
}
