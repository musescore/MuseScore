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
#include "rcommand/commandtypes.h"

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
    FlipCommand::id,
    FLIP_HORIZONTALLY_COMMAND,
    SelectNextSegmentCommand::id,
    SelectPrevSegmentCommand::id,
    SelectUpNoteInChordCommand::id,
    SelectDownNoteInChordCommand::id,
    SelectTopNoteInChordCommand::id,
    SelectBottomNoteInChordCommand::id,
    SelectSimilarCommand::id,
    SelectSimilarInStaffCommand::id,
    SelectSimilarInRangeCommand::id,
    SelectNotesInChordCommand::id,
    OpenSelectionOptionsCommand::id,
    ChangeDurationCommand::id,
    ChangeDurationDoubleCommand::id,
    ChangeDurationHalveCommand::id,
    ChangeDurationDoubleDottedCommand::id,
    ChangeDurationHalveDottedCommand::id,
    TOGGLE_SNAP_TO_PREV_COMMAND,
    TOGGLE_SNAP_TO_NEXT_COMMAND,
    MIRROR_NOTEHEAD_COMMAND,
    MOVE_UP_COMMAND,
    MOVE_DOWN_COMMAND,
    INCREASE_DYNAMIC_COMMAND,
    DECREASE_DYNAMIC_COMMAND
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

static const std::vector<Command> NOT_TEXT_EDITING_COMMANDS = {
    TOGGLE_MMREST_COMMAND,
};

static const std::vector<Command> NOTE_INPUT_COMMANDS = {
    ToggleNoteInputCommand::id,
    ToggleNoteInputDefaultCommand::id,
    ToggleNoteInputByNameCommand::id,
    ToggleNoteInputByDurationCommand::id,
    ToggleNoteInputRhythmCommand::id,
    ToggleNoteInputRepitchCommand::id,
    ToggleNoteInputRealtimeAutoCommand::id,
    ToggleNoteInputRealtimeManualCommand::id,
    ToggleNoteInputTimewiseCommand::id,
    ToggleInsertModeCommand::id
};

static const std::map<Command, NoteInputMethod> NOTE_INPUT_METHODS = {
    { ToggleNoteInputDefaultCommand::id, NoteInputMethod::DEFAULT },
    { ToggleNoteInputByNameCommand::id, NoteInputMethod::BY_NOTE_NAME },
    { ToggleNoteInputByDurationCommand::id, NoteInputMethod::BY_DURATION },
    { ToggleNoteInputRhythmCommand::id, NoteInputMethod::RHYTHM },
    { ToggleNoteInputRepitchCommand::id, NoteInputMethod::REPITCH },
    { ToggleNoteInputRealtimeAutoCommand::id, NoteInputMethod::REALTIME_AUTO },
    { ToggleNoteInputRealtimeManualCommand::id, NoteInputMethod::REALTIME_MANUAL },
    { ToggleNoteInputTimewiseCommand::id, NoteInputMethod::TIMEWISE }
};

static const std::map<Command, DurationType> DURATION_COMMANDS = {
    { SetDurationLongaCommand::id, DurationType::V_LONG },
    { SetDurationBreveCommand::id, DurationType::V_BREVE },
    { SetDurationWholeCommand::id, DurationType::V_WHOLE },
    { SetDurationHalfCommand::id, DurationType::V_HALF },
    { SetDurationQuarterCommand::id, DurationType::V_QUARTER },
    { SetDurationEighthCommand::id, DurationType::V_EIGHTH },
    { SetDuration16thCommand::id, DurationType::V_16TH },
    { SetDuration32ndCommand::id, DurationType::V_32ND },
    { SetDuration64thCommand::id, DurationType::V_64TH },
    { SetDuration128thCommand::id, DurationType::V_128TH },
    { SetDuration256thCommand::id, DurationType::V_256TH },
    { SetDuration512thCommand::id, DurationType::V_512TH },
    { SetDuration1024thCommand::id, DurationType::V_1024TH }
};

static const std::map<Command, int> DOT_COUNT_COMMANDS = {
    { ToggleDotCommand::id, 1 },
    { ToggleDot2Command::id, 2 },
    { ToggleDot3Command::id, 3 },
    { ToggleDot4Command::id, 4 }
};

static const std::map<Command, AccidentalType> ACCIDENTAL_COMMANDS = {
    { ToggleFlat2Command::id, AccidentalType::FLAT2 },
    { ToggleFlatCommand::id, AccidentalType::FLAT },
    { ToggleNaturalCommand::id, AccidentalType::NATURAL },
    { ToggleSharpCommand::id, AccidentalType::SHARP },
    { ToggleSharp2Command::id, AccidentalType::SHARP2 }
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
    { UseVoice1Command::id, 0 },
    { UseVoice2Command::id, 1 },
    { UseVoice3Command::id, 2 },
    { UseVoice4Command::id, 3 }
};

