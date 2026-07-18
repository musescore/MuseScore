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

#include "notation/inotationinteraction.h"

#include "../playbackcommands.h"

using namespace muse;
using namespace muse::rcommand;
using namespace mu::playback;
using namespace mu::notation;

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

    globalContext()->currentNotationChanged().onNotify(this, [this]() {
        const INotationPtr currNotation = globalContext()->currentNotation();
        if (!currNotation) {
            return;
        }

        const INotationInteractionPtr interaction = currNotation->interaction();
        interaction->selectionChanged().onNotify(this, [this]() {
            updateCommandStates({ PLAY_SELECTION_COMMAND });
        }, Asyncable::Mode::SetReplace);

        interaction->isEditingElementChanged().onNotify(this, [this]() {
            updateCommandStates({ PLAY_SELECTION_COMMAND });
        }, Asyncable::Mode::SetReplace);
    });

    //! NOTE: IInteractive is not available in console mode
    if (interactive()) {
        interactive()->opened().onReceive(this, [this](const muse::Uri&) {
            updateCommandStates();
        });
    }

    playbackController()->isPlayAllowedChanged().onReceive(this, [this](bool) {
        updateCommandStates();
    });

    playbackController()->isPlayingChanged().onReceive(this, [this](bool) {
        updateCommandStates();
    });

    playbackController()->loopEnabledChanged().onReceive(this, [this](bool) {
        updateCommandStates({ LOOP_TOGGLE_COMMAND });
    });

    playbackController()->onlineSoundsChanged().onNotify(this, [this]() {
        updateCommandStates({ CLEAR_ONLINESOUNDS_CACHE_COMMAND });
    });

    notationConfiguration()->isMetronomeEnabledChanged().onNotify(this, [this]() {
        updateCommandStates({ METRONOME_TOGGLE_COMMAND });
    });

    notationConfiguration()->isMidiInputEnabledChanged().onNotify(this, [this]() {
        updateCommandStates({ MIDI_TOGGLE_COMMAND });
    });

    notationConfiguration()->midiUseWrittenPitch().ch.onReceive(this, [this](bool) {
        updateCommandStates({ MIDI_INPUT_WRITTEN_PITCH_COMMAND, MIDI_INPUT_SOUNDING_PITCH_COMMAND });
    });

    notationConfiguration()->isPlayRepeatsChanged().onNotify(this, [this]() {
        updateCommandStates({ REPEATS_TOGGLE_COMMAND });
    });

    notationConfiguration()->isPlayChordSymbolsChanged().onNotify(this, [this]() {
        updateCommandStates({ CHORDSYMBOLS_TOGGLE_COMMAND });
    });

    notationConfiguration()->isAutomaticallyPanEnabledChanged().onNotify(this, [this]() {
        updateCommandStates({ PAN_TOGGLE_COMMAND });
    });

    notationConfiguration()->isCountInEnabledChanged().onNotify(this, [this]() {
        updateCommandStates({ COUNTIN_TOGGLE_COMMAND });
    });

    playbackConfiguration()->playNotesWhenEditingChanged().onNotify(this, [this]() {
        updateCommandStates({ HEAR_PLAYBACK_WHEN_EDITING_TOGGLE_COMMAND });
    });

    m_moduleRegister = commandsRegister()->moduleRegister(moduleName());
    IF_ASSERT_FAILED(m_moduleRegister) {
        return;
    }

    updateCommandStates();
}

void PlaybackCommandsState::deinit()
{
    globalContext()->currentProjectChanged().disconnect(this);
    globalContext()->currentNotationChanged().disconnect(this);
    if (interactive()) {
        interactive()->opened().disconnect(this);
    }
    playbackController()->isPlayAllowedChanged().disconnect(this);
    playbackController()->isPlayingChanged().disconnect(this);
    playbackController()->loopEnabledChanged().disconnect(this);
    playbackController()->onlineSoundsChanged().disconnect(this);
    playbackConfiguration()->playNotesWhenEditingChanged().disconnect(this);
    notationConfiguration()->isMetronomeEnabledChanged().disconnect(this);
    notationConfiguration()->isMidiInputEnabledChanged().disconnect(this);
    notationConfiguration()->midiUseWrittenPitch().ch.disconnect(this);
    notationConfiguration()->isPlayRepeatsChanged().disconnect(this);
    notationConfiguration()->isPlayChordSymbolsChanged().disconnect(this);
    notationConfiguration()->isAutomaticallyPanEnabledChanged().disconnect(this);
    notationConfiguration()->isCountInEnabledChanged().disconnect(this);
}

void PlaybackCommandsState::updateCommandStates(const std::vector<Command>& commands)
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
    } else if (command == PLAY_SELECTION_COMMAND) {
        const INotationPtr currNotation = globalContext()->currentNotation();
        const INotationInteractionPtr interaction = currNotation ? currNotation->interaction() : nullptr;
        bool enabled = interaction && !interaction->isEditingElement();
        return CommandState(enabled, false);
    } else if (command == LOOP_TOGGLE_COMMAND) {
        return CommandState(true, playbackController()->isLoopEnabled());
    } else if (command == METRONOME_TOGGLE_COMMAND) {
        return CommandState(true, notationConfiguration()->isMetronomeEnabled());
    } else if (command == MIDI_TOGGLE_COMMAND) {
        return CommandState(true, notationConfiguration()->isMidiInputEnabled());
    } else if (command == MIDI_INPUT_WRITTEN_PITCH_COMMAND) {
        return CommandState(true, notationConfiguration()->midiUseWrittenPitch().val);
    } else if (command == MIDI_INPUT_SOUNDING_PITCH_COMMAND) {
        return CommandState(true, !notationConfiguration()->midiUseWrittenPitch().val);
    } else if (command == REPEATS_TOGGLE_COMMAND) {
        return CommandState(true, notationConfiguration()->isPlayRepeatsEnabled());
    } else if (command == CHORDSYMBOLS_TOGGLE_COMMAND) {
        return CommandState(true, notationConfiguration()->isPlayChordSymbolsEnabled());
    } else if (command == HEAR_PLAYBACK_WHEN_EDITING_TOGGLE_COMMAND) {
        return CommandState(true, playbackConfiguration()->playNotesWhenEditing());
    } else if (command == PAN_TOGGLE_COMMAND) {
        return CommandState(true, notationConfiguration()->isAutomaticallyPanEnabled());
    } else if (command == COUNTIN_TOGGLE_COMMAND) {
        return CommandState(true, notationConfiguration()->isCountInEnabled());
    } else if (command == CLEAR_ONLINESOUNDS_CACHE_COMMAND) {
        return CommandState(true, !playbackController()->onlineSounds().empty());
    }

    return CommandState(true, false);
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

    //! NOTE: IInteractive is not available in console mode; in that case there is no page to check
    if (interactive() && !interactive()->isOpened(PROJECT_PAGE_URI).val) {
        return false;
    }

    return true;
}
