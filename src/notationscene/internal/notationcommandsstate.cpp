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
    COPY_PASTE_SWAP_COMMAND,
    DELETE_COMMAND,
    FLIP_COMMAND,
    FLIP_HORIZONTALLY_COMMAND,
    GOTO_NEXT_SEGMENT_ELEMENT_COMMAND,
    GOTO_PREV_SEGMENT_ELEMENT_COMMAND,
    GOTO_UPNOTE_IN_CHORD_COMMAND,
    GOTO_DOWNNOTE_IN_CHORD_COMMAND,
    GOTO_TOPNOTE_IN_CHORD_COMMAND,
    GOTO_BOTTOMNOTE_IN_CHORD_COMMAND,
    SELECT_SIMILAR_COMMAND,
    SELECT_SIMILAR_IN_STAFF_COMMAND,
    SELECT_SIMILAR_IN_RANGE_COMMAND,
    SELECT_NOTES_IN_CHORD_COMMAND,
    OPEN_SELECTION_OPTIONS_COMMAND,
    SET_DOUBLE_DURATION_COMMAND,
    SET_HALVE_DURATION_COMMAND,
    SET_DOUBLE_DURATION_DOTTED_COMMAND,
    SET_HALVE_DURATION_DOTTED_COMMAND,
    TOGGLE_SNAP_TO_PREV_COMMAND,
    TOGGLE_SNAP_TO_NEXT_COMMAND
};

static const std::vector<Command> UNDO_REDO_COMMANDS = {
    UNDO_COMMAND,
    REDO_COMMAND
};

static const std::vector<Command> TEXT_EDITING_COMMANDS = {
    EDITTEXT_TOGGLE_BOLD_COMMAND,
    EDITTEXT_TOGGLE_ITALIC_COMMAND,
    EDITTEXT_TOGGLE_UNDERLINE_COMMAND,
    EDITTEXT_TOGGLE_STRIKE_COMMAND,
    EDITTEXT_TOGGLE_SUBSCRIPT_COMMAND,
    EDITTEXT_TOGGLE_SUPERSCRIPT_COMMAND,
    EDITTEXT_NEXT_ELEMENT_COMMAND,
    EDITTEXT_PREV_ELEMENT_COMMAND,
    EDITTEXT_NEXT_WORD_COMMAND,
    EDITTEXT_NEXT_BEAT_COMMAND,
    EDITTEXT_PREV_BEAT_COMMAND,
    EDITTEXT_ADVANCE_LONGA_COMMAND,
    EDITTEXT_ADVANCE_BREVE_COMMAND,
    EDITTEXT_ADVANCE_1_COMMAND,
    EDITTEXT_ADVANCE_2_COMMAND,
    EDITTEXT_ADVANCE_4_COMMAND,
    EDITTEXT_ADVANCE_8_COMMAND,
    EDITTEXT_ADVANCE_16_COMMAND,
    EDITTEXT_ADVANCE_32_COMMAND,
    EDITTEXT_ADVANCE_64_COMMAND,
};

static const std::vector<Command> LYRICS_EDITING_COMMANDS = {
    EDITLYRIC_NEXT_VERSE_COMMAND,
    EDITLYRIC_PREV_VERSE_COMMAND,
    EDITLYRIC_NEXT_SYLLABLE_COMMAND,
    EDITLYRIC_ADD_MELISMA_COMMAND,
    EDITLYRIC_ADD_VERSE_COMMAND
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
    { SET_DURATION_LONGA_COMMAND, DurationType::V_LONG },
    { SET_DURATION_BREVE_COMMAND, DurationType::V_BREVE },
    { SET_DURATION_WHOLE_COMMAND, DurationType::V_WHOLE },
    { SET_DURATION_HALF_COMMAND, DurationType::V_HALF },
    { SET_DURATION_QUARTER_COMMAND, DurationType::V_QUARTER },
    { SET_DURATION_EIGHTH_COMMAND, DurationType::V_EIGHTH },
    { SET_DURATION_16TH_COMMAND, DurationType::V_16TH },
    { SET_DURATION_32ND_COMMAND, DurationType::V_32ND },
    { SET_DURATION_64TH_COMMAND, DurationType::V_64TH },
    { SET_DURATION_128TH_COMMAND, DurationType::V_128TH },
    { SET_DURATION_256TH_COMMAND, DurationType::V_256TH },
    { SET_DURATION_512TH_COMMAND, DurationType::V_512TH },
    { SET_DURATION_1024TH_COMMAND, DurationType::V_1024TH }
};

static const std::map<Command, int> DOT_COUNT_COMMANDS = {
    { TOGGLE_DOT_COMMAND, 1 },
    { TOGGLE_DOT2_COMMAND, 2 },
    { TOGGLE_DOT3_COMMAND, 3 },
    { TOGGLE_DOT4_COMMAND, 4 }
};