static const std::vector<Command> NOTE_COMMANDS = {
    AddNoteCommand::id,
    AddNoteC_ToNextChordCommand::id,
    AddNoteD_ToNextChordCommand::id,
    AddNoteE_ToNextChordCommand::id,
    AddNoteF_ToNextChordCommand::id,
    AddNoteG_ToNextChordCommand::id,
    AddNoteA_ToNextChordCommand::id,
    AddNoteB_ToNextChordCommand::id,
    ENTER_REST_COMMAND,
    AddNoteC_ToCurrentChordCommand::id,
    AddNoteD_ToCurrentChordCommand::id,
    AddNoteE_ToCurrentChordCommand::id,
    AddNoteF_ToCurrentChordCommand::id,
    AddNoteG_ToCurrentChordCommand::id,
    AddNoteA_ToCurrentChordCommand::id,
    AddNoteB_ToCurrentChordCommand::id,
    AddNoteC_ToInsertChordCommand::id,
    AddNoteD_ToInsertChordCommand::id,
    AddNoteE_ToInsertChordCommand::id,
    AddNoteF_ToInsertChordCommand::id,
    AddNoteG_ToInsertChordCommand::id,
    AddNoteA_ToInsertChordCommand::id,
    AddNoteB_ToInsertChordCommand::id
};

static const std::vector<Command> TUPLET_COMMANDS = {
    OpenTupletConfigureCommand::id,
    AddTupletCommand::id,
    AddDupletCommand::id,
    AddTripletCommand::id,
    AddQuadrupletCommand::id,
    AddQuintupletCommand::id,
    AddSextupletCommand::id,
    AddSeptupletCommand::id,
    AddOctupletCommand::id,
    AddNonupletCommand::id,
};

static const std::vector<Command> NOTE_OR_REST_SELECTED_COMMANDS = {
    ADD_DYNAMIC_COMMAND,
    ADD_HAIRPIN_COMMAND,
    ADD_HAIRPIN_REVERSE_COMMAND
};

static const std::map<Command, MoveSelectionType> MOVE_SELECTION_COMMANDS = {
    { SelectFirstItemCommand::id, MoveSelectionType::EngravingItem },
    { SelectLastItemCommand::id, MoveSelectionType::EngravingItem },
    { SelectNextItemCommand::id, MoveSelectionType::EngravingItem },
    { SelectPrevItemCommand::id, MoveSelectionType::EngravingItem },
    { SelectNextTrackCommand::id, MoveSelectionType::Track },
    { SelectPrevTrackCommand::id, MoveSelectionType::Track },
    { SelectNextFrameCommand::id, MoveSelectionType::Frame },
    { SelectPrevFrameCommand::id, MoveSelectionType::Frame },
    { SelectNextSystemCommand::id, MoveSelectionType::System },
    { SelectPrevSystemCommand::id, MoveSelectionType::System }
};

static const std::vector<Command> LAYOUT_BREAK_COMMANDS = {
    TOGGLE_SYSTEM_BREAK_COMMAND,
    TOGGLE_PAGE_BREAK_COMMAND,
    TOGGLE_SECTION_BREAK_COMMAND
};

static const std::vector<Command> SYSTEM_LAYOUT_COMMANDS = {
    APPLY_SYSTEM_LOCK_COMMAND,
    TOGGLE_SYSTEM_LOCK_COMMAND,
    TOGGLE_SCORE_LOCK_COMMAND,
    MOVE_MEASURE_TO_PREV_SYSTEM_COMMAND,
    MOVE_MEASURE_TO_NEXT_SYSTEM_COMMAND
};

