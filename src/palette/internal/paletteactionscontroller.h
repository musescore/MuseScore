//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2020 MuseScore BVBA and others
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
//=============================================================================

#ifndef MU_PALETTE_PALETTEACTIONSCONTROLLER_H
#define MU_PALETTE_PALETTEACTIONSCONTROLLER_H

#include "modularity/ioc.h"
#include "actions/actionable.h"
#include "actions/iactionsdispatcher.h"
#include "actions/iactionsregister.h"
#include "async/asyncable.h"
#include "iinteractive.h"
#include "ipaletteactionscontroller.h"

namespace mu::palette {
class PaletteActionsController : public IPaletteActionsController, public actions::Actionable, public async::Asyncable
{
    INJECT(palette, actions::IActionsDispatcher, dispatcher)
    INJECT(palette, actions::IActionsRegister, actionsRegister)
    INJECT(palette, framework::IInteractive, interactive)

public:
    void init();

    ValCh<bool> isMasterPaletteOpened() const override;

    bool actionAvailable(const actions::ActionCode& actionCode) const override;
    async::Channel<actions::ActionCodeList> actionsAvailableChanged() const override;

private:
    void setupConnections();

    void toggleMasterPalette();

    bool isNotationPage() const;

    async::Channel<bool> m_masterPaletteOpenChannel;
    async::Channel<actions::ActionCodeList> m_actionsReceiveAvailableChanged;
};
}

#endif // MU_PALETTE_PALETTEACTIONSCONTROLLER_H
