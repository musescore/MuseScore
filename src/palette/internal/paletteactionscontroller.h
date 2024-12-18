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

#ifndef MU_PALETTE_PALETTEACTIONSCONTROLLER_H
#define MU_PALETTE_PALETTEACTIONSCONTROLLER_H

#include "modularity/ioc.h"
#include "actions/actionable.h"
#include "actions/iactionsdispatcher.h"
#include "ui/iuiactionsregister.h"
#include "async/asyncable.h"
#include "iinteractive.h"
#include "context/iglobalcontext.h"

namespace mu::palette {
class PaletteActionsController : public muse::actions::Actionable, public muse::async::Asyncable
{
    INJECT(muse::actions::IActionsDispatcher, dispatcher)
    INJECT(muse::IInteractive, interactive)
    INJECT(context::IGlobalContext, globalContext)

public:
    void init();

    muse::ValCh<bool> isMasterPaletteOpened() const;

private:
    void toggleMasterPalette(const muse::actions::ActionData& args);
    void toggleSpecialCharactersDialog();
    void openTimeSignaturePropertiesDialog();
    void openCustomizeKitDialog();

    muse::ValCh<bool> m_masterPaletteOpened;
    muse::async::Channel<muse::actions::ActionCodeList> m_actionsReceiveAvailableChanged;
};
}

#endif // MU_PALETTE_PALETTEACTIONSCONTROLLER_H