static const std::vector<Command> PAGE_LAYOUT_COMMANDS = {
    APPLY_PAGE_LOCK_COMMAND,
    TOGGLE_PAGE_LOCK_COMMAND,
    MOVE_SYSTEM_TO_PREV_PAGE_COMMAND,
    MOVE_SYSTEM_TO_NEXT_PAGE_COMMAND
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

static const std::vector<Command> TAB_COMMANDS = {
    SET_DURATION_WHOLE_TAB_COMMAND,
    SET_DURATION_HALF_TAB_COMMAND,
    SET_DURATION_QUARTER_TAB_COMMAND,
    SET_DURATION_EIGHTH_TAB_COMMAND,
    SET_DURATION_16TH_TAB_COMMAND,
    SET_DURATION_32ND_TAB_COMMAND,
    SET_DURATION_64TH_TAB_COMMAND,
    SET_DURATION_128TH_TAB_COMMAND,
    SET_DURATION_256TH_TAB_COMMAND,
    SET_DURATION_512TH_TAB_COMMAND,
    SET_DURATION_1024TH_TAB_COMMAND,
    ENTER_REST_TAB_COMMAND,
    ENTER_FRET_0_COMMAND,
    ENTER_FRET_1_COMMAND,
    ENTER_FRET_2_COMMAND,
    ENTER_FRET_3_COMMAND,
    ENTER_FRET_4_COMMAND,
    ENTER_FRET_5_COMMAND,
    ENTER_FRET_6_COMMAND,
    ENTER_FRET_7_COMMAND,
    ENTER_FRET_8_COMMAND,
    ENTER_FRET_9_COMMAND,
    ENTER_FRET_10_COMMAND,
    ENTER_FRET_11_COMMAND,
    ENTER_FRET_12_COMMAND,
    ENTER_FRET_13_COMMAND,
    ENTER_FRET_14_COMMAND,
    GOTO_STRING_ABOVE_COMMAND,
    GOTO_STRING_BELOW_COMMAND
};

static const std::vector<Command> DEBUG_COMMANDS = {
    SHOW_ELEMENT_BOUNDING_RECTS_COMMAND,
    COLOR_ELEMENT_SHAPES_COMMAND,
    SHOW_SEGMENT_SHAPES_COMMAND,
    COLOR_SEGMENT_SHAPES_COMMAND,
    SHOW_SKYLINES_COMMAND,
    SHOW_SYSTEM_BOUNDING_RECTS_COMMAND,
    SHOW_ELEMENT_MASKS_COMMAND,
    SHOW_LINE_ATTACH_POINTS_COMMAND,
    MARK_EMPTY_STAFF_COMMAND,
    MARK_CORRUPTED_MEASURES_COMMAND,
    SHOW_GAP_RESTS_COMMAND,
    SHOW_ORIGIN_AND_COMBINED_COMMAND,
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

    projectSession()->projectChanged().onNotify(this, [this]() {
        updateCommandStates();
    });

    if (interactive()) {
        interactive()->opened().onReceive(this, [this](const muse::Uri&) {
            updateCommandStates();
        });
    }

    controller()->selectionChanged().onNotify(this, [this]() {
        updateCommandStates(HAS_SELECTION_REQUIRED_COMMANDS);
        updateCommandStates(ADD_COMMANDS);
        updateCommandStates({ ToggleRestCommand::id });
        updateCommandStates(commands(VOICE_COMMANDS));
        updateCommandStates(NOTE_OR_REST_SELECTED_COMMANDS);
        updateCommandStates(TUPLET_COMMANDS);
        updateCommandStates(commands(MOVE_SELECTION_COMMANDS));
        updateCommandStates(LAYOUT_BREAK_COMMANDS);
        updateCommandStates(TAB_COMMANDS);
    });

    controller()->stackChanged().onNotify(this, [this]() {
        updateCommandStates(UNDO_REDO_COMMANDS);
    });

    controller()->textEditingChanged().onReceive(this, [this](bool) {
        updateCommandStates(TEXT_EDITING_COMMANDS);
        updateCommandStates(LYRICS_EDITING_COMMANDS);
        updateCommandStates(LAYOUT_BREAK_COMMANDS);
        updateCommandStates(NOT_TEXT_EDITING_COMMANDS);
    });

    controller()->isNoteInputAllowedChanged().onReceive(this, [this](bool) {
        updateCommandStates(NOTE_INPUT_COMMANDS);
    });

    controller()->noteInputStateChanged().onNotify(this, [this]() {
        updateCommandStates(NOTE_INPUT_COMMANDS);
        updateCommandStates(commands(DURATION_COMMANDS));
        updateCommandStates(commands(DOT_COUNT_COMMANDS));
        updateCommandStates({ ToggleRestCommand::id });
        updateCommandStates(commands(ACCIDENTAL_COMMANDS));
        updateCommandStates({ REALTIME_ADVANCE_COMMAND });
        updateCommandStates(ADD_COMMANDS);
        updateCommandStates(commands(ADD_ARTICULATION_COMMANDS));
        updateCommandStates(commands(VOICE_COMMANDS));
        updateCommandStates(NOTE_COMMANDS);
        updateCommandStates(NOTE_OR_REST_SELECTED_COMMANDS);
        updateCommandStates(TUPLET_COMMANDS);
        updateCommandStates(TAB_COMMANDS);
    });

    controller()->scoreConfigChanged().onReceive(this, [this](ScoreConfigType configType) {
        updateCommandStates({ muse::key(SCORE_CONFIG_COMMANDS, configType) });
    });

    controller()->notationStyleChanged().onNotify(this, [this]() {
        updateCommandStates(STYLE_COMMANDS);
    });

    controller()->automationModeEnabledChanged().onNotify(this, [this]() {
        updateCommandStates({ TOGGLE_AUTOMATION_COMMAND });
    });

    controller()->debuggingOptionsChanged().onNotify(this, [this]() {
        updateCommandStates(DEBUG_COMMANDS);
    });

    controller()->viewModeChanged().onNotify(this, [this]() {
        updateCommandStates(SYSTEM_LAYOUT_COMMANDS);
        updateCommandStates(PAGE_LAYOUT_COMMANDS);
    });

    updateCommandStates();
}

void NotationCommandsState::deinit()
{
    projectSession()->projectChanged().disconnect(this);
    if (interactive()) {
        interactive()->opened().disconnect(this);
    }
    controller()->selectionChanged().disconnect(this);
    controller()->stackChanged().disconnect(this);
    controller()->textEditingChanged().disconnect(this);
    controller()->isNoteInputAllowedChanged().disconnect(this);
    controller()->noteInputStateChanged().disconnect(this);
    controller()->scoreConfigChanged().disconnect(this);
    controller()->notationStyleChanged().disconnect(this);
    controller()->automationModeEnabledChanged().disconnect(this);
    controller()->debuggingOptionsChanged().disconnect(this);
    controller()->viewModeChanged().disconnect(this);
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

    if (muse::contains(NOT_TEXT_EDITING_COMMANDS, command)) {
        return CommandState(!controller()->isTextEditing(), false);
    }

    if (muse::contains(NOTE_INPUT_COMMANDS, command)) {
        if (muse::contains(NOTE_INPUT_METHODS, command)) {
            return CommandState(controller()->isNoteInputAllowed(),
                                controller()->isNoteInputMode()
                                && controller()->noteInputMethod() == NOTE_INPUT_METHODS.at(command));
        } else {
            return CommandState(controller()->isNoteInputAllowed(), false);
        }
    }

    if (muse::contains(DURATION_COMMANDS, command)) {
        return CommandState(true, controller()->currentDurationType() == DURATION_COMMANDS.at(command));
    }

    if (muse::contains(DOT_COUNT_COMMANDS, command)) {
        return CommandState(true, controller()->currentDotCount() == DOT_COUNT_COMMANDS.at(command));
    }

    if (command == ToggleRestCommand::id) {
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

    if (command == ADD_DYNAMIC_COMMAND) {
        return CommandState(controller()->isNoteOrRestSelected() || controller()->isHairpinSelected(), false);
    }

    if (muse::contains(NOTE_OR_REST_SELECTED_COMMANDS, command)) {
        return CommandState(controller()->isNoteOrRestSelected(), false);
    }

    if (muse::contains(TUPLET_COMMANDS, command)) {
        // Re-pitch must not change durations
        const bool isRepitch = controller()->isNoteInputMode() && controller()->noteInputMethod() == NoteInputMethod::REPITCH;
        return CommandState(controller()->isNoteOrRestSelected() && !isRepitch, false);
    }

    if (muse::contains(LAYOUT_BREAK_COMMANDS, command)) {
        return CommandState(controller()->isToggleLayoutBreakAvailable(), false);
    }

    if (muse::contains(SYSTEM_LAYOUT_COMMANDS, command)) {
        return CommandState(isSystemLayoutAvailable(controller()->viewMode()), false);
    }

    if (muse::contains(PAGE_LAYOUT_COMMANDS, command)) {
        return CommandState(isPageLayoutAvailable(controller()->viewMode()), false);
    }

    if (muse::contains(SCORE_CONFIG_COMMANDS, command)) {
        return CommandState(true, controller()->scoreConfig().isShown(SCORE_CONFIG_COMMANDS.at(command)));
    }

    if (muse::contains(STYLE_COMMANDS, command)) {
        auto style = controller()->notationStyle();
        return CommandState(true, style ? style->styleValue(StyleId::concertPitch).toBool() : false);
    }

    if (muse::contains(TAB_COMMANDS, command)) {
        return CommandState(controller()->isTablatureStaff(), false);
    }

    if (command == TOGGLE_AUTOMATION_COMMAND) {
        return CommandState(true, controller()->isAutomationModeEnabled());
    }

    if (muse::contains(DEBUG_COMMANDS, command)) {
        return CommandState(true, controller()->isDebuggingCommandEnabled(command));
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
    if (!projectSession()->project()) {
        return false;
    }

    if (!interactive() || !interactive()->isOpened(PROJECT_PAGE_URI).val) {
        return false;
    }

    return true;
}
