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

#include "appshellcommandsstate.h"

#include "../appshellcommands.h"

using namespace muse;
using namespace muse::rcommand;
using namespace mu::appshell;

std::string AppshellCommandsState::moduleName() const
{
    return "appshell";
}

void AppshellCommandsState::init()
{
    m_moduleRegister = commandsRegister()->moduleRegister(moduleName());
    IF_ASSERT_FAILED(m_moduleRegister) {
        return;
    }

    mainWindow()->isFullScreenChanged().onNotify(this, [this]() {
        updateCommandStates({ APP_FULLSCREEN_COMMAND });
    });

    updateCommandStates();
}

void AppshellCommandsState::deinit()
{
    mainWindow()->isFullScreenChanged().disconnect(this);
}

void AppshellCommandsState::updateCommandStates(const std::vector<Command>& commands)
{
    IF_ASSERT_FAILED(m_moduleRegister) {
        return;
    }

    const auto& commandList = commands.empty() ? m_moduleRegister->commandList() : commands;

    for (const auto& command : commandList) {
        CommandState newState = commandState(command);
        if (m_commandStates[command] != newState) {
            m_commandStates[command] = newState;
            m_commandStateChanged.send(command, newState);
        }
    }
}

CommandState AppshellCommandsState::commandState(const Command& command) const
{
    if (command == APP_FULLSCREEN_COMMAND) {
        return CommandState(true, mainWindow()->isFullScreen());
    }

    return CommandState(true, false);
}

async::Channel<Command, CommandState> AppshellCommandsState::commandStateChanged() const
{
    return m_commandStateChanged;
}
