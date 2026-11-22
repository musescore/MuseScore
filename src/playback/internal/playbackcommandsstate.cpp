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

#include "notation/inotation.h"
#include "notation/inotationinteraction.h"

#include "engraving/types/constants.h"

#include "../playbackcommands.h"

using namespace muse;
using namespace muse::rcommand;
using namespace mu::playback;
using namespace mu::notation;

static const muse::Uri PROJECT_PAGE_URI("musescore://notation");

static const std::vector<Command> MIDI_RECORD_SETTINGS_COMMANDS {
    MIDI_RECORD_QUANTIZATION_4_COMMAND,
    MIDI_RECORD_QUANTIZATION_8_COMMAND,
    MIDI_RECORD_QUANTIZATION_16_COMMAND,
    MIDI_RECORD_QUANTIZATION_32_COMMAND,
    MIDI_RECORD_TUPLETS_TOGGLE_COMMAND,
    MIDI_RECORD_LEGATO_TOGGLE_COMMAND,
    MIDI_RECORD_COUNTIN_0_COMMAND,
    MIDI_RECORD_COUNTIN_1_COMMAND,
    MIDI_RECORD_COUNTIN_2_COMMAND,
    MIDI_RECORD_ONE_STAFF_COMMAND,
    MIDI_RECORD_TWO_STAVES_COMMAND,
};

//! <command, minimum quantization in ticks>
static const std::map<Command, int> MIDI_RECORD_QUANTIZATION_TICKS {
    { MIDI_RECORD_QUANTIZATION_4_COMMAND, mu::engraving::Constants::DIVISION },
    { MIDI_RECORD_QUANTIZATION_8_COMMAND, mu::engraving::Constants::DIVISION / 2 },
    { MIDI_RECORD_QUANTIZATION_16_COMMAND, mu::engraving::Constants::DIVISION / 4 },
    { MIDI_RECORD_QUANTIZATION_32_COMMAND, mu::engraving::Constants::DIVISION / 8 },
};

//! <command, count-in measures>
static const std::map<Command, int> MIDI_RECORD_COUNTIN_MEASURES {
    { MIDI_RECORD_COUNTIN_0_COMMAND, 0 },
    { MIDI_RECORD_COUNTIN_1_COMMAND, 1 },
    { MIDI_RECORD_COUNTIN_2_COMMAND, 2 },
};

std::string PlaybackCommandsState::moduleName() const
{
    return "playback";
}

void PlaybackCommandsState::init()
{
    projectSession()->projectChanged().onNotify(this, [this]() {
        updateCommandStates();
    });

    projectSession()->currentNotationChanged().onNotify(this, [this]() {
        const INotationPtr currNotation = projectSession()->currentNotation();
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

    playbackController()->isMidiRecordingChanged().onNotify(this, [this]() {
        updateCommandStates();
    });

    playbackController()->lastMidiRecordingChanged().onNotify(this, [this]() {
        updateCommandStates({ MIDI_RECORD_REQUANTIZE_COMMAND });
    });

    notationConfiguration()->midiRecordingSettingsChanged().onNotify(this, [this]() {
        updateCommandStates(MIDI_RECORD_SETTINGS_COMMANDS);
    });

    m_moduleRegister = commandsRegister()->moduleRegister(moduleName());
    IF_ASSERT_FAILED(m_moduleRegister) {
        return;
    }

    updateCommandStates();
}

void PlaybackCommandsState::deinit()
{
    projectSession()->projectChanged().disconnect(this);
    projectSession()->currentNotationChanged().disconnect(this);
    if (interactive()) {
        interactive()->opened().disconnect(this);
    }
    playbackController()->isPlayAllowedChanged().disconnect(this);
    playbackController()->isPlayingChanged().disconnect(this);
    playbackController()->loopEnabledChanged().disconnect(this);
    playbackController()->onlineSoundsChanged().disconnect(this);
    playbackConfiguration()->playNotesWhenEditingChanged().disconnect(this);
    playbackController()->isMidiRecordingChanged().disconnect(this);
    playbackController()->lastMidiRecordingChanged().disconnect(this);
    notationConfiguration()->midiRecordingSettingsChanged().disconnect(this);
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

    if (command == MIDI_RECORD_TOGGLE_COMMAND) {
        return CommandState(true, playbackController()->isMidiRecording());
    } else if (command == MIDI_RECORD_REQUANTIZE_COMMAND) {
        return CommandState(playbackController()->canRequantizeLastMidiRecording(), false);
    } else if (command == MIDI_RECORD_SPLIT_POINT_COMMAND) {
        return CommandState(!playbackController()->isMidiRecording(), false);
    } else if (command == MIDI_RECORD_TUPLETS_TOGGLE_COMMAND) {
        return CommandState(true, notationConfiguration()->midiRecordingSearchTuplets());
    } else if (command == MIDI_RECORD_LEGATO_TOGGLE_COMMAND) {
        return CommandState(true, notationConfiguration()->midiRecordingLegato());
    } else if (command == MIDI_RECORD_ONE_STAFF_COMMAND) {
        return CommandState(true, !notationConfiguration()->midiRecordingTwoStaves());
    } else if (command == MIDI_RECORD_TWO_STAVES_COMMAND) {
        return CommandState(true, notationConfiguration()->midiRecordingTwoStaves());
    } else if (auto it = MIDI_RECORD_QUANTIZATION_TICKS.find(command); it != MIDI_RECORD_QUANTIZATION_TICKS.end()) {
        return CommandState(true, notationConfiguration()->midiRecordingMinQuantizationTicks() == it->second);
    } else if (auto it = MIDI_RECORD_COUNTIN_MEASURES.find(command); it != MIDI_RECORD_COUNTIN_MEASURES.end()) {
        return CommandState(true, notationConfiguration()->midiRecordingCountInMeasures() == it->second);
    }

    if (command == PLAY_COMMAND) {
        return CommandState(true, playbackController()->isPlaying());
    } else if (command == PLAY_SELECTION_COMMAND) {
        const INotationPtr currNotation = projectSession()->currentNotation();
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
    if (!projectSession()->project()) {
        return false;
    }

    return true;
}
