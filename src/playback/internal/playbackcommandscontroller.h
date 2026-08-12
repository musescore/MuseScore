/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) MuseScore Limited and others
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

#include <functional>

#include "iplaybackcommandscontroller.h"

#include "global/types/ret.h"

#include "global/modularity/ioc.h"
#include "actions/actionable.h"
#include "actions/iactionsdispatcher.h"
#include "rcommand/commandable.h"
#include "rcommand/commandtypes.h"
#include "rcommand/icommanddispatcher.h"
#include "rcommand/icommandsstate.h"
#include "interactive/iinteractive.h"
#include "../iplaybackcontroller.h"
#include "../iplaybackconfiguration.h"

namespace mu::playback {
class PlaybackCommandsController : public IPlaybackCommandsController, public muse::Contextable, public muse::actions::Actionable,
    public muse::rcommand::Commandable
{
    muse::GlobalInject<IPlaybackConfiguration> configuration;
    muse::ContextInject<muse::actions::IActionsDispatcher> actionsDispatcher = { this };
    muse::ContextInject<muse::rcommand::ICommandDispatcher> dispatcher = { this };
    muse::ContextInject<muse::rcommand::ICommandsState> commandsState = { this };
    muse::ContextInject<IPlaybackController> playbackController = { this };
    muse::ContextInject<muse::IInteractive> interactive = { this };
public:
    PlaybackCommandsController(const muse::modularity::ContextPtr& ctx)
        : muse::Contextable(ctx) {}

    void init();

private:

    muse::Ret rewind(const muse::rcommand::CommandQuery& query);
    muse::Ret showPlaybackSetup();

    muse::Ret toggleMixerSection(const muse::rcommand::CommandQuery& query);
    muse::Ret toggleAuxSend(const muse::rcommand::CommandQuery& query);
    muse::Ret toggleAuxChannel(const muse::rcommand::CommandQuery& query);

    void registerCommand(const muse::rcommand::Command&, const std::function<muse::Ret()>&);
    void registerCommand(const muse::rcommand::Command&, const std::function<muse::Ret(const muse::rcommand::CommandQuery&)>&);

    void registerCommand(const muse::rcommand::Command&, muse::Ret (IPlaybackController::*)());
    template<typename P1>
    void registerCommand(const muse::rcommand::Command&, muse::Ret (IPlaybackController::*)(P1), P1);
};
}
