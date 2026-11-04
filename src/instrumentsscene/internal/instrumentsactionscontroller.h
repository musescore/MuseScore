/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2021 MuseScore Limited and others
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

#pragma once

#include "global/async/asyncable.h"

#include "modularity/ioc.h"
#include "rcommand/commandable.h"
#include "rcommand/icommanddispatcher.h"
#include "notationscene/iselectinstrumentscenario.h"
#include "project/iprojectsession.h"
#include "interactive/iinteractive.h"

namespace mu::instrumentsscene {
class InstrumentsActionsController : public muse::rcommand::Commandable, public muse::async::Asyncable, public muse::Contextable
{
    muse::ContextInject<muse::rcommand::ICommandDispatcher> commandDispatcher = { this };
    muse::ContextInject<notation::ISelectInstrumentsScenario> selectInstrumentsScenario = { this };
    muse::ContextInject<project::IProjectSession> context = { this };
    muse::ContextInject<muse::IInteractive> interactive = { this };

public:

    InstrumentsActionsController(const muse::modularity::ContextPtr& iocCtx)
        : muse::Contextable(iocCtx)
    {
    }

    virtual ~InstrumentsActionsController() = default;

    void init();

private:
    void selectInstruments();
    void changeInstrument();
};
}
