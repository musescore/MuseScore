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

#include "notationcommandsregister.h"

#include "../notationcommands.h"
#include "rcommand/commandtypes.h"

using namespace muse;
using namespace muse::rcommand;
using namespace muse::ui;
using namespace mu::notation;

// avoid translation duplication

// //: This is comment for translator

//: Addition to the name of an action to indicate that this action only applies to tablature notation.
//: '%1' is the name of the action.
static const TranslatableString X_TAB = TranslatableString("action", "%1 (TAB)");

static const TranslatableString fret_X_TAB = TranslatableString("action", "Fret %1 (TAB)");
static const TranslatableString enter_TAB_fret_X = TranslatableString("action", "Enter TAB: fret %1");

static const std::vector<CommandInfo> s_commandInfos = {
    makeCommandInfo<CancelCommand>(),
    makeCommandInfo<UndoCommand>(),
    makeCommandInfo<RedoCommand>(),
    makeCommandInfo<CopyCommand>(),
    makeCommandInfo<CopyPasteSwapCommand>(),
    makeCommandInfo<CopyLyricsCommand>(),
    makeCommandInfo<CutCommand>(),
    makeCommandInfo<PasteCommand>(),
    makeCommandInfo<PasteDefaultCommand>(),
    makeCommandInfo<PasteHalfCommand>(),
    makeCommandInfo<PasteDoubleCommand>(),
    makeCommandInfo<PasteSpecialCommand>(),
    makeCommandInfo<DeleteCommand>(),

    // navigation and selection commands

    makeCommandInfo<SelectCommand>(),
    makeCommandInfo<SelectFirstItemCommand>(),
    makeCommandInfo<SelectLastItemCommand>(),
    makeCommandInfo<SelectNextItemCommand>(),
    makeCommandInfo<SelectPrevItemCommand>(),
    makeCommandInfo<SelectNextSegmentCommand>(),
    makeCommandInfo<SelectPrevSegmentCommand>(),
    makeCommandInfo<SelectNextTrackCommand>(),
    makeCommandInfo<SelectPrevTrackCommand>(),
    makeCommandInfo<SelectNextFrameCommand>(),
    makeCommandInfo<SelectPrevFrameCommand>(),
    makeCommandInfo<SelectNextSystemCommand>(),
    makeCommandInfo<SelectPrevSystemCommand>(),
    makeCommandInfo<SelectUpNoteInChordCommand>(),
    makeCommandInfo<SelectDownNoteInChordCommand>(),
    makeCommandInfo<SelectTopNoteInChordCommand>(),
    makeCommandInfo<SelectBottomNoteInChordCommand>(),
    makeCommandInfo<SelectTopStaffCommand>(),
    makeCommandInfo<SelectEmptyTrailingMeasureCommand>(),
    makeCommandInfo<SelectSimilarCommand>(),
    makeCommandInfo<SelectSimilarInStaffCommand>(),
    makeCommandInfo<SelectSimilarInRangeCommand>(),
    makeCommandInfo<SelectNotesInChordCommand>(),
    makeCommandInfo<SelectAllCommand>(),
    makeCommandInfo<SelectSectionCommand>(),

    makeCommandInfo<AddToSelectionCommand>(),
    makeCommandInfo<AddToSelectionNextChordCommand>(),
    makeCommandInfo<AddToSelectionPrevChordCommand>(),
    makeCommandInfo<AddToSelectionNextMeasureCommand>(),
    makeCommandInfo<AddToSelectionPrevMeasureCommand>(),
    makeCommandInfo<AddToSelectionAboveStaffCommand>(),
    makeCommandInfo<AddToSelectionBelowStaffCommand>(),
    makeCommandInfo<AddToSelectionBeginSystemCommand>(),
    makeCommandInfo<AddToSelectionEndSystemCommand>(),
    makeCommandInfo<AddToSelectionBeginScoreCommand>(),
    makeCommandInfo<AddToSelectionEndScoreCommand>(),

    makeCommandInfo<OpenSelectionOptionsCommand>(),
    makeCommandInfo<ShowLocationCommand>(),

    // note input
    makeCommandInfo<ToggleNoteInputCommand>(),
    makeCommandInfo<ToggleNoteInputDefaultCommand>(),
    makeCommandInfo<ToggleNoteInputByNameCommand>(),
    makeCommandInfo<ToggleNoteInputByDurationCommand>(),
    makeCommandInfo<ToggleNoteInputRhythmCommand>(),
    makeCommandInfo<ToggleNoteInputRepitchCommand>(),
    makeCommandInfo<ToggleNoteInputRealtimeAutoCommand>(),
    makeCommandInfo<ToggleNoteInputRealtimeManualCommand>(),
    makeCommandInfo<ToggleNoteInputTimewiseCommand>(),
    makeCommandInfo<ToggleInsertModeCommand>(),

    makeCommandInfo<SetDurationCommand>(),
    makeCommandInfo<SetDurationLongaCommand>(),
    makeCommandInfo<SetDurationBreveCommand>(),
    makeCommandInfo<SetDurationWholeCommand>(),
    makeCommandInfo<SetDurationHalfCommand>(),
    makeCommandInfo<SetDurationQuarterCommand>(),
    makeCommandInfo<SetDurationEighthCommand>(),
    makeCommandInfo<SetDuration16thCommand>(),
    makeCommandInfo<SetDuration32ndCommand>(),
    makeCommandInfo<SetDuration64thCommand>(),
    makeCommandInfo<SetDuration128thCommand>(),
    makeCommandInfo<SetDuration256thCommand>(),
    makeCommandInfo<SetDuration512thCommand>(),
    makeCommandInfo<SetDuration1024thCommand>(),
    makeCommandInfo<ChangeDurationCommand>(),
    makeCommandInfo<ChangeDurationDoubleCommand>(),
    makeCommandInfo<ChangeDurationHalveCommand>(),
    makeCommandInfo<ChangeDurationDoubleDottedCommand>(),
    makeCommandInfo<ChangeDurationHalveDottedCommand>(),
    makeCommandInfo<ToggleDotsCommand>(),
    makeCommandInfo<ToggleDotCommand>(),
    makeCommandInfo<ToggleDot2Command>(),
    makeCommandInfo<ToggleDot3Command>(),
    makeCommandInfo<ToggleDot4Command>(),

    makeCommandInfo<ToggleRestCommand>(),

    makeCommandInfo<RequestNoteInputStateCommand>(),

    // note operation
    makeCommandInfo<AddNoteCommand>(),
    makeCommandInfo<AddNoteC_ToNextChordCommand>(),
    makeCommandInfo<AddNoteD_ToNextChordCommand>(),
    makeCommandInfo<AddNoteE_ToNextChordCommand>(),
    makeCommandInfo<AddNoteF_ToNextChordCommand>(),
    makeCommandInfo<AddNoteG_ToNextChordCommand>(),
    makeCommandInfo<AddNoteA_ToNextChordCommand>(),
    makeCommandInfo<AddNoteB_ToNextChordCommand>(),
    makeCommandInfo<AddNoteC_ToCurrentChordCommand>(),
    makeCommandInfo<AddNoteD_ToCurrentChordCommand>(),
    makeCommandInfo<AddNoteE_ToCurrentChordCommand>(),
    makeCommandInfo<AddNoteF_ToCurrentChordCommand>(),
    makeCommandInfo<AddNoteG_ToCurrentChordCommand>(),
    makeCommandInfo<AddNoteA_ToCurrentChordCommand>(),
    makeCommandInfo<AddNoteB_ToCurrentChordCommand>(),
    makeCommandInfo<AddNoteC_ToInsertChordCommand>(),
    makeCommandInfo<AddNoteD_ToInsertChordCommand>(),
    makeCommandInfo<AddNoteE_ToInsertChordCommand>(),
    makeCommandInfo<AddNoteF_ToInsertChordCommand>(),
    makeCommandInfo<AddNoteG_ToInsertChordCommand>(),
    makeCommandInfo<AddNoteA_ToInsertChordCommand>(),
    makeCommandInfo<AddNoteB_ToInsertChordCommand>(),

    makeCommandInfo<ToggleAccidentalCommand>(),
    makeCommandInfo<ToggleFlat2Command>(),
    makeCommandInfo<ToggleFlatCommand>(),
    makeCommandInfo<ToggleNaturalCommand>(),
    makeCommandInfo<ToggleSharpCommand>(),
    makeCommandInfo<ToggleSharp2Command>(),

    makeCommandInfo<UseVoiceCommand>(),
    makeCommandInfo<UseVoice1Command>(),
    makeCommandInfo<UseVoice2Command>(),
    makeCommandInfo<UseVoice3Command>(),
    makeCommandInfo<UseVoice4Command>(),

    makeCommandInfo<SwapVoicesCommand>(),
    makeCommandInfo<SwapVoice12Command>(),
    makeCommandInfo<SwapVoice13Command>(),
    makeCommandInfo<SwapVoice14Command>(),
    makeCommandInfo<SwapVoice23Command>(),
    makeCommandInfo<SwapVoice24Command>(),
    makeCommandInfo<SwapVoice34Command>(),

    makeCommandInfo<AddTupletCommand>(),
    makeCommandInfo<AddDupletCommand>(),
    makeCommandInfo<AddTripletCommand>(),
    makeCommandInfo<AddQuadrupletCommand>(),
    makeCommandInfo<AddQuintupletCommand>(),
    makeCommandInfo<AddSextupletCommand>(),
    makeCommandInfo<AddSeptupletCommand>(),
    makeCommandInfo<AddOctupletCommand>(),
    makeCommandInfo<AddNonupletCommand>(),
    makeCommandInfo<OpenTupletConfigureCommand>(),

    makeCommandInfo<AddBoxesCommand>(),
    makeCommandInfo<InsertHBoxCommand>(),
    makeCommandInfo<InsertVBoxCommand>(),
    makeCommandInfo<InsertTextFrameCommand>(),
    makeCommandInfo<InsertFretFrameCommand>(),
    makeCommandInfo<AppendHBoxCommand>(),
    makeCommandInfo<AppendVBoxCommand>(),
    makeCommandInfo<AppendTextFrameCommand>(),
    makeCommandInfo<AppendFretFrameCommand>(),
    makeCommandInfo<InsertMeasureCommand>(),
    makeCommandInfo<AppendMeasureCommand>(),
    makeCommandInfo<InsertMeasuresCommand>(),
    makeCommandInfo<InsertMeasuresAfterSelectionCommand>(),
    makeCommandInfo<InsertMeasuresAtStartOfScoreCommand>(),
    makeCommandInfo<AppendMeasuresCommand>(),

    makeCommandInfo<AddGraceNoteCommand>(),
    makeCommandInfo<AddAcciaccaturaCommand>(),
    makeCommandInfo<AddAppoggiaturaCommand>(),
    makeCommandInfo<AddGrace4Command>(),
    makeCommandInfo<AddGrace16Command>(),
    makeCommandInfo<AddGrace32Command>(),
    makeCommandInfo<AddGrace8AfterCommand>(),
    makeCommandInfo<AddGrace16AfterCommand>(),
    makeCommandInfo<AddGrace32AfterCommand>(),

    makeCommandInfo<AddIntervalCommand>(),
    makeCommandInfo<AddInterval1Command>(),
    makeCommandInfo<AddInterval2Command>(),
    makeCommandInfo<AddInterval3Command>(),
    makeCommandInfo<AddInterval4Command>(),
    makeCommandInfo<AddInterval5Command>(),
    makeCommandInfo<AddInterval6Command>(),
    makeCommandInfo<AddInterval7Command>(),
    makeCommandInfo<AddInterval8Command>(),
    makeCommandInfo<AddInterval9Command>(),
    makeCommandInfo<AddInterval10Command>(),
    makeCommandInfo<AddIntervalMinus2Command>(),
    makeCommandInfo<AddIntervalMinus3Command>(),
    makeCommandInfo<AddIntervalMinus4Command>(),
    makeCommandInfo<AddIntervalMinus5Command>(),
    makeCommandInfo<AddIntervalMinus6Command>(),
    makeCommandInfo<AddIntervalMinus7Command>(),
    makeCommandInfo<AddIntervalMinus8Command>(),
    makeCommandInfo<AddIntervalMinus9Command>(),
    makeCommandInfo<AddIntervalMinus10Command>(),

    makeCommandInfo<AddTextCommand>(),
    makeCommandInfo<AddTitleTextCommand>(),
    makeCommandInfo<AddSubtitleTextCommand>(),
    makeCommandInfo<AddComposerTextCommand>(),
    makeCommandInfo<AddLyricistTextCommand>(),
    makeCommandInfo<AddPartTextCommand>(),
    makeCommandInfo<AddFrameTextCommand>(),
    makeCommandInfo<AddSystemTextCommand>(),
    makeCommandInfo<AddStaffTextCommand>(),
    makeCommandInfo<AddExpressionTextCommand>(),
    makeCommandInfo<AddRehearsalMarkTextCommand>(),
    makeCommandInfo<AddInstrumentChangeTextCommand>(),
    makeCommandInfo<AddFingeringTextCommand>(),
    makeCommandInfo<AddStickingTextCommand>(),
    makeCommandInfo<AddChordTextCommand>(),
    makeCommandInfo<AddRomanNumeralTextCommand>(),
    makeCommandInfo<AddNashvilleNumberTextCommand>(),
    makeCommandInfo<AddLyricsCommand>(),
    makeCommandInfo<AddTempoCommand>(),

    makeCommandInfo<SetBeamCommand>(),
    makeCommandInfo<SetBeamAutoCommand>(),
    makeCommandInfo<SetBeamNoneCommand>(),
    makeCommandInfo<SetBeamBeginCommand>(),
    makeCommandInfo<SetBeamBegin16Command>(),
    makeCommandInfo<SetBeamBegin32Command>(),
    makeCommandInfo<SetBeamMidCommand>(),

    makeCommandInfo<AddOrnamentCommand>(),
    makeCommandInfo<AddOrnamentTurnCommand>(),
    makeCommandInfo<AddOrnamentTurnInvertedCommand>(),
    makeCommandInfo<AddOrnamentTurnSlashCommand>(),
    makeCommandInfo<AddOrnamentTurnUpCommand>(),
    makeCommandInfo<AddOrnamentTurnInvertedUpCommand>(),
    makeCommandInfo<AddOrnamentTrillCommand>(),
    makeCommandInfo<AddOrnamentShortTrillCommand>(),
    makeCommandInfo<AddOrnamentMordentCommand>(),
    makeCommandInfo<AddOrnamentHaydnCommand>(),
    makeCommandInfo<AddOrnamentTremblementCommand>(),
    makeCommandInfo<AddOrnamentPrallMordentCommand>(),
    makeCommandInfo<AddOrnamentShakeCommand>(),
    makeCommandInfo<AddOrnamentShakeMuffatCommand>(),
    makeCommandInfo<AddOrnamentTremblementCouperinCommand>(),

    // Tab
    makeCommandInfo<AddFretCommand>(),
    makeCommandInfo<AddFret0Command>(),
    makeCommandInfo<AddFret1Command>(),
    makeCommandInfo<AddFret2Command>(),
    makeCommandInfo<AddFret3Command>(),
    makeCommandInfo<AddFret4Command>(),
    makeCommandInfo<AddFret5Command>(),
    makeCommandInfo<AddFret6Command>(),
    makeCommandInfo<AddFret7Command>(),
    makeCommandInfo<AddFret8Command>(),
    makeCommandInfo<AddFret9Command>(),
    makeCommandInfo<AddFret10Command>(),
    makeCommandInfo<AddFret11Command>(),
    makeCommandInfo<AddFret12Command>(),
    makeCommandInfo<AddFret13Command>(),
    makeCommandInfo<AddFret14Command>(),

    makeCommandInfo<EditTextAdvanceCommand>(),
    makeCommandInfo<EditTextAdvanceLongaCommand>(),
    makeCommandInfo<EditTextAdvanceBreveCommand>(),
    makeCommandInfo<EditTextAdvance1Command>(),
    makeCommandInfo<EditTextAdvance2Command>(),
    makeCommandInfo<EditTextAdvance4Command>(),
    makeCommandInfo<EditTextAdvance8Command>(),
    makeCommandInfo<EditTextAdvance16Command>(),
    makeCommandInfo<EditTextAdvance32Command>(),
    makeCommandInfo<EditTextAdvance64Command>(),

    // navigation
    CommandInfo{
        MOVE_RIGHT_COMMAND,
        TranslatableString("action", "Move right"),
        TranslatableString("action", "Go to next notation element"),
        InputSchema(),
        Decoration()
    },
    CommandInfo{
        MOVE_RIGHT_QUICKLY_COMMAND,
        TranslatableString("action", "Move right quickly"),
        TranslatableString("action", "Go to next notation element quickly"),
        InputSchema(),
        Decoration()
    },
    CommandInfo{
        MOVE_LEFT_COMMAND,
        TranslatableString("action", "Move left"),
        TranslatableString("action", "Go to previous notation element"),
        InputSchema(),
        Decoration()
    },
    CommandInfo{
        MOVE_LEFT_QUICKLY_COMMAND,
        TranslatableString("action", "Move left quickly"),
        TranslatableString("action", "Go to previous notation element quickly"),
        InputSchema(),
        Decoration()
    },
    CommandInfo{
        MOVE_UP_COMMAND,
        TranslatableString("action", "Move to staff above"),
        TranslatableString("action", "Move selected note/rest to staff above"),
        InputSchema(),
        Decoration()
    },
    CommandInfo{
        MOVE_DOWN_COMMAND,
        TranslatableString("action", "Move to staff below"),
        TranslatableString("action", "Move selected note/rest to staff below"),
        InputSchema(),
        Decoration()
    },
    CommandInfo{
        SWAP_LEFT_COMMAND,
        TranslatableString("action", "Move chord/rest left"),
        TranslatableString("action", "Move chord/rest left"),
        InputSchema(),
        Decoration()
    },
    CommandInfo{
        SWAP_RIGHT_COMMAND,
        TranslatableString("action", "Move chord/rest right"),
        TranslatableString("action", "Move chord/rest right"),
        InputSchema(),
        Decoration()
    },

    CommandInfo{
        PITCH_UP_COMMAND,
        TranslatableString("action", "Pitch up"),
        TranslatableString("action", "Pitch up the current note"),
        InputSchema(),
        Decoration()
    },
    CommandInfo{
        PITCH_DOWN_COMMAND,
        TranslatableString("action", "Pitch down"),
        TranslatableString("action", "Pitch down the current note"),
        InputSchema(),
        Decoration()
    },
    CommandInfo{
        PITCH_UP_OCTAVE_COMMAND,
        TranslatableString("action", "Pitch up octave"),
        TranslatableString("action", "Pitch up the current note by an octave"),
        InputSchema(),
        Decoration()
    },
    CommandInfo{
        PITCH_DOWN_OCTAVE_COMMAND,
        TranslatableString("action", "Pitch down octave"),
        TranslatableString("action", "Pitch down the current note by an octave"),
        InputSchema(),
        Decoration()
    },
    CommandInfo{
        PITCH_UP_DIATONIC_COMMAND,
        TranslatableString("action", "Diatonic pitch up"),
        TranslatableString("action", "Move pitch up diatonically"),
        InputSchema(),
        Decoration()
    },
    CommandInfo{
        PITCH_DOWN_DIATONIC_COMMAND,
        TranslatableString("action", "Diatonic pitch down"),
        TranslatableString("action", "Move pitch down diatonically"),
        InputSchema(),
        Decoration()
    },
    CommandInfo{
        PITCH_UP_DIATONIC_ALTERATIONS_COMMAND,
        TranslatableString("action", "Diatonic pitch up (keep degree alterations)"),
        TranslatableString("action", "Move pitch up diatonically (keep degree alterations)"),
        InputSchema(),
        Decoration()
    },
    CommandInfo{
        PITCH_DOWN_DIATONIC_ALTERATIONS_COMMAND,
        TranslatableString("action", "Diatonic pitch down (keep degree alterations)"),
        TranslatableString("action", "Move pitch down diatonically (keep degree alterations)"),
        InputSchema(),
        Decoration()
    },

    makeCommandInfo<RealtimeAdvanceCommand>(),

    makeCommandInfo<ToggleTieCommand>(),
    makeCommandInfo<AddSlurCommand>(),
    makeCommandInfo<ToggleLaissezVibrerCommand>(),
    makeCommandInfo<ToggleArticulationCommand>(),
    makeCommandInfo<ToggleMarcatoCommand>(),
    makeCommandInfo<ToggleSforzatoCommand>(),
    makeCommandInfo<ToggleTenutoCommand>(),
    makeCommandInfo<ToggleStaccatoCommand>(),

    CommandInfo{
        VOICE_ASSIGNMENT_ALL_IN_INSTR_COMMAND,
        TranslatableString("action", "All voices on instrument"),
        TranslatableString("action", "Use all voices on instrument"),
        InputSchema(),
        Decoration()
    },
    CommandInfo{
        VOICE_ASSIGNMENT_ALL_IN_STAFF_COMMAND,
        TranslatableString("action", "All voices on staff"),
        TranslatableString("action", "Use all voices on staff"),
        InputSchema(),
        Decoration()
    },

    makeCommandInfo<FlipCommand>(),
    makeCommandInfo<FlipHorizontallyCommand>(),

    makeCommandInfo<AddDrumNoteCommand>(),

    makeCommandInfo<EnterRestCommand>(),

    // text navigation commands
    makeCommandInfo<TextNavigateCommand>(),
    makeCommandInfo<TextNavigateNextWordCommand>(),
    makeCommandInfo<TextNavigateNextElementCommand>(),
    makeCommandInfo<TextNavigatePrevElementCommand>(),
    makeCommandInfo<TextNavigateNextBeatCommand>(),
    makeCommandInfo<TextNavigatePrevBeatCommand>(),

    makeCommandInfo<LyricsNavigateCommand>(),
    makeCommandInfo<LyricsNavigateNextVerseCommand>(),
    makeCommandInfo<LyricsNavigatePrevVerseCommand>(),
    makeCommandInfo<LyricsNavigateNextSyllableCommand>(),
    makeCommandInfo<EditLyricsAddMelismaCommand>(),
    makeCommandInfo<EditLyricsAddVerseCommand>(),

    // properties commands
    makeCommandInfo<ToggleVisibleCommand>(),

    // snap commands
    makeCommandInfo<ToggleSnapToCommand>(),
    makeCommandInfo<ToggleSnapToPrevCommand>(),
    makeCommandInfo<ToggleSnapToNextCommand>(),

    // layout commands
    CommandInfo{
        TOGGLE_SYSTEM_BREAK_COMMAND,
        TranslatableString("action", "Add/remove system break"),
        TranslatableString("action", "Add/remove system break"),
        InputSchema(),
        Decoration()
    },
    CommandInfo{
        TOGGLE_PAGE_BREAK_COMMAND,
        TranslatableString("action", "Add/remove page break"),
        TranslatableString("action", "Add/remove page break"),
        InputSchema(),
        Decoration()
    },
    CommandInfo{
        TOGGLE_SECTION_BREAK_COMMAND,
        TranslatableString("action", "Add/remove section break"),
        TranslatableString("action", "Add/remove section break"),
        InputSchema(),
        Decoration()
    },
    CommandInfo{
        APPLY_SYSTEM_LOCK_COMMAND,
        TranslatableString("action", "Add/remove system lock"),
        TranslatableString("action", "Add/remove system lock"),
        InputSchema(),
        Decoration()
    },
    CommandInfo{
        TOGGLE_SYSTEM_LOCK_COMMAND,
        TranslatableString("action", "Lock/unlock selected system(s)"),
        TranslatableString("action", "Lock/unlock selected system(s)"),
        InputSchema(),
        Decoration(IconCode::Code::SYSTEM_LOCK)
    },
    CommandInfo{
        APPLY_PAGE_LOCK_COMMAND,
        TranslatableString("action", "Add/remove page lock"),
        TranslatableString("action", "Add/remove page lock"),
        InputSchema(),
        Decoration()
    },
    CommandInfo{
        TOGGLE_PAGE_LOCK_COMMAND,
        TranslatableString("action", "Lock/unlock selected page(s)"),
        TranslatableString("action", "Lock/unlock selected page(s)"),
        InputSchema(),
        Decoration()
    },
    CommandInfo{
        TOGGLE_SCORE_LOCK_COMMAND,
        TranslatableString("action", "Lock/unlock all systems"),
        TranslatableString("action", "Lock/unlock all systems"),
        InputSchema(),
        Decoration()
    },
    CommandInfo{
        MAKE_INTO_SYSTEM_COMMAND,
        TranslatableString("action", "Create system from selection"),
        TranslatableString("action", "Create system from selection"),
        InputSchema(),
        Decoration()
    },
    CommandInfo{
        MAKE_INTO_PAGE_COMMAND,
        TranslatableString("action", "Create page from selection"),
        TranslatableString("action", "Create page from selection"),
        InputSchema(),
        Decoration()
    },
    CommandInfo{
        MOVE_MEASURE_TO_PREV_SYSTEM_COMMAND,
        TranslatableString("action", "Move measure to previous system"),
        TranslatableString("action", "Move measure to previous system"),
        InputSchema(),
        Decoration(IconCode::Code::ARROW_UP)
    },
    CommandInfo{
        MOVE_MEASURE_TO_NEXT_SYSTEM_COMMAND,
        TranslatableString("action", "Move measure to next system"),
        TranslatableString("action", "Move measure to next system"),
        InputSchema(),
        Decoration(IconCode::Code::ARROW_DOWN)
    },
    CommandInfo{
        MOVE_SYSTEM_TO_PREV_PAGE_COMMAND,
        TranslatableString("action", "Move system to previous page"),
        TranslatableString("action", "Move system to previous page"),
        InputSchema(),
        Decoration()
    },
    CommandInfo{
        MOVE_SYSTEM_TO_NEXT_PAGE_COMMAND,
        TranslatableString("action", "Move system to next page"),
        TranslatableString("action", "Move system to next page"),
        InputSchema(),
        Decoration()
    },
    CommandInfo{
        SPLIT_MEASURE_COMMAND,
        TranslatableString("action", "&Split measure before selected note/rest"),
        TranslatableString("action", "Split measure before selected note/rest"),
        InputSchema(),
        Decoration()
    },
    CommandInfo{
        JOIN_MEASURES_COMMAND,
        TranslatableString("action", "&Join selected measures"),
        TranslatableString("action", "Join selected measures"),
        InputSchema(),
        Decoration()
    },
    CommandInfo{
        OPEN_EDIT_STYLE_COMMAND,
        TranslatableString("action", "&Style…"),
        TranslatableString("action", "Format style"),
        InputSchema({
            { "page_code", Arg(DataType::String, u"Page code") },
            { "sub_page_code", Arg(DataType::String, u"Sub page code") },
        }),
        Decoration()
    },
    makeCommandInfo<OpenPageSettingsCommand>(),
    makeCommandInfo<OpenStaffPropertiesCommand>(),
    makeCommandInfo<OpenEditStringsCommand>(),
    makeCommandInfo<OpenBreaksCommand>(),
    makeCommandInfo<OpenStaffTextPropertiesCommand>(),
    makeCommandInfo<OpenSystemTextPropertiesCommand>(),
    makeCommandInfo<OpenMeasurePropertiesCommand>(),
    makeCommandInfo<OpenTransposeCommand>(),
    makeCommandInfo<OpenPartsCommand>(),
    makeCommandInfo<OpenEditGridSizeCommand>(),
    makeCommandInfo<OpenRealizeChordSymbolsCommand>(),

    // style commands
    makeCommandInfo<LoadStyleCommand>(),
    makeCommandInfo<SaveStyleCommand>(),

    // fretboard diagram commands
    makeCommandInfo<AddFretboardDiagramCommand>(),

    makeCommandInfo<AddOttavaCommand>(),
    makeCommandInfo<AddOttava8vaCommand>(),
    makeCommandInfo<AddOttava8vbCommand>(),
    CommandInfo{
        ADD_DYNAMIC_COMMAND,
        TranslatableString("action", "&Dynamic"),
        TranslatableString("action", "Add dynamic"),
        InputSchema(),
        Decoration()
    },
    makeCommandInfo<AddHairpinCommand>(),
    makeCommandInfo<AddHairpinCrescendoCommand>(),
    makeCommandInfo<AddHairpinDiminuendoCommand>(),
    makeCommandInfo<ChangeDynamicCommand>(),
    makeCommandInfo<IncreaseDynamicCommand>(),
    makeCommandInfo<DecreaseDynamicCommand>(),
    CommandInfo{
        ADD_NOTELINE_COMMAND,
        TranslatableString("action", "&Note-anchored line"),
        TranslatableString("action", "Add note-anchored line"),
        InputSchema(),
        Decoration()
    },
    CommandInfo{
        ADD_IMAGE_COMMAND,
        TranslatableString("action", "Image"),
        TranslatableString("action", "Add image"),
        InputSchema(),
        Decoration()
    },

    makeCommandInfo<AddFiguredBassCommand>(),

    // layout commands: stretch
    makeCommandInfo<ChangeStretchCommand>(),
    makeCommandInfo<IncreaseStretchCommand>(),
    makeCommandInfo<DecreaseStretchCommand>(),
    CommandInfo{
        STRETCH_RESET_COMMAND,
        TranslatableString("action", "&Reset layout stretch"),
        TranslatableString("action", "Reset layout stretch"),
        InputSchema(),
        Decoration()
    },
    CommandInfo{
        RESET_TEXT_STYLE_OVERRIDES_COMMAND,
        TranslatableString("action", "Reset &text style overrides"),
        TranslatableString("action", "Reset all text style overrides to default"),
        InputSchema(),
        Decoration()
    },
    CommandInfo{
        RESET_BEAMS_COMMAND,
        TranslatableString("action", "Reset &beams"),
        TranslatableString("action", "Reset beams to default grouping"),
        InputSchema(),
        Decoration()
    },
    CommandInfo{
        RESET_SHAPES_AND_POSITIONS_COMMAND,
        TranslatableString("action", "Reset s&hapes and positions"),
        TranslatableString("action", "Reset shapes and positions"),
        InputSchema(),
        Decoration()
    },
    CommandInfo{
        RESET_TO_DEFAULT_LAYOUT_COMMAND,
        TranslatableString("action", "Reset entire score to &default layout"),
        TranslatableString("action", "Reset entire score to default layout"),
        InputSchema(),
        Decoration()
    },

    // appearance commands
    CommandInfo{
        FREEZE_CURRENT_PLACEMENT_COMMAND,
        TranslatableString("action", "Freeze current placement"),
        TranslatableString("action", "Turn off auto-place and freeze the selection’s current position"),
        InputSchema(),
        Decoration()
    },

    // show commands
    makeCommandInfo<ToggleScoreConfigCommand>(),
    makeCommandInfo<ShowInvisibleCommand>(),
    makeCommandInfo<ShowUnprintableCommand>(),
    makeCommandInfo<ShowFramesCommand>(),
    makeCommandInfo<ShowPageBordersCommand>(),
    makeCommandInfo<ShowSoundFlagsCommand>(),
    makeCommandInfo<ShowIrregularCommand>(),
    CommandInfo{
        TOGGLE_CONCERT_PITCH_COMMAND,
        TranslatableString("action", "Concert pitch"),
        TranslatableString("action", "Toggle concert pitch"),
        InputSchema(),
        Decoration(IconCode::Code::TUNING_FORK, rcommand::Checkable::Yes)
    },
    CommandInfo{
        STAFF_EXPLODE_COMMAND,
        TranslatableString("action", "&Explode"),
        TranslatableString("action", "Explode"),
        InputSchema(),
        Decoration()
    },
    CommandInfo{
        STAFF_IMPLODE_COMMAND,
        TranslatableString("action", "&Implode"),
        TranslatableString("action", "Implode"),
        InputSchema(),
        Decoration()
    },

    makeCommandInfo<SetBeamSelectedRangeCommand>(),

    // add brackets commands
    CommandInfo{
        ADD_BRACKETS_COMMAND,
        TranslatableString("action", "Add brackets to accidental"),
        TranslatableString("action", "Add brackets to accidental"),
        InputSchema(),
        Decoration(IconCode::Code::BRACKET_PARENTHESES_SQUARE)
    },
    CommandInfo{
        ADD_BRACES_COMMAND,
        TranslatableString("action", "Add braces to element"),
        TranslatableString("action", "Add braces to element"),
        InputSchema(),
        Decoration(IconCode::Code::BRACE)
    },
    CommandInfo{
        ADD_PARENTHESES_COMMAND,
        TranslatableString("action", "Add parentheses to element"),
        TranslatableString("action", "Add parentheses to element"),
        InputSchema(),
        Decoration(IconCode::Code::BRACKET_PARENTHESES)
    },

    // text editing commands
    makeCommandInfo<EditTextFontStyleCommand>(),
    makeCommandInfo<EditTextBoldCommand>(),
    makeCommandInfo<EditTextItalicCommand>(),
    makeCommandInfo<EditTextUnderlineCommand>(),
    makeCommandInfo<EditTextStrikeCommand>(),
    makeCommandInfo<EditTextValignCommand>(),
    makeCommandInfo<EditTextSubScriptCommand>(),
    makeCommandInfo<EditTextSuperScriptCommand>(),

    makeCommandInfo<ExtendToNextNoteCommand>(),
    CommandInfo{
        REMOVE_SELECTED_RANGE_COMMAND,
        TranslatableString("action", "Remove selected ran&ge"),
        TranslatableString("action", "Delete selected measures"),
        InputSchema(),
        Decoration(IconCode::Code::DELETE_TANK)
    },
    CommandInfo{
        REMOVE_EMPTY_TRAILING_MEASURES_COMMAND,
        TranslatableString("action", "Remove empty trailing meas&ures"),
        TranslatableString("action", "Remove empty trailing measures"),
        InputSchema(),
        Decoration()
    },
    CommandInfo{
        SLASH_FILL_COMMAND,
        TranslatableString("action", "Fill with &slashes"),
        TranslatableString("action", "Fill with slashes"),
        InputSchema(),
        Decoration()
    },
    CommandInfo{
        SLASH_RHYTHM_COMMAND,
        TranslatableString("action", "Toggle rhythmic sl&ash notation"),
        TranslatableString("action", "Toggle rhythmic slash notation"),
        InputSchema(),
        Decoration()
    },
    CommandInfo{
        PITCH_SPELL_COMMAND,
        TranslatableString("action", "&Optimize enharmonic spelling"),
        TranslatableString("action", "Optimize enharmonic spelling"),
        InputSchema(),
        Decoration()
    },
    CommandInfo{
        PITCH_SPELL_SHARPS_COMMAND,
        TranslatableString("action", "Respell pitches with &sharps"),
        TranslatableString("action", "Respell pitches with sharps"),
        InputSchema(),
        Decoration()
    },
    CommandInfo{
        PITCH_SPELL_FLATS_COMMAND,
        TranslatableString("action", "Respell pitches with &flats"),
        TranslatableString("action", "Respell pitches with flats"),
        InputSchema(),
        Decoration()
    },
    CommandInfo{
        ENHARMONIC_SPELL_BOTH_COMMAND,
        TranslatableString("action", "Change enharmonic spelling (concert and transposed &pitch)"),
        TranslatableString("action", "Change enharmonic spelling (concert and transposed pitch)"),
        InputSchema(),
        Decoration()
    },
    CommandInfo{
        ENHARMONIC_SPELL_CURRENT_COMMAND,
        TranslatableString("action", "Change enharmonic spelling (&current pitch mode only)"),
        TranslatableString("action", "Change enharmonic spelling (current pitch mode only)"),
        InputSchema(),
        Decoration()
    },

    CommandInfo{
        REGROUP_RHYTHMS_COMMAND,
        TranslatableString("action", "Regroup &rhythms"),
        TranslatableString("action", "Regroup rhythms"),
        InputSchema(),
        Decoration()
    },
    CommandInfo{
        RESEQUENCE_REHEARSAL_MARKS_COMMAND,
        TranslatableString("action", "Resequence re&hearsal marks"),
        TranslatableString("action", "Resequence rehearsal marks"),
        InputSchema(),
        Decoration()
    },
    CommandInfo{
        UNROLL_REPEATS_COMMAND,
        TranslatableString("action", "U&nroll repeats"),
        TranslatableString("action", "Unroll repeats"),
        InputSchema(),
        Decoration()
    },
    CommandInfo{
        REPEAT_SELECTION_COMMAND,
        TranslatableString("action", "Repeat selection"),
        TranslatableString("action", "Repeat selection"),
        InputSchema(),
        Decoration()
    },

    CommandInfo{
        ADD_UP_BOW_COMMAND,
        TranslatableString("action", "Toggle up bow"),
        TranslatableString("action", "Add bowing: up bow"),
        InputSchema(),
        Decoration()
    },
    CommandInfo{
        ADD_DOWN_BOW_COMMAND,
        TranslatableString("action", "Toggle down bow"),
        TranslatableString("action", "Add bowing: down bow"),
        InputSchema(),
        Decoration()
    },
    CommandInfo{
        TRANSPOSE_UP_COMMAND,
        TranslatableString("action", "Transpose up"),
        TranslatableString("action", "Transpose up a semitone"),
        InputSchema(),
        Decoration()
    },
    CommandInfo{
        TRANSPOSE_DOWN_COMMAND,
        TranslatableString("action", "Transpose down"),
        TranslatableString("action", "Transpose down a semitone"),
        InputSchema(),
        Decoration()
    },
    CommandInfo{
        TOGGLE_MMREST_COMMAND,
        TranslatableString("action", "Toggle multimeasure rests"),
        TranslatableString("action", "Toggle multimeasure rests"),
        InputSchema(),
        Decoration()
    },
    CommandInfo{
        TOGGLE_HIDE_EMPTY_COMMAND,
        TranslatableString("action", "Toggle empty staves"),
        TranslatableString("action", "Show/hide empty staves"),
        InputSchema(),
        Decoration()
    },
    CommandInfo{
        MIRROR_NOTEHEAD_COMMAND,
        TranslatableString("action", "Mirror notehead"),
        TranslatableString("action", "Mirror notehead"),
        InputSchema(),
        Decoration()
    },

    // clef commands
    // clef commands
    makeCommandInfo<AddClefCommand>(),
    makeCommandInfo<AddClefViolinCommand>(),
    makeCommandInfo<AddClefBassCommand>(),

    CommandInfo{
        SET_VISIBLE_COMMAND,
        TranslatableString("action", "Set visible"),
        TranslatableString("action", "Make selected element(s) visible"),
        InputSchema(),
        Decoration()
    },
    CommandInfo{
        UNSET_VISIBLE_COMMAND,
        TranslatableString("action", "Set invisible"),
        TranslatableString("action", "Make selected element(s) invisible"),
        InputSchema(),
        Decoration()
    },
    CommandInfo{
        TOGGLE_AUTOPLACE_COMMAND,
        TranslatableString("action", "Toggle automatic placement for selected elements"),
        TranslatableString("action", "Toggle automatic placement for selected elements"),
        InputSchema(),
        Decoration()
    },
    CommandInfo{
        AUTOPLACE_ENABLED_COMMAND,
        TranslatableString("action", "Toggle automatic placement for entire score"),
        TranslatableString("action", "Toggle automatic placement for entire score"),
        InputSchema(),
        Decoration()
    },
    CommandInfo{
        ADD_FULL_MEASURE_REST_COMMAND,
        TranslatableString("action", "Full measure rest"),
        TranslatableString("action", "Insert full measure rest"),
        InputSchema(),
        Decoration()
    },

    // TAB commands
    CommandInfo {
        SET_DURATION_WHOLE_TAB_COMMAND,
        X_TAB.arg(TranslatableString("action", "Whole note")),
        X_TAB.arg(TranslatableString("action", "Set duration: whole note")),
        InputSchema(),
        Decoration(IconCode::Code::NOTE_WHOLE)
    },
    CommandInfo {
        SET_DURATION_HALF_TAB_COMMAND,
        X_TAB.arg(TranslatableString("action", "Half note")),
        X_TAB.arg(TranslatableString("action", "Set duration: half note")),
        InputSchema(),
        Decoration(IconCode::Code::NOTE_HALF)
    },
    CommandInfo {
        SET_DURATION_QUARTER_TAB_COMMAND,
        X_TAB.arg(TranslatableString("action", "Quarter note")),
        X_TAB.arg(TranslatableString("action", "Set duration: quarter note")),
        InputSchema(),
        Decoration(IconCode::Code::NOTE_QUARTER)
    },
    CommandInfo {
        SET_DURATION_EIGHTH_TAB_COMMAND,
        X_TAB.arg(TranslatableString("action", "Eighth note")),
        X_TAB.arg(TranslatableString("action", "Set duration: eighth note")),
        InputSchema(),
        Decoration(IconCode::Code::NOTE_8TH)
    },
    CommandInfo {
        SET_DURATION_16TH_TAB_COMMAND,
        X_TAB.arg(TranslatableString("action", "16th note")),
        X_TAB.arg(TranslatableString("action", "Set duration: 16th note")),
        InputSchema(),
        Decoration(IconCode::Code::NOTE_16TH)
    },
    CommandInfo {
        SET_DURATION_32ND_TAB_COMMAND,
        X_TAB.arg(TranslatableString("action", "32nd note")),
        X_TAB.arg(TranslatableString("action", "Set duration: 32nd note")),
        InputSchema(),
        Decoration(IconCode::Code::NOTE_32ND)
    },
    CommandInfo {
        SET_DURATION_64TH_TAB_COMMAND,
        X_TAB.arg(TranslatableString("action", "64th note")),
        X_TAB.arg(TranslatableString("action", "Set duration: 64th note")),
        InputSchema(),
        Decoration(IconCode::Code::NOTE_64TH)
    },
    CommandInfo {
        SET_DURATION_128TH_TAB_COMMAND,
        X_TAB.arg(TranslatableString("action", "128th note")),
        X_TAB.arg(TranslatableString("action", "Set duration: 128th note")),
        InputSchema(),
        Decoration(IconCode::Code::NOTE_128TH)
    },
    CommandInfo {
        SET_DURATION_256TH_TAB_COMMAND,
        X_TAB.arg(TranslatableString("action", "256th note")),
        X_TAB.arg(TranslatableString("action", "Set duration: 256th note")),
        InputSchema(),
        Decoration(IconCode::Code::NOTE_256TH)
    },
    CommandInfo {
        SET_DURATION_512TH_TAB_COMMAND,
        X_TAB.arg(TranslatableString("action", "512th note")),
        X_TAB.arg(TranslatableString("action", "Set duration: 512th note")),
        InputSchema(),
        Decoration(IconCode::Code::NOTE_512TH)
    },
    CommandInfo {
        SET_DURATION_1024TH_TAB_COMMAND,
        X_TAB.arg(TranslatableString("action", "1024th note")),
        X_TAB.arg(TranslatableString("action", "Set duration: 1024th note")),
        InputSchema(),
        Decoration(IconCode::Code::NOTE_1024TH)
    },
    makeCommandInfo<EnterRestTabCommand>(),

    CommandInfo {
        ADD_STANDARD_BEND_COMMAND,
        TranslatableString("action", "Standard bend"),
        TranslatableString("action", "Add standard bend"),
        InputSchema(),
        Decoration(IconCode::Code::GUITAR_BEND_REGULAR)
    },
    CommandInfo {
        ADD_PRE_BEND_COMMAND,
        TranslatableString("action", "Pre-bend"),
        TranslatableString("action", "Add pre-bend"),
        InputSchema(),
        Decoration(IconCode::Code::GUITAR_PRE_BEND)
    },
    CommandInfo {
        ADD_GRACE_NOTE_BEND_COMMAND,
        TranslatableString("action", "Grace note bend"),
        TranslatableString("action", "Add grace note bend"),
        InputSchema(),
        Decoration(IconCode::Code::GUITAR_GRACE_NOTE_BEND)
    },
    CommandInfo {
        ADD_SLIGHT_BEND_COMMAND,
        TranslatableString("action", "Slight bend"),
        TranslatableString("action", "Add slight bend"),
        InputSchema(),
        Decoration(IconCode::Code::GUITAR_SLIGHT_BEND)
    },
    CommandInfo {
        ADD_DIVE_COMMAND,
        //: Standard guitar dive, i.e. a movement of the tremolo bar between two pitches
        TranslatableString("action", "Dive"),
        TranslatableString("action", "Add dive"),
        InputSchema(),
        Decoration(IconCode::Code::GUITAR_DIVE_REGULAR)
    },
    CommandInfo {
        ADD_PRE_DIVE_COMMAND,
        //: Pre-dive, i.e. a movement of the tremolo bar prepared before picking the note
        TranslatableString("action", "Pre-dive"),
        TranslatableString("action", "Add pre-dive"),
        InputSchema(),
        Decoration(IconCode::Code::GUITAR_PRE_DIVE)
    },
    CommandInfo {
        ADD_DIP_COMMAND,
        //: Dip, i.e. a quick touch of the tremolo bar after the note is picked
        TranslatableString("action", "Dip"),
        TranslatableString("action", "Add dip"),
        InputSchema(),
        Decoration(IconCode::Code::GUITAR_DIP_DOWN)
    },
    CommandInfo {
        ADD_SCOOP_COMMAND,
        //: Scoop, i.e. a quick movement of the tremolo bar at the start of the note
        TranslatableString("action", "Scoop"),
        TranslatableString("action", "Add scoop"),
        InputSchema(),
        Decoration(IconCode::Code::GUITAR_SCOOP)
    },
    CommandInfo {
        ADD_HAMMER_ON_PULL_OFF_COMMAND,
        TranslatableString("action", "Hammer-on/pull-off"),
        TranslatableString("action", "Add hammer-on/pull-off"),
        InputSchema(),
        Decoration()
    },

    CommandInfo {
        GOTO_STRING_ABOVE_COMMAND,
        TranslatableString("action", "String above (TAB)"),
        TranslatableString("action", "Go to string above (TAB)"),
        InputSchema(),
        Decoration()
    },
    CommandInfo {
        GOTO_STRING_BELOW_COMMAND,
        TranslatableString("action", "String below (TAB)"),
        TranslatableString("action", "Go to string below (TAB)"),
        InputSchema(),
        Decoration()
    },

    CommandInfo {
        TOGGLE_AUTOMATION_COMMAND,
        TranslatableString("action", "Automation"),
        TranslatableString("action", "Toggle automation"),
        InputSchema(),
        Decoration(IconCode::Code::AUTOMATION, rcommand::Checkable::Yes)
    },
    CommandInfo {
        SELECT_AUTOMATION_TYPE_COMMAND,
        TranslatableString::untranslatable("Automation type"),
        TranslatableString::untranslatable("Select automation type"),
        InputSchema({
            { "type", Arg(DataType::String, u"Automation type (dynamics, volume, pan)") },
        }),
        Decoration(rcommand::Checkable::Yes)
    },

    // screen commands

    CommandInfo{
        SCREEN_PUT_NOTE_COMMAND,
        TranslatableString("action", "Put note"),
        TranslatableString("action", "Put note"),
        InputSchema({
            { "pos_x", Arg(DataType::Float, u"X position") },
            { "pos_y", Arg(DataType::Float, u"Y position") },
            { "replace", Arg(DataType::Bool, u"Replace") },
            { "insert", Arg(DataType::Bool, u"Insert") },
        }),
        Decoration()
    },
    CommandInfo{
        SCREEN_REMOVE_NOTE_COMMAND,
        TranslatableString("action", "Remove note"),
        TranslatableString("action", "Remove note"),
        InputSchema({
            { "pos_x", Arg(DataType::Float, u"X position") },
            { "pos_y", Arg(DataType::Float, u"Y position") },
        }),
        Decoration()
    },
    CommandInfo{
        SCREEN_EDIT_TEXT_COMMAND,
        TranslatableString("action", "Edit text"),
        TranslatableString("action", "Edit text"),
        InputSchema({
            { "pos_x", Arg(DataType::Float, u"X cursor position") },
            { "pos_y", Arg(DataType::Float, u"Y cursor position") },
        }),
        Decoration()
    },
    CommandInfo{
        EDIT_ELEMENT_COMMAND,
        TranslatableString("action", "Edit element"),
        TranslatableString("action", "Edit current element"),
        InputSchema(),
        Decoration()
    },
    CommandInfo{
        SCREEN_EDIT_ELEMENT_COMMAND,
        TranslatableString("action", "Edit element"),
        TranslatableString("action", "Edit screen element"),
        InputSchema({
            { "pos_x", Arg(DataType::Float, u"X cursor position") },
            { "pos_y", Arg(DataType::Float, u"Y cursor position") },
        }),
        Decoration(),
        Availability::Internal  // only dispatch from view
    },

    // view commands
    CommandInfo {
        ZOOM_IN_COMMAND,
        TranslatableString("action", "Zoom in"),
        TranslatableString("action", "Zoom in"),
        InputSchema(),
        Decoration(IconCode::Code::ZOOM_IN)
    },
    CommandInfo {
        ZOOM_OUT_COMMAND,
        TranslatableString("action", "Zoom out"),
        TranslatableString("action", "Zoom out"),
        InputSchema(),
        Decoration(IconCode::Code::ZOOM_OUT)
    },
    CommandInfo {
        ZOOM_TO_100_COMMAND,
        TranslatableString("action", "Zoom to 100%"),
        TranslatableString("action", "Zoom to 100%"),
        InputSchema(),
        Decoration()
    },
    CommandInfo {
        ZOOM_TO_PAGE_WIDTH_COMMAND,
        TranslatableString("action", "Zoom to page width"),
        TranslatableString("action", "Zoom to page width"),
        InputSchema(),
        Decoration()
    },
    CommandInfo {
        ZOOM_TO_WHOLE_PAGE_COMMAND,
        TranslatableString("action", "Zoom to whole page"),
        TranslatableString("action", "Zoom to whole page"),
        InputSchema(),
        Decoration()
    },
    CommandInfo {
        ZOOM_TO_TWO_PAGES_COMMAND,
        TranslatableString("action", "Zoom to two pages"),
        TranslatableString("action", "Zoom to two pages"),
        InputSchema(),
        Decoration()
    },
    CommandInfo {
        ZOOM_TO_PERCENT_COMMAND,
        TranslatableString("action", "Zoom to percent"),
        TranslatableString("action", "Zoom to percent"),
        InputSchema({
            { "percent", Arg(DataType::Int, u"Percent") },
        }),
        Decoration()
    },

    CommandInfo {
        VIEW_MODE_PAGE_COMMAND,
        TranslatableString("action", "Page view"),
        TranslatableString("action", "Display page view"),
        InputSchema(),
        Decoration(IconCode::Code::PAGE_VIEW)
    },
    CommandInfo {
        VIEW_MODE_FLOAT_COMMAND,
        TranslatableString("action", "Floating"),
        TranslatableString("action", "Display floating view"),
        InputSchema(),
        Decoration()
    },
    CommandInfo {
        VIEW_MODE_CONTINUOUS_COMMAND,
        TranslatableString("action", "Continuous view (horizontal)"),
        TranslatableString("action", "Display continuous view (horizontal)"),
        InputSchema(),
        Decoration(IconCode::Code::CONTINUOUS_VIEW)
    },
    CommandInfo {
        VIEW_MODE_SINGLE_COMMAND,
        TranslatableString("action", "Continuous view (vertical)"),
        TranslatableString("action", "Display continuous view (vertical)"),
        InputSchema(),
        Decoration(IconCode::Code::CONTINUOUS_VIEW_VERTICAL)
    },

    CommandInfo {
        NEXT_SCREEN_COMMAND,
        TranslatableString("action", "Screen: Next"),
        TranslatableString("action", "Jump to next screen"),
        InputSchema(),
        Decoration()
    },
    CommandInfo {
        PREV_SCREEN_COMMAND,
        TranslatableString("action", "Screen: Previous"),
        TranslatableString("action", "Jump to previous screen"),
        InputSchema(),
        Decoration()
    },
    CommandInfo {
        NEXT_PAGE_COMMAND,
        TranslatableString("action", "Page: Next"),
        TranslatableString("action", "Jump to next page"),
        InputSchema(),
        Decoration()
    },
    CommandInfo {
        PREV_PAGE_COMMAND,
        TranslatableString("action", "Page: Previous"),
        TranslatableString("action", "Jump to previous page"),
        InputSchema(),
        Decoration()
    },
    CommandInfo {
        TOP_OF_FIRST_PAGE_COMMAND,
        TranslatableString("action", "Page: Top of first"),
        TranslatableString("action", "Jump to top of first page"),
        InputSchema(),
        Decoration()
    },
    CommandInfo {
        BOTTOM_OF_LAST_PAGE_COMMAND,
        TranslatableString("action", "Page: Bottom of last"),
        TranslatableString("action", "Jump to bottom of last page"),
        InputSchema(),
        Decoration()
    },

    CommandInfo {
        CONTEXT_MENU_OF_SELECTION_COMMAND,
        TranslatableString("action", "Context menu of selection"),
        TranslatableString("action", "Open context menu of selection"),
        InputSchema(),
        Decoration()
    },

    CommandInfo {
        SHOW_SEARCH_COMMAND,
        TranslatableString("action", "Show search"),
        TranslatableString("action", "Show search"),
        InputSchema(),
        Decoration()
    },

    // piano keyboard commands
    CommandInfo {
        PIANO_KEYBOARD_SET_NUMBER_OF_KEYS_COMMAND,
        TranslatableString("action", "Piano keyboard: Set number of keys"),
        TranslatableString("action", "Set number of keys for piano keyboard"),
        InputSchema({
            { "keys", Arg(DataType::Int, u"Number of keys") },
        }),
        Decoration()
    },

    // diagnostic commands
    CommandInfo {
        DIAGNOSTIC_VIEW_REDRAW_COMMAND,
        TranslatableString("action", "View redraw"),
        TranslatableString("action", "Diagnostic: Redraw the view"),
        InputSchema(),
        Decoration()
    },
    CommandInfo {
        SHOW_ELEMENT_BOUNDING_RECTS_COMMAND,
        TranslatableString("action", "Show element bounding rects"),
        TranslatableString("action", "Diagnostic: Show element bounding rects"),
        InputSchema(),
        Decoration()
    },
    CommandInfo {
        COLOR_ELEMENT_SHAPES_COMMAND,
        TranslatableString("action", "Color element shapes"),
        TranslatableString("action", "Diagnostic: Color element shapes"),
        InputSchema(),
        Decoration()
    },
    CommandInfo {
        SHOW_SEGMENT_SHAPES_COMMAND,
        TranslatableString("action", "Show segment shapes"),
        TranslatableString("action", "Diagnostic: Show segment shapes"),
        InputSchema(),
        Decoration()
    },
    CommandInfo {
        COLOR_SEGMENT_SHAPES_COMMAND,
        TranslatableString("action", "Color segment shapes"),
        TranslatableString("action", "Diagnostic: Color segment shapes"),
        InputSchema(),
        Decoration()
    },
    CommandInfo {
        SHOW_SKYLINES_COMMAND,
        TranslatableString("action", "Show Skylines"),
        TranslatableString("action", "Diagnostic: Show Skylines"),
        InputSchema(),
        Decoration()
    },
    CommandInfo {
        SHOW_SYSTEM_BOUNDING_RECTS_COMMAND,
        TranslatableString("action", "Show system bounding rects"),
        TranslatableString("action", "Diagnostic: Show system bounding rects"),
        InputSchema(),
        Decoration()
    },
    CommandInfo {
        SHOW_ELEMENT_MASKS_COMMAND,
        TranslatableString("action", "Show element masks"),
        TranslatableString("action", "Diagnostic: Show element masks"),
        InputSchema(),
        Decoration()
    },
    CommandInfo {
        SHOW_LINE_ATTACH_POINTS_COMMAND,
        TranslatableString("action", "Show line attach points"),
        TranslatableString("action", "Diagnostic: Show line attach points"),
        InputSchema(),
        Decoration()
    },
    CommandInfo {
        MARK_EMPTY_STAFF_COMMAND,
        TranslatableString("action", "Mark empty staff"),
        TranslatableString("action", "Diagnostic: Mark empty staff"),
        InputSchema(),
        Decoration()
    },
    CommandInfo {
        MARK_CORRUPTED_MEASURES_COMMAND,
        TranslatableString("action", "Mark corrupted measures"),
        TranslatableString("action", "Diagnostic: Mark corrupted measures"),
        InputSchema(),
        Decoration()
    },
    CommandInfo {
        SHOW_GAP_RESTS_COMMAND,
        TranslatableString("action", "Show gap rests"),
        TranslatableString("action", "Diagnostic: Show gap rests"),
        InputSchema(),
        Decoration()
    },
    CommandInfo {
        SHOW_ORIGIN_AND_COMBINED_COMMAND,
        TranslatableString("action", "Show origin and combined"),
        TranslatableString("action", "Diagnostic: Show origin and combined"),
        InputSchema(),
        Decoration()
    },
    CommandInfo {
        CHECK_FOR_SCORE_CORRUPTIONS_COMMAND,
        TranslatableString("action", "Check for score corruptions"),
        TranslatableString("action", "Diagnostic: Check for score corruptions"),
        InputSchema(),
        Decoration()
    }
};

std::string NotationCommandsRegister::moduleName() const
{
    return "notation";
}

const std::vector<Command>& NotationCommandsRegister::commandList() const
{
    static std::vector<Command> commands;
    if (commands.empty()) {
        commands.reserve(s_commandInfos.size());
        for (const auto& info : s_commandInfos) {
            commands.push_back(info.command);
        }
    }
    return commands;
}

const std::vector<CommandInfo>& NotationCommandsRegister::commandInfoList() const
{
    return s_commandInfos;
}
