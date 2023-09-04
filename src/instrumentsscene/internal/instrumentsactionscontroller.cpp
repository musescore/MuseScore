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

#include "instrumentsactionscontroller.h"

#include "engraving/dom/instrchange.h"

using namespace mu::instrumentsscene;
using namespace mu::notation;

void InstrumentsActionsController::init()
{
    dispatcher()->reg(this, "instruments", this, &InstrumentsActionsController::selectInstruments);
    dispatcher()->reg(this, "change-instrument", this, &InstrumentsActionsController::changeInstrument);
}

bool InstrumentsActionsController::canReceiveAction(const actions::ActionCode&) const
{
    return context()->currentMasterNotation() != nullptr;
}

void InstrumentsActionsController::selectInstruments()
{
    IMasterNotationPtr master = context()->currentMasterNotation();
    IF_ASSERT_FAILED(master) {
        return;
    }

    RetVal<PartInstrumentListScoreOrder> selectedInstruments = selectInstrumentsScenario()->selectInstruments();
    if (!selectedInstruments.ret) {
        LOGE() << selectedInstruments.ret.toString();
        return;
    }

    master->parts()->setParts(selectedInstruments.val.instruments, selectedInstruments.val.scoreOrder);
}

void InstrumentsActionsController::changeInstrument()
{
    IMasterNotationPtr master = context()->currentMasterNotation();
    IF_ASSERT_FAILED(master) {
        return;
    }

    const mu::engraving::EngravingItem* element = master->notation()->interaction()->selection()->element();
    const mu::engraving::InstrumentChange* instrumentChange = element ? mu::engraving::toInstrumentChange(element) : nullptr;
    if (!instrumentChange) {
        return;
    }

    InstrumentKey key;
    key.instrumentId = instrumentChange->instrument()->id();
    key.partId = instrumentChange->part()->id();
    key.tick = instrumentChange->tick();

    RetVal<Instrument> instrument = selectInstrumentsScenario()->selectInstrument(key);
    if (!instrument.ret) {
        LOGE() << instrument.ret.toString();
        return;
    }

    master->parts()->replaceInstrument(key, instrument.val);
}
