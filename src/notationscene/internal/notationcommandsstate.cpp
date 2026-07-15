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
using namespace mu::engraving;

static const muse::Uri PROJECT_PAGE_URI("musescore://notation");

template<typename Map>
static inline auto commands(const Map& m) -> std::vector<typename Map::key_type>
{
    std::vector<typename Map::key_type> result;
    result.reserve(m.size());
    for (const auto& p : m) {
        result.push_back(p.first);
    }
    return result;
}

static const std::vector<Command> HAS_SELECTION_REQUIRED_COMMANDS = {
    CUT_COMMAND,
    COPY_COMMAND,
    DELETE_COMMAND,
    FLIP_COMMAND,
    FLIP_HORIZONTALLY_COMMAND
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

static const std::map<Command, NoteInputMethod> NOTE_INPUT_COMMANDS = {
    { TOGGLE_NOTE_INPUT_COMMAND, NoteInputMethod::UNKNOWN },
    { TOGGLE_NOTE_INPUT_BY_NOTE_NAME_COMMAND, NoteInputMethod::BY_NOTE_NAME },
    { TOGGLE_NOTE_INPUT_BY_DURATION_COMMAND, NoteInputMethod::BY_DURATION },
    { TOGGLE_NOTE_INPUT_RHYTHM_COMMAND, NoteInputMethod::RHYTHM },
    { TOGGLE_NOTE_INPUT_REPITCH_COMMAND, NoteInputMethod::REPITCH },
    { TOGGLE_NOTE_INPUT_REALTIME_AUTO_COMMAND, NoteInputMethod::REALTIME_AUTO },
    { TOGGLE_NOTE_INPUT_REALTIME_MANUAL_COMMAND, NoteInputMethod::REALTIME_MANUAL },
    { TOGGLE_NOTE_INPUT_TIMEWISE_COMMAND, NoteInputMethod::TIMEWISE }
};

static const std::map<Command, DurationType> DURATION_COMMANDS = {
    { NOTE_LONGA_COMMAND, DurationType::V_LONG },
    { NOTE_BREVE_COMMAND, DurationType::V_BREVE },
    { PAD_NOTE_1_COMMAND, DurationType::V_WHOLE },
    { PAD_NOTE_2_COMMAND, DurationType::V_HALF },
    { PAD_NOTE_4_COMMAND, DurationType::V_QUARTER },
    { PAD_NOTE_8_COMMAND, DurationType::V_EIGHTH },
    { PAD_NOTE_16_COMMAND, DurationType::V_16TH },
    { PAD_NOTE_32_COMMAND, DurationType::V_32ND },
    { PAD_NOTE_64_COMMAND, DurationType::V_64TH },
    { PAD_NOTE_128_COMMAND, DurationType::V_128TH },
    { PAD_NOTE_256_COMMAND, DurationType::V_256TH },
    { PAD_NOTE_512_COMMAND, DurationType::V_512TH },
    { PAD_NOTE_1024_COMMAND, DurationType::V_1024TH }
};

static const std::map<Command, int> DOT_COUNT_COMMANDS = {
    { PAD_DOT_COMMAND, 1 },
    { PAD_DOT2_COMMAND, 2 },
    { PAD_DOT3_COMMAND, 3 },
    { PAD_DOT4_COMMAND, 4 }
};

static const std::map<Command, AccidentalType> ACCIDENTAL_COMMANDS = {
    { TOGGLE_FLAT2_COMMAND, AccidentalType::FLAT2 },
    { TOGGLE_FLAT_COMMAND, AccidentalType::FLAT },
    { TOGGLE_NAT_COMMAND, AccidentalType::NATURAL },
    { TOGGLE_SHARP_COMMAND, AccidentalType::SHARP },
    { TOGGLE_SHARP2_COMMAND, AccidentalType::SHARP2 }
};

static const std::vector<Command> ADD_COMMANDS = {
    ADD_TIE_COMMAND,
    ADD_SLUR_COMMAND,
    ADD_LV_COMMAND
};

static const std::map<Command, SymId> ADD_ARTICULATION_COMMANDS = {
    { ADD_MARCATO_COMMAND, SymId::articMarcatoAbove },
    { ADD_SFORZATO_COMMAND, SymId::articAccentAbove },
    { ADD_TENUTO_COMMAND, SymId::articTenutoAbove },
    { ADD_STACCATO_COMMAND, SymId::articStaccatoAbove }
};

static const std::map<Command, voice_idx_t> VOICE_COMMANDS = {
    { USE_VOICE_1_COMMAND, 0 },
    { USE_VOICE_2_COMMAND, 1 },
    { USE_VOICE_3_COMMAND, 2 },
    { USE_VOICE_4_COMMAND, 3 }
};

static const std::vector<Command> NOTE_COMMANDS = {
    ENTER_NOTE_C_COMMAND,
    ENTER_NOTE_D_COMMAND,
    ENTER_NOTE_E_COMMAND,
    ENTER_NOTE_F_COMMAND,
    ENTER_NOTE_G_COMMAND,
    ENTER_NOTE_A_COMMAND,
    ENTER_NOTE_B_COMMAND,
    ADD_NOTE_C_COMMAND,
    ADD_NOTE_D_COMMAND,
    ADD_NOTE_E_COMMAND,
    ADD_NOTE_F_COMMAND,
    ADD_NOTE_G_COMMAND,
    ADD_NOTE_A_COMMAND,
    ADD_NOTE_B_COMMAND,
    INSERT_NOTE_C_COMMAND,
    INSERT_NOTE_D_COMMAND,
    INSERT_NOTE_E_COMMAND,
    INSERT_NOTE_F_COMMAND,
    INSERT_NOTE_G_COMMAND,
    INSERT_NOTE_A_COMMAND,
    INSERT_NOTE_B_COMMAND
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

    controller()->selectionChanged().onNotify(this, [this]() {
        updateCommandStates(HAS_SELECTION_REQUIRED_COMMANDS);
        updateCommandStates(ADD_COMMANDS);
        updateCommandStates({ PAD_REST_COMMAND });
        updateCommandStates(commands(VOICE_COMMANDS));
    });

    controller()->stackChanged().onNotify(this, [this]() {
        updateCommandStates(UNDO_REDO_COMMANDS);
    });

    controller()->textEditingChanged().onReceive(this, [this](bool) {
        updateCommandStates(TEXT_EDITING_COMMANDS);
    });

    controller()->isNoteInputAllowedChanged().onReceive(this, [this](bool) {
        updateCommandStates(commands(NOTE_INPUT_COMMANDS));
    });

    controller()->noteInputStateChanged().onNotify(this, [this]() {
        updateCommandStates(commands(NOTE_INPUT_COMMANDS));
        updateCommandStates(commands(DURATION_COMMANDS));
        updateCommandStates(commands(DOT_COUNT_COMMANDS));
        updateCommandStates({ PAD_REST_COMMAND });
        updateCommandStates(commands(ACCIDENTAL_COMMANDS));
        updateCommandStates({ REALTIME_ADVANCE_COMMAND });
        updateCommandStates(ADD_COMMANDS);
        updateCommandStates(commands(ADD_ARTICULATION_COMMANDS));
        updateCommandStates(commands(VOICE_COMMANDS));
        updateCommandStates(NOTE_COMMANDS);
    });

    updateCommandStates();
}

void NotationCommandsState::deinit()
{
    globalContext()->currentProjectChanged().disconnect(this);
    interactive()->opened().disconnect(this);
    controller()->selectionChanged().disconnect(this);
    controller()->stackChanged().disconnect(this);
    controller()->textEditingChanged().disconnect(this);
    controller()->isNoteInputAllowedChanged().disconnect(this);
    controller()->noteInputStateChanged().disconnect(this);
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

CommandState NotationCommandsState::doCommandState(const Command& command) const
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

    if (muse::contains(NOTE_COMMANDS, command)) {
        return CommandState(controller()->isNoteInputActionAllowed(), false);
    }

    if (muse::contains(TEXT_EDITING_COMMANDS, command)) {
        return CommandState(controller()->isTextEditing(), false);
    }

    if (muse::contains(NOTE_INPUT_COMMANDS, command)) {
        return CommandState(controller()->isNoteInputAllowed(),
                            controller()->isNoteInputMode() && controller()->noteInputMethod() == NOTE_INPUT_COMMANDS.at(command));
    } else if (command == REALTIME_ADVANCE_COMMAND) {
        return CommandState(controller()->isNoteInputMode(), false);
    }

    if (muse::contains(DURATION_COMMANDS, command)) {
        return CommandState(true, controller()->currentDurationType() == DURATION_COMMANDS.at(command));
    }

    if (muse::contains(DOT_COUNT_COMMANDS, command)) {
        return CommandState(true, controller()->currentDotCount() == DOT_COUNT_COMMANDS.at(command));
    }

    if (command == PAD_REST_COMMAND) {
        return CommandState(true, controller()->currentIsRest());
    }

    if (muse::contains(ACCIDENTAL_COMMANDS, command)) {
        return CommandState(true, controller()->currentAccidentalType() == ACCIDENTAL_COMMANDS.at(command));
    }

    if (command == ADD_TIE_COMMAND) {
        return CommandState(true, controller()->selectionHasTie());
    }
    if (command == ADD_SLUR_COMMAND) {
        return CommandState(true, controller()->selectionHasSlur());
    }
    if (command == ADD_LV_COMMAND) {
        return CommandState(true, controller()->selectionHasLaissezVib());
    }

    if (muse::contains(ADD_ARTICULATION_COMMANDS, command)) {
        return CommandState(true, controller()->currentArticulations().contains(ADD_ARTICULATION_COMMANDS.at(command)));
    }

    if (muse::contains(VOICE_COMMANDS, command)) {
        return CommandState(true, controller()->currentVoice() == VOICE_COMMANDS.at(command));
    }

    return CommandState(true, false);
}

CommandState NotationCommandsState::commandState(const Command& command) const
{
    CommandState state = doCommandState(command);
    // LOGDA() << "command: " << command
    //         << ", enabled: " << state.enabled
    //         << ", checked: " << state.checked;
    return state;
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
