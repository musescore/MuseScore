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

#ifndef MU_PALETTE_PALETTEACTIONSCONTROLLER_H
#define MU_PALETTE_PALETTEACTIONSCONTROLLER_H

#include "ipalettecommandscontroller.h"

#include "global/types/ret.h"

#include "modularity/ioc.h"
#include "actions/actionable.h"
#include "actions/iactionsdispatcher.h"
#include "rcommand/commandable.h"
#include "rcommand/icommanddispatcher.h"
#include "async/asyncable.h"
#include "interactive/iinteractive.h"
#include "context/iglobalcontext.h"
#include "ipaletteconfiguration.h"

namespace mu::palette {
class PaletteActionsController : public IPaletteCommandsController, public muse::actions::Actionable, public muse::async::Asyncable,
    public muse::Contextable, public muse::rcommand::Commandable
{
    muse::GlobalInject<IPaletteConfiguration> configuration;
    muse::ContextInject<muse::actions::IActionsDispatcher> dispatcher = { this };
    muse::ContextInject<muse::rcommand::ICommandDispatcher> commandDispatcher = { this };
    muse::ContextInject<muse::IInteractive> interactive = { this };
    muse::ContextInject<context::IGlobalContext> globalContext = { this };

public:
    PaletteActionsController(const muse::modularity::ContextPtr& iocCtx)
        : muse::Contextable(iocCtx)
    {
    }

    void init();

    muse::ValCh<bool> isMasterPaletteOpened() const;

    muse::async::Notification paletteSearchRequested() const override;
    muse::async::Notification applyCurrentPaletteElementRequested() const override;
    muse::async::Channel<bool /* expand */> expandCollapseAllRequested() const override;

private:
    muse::Ret toggleMasterPalette(const muse::rcommand::CommandQuery& query);
    void toggleSpecialCharactersDialog();
    void openTimeSignaturePropertiesDialog();
    void openCustomizeKitDialog();

    muse::Ret toggleSingleClickToOpen();
    muse::Ret toggleSinglePalette();
    muse::Ret toggleDragEnabled();

    notation::INotationInteractionPtr notationInteraction() const;

    muse::ValCh<bool> m_masterPaletteOpened;
    muse::async::Channel<muse::actions::ActionCodeList> m_actionsReceiveAvailableChanged;

    muse::async::Notification m_paletteSearchRequested;
    muse::async::Notification m_applyElementRequested;
    muse::async::Channel<bool /* expand */> m_expandCollapseAllRequested;
};
}

#endif // MU_PALETTE_PALETTEACTIONSCONTROLLER_H
