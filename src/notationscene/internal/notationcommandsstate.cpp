/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2026 MuseScore Limited and others
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
 #include "notationcommandsstate.h"

 #include "global/containers.h"

 #include "../notationcommands.h"

using namespace muse;
using namespace muse::rcommand;
using namespace mu::notation;

static const muse::Uri PROJECT_PAGE_URI("musescore://notation");

static const std::vector<Command> HAS_SELECTION_REQUIRED_COMMANDS = {
    CUT_COMMAND,
    COPY_COMMAND,
    DELETE_COMMAND
};

static const std::vector<Command> UNDO_REDO_COMMANDS = {
    UNDO_COMMAND,
    REDO_COMMAND
};

static const std::vector<Command> TEXT_EDITING_COMMANDS = {
    EDIT_NEXT_TEXT_ELEMENT_COMMAND,
    EDIT_PREV_TEXT_ELEMENT_COMMAND,
    EDIT_NEXT_WORD_COMMAND
};

std::string NotationCommandsState::moduleName() const
{
    return "notation";
}

void NotationCommandsState::init()
{
    m_moduleRegister = commandsRegister()->moduleRegister(moduleName());
    IF_ASSERT_FAILED(m_moduleRegister) {
        return;
    }

    globalContext()->currentProjectChanged().onNotify(this, [this]() {
        updateCommandStates();
    });

    interactive()->opened().onReceive(this, [this](const muse::Uri&) {
        updateCommandStates();
    });

    controller()->hasSelectionChanged().onReceive(this, [this](bool) {
        updateCommandStates(HAS_SELECTION_REQUIRED_COMMANDS);
    });

    controller()->stackChanged().onNotify(this, [this]() {
        updateCommandStates(UNDO_REDO_COMMANDS);
    });

    controller()->textEditingChanged().onReceive(this, [this](bool) {
        updateCommandStates(TEXT_EDITING_COMMANDS);
    });

    updateCommandStates();
}

void NotationCommandsState::deinit()
{
    globalContext()->currentProjectChanged().disconnect(this);
    interactive()->opened().disconnect(this);
    controller()->hasSelectionChanged().disconnect(this);
    controller()->stackChanged().disconnect(this);
    controller()->textEditingChanged().disconnect(this);
}

void NotationCommandsState::updateCommandStates(const std::vector<Command>& commands)
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

CommandState NotationCommandsState::commandState(const Command& command) const
{
    if (!isProjectOpened()) {
        return CommandState(false, false);
    }

    if (muse::contains(HAS_SELECTION_REQUIRED_COMMANDS, command)) {
        return CommandState(controller()->hasSelection(), false);
    }

    if (command == UNDO_COMMAND) {
        return CommandState(controller()->canUndo(), false);
    }
    if (command == REDO_COMMAND) {
        return CommandState(controller()->canRedo(), false);
    }

    if (muse::contains(TEXT_EDITING_COMMANDS, command)) {
        return CommandState(controller()->isTextEditing(), false);
    }

    return CommandState(true, false);
}

async::Channel<Command, CommandState> NotationCommandsState::commandStateChanged() const
{
    return m_commandStateChanged;
}

bool NotationCommandsState::isProjectOpened() const
{
    if (!globalContext()->currentProject()) {
        return false;
    }

    if (!interactive()->isOpened(PROJECT_PAGE_URI).val) {
        return false;
    }

    return true;
}
