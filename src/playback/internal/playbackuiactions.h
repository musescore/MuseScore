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
#ifndef MU_PLAYBACK_PLAYBACKUIACTIONS_H
#define MU_PLAYBACK_PLAYBACKUIACTIONS_H

#include "ui/iuiactionsmodule.h"
#include "playbackcontroller.h"
#include "modularity/ioc.h"
#include "context/iuicontextresolver.h"
#include "async/asyncable.h"

namespace mu::playback {
class PlaybackUiActions : public ui::IUiActionsModule, public async::Asyncable
{
    INJECT(playback, context::IUiContextResolver, uicontextResolver)

public:
    PlaybackUiActions(std::shared_ptr<PlaybackController> controller);

    void init();

    const ui::UiActionList& actionsList() const override;

    bool actionEnabled(const ui::UiAction& act) const override;
    async::Channel<actions::ActionCodeList> actionEnabledChanged() const override;

    bool actionChecked(const ui::UiAction& act) const override;
    async::Channel<actions::ActionCodeList> actionCheckedChanged() const override;

    static const ui::UiActionList& settingsActions();
    static const ui::UiActionList& loopBoundaryActions();

private:
    static const ui::UiActionList m_mainActions;
    static const ui::UiActionList m_settingsActions;
    static const ui::UiActionList m_loopBoundaryActions;

    std::shared_ptr<PlaybackController> m_controller;
    async::Channel<actions::ActionCodeList> m_actionEnabledChanged;
    async::Channel<actions::ActionCodeList> m_actionCheckedChanged;
};
}

#endif // MU_PLAYBACK_PLAYBACKUIACTIONS_H
