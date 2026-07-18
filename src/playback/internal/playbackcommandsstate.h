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

#include <map>

#include "rcommand/imodulecommandsstate.h"
#include "global/async/asyncable.h"

#include "global/modularity/ioc.h"
#include "interactive/iinteractive.h"
#include "context/iglobalcontext.h"
#include "../iplaybackcontroller.h"
#include "rcommand/icommandsregister.h"
#include "notation/inotationconfiguration.h"
#include "../iplaybackconfiguration.h"

namespace mu::playback {
class PlaybackCommandsState : public muse::rcommand::IModuleCommandsState, public muse::Contextable, public muse::async::Asyncable
{
    muse::GlobalInject<muse::rcommand::ICommandsRegister> commandsRegister;
    muse::GlobalInject<notation::INotationConfiguration> notationConfiguration;
    muse::GlobalInject<IPlaybackConfiguration> playbackConfiguration;
    muse::ContextInject<muse::IInteractive> interactive = { this };
    muse::ContextInject<context::IGlobalContext> globalContext = { this };
    muse::ContextInject<IPlaybackController> playbackController = { this };

public:
    PlaybackCommandsState(const muse::modularity::ContextPtr& ctx)
        : muse::Contextable(ctx) {}

    std::string moduleName() const override;

    void init() override;
    void deinit() override;

    muse::rcommand::CommandState commandState(const muse::rcommand::Command& command) const override;
    muse::async::Channel<muse::rcommand::Command, muse::rcommand::CommandState> commandStateChanged() const override;

private:

    bool isProjectOpened() const;
    void updateCommandStates(const std::vector<muse::rcommand::Command>& commands = {});

    muse::rcommand::IModuleCommandsRegisterPtr m_moduleRegister;
    std::map<muse::rcommand::Command, muse::rcommand::CommandState> m_commandStates;
    muse::async::Channel<muse::rcommand::Command, muse::rcommand::CommandState> m_commandStateChanged;
};
}