static const std::map<Command, AccidentalType> ACCIDENTAL_COMMANDS = {
    { TOGGLE_FLAT2_COMMAND, AccidentalType::FLAT2 },
    { TOGGLE_FLAT_COMMAND, AccidentalType::FLAT },
    { TOGGLE_NAT_COMMAND, AccidentalType::NATURAL },
    { TOGGLE_SHARP_COMMAND, AccidentalType::SHARP },
    { TOGGLE_SHARP2_COMMAND, AccidentalType::SHARP2 }
};

static const std::vector<Command> ADD_COMMANDS = {
    TOGGLE_TIE_COMMAND,
    ADD_SLUR_COMMAND,
    TOGGLE_LV_COMMAND
};

static const std::map<Command, SymId> ADD_ARTICULATION_COMMANDS = {
    { TOGGLE_MARCATO_COMMAND, SymId::articMarcatoAbove },
    { TOGGLE_SFORZATO_COMMAND, SymId::articAccentAbove },
    { TOGGLE_TENUTO_COMMAND, SymId::articTenutoAbove },
    { TOGGLE_STACCATO_COMMAND, SymId::articStaccatoAbove }
};

static const std::map<Command, voice_idx_t> VOICE_COMMANDS = {
    { USE_VOICE_1_COMMAND, 0 },
    { USE_VOICE_2_COMMAND, 1 },
    { USE_VOICE_3_COMMAND, 2 },
    { USE_VOICE_4_COMMAND, 3 }
};

