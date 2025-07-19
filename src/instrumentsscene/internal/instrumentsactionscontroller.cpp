/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2021 MuseScore Limited
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
using namespace muse;
using namespace muse::actions;

void InstrumentsActionsController::init()
{
    dispatcher()->reg(this, "instruments", this, &InstrumentsActionsController::selectInstruments);
    dispatcher()->reg(this, "change-instrument", this, &InstrumentsActionsController::changeInstrument);
}

bool InstrumentsActionsController::canReceiveAction(const ActionCode&) const
{
    return context()->currentMasterNotation() != nullptr;
}

void InstrumentsActionsController::selectInstruments()
{
    IMasterNotationPtr master = context()->currentMasterNotation();
    IF_ASSERT_FAILED(master) {
        return;
    }

    async::Promise<PartInstrumentListScoreOrder> selectedInstruments = selectInstrumentsScenario()->selectInstruments();
    selectedInstruments.onResolve(this, [master](const PartInstrumentListScoreOrder& sel) {
        master->parts()->setParts(sel.instruments, sel.scoreOrder);
    });
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

    async::Promise<InstrumentTemplate> templ = selectInstrumentsScenario()->selectInstrument(key);
    templ.onResolve(this, [master, key](const InstrumentTemplate& val) {
        Instrument instrument = Instrument::fromTemplate(&val);
        master->parts()->replaceInstrument(key, instrument);
    });
}
