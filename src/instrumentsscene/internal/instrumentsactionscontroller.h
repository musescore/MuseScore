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
#ifndef MU_INSTRUMENTSSCENE_INSTRUMENTSACTIONSCONTROLLER_H
#define MU_INSTRUMENTSSCENE_INSTRUMENTSACTIONSCONTROLLER_H

#include "actions/actionable.h"
#include "global/async/asyncable.h"

#include "modularity/ioc.h"
#include "actions/iactionsdispatcher.h"
#include "notation/iselectinstrumentscenario.h"
#include "context/iglobalcontext.h"

namespace mu::instrumentsscene {
class InstrumentsActionsController : public muse::actions::Actionable, public muse::async::Asyncable
{
    INJECT(muse::actions::IActionsDispatcher, dispatcher)
    INJECT(notation::ISelectInstrumentsScenario, selectInstrumentsScenario)
    INJECT(context::IGlobalContext, context)

public:
    virtual ~InstrumentsActionsController() = default;

    bool canReceiveAction(const muse::actions::ActionCode&) const override;

    void init();

private:
    void selectInstruments();
    void changeInstrument();
};
}

#endif // MU_INSTRUMENTSSCENE_INSTRUMENTSACTIONSCONTROLLER_H