static const std::vector<Command> NOTE_COMMANDS = {
    ADD_NOTE_COMMAND,
    ENTER_NOTE_C_COMMAND,
    ENTER_NOTE_D_COMMAND,
    ENTER_NOTE_E_COMMAND,
    ENTER_NOTE_F_COMMAND,
    ENTER_NOTE_G_COMMAND,
    ENTER_NOTE_A_COMMAND,
    ENTER_NOTE_B_COMMAND,
    ENTER_REST_COMMAND,
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

static const std::vector<Command> NOTE_OR_REST_SELECTED_COMMANDS = {
    OPEN_TUPLET_CONFIGURE_COMMAND,
    ADD_TUPLET_COMMAND,
    ADD_DUPLET_COMMAND,
    ADD_TRIPLET_COMMAND,
    ADD_QUADRUPLET_COMMAND,
    ADD_QUINTUPLET_COMMAND,
    ADD_SEXTUPLET_COMMAND,
    ADD_SEPTUPLET_COMMAND,
    ADD_OCTUPLET_COMMAND,
    ADD_NONUPLET_COMMAND,
    ADD_DYNAMIC_COMMAND,
    ADD_HAIRPIN_COMMAND,
    ADD_HAIRPIN_REVERSE_COMMAND
};

static const std::map<Command, MoveSelectionType> MOVE_SELECTION_COMMANDS = {
    { GOTO_FIRST_ELEMENT_COMMAND, MoveSelectionType::EngravingItem },
    { GOTO_LAST_ELEMENT_COMMAND, MoveSelectionType::EngravingItem },
    { GOTO_NEXT_ELEMENT_COMMAND, MoveSelectionType::EngravingItem },
    { GOTO_PREV_ELEMENT_COMMAND, MoveSelectionType::EngravingItem },
    { GOTO_NEXT_TRACK_COMMAND, MoveSelectionType::Track },
    { GOTO_PREV_TRACK_COMMAND, MoveSelectionType::Track },
    { GOTO_NEXT_FRAME_COMMAND, MoveSelectionType::Frame },
    { GOTO_PREV_FRAME_COMMAND, MoveSelectionType::Frame },
    { GOTO_NEXT_SYSTEM_COMMAND, MoveSelectionType::System },
    { GOTO_PREV_SYSTEM_COMMAND, MoveSelectionType::System }
};

static const std::vector<Command> LAYOUT_BREAK_COMMANDS = {
    TOGGLE_SYSTEM_BREAK_COMMAND,
    TOGGLE_PAGE_BREAK_COMMAND,
    TOGGLE_SECTION_BREAK_COMMAND
};

static const std::map<Command, ScoreConfigType> SCORE_CONFIG_COMMANDS = {
    { SHOW_INVISIBLE_COMMAND, ScoreConfigType::ShowInvisibleElements },
    { SHOW_UNPRINTABLE_COMMAND, ScoreConfigType::ShowUnprintableElements },
    { SHOW_FRAMES_COMMAND, ScoreConfigType::ShowFrames },
    { SHOW_PAGEBORDERS_COMMAND, ScoreConfigType::ShowPageMargins },
    { SHOW_SOUNDFLAGS_COMMAND, ScoreConfigType::ShowSoundFlags },
    { SHOW_IRREGULAR_COMMAND, ScoreConfigType::MarkIrregularMeasures }
};

static const std::vector<Command> STYLE_COMMANDS = {
    TOGGLE_CONCERT_PITCH_COMMAND
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
        updateCommandStates({ TOGGLE_REST_COMMAND });
        updateCommandStates(commands(VOICE_COMMANDS));
        updateCommandStates(NOTE_OR_REST_SELECTED_COMMANDS);
        updateCommandStates(commands(MOVE_SELECTION_COMMANDS));
        updateCommandStates(LAYOUT_BREAK_COMMANDS);
    });

    controller()->stackChanged().onNotify(this, [this]() {
        updateCommandStates(UNDO_REDO_COMMANDS);
    });

    controller()->textEditingChanged().onReceive(this, [this](bool) {
        updateCommandStates(TEXT_EDITING_COMMANDS);
        updateCommandStates(LYRICS_EDITING_COMMANDS);
        updateCommandStates(LAYOUT_BREAK_COMMANDS);
    });

    controller()->isNoteInputAllowedChanged().onReceive(this, [this](bool) {
        updateCommandStates(commands(NOTE_INPUT_COMMANDS));
    });

    controller()->noteInputStateChanged().onNotify(this, [this]() {
        updateCommandStates(commands(NOTE_INPUT_COMMANDS));
        updateCommandStates(commands(DURATION_COMMANDS));
        updateCommandStates(commands(DOT_COUNT_COMMANDS));
        updateCommandStates({ TOGGLE_REST_COMMAND });
        updateCommandStates(commands(ACCIDENTAL_COMMANDS));
        updateCommandStates({ REALTIME_ADVANCE_COMMAND });
        updateCommandStates(ADD_COMMANDS);
        updateCommandStates(commands(ADD_ARTICULATION_COMMANDS));
        updateCommandStates(commands(VOICE_COMMANDS));
        updateCommandStates(NOTE_COMMANDS);
        updateCommandStates(NOTE_OR_REST_SELECTED_COMMANDS);
    });

    controller()->scoreConfigChanged().onReceive(this, [this](ScoreConfigType configType) {
        updateCommandStates({ muse::key(SCORE_CONFIG_COMMANDS, configType) });
    });

    controller()->notationStyleChanged().onNotify(this, [this]() {
        updateCommandStates(STYLE_COMMANDS);
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
    controller()->scoreConfigChanged().disconnect(this);
    controller()->notationStyleChanged().disconnect(this);
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

    if (muse::contains(MOVE_SELECTION_COMMANDS, command)) {
        return CommandState(controller()->isMoveSelectionAvailable(MOVE_SELECTION_COMMANDS.at(command)), false);
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

    if (muse::contains(LYRICS_EDITING_COMMANDS, command)) {
        return CommandState(controller()->isLyricsEditing(), false);
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

    if (command == TOGGLE_REST_COMMAND) {
        return CommandState(true, controller()->currentIsRest());
    }

    if (muse::contains(ACCIDENTAL_COMMANDS, command)) {
        return CommandState(true, controller()->currentAccidentalType() == ACCIDENTAL_COMMANDS.at(command));
    }

    if (command == TOGGLE_TIE_COMMAND) {
        return CommandState(true, controller()->selectionHasTie());
    }
    if (command == ADD_SLUR_COMMAND) {
        return CommandState(true, controller()->selectionHasSlur());
    }
    if (command == TOGGLE_LV_COMMAND) {
        return CommandState(true, controller()->selectionHasLaissezVib());
    }

    if (muse::contains(ADD_ARTICULATION_COMMANDS, command)) {
        return CommandState(true, controller()->currentArticulations().contains(ADD_ARTICULATION_COMMANDS.at(command)));
    }

    if (muse::contains(VOICE_COMMANDS, command)) {
        return CommandState(true, controller()->currentVoice() == VOICE_COMMANDS.at(command));
    }

    if (muse::contains(NOTE_OR_REST_SELECTED_COMMANDS, command)) {
        return CommandState(controller()->isNoteOrRestSelected(), false);
    }

    if (muse::contains(LAYOUT_BREAK_COMMANDS, command)) {
        return CommandState(controller()->isToggleLayoutBreakAvailable(), false);
    }

    if (muse::contains(SCORE_CONFIG_COMMANDS, command)) {
        return CommandState(true, controller()->scoreConfig().isShown(SCORE_CONFIG_COMMANDS.at(command)));
    }

    if (muse::contains(STYLE_COMMANDS, command)) {
        auto style = controller()->notationStyle();
        return CommandState(true, style ? style->styleValue(StyleId::concertPitch).toBool() : false);
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
