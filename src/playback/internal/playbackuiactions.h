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
#ifndef MU_PLAYBACK_PLAYBACKUIACTIONS_H
#define MU_PLAYBACK_PLAYBACKUIACTIONS_H

#include "ui/iuiactionsmodule.h"
#include "playbackcontroller.h"
#include "modularity/ioc.h"
#include "context/iuicontextresolver.h"
#include "async/asyncable.h"
#include "ui/uitypes.h"

namespace mu::playback {
class PlaybackUiActions : public ui::IUiActionsModule, public async::Asyncable
{
    INJECT(context::IUiContextResolver, uicontextResolver)

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

    static const ui::ToolConfig& defaultPlaybackToolConfig();

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
