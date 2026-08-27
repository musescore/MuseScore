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

#include "appshelltypes.h"
#include "global/containers.h"

#include "dockwindow/idockwindow.h"

#include "../appshellcommands.h"
#include <vector>

using namespace muse;
using namespace muse::rcommand;
using namespace muse::dock;
using namespace mu::appshell;

static const muse::Uri PROJECT_PAGE_URI("musescore://notation");

static const std::vector<Command> PROJECT_PAGE_COMMANDS = {
    DOCK_TOGGLE_PLAYBACK_COMMAND,
    DOCK_TOGGLE_NOTEINPUT_COMMAND,
    DOCK_TOGGLE_PALETTES_COMMAND,
    DOCK_TOGGLE_INSTRUMENTS_COMMAND,
    DOCK_TOGGLE_PROPERTIES_COMMAND,
    DOCK_TOGGLE_SELECTION_FILTER_COMMAND,
    DOCK_TOGGLE_UNDO_HISTORY_COMMAND,
    DOCK_TOGGLE_NAVIGATOR_COMMAND,
    DOCK_TOGGLE_BRAILLE_COMMAND,
    DOCK_TOGGLE_TIMELINE_COMMAND,
    DOCK_TOGGLE_MIXER_COMMAND,
    DOCK_TOGGLE_PIANO_KEYBOARD_COMMAND,
    DOCK_TOGGLE_PERCUSSION_COMMAND,
    DOCK_TOGGLE_STATUSBAR_COMMAND,
};

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

    appShellState()->isNotationNavigatorVisibleChanged().onNotify(this, [this]() {
        updateCommandStates({ DOCK_TOGGLE_NAVIGATOR_COMMAND });
    });

    brailleConfiguration()->braillePanelEnabledChanged().onNotify(this, [this]() {
        updateCommandStates({ DOCK_TOGGLE_BRAILLE_COMMAND });
    });

    dockWindowProvider()->windowChanged().onNotify(this, [this]() {
        const IDockWindow* window = dockWindowProvider()->window();
        if (!window) {
            return;
        }

        window->docksOpenStatusChanged().onReceive(this, [this](const QStringList& dockNames) {
            std::vector<Command> commands;
            commands.reserve(dockNames.size());
            for (const QString& dockName : dockNames) {
                Command cmd = commandsController()->dockToggleCommand(dockName);
                if (cmd.isValid()) {
                    commands.push_back(std::move(cmd));
                }
            }

            updateCommandStates({ commands });
        });
    });

    interactive()->currentUri().ch.onReceive(this, [this](const muse::Uri&) {
        updateCommandStates(PROJECT_PAGE_COMMANDS);
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

    if (muse::contains(PROJECT_PAGE_COMMANDS, command)) {
        if (!isProjectPage()) {
            return CommandState(false, false);
        }
    }

    if (command == DOCK_TOGGLE_NAVIGATOR_COMMAND) {
        return CommandState(true, appShellState()->isNotationNavigatorVisible());
    }

    if (command == DOCK_TOGGLE_BRAILLE_COMMAND) {
        return CommandState(true, brailleConfiguration()->braillePanelEnabled());
    }

    const DockName dockName = commandsController()->commandDockName(command);
    if (!dockName.isEmpty()) {
        const IDockWindow* window = dockWindowProvider()->window();
        bool opened = window ? window->isDockOpen(dockName) : false;
        return CommandState(true, opened);
    }

    return CommandState(true, false);
}

async::Channel<Command, CommandState> AppshellCommandsState::commandStateChanged() const
{
    return m_commandStateChanged;
}

bool AppshellCommandsState::isProjectPage() const
{
    if (!interactive()) {
        return false;
    }

    return interactive()->isOpened(PROJECT_PAGE_URI).val;
}
