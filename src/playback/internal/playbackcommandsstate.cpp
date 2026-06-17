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

#include "playbackcommandsstate.h"

#include "../playbackcommands.h"

using namespace muse;
using namespace muse::rcommand;
using namespace mu::playback;

static const muse::Uri PROJECT_PAGE_URI("musescore://notation");

std::string PlaybackCommandsState::moduleName() const
{
    return "playback";
}

void PlaybackCommandsState::init()
{
    globalContext()->currentProjectChanged().onNotify(this, [this]() {
        updateCommandStates();
    });

    interactive()->opened().onReceive(this, [this](const muse::Uri&) {
        updateCommandStates();
    });

    playbackController()->isPlayAllowedChanged().onNotify(this, [this]() {
        updateCommandStates();
    });

    // playbackController()->isPlayingChanged().onNotify(this, [this]() {
    //     updateCommandStates();
    // });

    m_moduleRegister = commandsRegister()->moduleRegister(moduleName());
    IF_ASSERT_FAILED(m_moduleRegister) {
        return;
    }

    updateCommandStates();
}

void PlaybackCommandsState::deinit()
{
    globalContext()->currentProjectChanged().disconnect(this);
    interactive()->opened().disconnect(this);
    playbackController()->isPlayAllowedChanged().disconnect(this);
    playbackController()->isPlayingChanged().disconnect(this);
}

void PlaybackCommandsState::updateCommandStates()
{
    IF_ASSERT_FAILED(m_moduleRegister) {
        return;
    }
    for (const auto& command : m_moduleRegister->commandList()) {
        CommandState newState = commandState(command);
        if (m_commandStates[command] != newState) {
            m_commandStates[command] = newState;
            m_commandStateChanged.send(command, newState);
        }
    }
}

CommandState PlaybackCommandsState::commandState(const Command& command) const
{
    if (!isProjectOpened()) {
        return CommandState(false, false);
    }

    if (!playbackController()->isPlayAllowed()) {
        return CommandState(false, false);
    }

    if (command == PLAY_COMMAND) {
        return CommandState(true, playbackController()->isPlaying());
    } else if (command == PAUSE_COMMAND) {
        return CommandState(true, false);
    } else if (command == STOP_COMMAND) {
        return CommandState(true, false);
    }

    return CommandState();
}

async::Channel<Command, CommandState> PlaybackCommandsState::commandStateChanged() const
{
    return m_commandStateChanged;
}

bool PlaybackCommandsState::isProjectOpened() const
{
    if (!globalContext()->currentProject()) {
        return false;
    }

    if (!interactive()->isOpened(PROJECT_PAGE_URI).val) {
        return false;
    }

    return true;
}
