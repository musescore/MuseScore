/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2021 MuseScore Limited and others
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

#include "notationactioncontroller.h"

#include "global/io/file.h"
#include "global/translation.h"
#include "global/types/ret.h"

#include "rcommand/actiontocommand.h"

#include "engraving/dom/harmony.h"
#include "engraving/dom/masterscore.h"
#include "engraving/dom/note.h"
#include "engraving/dom/chord.h"
#include "engraving/dom/text.h"
#include "engraving/dom/sig.h"
#include "engraving/editing/noteinput.h"

#include "notation/imasternotation.h"
#include "notation/inotation.h"
#include "notation/inotationautomation.h" // IWYU pragma: keep
#include "notation/inotationelements.h"
#include "notation/inotationmidiinput.h"
#include "notation/inotationnoteinput.h"
#include "notation/inotationplayback.h" // IWYU pragma: keep
#include "notation/inotationselection.h"
#include "notation/inotationstyle.h"
#include "notation/inotationundostack.h"
#include "notation/inotationinteraction.h"

#include "project/inotationproject.h"

#include "qml/MuseScore/NotationScene/abstractelementpopupmodel.h"
#include "qml/MuseScore/NotationScene/notationviewinputcontroller.h"

#include "../notationcommands.h"

#include "log.h"
#include "rcommand/commandtypes.h"

using namespace mu;
using namespace muse;
using namespace muse::io;
using namespace mu::notation;
using namespace muse::actions;
using namespace mu::context;

static constexpr qreal STRETCH_STEP = 0.1;
static constexpr bool NEAR_NOTE_OR_REST = true;

static constexpr bool DONT_PLAY_CHORD = false;

using EngravingDebuggingOptions = engraving::IEngravingConfiguration::DebuggingOptions;
static const std::map<muse::rcommand::Command, bool EngravingDebuggingOptions::*> s_debuggingCommands = {
    { SHOW_ELEMENT_BOUNDING_RECTS_COMMAND, &EngravingDebuggingOptions::showElementBoundingRects },
    { COLOR_ELEMENT_SHAPES_COMMAND, &EngravingDebuggingOptions::colorElementShapes },
    { SHOW_SEGMENT_SHAPES_COMMAND, &EngravingDebuggingOptions::showSegmentShapes },
    { COLOR_SEGMENT_SHAPES_COMMAND, &EngravingDebuggingOptions::colorSegmentShapes },
    { SHOW_SKYLINES_COMMAND, &EngravingDebuggingOptions::showSkylines },
    { SHOW_SYSTEM_BOUNDING_RECTS_COMMAND, &EngravingDebuggingOptions::showSystemBoundingRects },
    { SHOW_ELEMENT_MASKS_COMMAND, &EngravingDebuggingOptions::showElementMasks },
    { SHOW_LINE_ATTACH_POINTS_COMMAND, &EngravingDebuggingOptions::showLineAttachPoints },
    { MARK_EMPTY_STAFF_COMMAND, &EngravingDebuggingOptions::markEmptyStaffVisibilityOverrides },
    { MARK_CORRUPTED_MEASURES_COMMAND, &EngravingDebuggingOptions::markCorruptedMeasures },
    { SHOW_GAP_RESTS_COMMAND, &EngravingDebuggingOptions::showGapRests },
    { SHOW_ORIGIN_AND_COMBINED_COMMAND, &EngravingDebuggingOptions::showOriginAndCombinedStaves },
};

//! NOTE Just for more readable
using Controller = NotationActionController;
using Interaction = INotationInteraction;
using ViewController = INotationViewController;

// tuplet options convertor
static muse::rcommand::CommandQuery tupletOptions(const rcommand::Command& command, const ActionData& args)
{
    IF_ASSERT_FAILED(args.count() > 0) {
        return muse::rcommand::CommandQuery();
    }
    TupletOptions options = args.arg<TupletOptions>(0);

    rcommand::CommandQuery query(command);
    query.addParam("ratio", Val(options.ratio.toString().toStdString()));
    query.addParam("number-type", Val(engraving::str_conv(options.numberType)));
    query.addParam("bracket-type", Val(engraving::str_conv(options.bracketType)));
    query.addParam("auto-baselen", Val(options.autoBaseLen));
    return query;
}

void NotationActionController::init()
{
    TRACEFUNC;

    // global commands
    registerCommand(CANCEL_COMMAND, &Controller::resetState);
    registerCommand(UNDO_COMMAND, &Interaction::undo);
    registerCommand(REDO_COMMAND, &Interaction::redo);

    // navigation and selection commands

    registerCommandWithParams(SELECT_COMMAND, &Controller::select);
    registerCommand(GOTO_FIRST_ELEMENT_COMMAND, [this]() { select(SelectionTarget::FirstItem, PlayMode::PlayChord); });
    registerCommand(GOTO_LAST_ELEMENT_COMMAND, [this]() { select(SelectionTarget::LastItem, PlayMode::PlayChord); });
    registerCommand(GOTO_NEXT_ELEMENT_COMMAND, [this]() { select(SelectionTarget::NextItem, PlayMode::PlayNote); });
    registerCommand(GOTO_PREV_ELEMENT_COMMAND, [this]() { select(SelectionTarget::PrevItem, PlayMode::PlayNote); });
    registerCommand(GOTO_NEXT_SEGMENT_ELEMENT_COMMAND, [this]() { select(SelectionTarget::NextSegmentItem, PlayMode::PlayNote); });
    registerCommand(GOTO_PREV_SEGMENT_ELEMENT_COMMAND, [this]() { select(SelectionTarget::PrevSegmentItem, PlayMode::PlayNote); });
    registerCommand(GOTO_NEXT_TRACK_COMMAND, [this]() { select(SelectionTarget::NextTrack, PlayMode::PlayChord); });
    registerCommand(GOTO_PREV_TRACK_COMMAND, [this]() { select(SelectionTarget::PrevTrack, PlayMode::PlayChord); });
    registerCommand(GOTO_NEXT_FRAME_COMMAND, [this]() { select(SelectionTarget::NextFrame); });
    registerCommand(GOTO_PREV_FRAME_COMMAND, [this]() { select(SelectionTarget::PrevFrame); });
    registerCommand(GOTO_NEXT_SYSTEM_COMMAND, [this]() { select(SelectionTarget::NextSystem); });
    registerCommand(GOTO_PREV_SYSTEM_COMMAND, [this]() { select(SelectionTarget::PrevSystem); });
    registerCommand(GOTO_UPNOTE_IN_CHORD_COMMAND, [this]() { select(SelectionTarget::UpNoteInChord); });
    registerCommand(GOTO_DOWNNOTE_IN_CHORD_COMMAND, [this]() { select(SelectionTarget::DownNoteInChord); });
    registerCommand(GOTO_TOPNOTE_IN_CHORD_COMMAND, [this]() { select(SelectionTarget::TopNoteInChord); });
    registerCommand(GOTO_BOTTOMNOTE_IN_CHORD_COMMAND, [this]() { select(SelectionTarget::BottomNoteInChord); });
    registerCommand(GOTO_TOP_STAFF_COMMAND, [this]() { select(SelectionTarget::TopStaff, PlayMode::PlayChord); });
    registerCommand(GOTO_EMPTY_TRAILING_MEASURE_COMMAND, [this]() { select(SelectionTarget::EmptyTrailingMeasure); });
    registerCommand(SELECT_SIMILAR_COMMAND, [this]() { select(SelectionTarget::Similar); });
    registerCommand(SELECT_SIMILAR_IN_STAFF_COMMAND, [this]() { select(SelectionTarget::SimilarInStaff); });
    registerCommand(SELECT_SIMILAR_IN_RANGE_COMMAND, [this]() { select(SelectionTarget::SimilarInRange); });
    registerCommand(SELECT_NOTES_IN_CHORD_COMMAND, [this]() { select(SelectionTarget::NotesInChord); });
    registerCommand(SELECT_ALL_COMMAND, [this]() { select(SelectionTarget::All); });
    registerCommand(SELECT_SECTION_COMMAND, [this]() { select(SelectionTarget::Section); });

    registerCommand(GET_LOCATION_COMMAND, &Interaction::getLocation);

    registerCommand(ADD_TO_SELECTION_NEXT_CHORD_COMMAND, &Interaction::addToSelection, SelectionTarget::NextChord);
    registerCommand(ADD_TO_SELECTION_PREV_CHORD_COMMAND, &Interaction::addToSelection, SelectionTarget::PrevChord);
    registerCommand(ADD_TO_SELECTION_NEXT_MEASURE_COMMAND, &Interaction::addToSelection, SelectionTarget::NextMeasure);
    registerCommand(ADD_TO_SELECTION_PREV_MEASURE_COMMAND, &Interaction::addToSelection, SelectionTarget::PrevMeasure);
    registerCommand(ADD_TO_SELECTION_ABOVE_STAFF_COMMAND, &Interaction::addToSelection, SelectionTarget::AboveStaff);
    registerCommand(ADD_TO_SELECTION_BELOW_STAFF_COMMAND, &Interaction::addToSelection, SelectionTarget::BelowStaff);
    registerCommand(ADD_TO_SELECTION_BEGIN_SYSTEM_COMMAND, &Interaction::expandSelection, ExpandSelectionMode::BeginSystem);
    registerCommand(ADD_TO_SELECTION_END_SYSTEM_COMMAND, &Interaction::expandSelection, ExpandSelectionMode::EndSystem);
    registerCommand(ADD_TO_SELECTION_BEGIN_SCORE_COMMAND, &Interaction::expandSelection, ExpandSelectionMode::BeginScore);
    registerCommand(ADD_TO_SELECTION_END_SCORE_COMMAND, &Interaction::expandSelection, ExpandSelectionMode::EndScore);

    registerCommand(OPEN_SELECTION_OPTIONS_COMMAND, &Controller::openSelectionMoreOptions);

    // text navigation commands
    registerCommand(EDITTEXT_NEXT_WORD_COMMAND, [this]() { nextWord(); });
    registerCommand(EDITTEXT_NEXT_ELEMENT_COMMAND, [this]() { nextTextElement(); });
    registerCommand(EDITTEXT_PREV_ELEMENT_COMMAND, [this]() { prevTextElement(); });
    registerCommand(EDITTEXT_NEXT_BEAT_COMMAND, &Controller::nextBeatTextElement, &Controller::textNavigationByBeatsAvailable);
    registerCommand(EDITTEXT_PREV_BEAT_COMMAND, &Controller::prevBeatTextElement, &Controller::textNavigationByBeatsAvailable);

    registerNavigationByFractionCommand(EDITTEXT_ADVANCE_LONGA_COMMAND, Fraction(4, 1));
    registerNavigationByFractionCommand(EDITTEXT_ADVANCE_BREVE_COMMAND, Fraction(2, 1));
    registerNavigationByFractionCommand(EDITTEXT_ADVANCE_1_COMMAND, Fraction(1, 1));
    registerNavigationByFractionCommand(EDITTEXT_ADVANCE_2_COMMAND, Fraction(1, 2));
    registerNavigationByFractionCommand(EDITTEXT_ADVANCE_4_COMMAND, Fraction(1, 4));
    registerNavigationByFractionCommand(EDITTEXT_ADVANCE_8_COMMAND, Fraction(1, 8));
    registerNavigationByFractionCommand(EDITTEXT_ADVANCE_16_COMMAND, Fraction(1, 16));
    registerNavigationByFractionCommand(EDITTEXT_ADVANCE_32_COMMAND, Fraction(1, 32));
    registerNavigationByFractionCommand(EDITTEXT_ADVANCE_64_COMMAND, Fraction(1, 64));

    // lyrics editing commands
    registerCommand(EDITLYRIC_NEXT_VERSE_COMMAND, &Interaction::navigateToLyricsVerse, MoveDirection::Down);
    registerCommand(EDITLYRIC_PREV_VERSE_COMMAND, &Interaction::navigateToLyricsVerse, MoveDirection::Up);
    registerCommand(EDITLYRIC_NEXT_SYLLABLE_COMMAND, &Interaction::navigateToNextSyllable);
    registerCommand(EDITLYRIC_ADD_MELISMA_COMMAND, &Interaction::addMelisma);
    registerCommand(EDITLYRIC_ADD_VERSE_COMMAND, &Interaction::addLyricsVerse);

    // text editing commands
    registerCommand(EDITTEXT_TOGGLE_BOLD_COMMAND, &Interaction::toggleBold);
    registerCommand(EDITTEXT_TOGGLE_ITALIC_COMMAND, &Interaction::toggleItalic);
    registerCommand(EDITTEXT_TOGGLE_UNDERLINE_COMMAND, &Interaction::toggleUnderline);
    registerCommand(EDITTEXT_TOGGLE_STRIKE_COMMAND, &Interaction::toggleStrike);
    registerCommand(EDITTEXT_TOGGLE_SUBSCRIPT_COMMAND, &Interaction::toggleSubScript);
    registerCommand(EDITTEXT_TOGGLE_SUPERSCRIPT_COMMAND, &Interaction::toggleSuperScript);

    // note input commands
    registerNoteInputCommand(TOGGLE_NOTE_INPUT_COMMAND, NoteInputMethod::UNKNOWN /*default*/);
    registerNoteInputCommand(TOGGLE_NOTE_INPUT_BY_NOTE_NAME_COMMAND, NoteInputMethod::BY_NOTE_NAME);
    registerNoteInputCommand(TOGGLE_NOTE_INPUT_BY_DURATION_COMMAND, NoteInputMethod::BY_DURATION);
    registerNoteInputCommand(TOGGLE_NOTE_INPUT_RHYTHM_COMMAND, NoteInputMethod::RHYTHM);
    registerNoteInputCommand(TOGGLE_NOTE_INPUT_REPITCH_COMMAND, NoteInputMethod::REPITCH);
    registerNoteInputCommand(TOGGLE_NOTE_INPUT_REALTIME_AUTO_COMMAND, NoteInputMethod::REALTIME_AUTO);
    registerNoteInputCommand(TOGGLE_NOTE_INPUT_REALTIME_MANUAL_COMMAND, NoteInputMethod::REALTIME_MANUAL);
    registerNoteInputCommand(TOGGLE_NOTE_INPUT_TIMEWISE_COMMAND, NoteInputMethod::TIMEWISE);
    registerCommand(TOGGLE_INSERT_MODE_COMMAND, [this]() { toggleNoteInputInsert(); }, &NotationActionController::isNotEditingElement);

    registerCommand(REALTIME_ADVANCE_COMMAND, &Controller::realtimeAdvance);

    registerCommand(SET_DURATION_LONGA_COMMAND, [this]() { setDuration(DurationType::V_LONG); });
    registerCommand(SET_DURATION_BREVE_COMMAND, [this]() { setDuration(DurationType::V_BREVE); });
    registerCommand(SET_DURATION_WHOLE_COMMAND, [this]() { setDuration(DurationType::V_WHOLE); });
    registerCommand(SET_DURATION_HALF_COMMAND, [this]() { setDuration(DurationType::V_HALF); });
    registerCommand(SET_DURATION_QUARTER_COMMAND, [this]() { setDuration(DurationType::V_QUARTER); });
    registerCommand(SET_DURATION_EIGHTH_COMMAND, [this]() { setDuration(DurationType::V_EIGHTH); });
    registerCommand(SET_DURATION_16TH_COMMAND, [this]() { setDuration(DurationType::V_16TH); });
    registerCommand(SET_DURATION_32ND_COMMAND, [this]() { setDuration(DurationType::V_32ND); });
    registerCommand(SET_DURATION_64TH_COMMAND, [this]() { setDuration(DurationType::V_64TH); });
    registerCommand(SET_DURATION_128TH_COMMAND, [this]() { setDuration(DurationType::V_128TH); });
    registerCommand(SET_DURATION_256TH_COMMAND, [this]() { setDuration(DurationType::V_256TH); });
    registerCommand(SET_DURATION_512TH_COMMAND, [this]() { setDuration(DurationType::V_512TH); });
    registerCommand(SET_DURATION_1024TH_COMMAND, [this]() { setDuration(DurationType::V_1024TH); });

    registerCommand(SET_DOUBLE_DURATION_COMMAND, [this]() { increaseDecreaseDuration(1, false); });
    registerCommand(SET_HALVE_DURATION_COMMAND, [this]() { increaseDecreaseDuration(-1, false); });
    registerCommand(SET_DOUBLE_DURATION_DOTTED_COMMAND, [this]() { increaseDecreaseDuration(1, true); });
    registerCommand(SET_HALVE_DURATION_DOTTED_COMMAND, [this]() { increaseDecreaseDuration(-1, true); });

    registerCommand(EXTEND_TO_NEXT_NOTE_COMMAND, &Interaction::extendToNextNote);

    registerCommand(TOGGLE_DOT_COMMAND, [this]() { toggleDots(1); });
    registerCommand(TOGGLE_DOT2_COMMAND, [this]() { toggleDots(2); });
    registerCommand(TOGGLE_DOT3_COMMAND, [this]() { toggleDots(3); });
    registerCommand(TOGGLE_DOT4_COMMAND, [this]() { toggleDots(4); });

    registerCommand(TOGGLE_REST_COMMAND, [this]() { toggleRest(); });

    registerCommand(TOGGLE_FLAT2_COMMAND, [this]() { toggleAccidental(AccidentalType::FLAT2); });
    registerCommand(TOGGLE_FLAT_COMMAND, [this]() { toggleAccidental(AccidentalType::FLAT); });
    registerCommand(TOGGLE_NAT_COMMAND, [this]() { toggleAccidental(AccidentalType::NATURAL); });
    registerCommand(TOGGLE_SHARP_COMMAND, [this]() { toggleAccidental(AccidentalType::SHARP); });
    registerCommand(TOGGLE_SHARP2_COMMAND, [this]() { toggleAccidental(AccidentalType::SHARP2); });

    registerCommand(ADD_SHARP2_COMMAND, &Interaction::changeAccidental, mu::engraving::AccidentalType::SHARP2, PlayMode::PlayNote);
    registerCommand(ADD_SHARP_COMMAND, &Interaction::changeAccidental, mu::engraving::AccidentalType::SHARP, PlayMode::PlayNote);
    registerCommand(ADD_NAT_COMMAND, &Interaction::changeAccidental, mu::engraving::AccidentalType::NATURAL, PlayMode::PlayNote);
    registerCommand(ADD_FLAT_COMMAND, &Interaction::changeAccidental, mu::engraving::AccidentalType::FLAT, PlayMode::PlayNote);
    registerCommand(ADD_FLAT2_COMMAND, &Interaction::changeAccidental, mu::engraving::AccidentalType::FLAT2, PlayMode::PlayNote);

    registerCommand(TOGGLE_TIE_COMMAND, &Controller::addTie);
    registerCommand(ADD_SLUR_COMMAND, &Controller::addSlur);
    registerCommand(TOGGLE_LV_COMMAND, &Controller::addLaissezVib);
    registerCommand(TOGGLE_MARCATO_COMMAND, [this]() { toggleArticulation(SymbolId::articMarcatoAbove); });
    registerCommand(TOGGLE_SFORZATO_COMMAND, [this]() { toggleArticulation(SymbolId::articAccentAbove); });
    registerCommand(TOGGLE_TENUTO_COMMAND, [this]() { toggleArticulation(SymbolId::articTenutoAbove); });
    registerCommand(TOGGLE_STACCATO_COMMAND, [this]() { toggleArticulation(SymbolId::articStaccatoAbove); });

    registerCommand(USE_VOICE_1_COMMAND, [this]() { changeVoice(0); });
    registerCommand(USE_VOICE_2_COMMAND, [this]() { changeVoice(1); });
    registerCommand(USE_VOICE_3_COMMAND, [this]() { changeVoice(2); });
    registerCommand(USE_VOICE_4_COMMAND, [this]() { changeVoice(3); });
    registerCommand(SWAP_VOICE_X12_COMMAND, [this]() { swapVoices(0, 1); });
    registerCommand(SWAP_VOICE_X13_COMMAND, [this]() { swapVoices(0, 2); });
    registerCommand(SWAP_VOICE_X14_COMMAND, [this]() { swapVoices(0, 3); });
    registerCommand(SWAP_VOICE_X23_COMMAND, [this]() { swapVoices(1, 2); });
    registerCommand(SWAP_VOICE_X24_COMMAND, [this]() { swapVoices(1, 3); });
    registerCommand(SWAP_VOICE_X34_COMMAND, [this]() { swapVoices(2, 3); });

    registerCommand(FLIP_COMMAND, &Interaction::flipSelection);
    registerCommand(FLIP_HORIZONTALLY_COMMAND, &Interaction::flipSelectionHorizontally);

    registerCommandWithParams(ADD_NOTE_COMMAND, &Controller::addNote);
    registerCommandWithParams(ADD_DRUM_NOTE_COMMAND, &Controller::addDrumNote);

    registerNoteCommand(ENTER_NOTE_C_COMMAND, NoteName::C);
    registerNoteCommand(ENTER_NOTE_D_COMMAND, NoteName::D);
    registerNoteCommand(ENTER_NOTE_E_COMMAND, NoteName::E);
    registerNoteCommand(ENTER_NOTE_F_COMMAND, NoteName::F);
    registerNoteCommand(ENTER_NOTE_G_COMMAND, NoteName::G);
    registerNoteCommand(ENTER_NOTE_A_COMMAND, NoteName::A);
    registerNoteCommand(ENTER_NOTE_B_COMMAND, NoteName::B);
    registerNoteCommand(ADD_NOTE_C_COMMAND, NoteName::C, NoteAddingMode::CurrentChord);
    registerNoteCommand(ADD_NOTE_D_COMMAND, NoteName::D, NoteAddingMode::CurrentChord);
    registerNoteCommand(ADD_NOTE_E_COMMAND, NoteName::E, NoteAddingMode::CurrentChord);
    registerNoteCommand(ADD_NOTE_F_COMMAND, NoteName::F, NoteAddingMode::CurrentChord);
    registerNoteCommand(ADD_NOTE_G_COMMAND, NoteName::G, NoteAddingMode::CurrentChord);
    registerNoteCommand(ADD_NOTE_A_COMMAND, NoteName::A, NoteAddingMode::CurrentChord);
    registerNoteCommand(ADD_NOTE_B_COMMAND, NoteName::B, NoteAddingMode::CurrentChord);
    registerNoteCommand(INSERT_NOTE_C_COMMAND, NoteName::C, NoteAddingMode::InsertChord);
    registerNoteCommand(INSERT_NOTE_D_COMMAND, NoteName::D, NoteAddingMode::InsertChord);
    registerNoteCommand(INSERT_NOTE_E_COMMAND, NoteName::E, NoteAddingMode::InsertChord);
    registerNoteCommand(INSERT_NOTE_F_COMMAND, NoteName::F, NoteAddingMode::InsertChord);
    registerNoteCommand(INSERT_NOTE_G_COMMAND, NoteName::G, NoteAddingMode::InsertChord);
    registerNoteCommand(INSERT_NOTE_A_COMMAND, NoteName::A, NoteAddingMode::InsertChord);
    registerNoteCommand(INSERT_NOTE_B_COMMAND, NoteName::B, NoteAddingMode::InsertChord);

    registerCommand(ENTER_REST_COMMAND, &Interaction::putRestToSelection);

    registerCommand(OPEN_TUPLET_CONFIGURE_COMMAND, [this]() { openTupletOtherDialog(); });
    registerCommandWithParams(ADD_TUPLET_COMMAND, &Controller::putTuplet);
    registerCommand(ADD_DUPLET_COMMAND, [this]() { putTuplet(2); });
    registerCommand(ADD_TRIPLET_COMMAND, [this]() { putTuplet(3); });
    registerCommand(ADD_QUADRUPLET_COMMAND, [this]() { putTuplet(4); });
    registerCommand(ADD_QUINTUPLET_COMMAND, [this]() { putTuplet(5); });
    registerCommand(ADD_SEXTUPLET_COMMAND, [this]() { putTuplet(6); });
    registerCommand(ADD_SEPTUPLET_COMMAND, [this]() { putTuplet(7); });
    registerCommand(ADD_OCTUPLET_COMMAND, [this]() { putTuplet(8); });
    registerCommand(ADD_NONUPLET_COMMAND, [this]() { putTuplet(9); });

    registerCommand(INSERT_HBOX_COMMAND, [this]() { addBoxes(BoxType::Horizontal, 1, AddBoxesTarget::BeforeSelection); });
    registerCommand(INSERT_VBOX_COMMAND, [this]() { addBoxes(BoxType::Vertical, 1, AddBoxesTarget::BeforeSelection); });
    registerCommand(INSERT_TEXTFRAME_COMMAND, [this]() { addBoxes(BoxType::Text, 1, AddBoxesTarget::BeforeSelection); });
    registerCommand(INSERT_FRETFRAME_COMMAND, [this]() { addBoxes(BoxType::Fret, 1, AddBoxesTarget::BeforeSelection); });
    registerCommand(APPEND_HBOX_COMMAND, [this]() { addBoxes(BoxType::Horizontal, 1, AddBoxesTarget::AtEndOfScore); });
    registerCommand(APPEND_VBOX_COMMAND, [this]() { addBoxes(BoxType::Vertical, 1, AddBoxesTarget::AtEndOfScore); });
    registerCommand(APPEND_TEXTFRAME_COMMAND, [this]() { addBoxes(BoxType::Text, 1, AddBoxesTarget::AtEndOfScore); });
    registerCommand(APPEND_FRETFRAME_COMMAND, [this]() { addBoxes(BoxType::Fret, 1, AddBoxesTarget::AtEndOfScore); });

    registerCommand(ADD_FRETBOARD_DIAGRAM_COMMAND, &Controller::addFretboardDiagram);

    registerCommand(ADD_OTTAVA_8VA_COMMAND, &Interaction::addOttavaToSelection, OttavaType::OTTAVA_8VA);
    registerCommand(ADD_OTTAVA_8VB_COMMAND, &Interaction::addOttavaToSelection, OttavaType::OTTAVA_8VB);

    registerCommand(ADD_DYNAMIC_COMMAND, &Interaction::toggleDynamicPopup);
    registerCommand(ADD_HAIRPIN_COMMAND, &Interaction::addHairpinsToSelection, HairpinType::CRESC_HAIRPIN);
    registerCommand(ADD_HAIRPIN_REVERSE_COMMAND, &Interaction::addHairpinsToSelection, HairpinType::DIM_HAIRPIN);
    registerCommand(INCREASE_DYNAMIC_COMMAND, &Interaction::increaseDecreaseSelectedDynamicsValues, /*delta*/ 1);
    registerCommand(DECREASE_DYNAMIC_COMMAND, &Interaction::increaseDecreaseSelectedDynamicsValues, /*delta*/ -1);
    registerCommand(ADD_NOTELINE_COMMAND, &Interaction::addAnchoredLineToSelectedNotes);

    registerCommand(ADD_IMAGE_COMMAND, [this]() { addImage(); });

    registerCommand(ADD_UP_BOW_COMMAND, &Interaction::toggleArticulation, mu::engraving::SymId::stringsUpBow);
    registerCommand(ADD_DOWN_BOW_COMMAND, &Interaction::toggleArticulation, mu::engraving::SymId::stringsDownBow);

    // add text commands
    registerCommand(ADD_TITLE_TEXT_COMMAND, [this]() { addText(TextStyleType::TITLE); });
    registerCommand(ADD_SUBTITLE_TEXT_COMMAND, [this]() { addText(TextStyleType::SUBTITLE); });
    registerCommand(ADD_COMPOSER_TEXT_COMMAND, [this]() { addText(TextStyleType::COMPOSER); });
    registerCommand(ADD_LYRICIST_TEXT_COMMAND, [this]() { addText(TextStyleType::LYRICIST); });
    registerCommand(ADD_PART_TEXT_COMMAND, [this]() { addText(TextStyleType::INSTRUMENT_EXCERPT); });
    registerCommand(ADD_FRAME_TEXT_COMMAND, [this]() { addText(TextStyleType::FRAME); });
    registerCommand(ADD_SYSTEM_TEXT_COMMAND, [this]() { addText(TextStyleType::SYSTEM); });
    registerCommand(ADD_STAFF_TEXT_COMMAND, [this]() { addText(TextStyleType::STAFF); });
    registerCommand(ADD_EXPRESSION_TEXT_COMMAND, [this]() { addText(TextStyleType::EXPRESSION); });
    registerCommand(ADD_REHEARSALMARK_TEXT_COMMAND, [this]() { addText(TextStyleType::REHEARSAL_MARK); });
    registerCommand(ADD_INSTRUMENT_CHANGE_TEXT_COMMAND, [this]() { addText(TextStyleType::INSTRUMENT_CHANGE); });
    registerCommand(ADD_FINGERING_TEXT_COMMAND, [this]() { addText(TextStyleType::FINGERING); });
    registerCommand(ADD_STICKING_TEXT_COMMAND, [this]() { addText(TextStyleType::STICKING); });
    registerCommand(ADD_CHORD_TEXT_COMMAND, [this]() { addText(TextStyleType::HARMONY_A); });
    registerCommand(ADD_ROMAN_NUMERAL_TEXT_COMMAND, [this]() { addText(TextStyleType::HARMONY_ROMAN); });
    registerCommand(ADD_NASHVILLE_NUMBER_TEXT_COMMAND, [this]() { addText(TextStyleType::HARMONY_NASHVILLE); });
    registerCommand(ADD_LYRICS_COMMAND, [this]() { addText(TextStyleType::LYRICS_ODD); });
    registerCommand(ADD_TEMPO_COMMAND, [this]() { addText(TextStyleType::TEMPO); });
    registerCommand(ADD_FIGURED_BASS_COMMAND, [this]() { addFiguredBass(); });

    // add grace notes commands
    registerCommand(ADD_ACCIACCATURA_COMMAND, &Interaction::addGraceNotesToSelectedNotes, GraceNoteType::ACCIACCATURA);
    registerCommand(ADD_APPOGGIATURA_COMMAND, &Interaction::addGraceNotesToSelectedNotes, GraceNoteType::APPOGGIATURA);
    registerCommand(ADD_GRACE4_COMMAND, &Interaction::addGraceNotesToSelectedNotes, GraceNoteType::GRACE4);
    registerCommand(ADD_GRACE16_COMMAND, &Interaction::addGraceNotesToSelectedNotes, GraceNoteType::GRACE16);
    registerCommand(ADD_GRACE32_COMMAND, &Interaction::addGraceNotesToSelectedNotes, GraceNoteType::GRACE32);
    registerCommand(ADD_GRACE8_AFTER_COMMAND, &Interaction::addGraceNotesToSelectedNotes, GraceNoteType::GRACE8_AFTER);
    registerCommand(ADD_GRACE16_AFTER_COMMAND, &Interaction::addGraceNotesToSelectedNotes, GraceNoteType::GRACE16_AFTER);
    registerCommand(ADD_GRACE32_AFTER_COMMAND, &Interaction::addGraceNotesToSelectedNotes, GraceNoteType::GRACE32_AFTER);

    // add beam commands
    registerCommand(ADD_BEAM_AUTO_COMMAND, &Interaction::addBeamToSelectedChordRests, BeamMode::AUTO);
    registerCommand(ADD_BEAM_NONE_COMMAND, &Interaction::addBeamToSelectedChordRests, BeamMode::NONE);
    registerCommand(ADD_BEAM_BEGIN_COMMAND, &Interaction::addBeamToSelectedChordRests, BeamMode::BEGIN);
    registerCommand(ADD_BEAM_BEGIN16_COMMAND, &Interaction::addBeamToSelectedChordRests, BeamMode::BEGIN16);
    registerCommand(ADD_BEAM_BEGIN32_COMMAND, &Interaction::addBeamToSelectedChordRests, BeamMode::BEGIN32);
    registerCommand(ADD_BEAM_MID_COMMAND, &Interaction::addBeamToSelectedChordRests, BeamMode::MID);
    registerCommand(ADD_BEAM_SELECTED_RANGE_COMMAND, &Interaction::beamSelectedRange);

    // add brackets commands
    registerCommand(ADD_BRACKETS_COMMAND, &Interaction::addBracketsToSelection, BracketsType::Brackets);
    registerCommand(ADD_BRACES_COMMAND, &Interaction::addBracketsToSelection, BracketsType::Braces);
    registerCommand(ADD_PARENTHESES_COMMAND, &Interaction::addBracketsToSelection, BracketsType::Parentheses);

    // add ornament commands
    registerCommand(ADD_TURN_COMMAND, &Interaction::toggleOrnament, mu::engraving::SymId::ornamentTurn);
    registerCommand(ADD_TURN_INVERTED_COMMAND, &Interaction::toggleOrnament, mu::engraving::SymId::ornamentTurnInverted);
    registerCommand(ADD_TURN_SLASH_COMMAND, &Interaction::toggleOrnament, mu::engraving::SymId::ornamentTurnSlash);
    registerCommand(ADD_TURN_UP_COMMAND, &Interaction::toggleOrnament, mu::engraving::SymId::ornamentTurnUp);
    registerCommand(ADD_TURN_INVERTED_UP_COMMAND, &Interaction::toggleOrnament, mu::engraving::SymId::ornamentTurnUpS);
    registerCommand(ADD_TRILL_COMMAND, &Interaction::toggleOrnament, mu::engraving::SymId::ornamentTrill);
    registerCommand(ADD_SHORT_TRILL_COMMAND, &Interaction::toggleOrnament, mu::engraving::SymId::ornamentShortTrill);
    registerCommand(ADD_MORDENT_COMMAND, &Interaction::toggleOrnament, mu::engraving::SymId::ornamentMordent);
    registerCommand(ADD_HAYDN_COMMAND, &Interaction::toggleOrnament, mu::engraving::SymId::ornamentHaydn);
    registerCommand(ADD_TREMBLEMENT_COMMAND, &Interaction::toggleOrnament, mu::engraving::SymId::ornamentTremblement);
    registerCommand(ADD_PRALL_MORDENT_COMMAND, &Interaction::toggleOrnament, mu::engraving::SymId::ornamentPrallMordent);
    registerCommand(ADD_SHAKE_COMMAND, &Interaction::toggleOrnament, mu::engraving::SymId::ornamentShake3);
    registerCommand(ADD_SHAKE_MUFFAT_COMMAND, &Interaction::toggleOrnament, mu::engraving::SymId::ornamentShakeMuffat1);
    registerCommand(ADD_TREMBLEMENT_COUPERIN_COMMAND, &Interaction::toggleOrnament, mu::engraving::SymId::ornamentTremblementCouperin);

    // clef commands
    registerCommand(ADD_CLEF_VIOLIN_COMMAND, [this]() { insertClef(mu::engraving::ClefType::G); });
    registerCommand(ADD_CLEF_BASS_COMMAND, [this]() { insertClef(mu::engraving::ClefType::F); });

    registerCommand(ADD_FULL_MEASURE_REST_COMMAND, &Controller::addFullMeasureRest);

    // interval commands
    registerCommand(ADD_INTERVAL_PLUS_1_COMMAND, &Interaction::addIntervalToSelectedNotes, 1);
    registerCommand(ADD_INTERVAL_PLUS_2_COMMAND, &Interaction::addIntervalToSelectedNotes, 2);
    registerCommand(ADD_INTERVAL_PLUS_3_COMMAND, &Interaction::addIntervalToSelectedNotes, 3);
    registerCommand(ADD_INTERVAL_PLUS_4_COMMAND, &Interaction::addIntervalToSelectedNotes, 4);
    registerCommand(ADD_INTERVAL_PLUS_5_COMMAND, &Interaction::addIntervalToSelectedNotes, 5);
    registerCommand(ADD_INTERVAL_PLUS_6_COMMAND, &Interaction::addIntervalToSelectedNotes, 6);
    registerCommand(ADD_INTERVAL_PLUS_7_COMMAND, &Interaction::addIntervalToSelectedNotes, 7);
    registerCommand(ADD_INTERVAL_PLUS_8_COMMAND, &Interaction::addIntervalToSelectedNotes, 8);
    registerCommand(ADD_INTERVAL_PLUS_9_COMMAND, &Interaction::addIntervalToSelectedNotes, 9);
    registerCommand(ADD_INTERVAL_PLUS_10_COMMAND, &Interaction::addIntervalToSelectedNotes, 10);
    registerCommand(ADD_INTERVAL_MINUS_2_COMMAND, &Interaction::addIntervalToSelectedNotes, -2);
    registerCommand(ADD_INTERVAL_MINUS_3_COMMAND, &Interaction::addIntervalToSelectedNotes, -3);
    registerCommand(ADD_INTERVAL_MINUS_4_COMMAND, &Interaction::addIntervalToSelectedNotes, -4);
    registerCommand(ADD_INTERVAL_MINUS_5_COMMAND, &Interaction::addIntervalToSelectedNotes, -5);
    registerCommand(ADD_INTERVAL_MINUS_6_COMMAND, &Interaction::addIntervalToSelectedNotes, -6);
    registerCommand(ADD_INTERVAL_MINUS_7_COMMAND, &Interaction::addIntervalToSelectedNotes, -7);
    registerCommand(ADD_INTERVAL_MINUS_8_COMMAND, &Interaction::addIntervalToSelectedNotes, -8);
    registerCommand(ADD_INTERVAL_MINUS_9_COMMAND, &Interaction::addIntervalToSelectedNotes, -9);
    registerCommand(ADD_INTERVAL_MINUS_10_COMMAND, &Interaction::addIntervalToSelectedNotes, -10);

    // editing commands
    registerCommand(COPY_COMMAND, &Interaction::copySelection);
    registerCommand(COPY_PASTE_SWAP_COMMAND, &Interaction::swapSelection);
    registerCommand(COPY_LYRICS_COMMAND, &Interaction::copyLyrics);
    registerCommand(CUT_COMMAND, &Controller::cutSelection);
    registerCommand(PASTE_COMMAND, [this]() { pasteSelection(PastingType::Default); });
    registerCommand(PASTE_HALF_COMMAND, [this]() { pasteSelection(PastingType::Half); });
    registerCommand(PASTE_DOUBLE_COMMAND, [this]() { pasteSelection(PastingType::Double); });
    registerCommand(PASTE_SPECIAL_COMMAND, [this]() { pasteSelection(PastingType::Special); });
    registerCommand(DELETE_COMMAND, &Interaction::deleteSelection);

    // move commands
    registerCommand(MOVE_RIGHT_COMMAND, [this]() { move(MoveDirection::Right, false); });
    registerCommand(MOVE_LEFT_COMMAND, [this]() { move(MoveDirection::Left, false); });
    registerCommand(MOVE_RIGHT_QUICKLY_COMMAND, [this]() { move(MoveDirection::Right, true); });
    registerCommand(MOVE_LEFT_QUICKLY_COMMAND, [this]() { move(MoveDirection::Left, true); });

    registerCommand(MOVE_UP_COMMAND, &Interaction::moveChordRestToStaff, MoveDirection::Up);
    registerCommand(MOVE_DOWN_COMMAND, &Interaction::moveChordRestToStaff, MoveDirection::Down);
    registerCommand(SWAP_LEFT_COMMAND, &Interaction::swapChordRest, MoveDirection::Left);
    registerCommand(SWAP_RIGHT_COMMAND, &Interaction::swapChordRest, MoveDirection::Right);

    registerCommand(PITCH_UP_COMMAND, [this]() { move(MoveDirection::Up, false); });
    registerCommand(PITCH_DOWN_COMMAND, [this]() { move(MoveDirection::Down, false); });
    registerCommand(PITCH_UP_OCTAVE_COMMAND, [this]() { move(MoveDirection::Up, true); });
    registerCommand(PITCH_DOWN_OCTAVE_COMMAND, [this]() { move(MoveDirection::Down, true); });
    registerCommand(PITCH_UP_DIATONIC_COMMAND, [this]() { movePitchDiatonic(MoveDirection::Up, false); });
    registerCommand(PITCH_DOWN_DIATONIC_COMMAND, [this]() { movePitchDiatonic(MoveDirection::Down, false); });

    registerCommand(PITCH_UP_DIATONIC_ALTERATIONS_COMMAND, &Interaction::transposeDiatonicAlterations,
                    mu::engraving::TransposeDirection::UP,
                    PlayMode::PlayNote);
    registerCommand(PITCH_DOWN_DIATONIC_ALTERATIONS_COMMAND, &Interaction::transposeDiatonicAlterations,
                    mu::engraving::TransposeDirection::DOWN,
                    PlayMode::PlayNote);

    // properties commands
    registerCommand(TOGGLE_VISIBLE_COMMAND, &Interaction::toggleVisible);

    // snap commands
    registerCommand(TOGGLE_SNAP_TO_PREV_COMMAND, &Interaction::toggleSnapToPrevious);
    registerCommand(TOGGLE_SNAP_TO_NEXT_COMMAND, &Interaction::toggleSnapToNext);

    // layout commands
    registerCommand(TOGGLE_SYSTEM_BREAK_COMMAND, &Interaction::toggleLayoutBreak, LayoutBreakType::LINE);
    registerCommand(TOGGLE_PAGE_BREAK_COMMAND, &Interaction::toggleLayoutBreak, LayoutBreakType::PAGE);
    registerCommand(TOGGLE_SECTION_BREAK_COMMAND, &Interaction::toggleLayoutBreak, LayoutBreakType::SECTION);

    registerCommand(APPLY_SYSTEM_LOCK_COMMAND, &Interaction::applySystemLock);
    registerCommand(TOGGLE_SYSTEM_LOCK_COMMAND, &Interaction::toggleSystemLock);
    registerCommand(APPLY_PAGE_LOCK_COMMAND, &Interaction::applyPageLock);
    registerCommand(TOGGLE_PAGE_LOCK_COMMAND, &Interaction::togglePageLock);
    registerCommand(TOGGLE_SCORE_LOCK_COMMAND, &Interaction::toggleScoreLock);

    registerCommand(MAKE_INTO_SYSTEM_COMMAND, &Interaction::makeIntoSystem);
    registerCommand(MAKE_INTO_PAGE_COMMAND, &Interaction::makeIntoPage);

    registerCommand(MOVE_MEASURE_TO_PREV_SYSTEM_COMMAND, &Interaction::moveMeasureToPrevSystem);
    registerCommand(MOVE_MEASURE_TO_NEXT_SYSTEM_COMMAND, &Interaction::moveMeasureToNextSystem);
    registerCommand(MOVE_SYSTEM_TO_PREV_PAGE_COMMAND, &Interaction::moveSystemToPrevPage);
    registerCommand(MOVE_SYSTEM_TO_NEXT_PAGE_COMMAND, &Interaction::moveSystemToNextPage);

    registerCommand(SPLIT_MEASURE_COMMAND, &Interaction::splitSelectedMeasure);
    registerCommand(JOIN_MEASURES_COMMAND, &Interaction::joinSelectedMeasures);
    registerCommand(INSERT_MEASURE_COMMAND, [this]() { addBoxes(BoxType::Measure, 1, AddBoxesTarget::BeforeSelection); });
    registerCommandWithParams(INSERT_MEASURES_COMMAND, [this](const muse::rcommand::Params& params) {
        return addMeasures(params, AddBoxesTarget::BeforeSelection);
    });
    registerCommandWithParams(INSERT_MEASURES_AFTER_SELECTION_COMMAND, [this](const muse::rcommand::Params& params) {
        return addMeasures(params, AddBoxesTarget::AfterSelection);
    });
    registerCommandWithParams(INSERT_MEASURES_AT_START_OF_SCORE_COMMAND, [this](const muse::rcommand::Params& params) {
        return addMeasures(params, AddBoxesTarget::AtStartOfScore);
    });
    registerCommand(APPEND_MEASURE_COMMAND, [this]() {
        addBoxes(BoxType::Measure, 1, AddBoxesTarget::AtEndOfScore);
    });
    registerCommandWithParams(APPEND_MEASURES_COMMAND, [this](const muse::rcommand::Params& params) {
        return addMeasures(params, AddBoxesTarget::AtEndOfScore);
    });

    registerCommand(STRETCH_DECREASE_COMMAND, [this]() { addStretch(-STRETCH_STEP); });
    registerCommand(STRETCH_INCREASE_COMMAND, [this]() { addStretch(STRETCH_STEP); });
    registerCommand(STRETCH_RESET_COMMAND, &Controller::resetStretch);

    // open properties
    registerCommand(OPEN_PAGE_SETTINGS_COMMAND, &Controller::openPageSettingsDialog);
    registerCommand(OPEN_STAFF_PROPERTIES_COMMAND, &Controller::openStaffProperties);
    registerCommand(OPEN_EDIT_STRINGS_COMMAND, &Controller::openEditStringsDialog);
    registerCommand(OPEN_BREAKS_COMMAND, &Controller::openBreaksDialog);
    registerCommand(OPEN_STAFF_TEXT_PROPERTIES_COMMAND, &Controller::openStaffTextPropertiesDialog);
    registerCommand(OPEN_SYSTEM_TEXT_PROPERTIES_COMMAND, &Controller::openStaffTextPropertiesDialog);
    registerCommand(OPEN_MEASURE_PROPERTIES_COMMAND, &Controller::openMeasurePropertiesDialog);
    registerCommand(OPEN_TRANSPOSE_COMMAND, &Controller::openTransposeDialog);
    registerCommand(OPEN_PARTS_COMMAND, &Controller::openPartsDialog);
    registerCommand(OPEN_EDITGRIDSIZE_COMMAND, &Controller::openEditGridSizeDialog);
    registerCommand(OPEN_REALIZECHORDSYMBOLS_COMMAND, &Controller::openRealizeChordSymbolsDialog);

    // style commands
    registerCommand(LOAD_STYLE_COMMAND, &Controller::loadStyle);
    registerCommand(SAVE_STYLE_COMMAND, &Controller::saveStyle);
    registerCommandWithParams(OPEN_EDIT_STYLE_COMMAND, &Controller::openEditStyleDialog);
    registerCommand(TOGGLE_CONCERT_PITCH_COMMAND, &Controller::toggleConcertPitch);

    // reset commands
    registerCommand(RESET_TEXT_STYLE_OVERRIDES_COMMAND, &Interaction::resetTextStyleOverrides);
    registerCommand(RESET_BEAMS_COMMAND, &Controller::resetBeamMode);
    registerCommand(RESET_SHAPES_AND_POSITIONS_COMMAND, &Interaction::resetShapesAndPosition);
    registerCommand(RESET_TO_DEFAULT_LAYOUT_COMMAND, &Interaction::resetToDefaultLayout);

    // show commands
    registerCommand(SHOW_INVISIBLE_COMMAND, [this]() { toggleScoreConfig(ScoreConfigType::ShowInvisibleElements); });
    registerCommand(SHOW_UNPRINTABLE_COMMAND, [this]() { toggleScoreConfig(ScoreConfigType::ShowUnprintableElements); });
    registerCommand(SHOW_FRAMES_COMMAND, [this]() { toggleScoreConfig(ScoreConfigType::ShowFrames); });
    registerCommand(SHOW_PAGEBORDERS_COMMAND, [this]() { toggleScoreConfig(ScoreConfigType::ShowPageMargins); });
    registerCommand(SHOW_SOUNDFLAGS_COMMAND, [this]() { toggleScoreConfig(ScoreConfigType::ShowSoundFlags); });
    registerCommand(SHOW_IRREGULAR_COMMAND, [this]() { toggleScoreConfig(ScoreConfigType::MarkIrregularMeasures); });

    // staff commands
    registerCommand(STAFF_EXPLODE_COMMAND, &Interaction::explodeSelectedStaff);
    registerCommand(STAFF_IMPLODE_COMMAND, &Interaction::implodeSelectedStaff);

    // remove commands
    registerCommand(REMOVE_SELECTED_RANGE_COMMAND, &Interaction::removeSelectedRange);
    registerCommand(REMOVE_EMPTY_TRAILING_MEASURES_COMMAND, &Interaction::removeEmptyTrailingMeasures);

    // slash commands
    registerCommand(SLASH_FILL_COMMAND, &Interaction::fillSelectionWithSlashes);
    registerCommand(SLASH_RHYTHM_COMMAND, &Interaction::replaceSelectedNotesWithSlashes);

    // spelling commands
    registerCommand(PITCH_SPELL_COMMAND, &Interaction::spellPitches);
    registerCommand(PITCH_SPELL_SHARPS_COMMAND, &Interaction::spellPitchesWithSharps);
    registerCommand(PITCH_SPELL_FLATS_COMMAND, &Interaction::spellPitchesWithFlats);
    registerCommand(ENHARMONIC_SPELL_BOTH_COMMAND, &Interaction::changeEnharmonicSpelling, true);
    registerCommand(ENHARMONIC_SPELL_CURRENT_COMMAND, &Interaction::changeEnharmonicSpelling, false);

    // screen commands
    registerCommandWithParams(SCREEN_PUT_NOTE_COMMAND, &Controller::putNote);
    registerCommandWithParams(SCREEN_REMOVE_NOTE_COMMAND, &Controller::removeNote);
    registerCommandWithParams(SCREEN_EDIT_TEXT_COMMAND, &Controller::startEditSelectedText);
    registerCommandWithParams(SCREEN_EDIT_ELEMENT_COMMAND, &Controller::startEditSelectedElement);

    // others commands
    registerCommand(REGROUP_RHYTHMS_COMMAND, &Interaction::regroupNotesAndRests);
    registerCommand(RESEQUENCE_REHEARSAL_MARKS_COMMAND, &Interaction::resequenceRehearsalMarks);
    registerCommand(UNROLL_REPEATS_COMMAND, &Controller::unrollRepeats);
    registerCommand(REPEAT_SELECTION_COMMAND, &Controller::repeatSelection);
    registerCommand(TRANSPOSE_UP_COMMAND, &Interaction::transposeSemitone, 1, PlayMode::PlayNote);
    registerCommand(TRANSPOSE_DOWN_COMMAND, &Interaction::transposeSemitone, -1, PlayMode::PlayNote);
    registerCommand(TOGGLE_MMREST_COMMAND, &Controller::toggleMmrest);
    registerCommand(TOGGLE_HIDE_EMPTY_COMMAND, &Controller::toggleHideEmpty);
    registerCommand(MIRROR_NOTEHEAD_COMMAND, &Interaction::mirrorNotes);
    registerCommand(SET_VISIBLE_COMMAND, &Interaction::setSelectionVisible, true);
    registerCommand(UNSET_VISIBLE_COMMAND, &Interaction::setSelectionVisible, false);
    registerCommand(TOGGLE_AUTOPLACE_COMMAND, &Interaction::toggleAutoplace, false);
    registerCommand(AUTOPLACE_ENABLED_COMMAND, &Interaction::toggleAutoplace, true);

    registerCommand(VOICE_ASSIGNMENT_ALL_IN_INSTR_COMMAND, &Interaction::changeSelectedElementsVoiceAssignment,
                    VoiceAssignment::ALL_VOICE_IN_INSTRUMENT);
    registerCommand(VOICE_ASSIGNMENT_ALL_IN_STAFF_COMMAND, &Interaction::changeSelectedElementsVoiceAssignment,
                    VoiceAssignment::ALL_VOICE_IN_STAFF);

    registerCommand(TOGGLE_AUTOMATION_COMMAND, &Controller::toggleAutomation);
    registerCommandWithParams(SELECT_AUTOMATION_TYPE_COMMAND, &Controller::selectAutomationType);

    // TAB
    registerCommand(SET_DURATION_WHOLE_TAB_COMMAND, [this]() { setDuration(DurationType::V_WHOLE); });
    registerCommand(SET_DURATION_HALF_TAB_COMMAND, [this]() { setDuration(DurationType::V_HALF); });
    registerCommand(SET_DURATION_QUARTER_TAB_COMMAND, [this]() { setDuration(DurationType::V_QUARTER); });
    registerCommand(SET_DURATION_EIGHTH_TAB_COMMAND, [this]() { setDuration(DurationType::V_EIGHTH); });
    registerCommand(SET_DURATION_16TH_TAB_COMMAND, [this]() { setDuration(DurationType::V_16TH); });
    registerCommand(SET_DURATION_32ND_TAB_COMMAND, [this]() { setDuration(DurationType::V_32ND); });
    registerCommand(SET_DURATION_64TH_TAB_COMMAND, [this]() { setDuration(DurationType::V_64TH); });
    registerCommand(SET_DURATION_128TH_TAB_COMMAND, [this]() { setDuration(DurationType::V_128TH); });
    registerCommand(SET_DURATION_256TH_TAB_COMMAND, [this]() { setDuration(DurationType::V_256TH); });
    registerCommand(SET_DURATION_512TH_TAB_COMMAND, [this]() { setDuration(DurationType::V_512TH); });
    registerCommand(SET_DURATION_1024TH_TAB_COMMAND, [this]() { setDuration(DurationType::V_1024TH); });

    registerCommand(ENTER_FRET_0_COMMAND, [this]() { addFret(0); });
    registerCommand(ENTER_FRET_1_COMMAND, [this]() { addFret(1); });
    registerCommand(ENTER_FRET_2_COMMAND, [this]() { addFret(2); });
    registerCommand(ENTER_FRET_3_COMMAND, [this]() { addFret(3); });
    registerCommand(ENTER_FRET_4_COMMAND, [this]() { addFret(4); });
    registerCommand(ENTER_FRET_5_COMMAND, [this]() { addFret(5); });
    registerCommand(ENTER_FRET_6_COMMAND, [this]() { addFret(6); });
    registerCommand(ENTER_FRET_7_COMMAND, [this]() { addFret(7); });
    registerCommand(ENTER_FRET_8_COMMAND, [this]() { addFret(8); });
    registerCommand(ENTER_FRET_9_COMMAND, [this]() { addFret(9); });
    registerCommand(ENTER_FRET_10_COMMAND, [this]() { addFret(10); });
    registerCommand(ENTER_FRET_11_COMMAND, [this]() { addFret(11); });
    registerCommand(ENTER_FRET_12_COMMAND, [this]() { addFret(12); });
    registerCommand(ENTER_FRET_13_COMMAND, [this]() { addFret(13); });
    registerCommand(ENTER_FRET_14_COMMAND, [this]() { addFret(14); });

    registerCommand(ENTER_REST_TAB_COMMAND, &Interaction::putRestToSelection);

    registerCommand(ADD_STANDARD_BEND_COMMAND, [this]() { addGuitarBend(GuitarBendType::BEND); });
    registerCommand(ADD_PRE_BEND_COMMAND, [this]() { addGuitarBend(GuitarBendType::PRE_BEND); });
    registerCommand(ADD_GRACE_NOTE_BEND_COMMAND, [this]() { addGuitarBend(GuitarBendType::GRACE_NOTE_BEND); });
    registerCommand(ADD_SLIGHT_BEND_COMMAND, [this]() { addGuitarBend(GuitarBendType::SLIGHT_BEND); });
    registerCommand(ADD_DIVE_COMMAND, [this]() { addGuitarBend(GuitarBendType::DIVE); });
    registerCommand(ADD_PRE_DIVE_COMMAND, [this]() { addGuitarBend(GuitarBendType::PRE_DIVE); });
    registerCommand(ADD_DIP_COMMAND, [this]() { addGuitarBend(GuitarBendType::DIP); });
    registerCommand(ADD_SCOOP_COMMAND, [this]() { addGuitarBend(GuitarBendType::SCOOP); });

    registerCommand(ADD_HAMMER_ON_PULL_OFF_COMMAND, [this]() { addHammerOnPullOff(); });

    registerCommand(GOTO_STRING_ABOVE_COMMAND, [this]() { move(MoveDirection::Up, false); });
    registerCommand(GOTO_STRING_BELOW_COMMAND, [this]() { move(MoveDirection::Down, false); });

    // view commands
    registerViewCommand(ZOOM_IN_COMMAND, &ViewController::zoomIn);
    registerViewCommand(ZOOM_OUT_COMMAND, &ViewController::zoomOut);
    registerViewCommand(ZOOM_TO_PAGE_WIDTH_COMMAND, &ViewController::zoomToPageWidth);
    registerViewCommand(ZOOM_TO_WHOLE_PAGE_COMMAND, &ViewController::zoomToWholePage);
    registerViewCommand(ZOOM_TO_TWO_PAGES_COMMAND, &ViewController::zoomToTwoPages);
    registerViewCommand(ZOOM_TO_100_COMMAND, &ViewController::setZoom, 100);
    registerCommandWithParams(ZOOM_TO_PERCENT_COMMAND, &Controller::zoomToPercent);

    registerViewCommand(VIEW_MODE_PAGE_COMMAND, &ViewController::setViewMode, ViewMode::PAGE);
    registerViewCommand(VIEW_MODE_FLOAT_COMMAND, &ViewController::setViewMode, ViewMode::FLOAT);
    registerViewCommand(VIEW_MODE_CONTINUOUS_COMMAND, &ViewController::setViewMode, ViewMode::LINE);
    registerViewCommand(VIEW_MODE_SINGLE_COMMAND, &ViewController::setViewMode, ViewMode::SYSTEM);

    registerViewCommand(NEXT_SCREEN_COMMAND, &ViewController::nextScreen);
    registerViewCommand(PREV_SCREEN_COMMAND, &ViewController::previousScreen);
    registerViewCommand(NEXT_PAGE_COMMAND, &ViewController::nextPage);
    registerViewCommand(PREV_PAGE_COMMAND, &ViewController::previousPage);
    registerViewCommand(TOP_OF_FIRST_PAGE_COMMAND, &ViewController::startOfScore);
    registerViewCommand(BOTTOM_OF_LAST_PAGE_COMMAND, &ViewController::endOfScore);

    registerViewCommand(CONTEXT_MENU_OF_SELECTION_COMMAND, &ViewController::openContextMenuOfSelection);

    registerViewCommand(SHOW_SEARCH_COMMAND, &ViewController::showSearch);

    registerCommandWithParams(PIANO_KEYBOARD_SET_NUMBER_OF_KEYS_COMMAND, &Controller::setPianoKeyboardNumberOfKeys);

    // diagnostic commands
    registerViewCommand(DIAGNOSTIC_VIEW_REDRAW_COMMAND, &ViewController::redrawView);

    registerCommand(CHECK_FOR_SCORE_CORRUPTIONS_COMMAND, &Controller::checkForScoreCorruptions);

    // debugging commands
    for (auto& [command, member] : s_debuggingCommands) {
        registerCommand(command, [this, member = member]() {
            EngravingDebuggingOptions options = engravingConfiguration()->debuggingOptions();
            options.*member = !(options.*member);
            engravingConfiguration()->setDebuggingOptions(options);
        });
    }

    // compat
    {
        using namespace muse::rcommand;
        static const std::vector<ActionToCommand> actionToCommand = {
            { "action://notation/copy", COPY_COMMAND, {} },
            { "action://notation/cut", CUT_COMMAND, {} },
            { "action://notation/paste", PASTE_COMMAND, {} },
            { "notation-paste-half", PASTE_HALF_COMMAND, {} },
            { "notation-paste-double", PASTE_DOUBLE_COMMAND, {} },
            { "notation-paste-special", PASTE_SPECIAL_COMMAND, {} },
            { "notation-swap", COPY_PASTE_SWAP_COMMAND, {} },
            { "action://notation/delete", DELETE_COMMAND, {} },
            { "action://notation/cancel", CANCEL_COMMAND, {} },
            { "action://notation/undo", UNDO_COMMAND, {} },
            { "action://notation/redo", REDO_COMMAND, {} },
            { "action://copy", COPY_COMMAND, {} },
            { "action://cut", CUT_COMMAND, {} },
            { "action://paste", PASTE_COMMAND, {} },
            { "action://delete", DELETE_COMMAND, {} },
            { "action://cancel", CANCEL_COMMAND, {} },
            { "action://undo", UNDO_COMMAND, {} },
            { "action://redo", REDO_COMMAND, {} },
            { "notation-move-right", MOVE_RIGHT_COMMAND, {} },
            { "notation-move-left", MOVE_LEFT_COMMAND, {} },
            { "notation-move-right-quickly", MOVE_RIGHT_QUICKLY_COMMAND, {} },
            { "notation-move-left-quickly", MOVE_LEFT_QUICKLY_COMMAND, {} },
            { "pitch-up", PITCH_UP_COMMAND, {} },
            { "pitch-down", PITCH_DOWN_COMMAND, {} },
            { "pitch-up-octave", PITCH_UP_OCTAVE_COMMAND, {} },
            { "pitch-down-octave", PITCH_DOWN_OCTAVE_COMMAND, {} },
            { "pitch-up-diatonic", PITCH_UP_DIATONIC_COMMAND, {} },
            { "pitch-down-diatonic", PITCH_DOWN_DIATONIC_COMMAND, {} },
            { "pitch-up-diatonic-alterations", PITCH_UP_DIATONIC_ALTERATIONS_COMMAND, {} },
            { "pitch-down-diatonic-alterations", PITCH_DOWN_DIATONIC_ALTERATIONS_COMMAND, {} },
            { "next-word", EDITTEXT_NEXT_WORD_COMMAND, {} },
            { "next-text-element", EDITTEXT_NEXT_ELEMENT_COMMAND, {} },
            { "prev-text-element", EDITTEXT_PREV_ELEMENT_COMMAND, {} },
            { "note-input", TOGGLE_NOTE_INPUT_COMMAND, {} },
            { "note-input-by-note-name", TOGGLE_NOTE_INPUT_BY_NOTE_NAME_COMMAND, {} },
            { "note-input-by-duration", TOGGLE_NOTE_INPUT_BY_DURATION_COMMAND, {} },
            { "note-input-rhythm", TOGGLE_NOTE_INPUT_RHYTHM_COMMAND, {} },
            { "note-input-repitch", TOGGLE_NOTE_INPUT_REPITCH_COMMAND, {} },
            { "note-input-realtime-auto", TOGGLE_NOTE_INPUT_REALTIME_AUTO_COMMAND, {} },
            { "note-input-realtime-manual", TOGGLE_NOTE_INPUT_REALTIME_MANUAL_COMMAND, {} },
            { "note-input-timewise", TOGGLE_NOTE_INPUT_TIMEWISE_COMMAND, {} },
            { "realtime-advance", REALTIME_ADVANCE_COMMAND, {} },
            { "note-longa", SET_DURATION_LONGA_COMMAND, {} },
            { "note-breve", SET_DURATION_BREVE_COMMAND, {} },
            { "pad-note-1", SET_DURATION_WHOLE_COMMAND, {} },
            { "pad-note-2", SET_DURATION_HALF_COMMAND, {} },
            { "pad-note-4", SET_DURATION_QUARTER_COMMAND, {} },
            { "pad-note-8", SET_DURATION_EIGHTH_COMMAND, {} },
            { "pad-note-16", SET_DURATION_16TH_COMMAND, {} },
            { "pad-note-32", SET_DURATION_32ND_COMMAND, {} },
            { "pad-note-64", SET_DURATION_64TH_COMMAND, {} },
            { "pad-note-128", SET_DURATION_128TH_COMMAND, {} },
            { "pad-note-256", SET_DURATION_256TH_COMMAND, {} },
            { "pad-note-512", SET_DURATION_512TH_COMMAND, {} },
            { "pad-note-1024", SET_DURATION_1024TH_COMMAND, {} },
            { "double-duration", SET_DOUBLE_DURATION_COMMAND, {} },
            { "half-duration", SET_HALVE_DURATION_COMMAND, {} },
            { "inc-duration-dotted", SET_DOUBLE_DURATION_DOTTED_COMMAND, {} },
            { "dec-duration-dotted", SET_HALVE_DURATION_DOTTED_COMMAND, {} },
            { "pad-dot", TOGGLE_DOT_COMMAND, {} },
            { "pad-dot2", TOGGLE_DOT2_COMMAND, {} },
            { "pad-dot3", TOGGLE_DOT3_COMMAND, {} },
            { "pad-dot4", TOGGLE_DOT4_COMMAND, {} },
            { "pad-rest", TOGGLE_REST_COMMAND, {} },
            { "flat2", TOGGLE_FLAT2_COMMAND, {} },
            { "flat", TOGGLE_FLAT_COMMAND, {} },
            { "nat", TOGGLE_NAT_COMMAND, {} },
            { "sharp", TOGGLE_SHARP_COMMAND, {} },
            { "sharp2", TOGGLE_SHARP2_COMMAND, {} },
            { "sharp2-post", ADD_SHARP2_COMMAND, {} },
            { "sharp-post", ADD_SHARP_COMMAND, {} },
            { "nat-post", ADD_NAT_COMMAND, {} },
            { "flat-post", ADD_FLAT_COMMAND, {} },
            { "flat2-post", ADD_FLAT2_COMMAND, {} },
            { "tie", TOGGLE_TIE_COMMAND, {} },
            { "chord-tie", TOGGLE_TIE_COMMAND, {} }, // removed, now as 'tie'
            { "lv", TOGGLE_LV_COMMAND, {} },
            { "add-slur", ADD_SLUR_COMMAND, {} },
            { "add-marcato", TOGGLE_MARCATO_COMMAND, {} },
            { "add-sforzato", TOGGLE_SFORZATO_COMMAND, {} },
            { "add-tenuto", TOGGLE_TENUTO_COMMAND, {} },
            { "add-staccato", TOGGLE_STACCATO_COMMAND, {} },
            { "voice-1", USE_VOICE_1_COMMAND, {} },
            { "voice-2", USE_VOICE_2_COMMAND, {} },
            { "voice-3", USE_VOICE_3_COMMAND, {} },
            { "voice-4", USE_VOICE_4_COMMAND, {} },
            { "voice-x12", SWAP_VOICE_X12_COMMAND, {} },
            { "voice-x13", SWAP_VOICE_X13_COMMAND, {} },
            { "voice-x14", SWAP_VOICE_X14_COMMAND, {} },
            { "voice-x23", SWAP_VOICE_X23_COMMAND, {} },
            { "voice-x24", SWAP_VOICE_X24_COMMAND, {} },
            { "voice-x34", SWAP_VOICE_X34_COMMAND, {} },
            { "flip", FLIP_COMMAND, {} },
            { "flip-horizontally", FLIP_HORIZONTALLY_COMMAND, {} },
            { "note-c", ENTER_NOTE_C_COMMAND, {} },
            { "note-d", ENTER_NOTE_D_COMMAND, {} },
            { "note-e", ENTER_NOTE_E_COMMAND, {} },
            { "note-f", ENTER_NOTE_F_COMMAND, {} },
            { "note-g", ENTER_NOTE_G_COMMAND, {} },
            { "note-a", ENTER_NOTE_A_COMMAND, {} },
            { "note-b", ENTER_NOTE_B_COMMAND, {} },
            { "chord-c", ADD_NOTE_C_COMMAND, {} },
            { "chord-d", ADD_NOTE_D_COMMAND, {} },
            { "chord-e", ADD_NOTE_E_COMMAND, {} },
            { "chord-f", ADD_NOTE_F_COMMAND, {} },
            { "chord-g", ADD_NOTE_G_COMMAND, {} },
            { "chord-a", ADD_NOTE_A_COMMAND, {} },
            { "chord-b", ADD_NOTE_B_COMMAND, {} },
            { "insert-c", INSERT_NOTE_C_COMMAND, {} },
            { "insert-d", INSERT_NOTE_D_COMMAND, {} },
            { "insert-e", INSERT_NOTE_E_COMMAND, {} },
            { "insert-f", INSERT_NOTE_F_COMMAND, {} },
            { "insert-g", INSERT_NOTE_G_COMMAND, {} },
            { "insert-a", INSERT_NOTE_A_COMMAND, {} },
            { "insert-b", INSERT_NOTE_B_COMMAND, {} },
            { "rest", ENTER_REST_COMMAND, {} },
            { "duplet", ADD_DUPLET_COMMAND, {} },
            { "triplet", ADD_TRIPLET_COMMAND, {} },
            { "quadruplet", ADD_QUADRUPLET_COMMAND, {} },
            { "quintuplet", ADD_QUINTUPLET_COMMAND, {} },
            { "sextuplet", ADD_SEXTUPLET_COMMAND, {} },
            { "septuplet", ADD_SEPTUPLET_COMMAND, {} },
            { "octuplet", ADD_OCTUPLET_COMMAND, {} },
            { "nonuplet", ADD_NONUPLET_COMMAND, {} },
            { "tuplet-dialog", OPEN_TUPLET_CONFIGURE_COMMAND, {} },
            { "first-element", GOTO_FIRST_ELEMENT_COMMAND, {} },
            { "last-element", GOTO_LAST_ELEMENT_COMMAND, {} },
            { "next-element", GOTO_NEXT_ELEMENT_COMMAND, {} },
            { "prev-element", GOTO_PREV_ELEMENT_COMMAND, {} },
            { "next-segment-element", GOTO_NEXT_SEGMENT_ELEMENT_COMMAND, {} },
            { "prev-segment-element", GOTO_PREV_SEGMENT_ELEMENT_COMMAND, {} },
            { "next-track", GOTO_NEXT_TRACK_COMMAND, {} },
            { "prev-track", GOTO_PREV_TRACK_COMMAND, {} },
            { "next-frame", GOTO_NEXT_FRAME_COMMAND, {} },
            { "prev-frame", GOTO_PREV_FRAME_COMMAND, {} },
            { "next-system", GOTO_NEXT_SYSTEM_COMMAND, {} },
            { "prev-system", GOTO_PREV_SYSTEM_COMMAND, {} },
            { "up-chord", GOTO_UPNOTE_IN_CHORD_COMMAND, {} },
            { "down-chord", GOTO_DOWNNOTE_IN_CHORD_COMMAND, {} },
            { "top-chord", GOTO_TOPNOTE_IN_CHORD_COMMAND, {} },
            { "bottom-chord", GOTO_BOTTOMNOTE_IN_CHORD_COMMAND, {} },
            { "top-staff", GOTO_TOP_STAFF_COMMAND, {} },
            { "empty-trailing-measure", GOTO_EMPTY_TRAILING_MEASURE_COMMAND, {} },
            { "select-similar", SELECT_SIMILAR_COMMAND, {} },
            { "select-similar-staff", SELECT_SIMILAR_IN_STAFF_COMMAND, {} },
            { "select-similar-range", SELECT_SIMILAR_IN_RANGE_COMMAND, {} },
            { "select-notes-in-chord", SELECT_NOTES_IN_CHORD_COMMAND, {} },
            { "notation-select-all", SELECT_ALL_COMMAND, {} },
            { "notation-select-section", SELECT_SECTION_COMMAND, {} },
            { "select-dialog", OPEN_SELECTION_OPTIONS_COMMAND, {} },
            { "next-beat-TEXT", EDITTEXT_NEXT_BEAT_COMMAND, {} },
            { "prev-beat-TEXT", EDITTEXT_PREV_BEAT_COMMAND, {} },
            { "advance-longa", EDITTEXT_ADVANCE_LONGA_COMMAND, {} },
            { "advance-breve", EDITTEXT_ADVANCE_BREVE_COMMAND, {} },
            { "advance-1", EDITTEXT_ADVANCE_1_COMMAND, {} },
            { "advance-2", EDITTEXT_ADVANCE_2_COMMAND, {} },
            { "advance-4", EDITTEXT_ADVANCE_4_COMMAND, {} },
            { "advance-8", EDITTEXT_ADVANCE_8_COMMAND, {} },
            { "advance-16", EDITTEXT_ADVANCE_16_COMMAND, {} },
            { "advance-32", EDITTEXT_ADVANCE_32_COMMAND, {} },
            { "advance-64", EDITTEXT_ADVANCE_64_COMMAND, {} },
            { "next-lyric-verse", EDITLYRIC_NEXT_VERSE_COMMAND, {} },
            { "prev-lyric-verse", EDITLYRIC_PREV_VERSE_COMMAND, {} },
            { "next-syllable", EDITLYRIC_NEXT_SYLLABLE_COMMAND, {} },
            { "add-melisma", EDITLYRIC_ADD_MELISMA_COMMAND, {} },
            { "add-lyric-verse", EDITLYRIC_ADD_VERSE_COMMAND, {} },
            { "toggle-visible", TOGGLE_VISIBLE_COMMAND, {} },
            { "toggle-snap-to-previous", TOGGLE_SNAP_TO_PREV_COMMAND, {} },
            { "toggle-snap-to-next", TOGGLE_SNAP_TO_NEXT_COMMAND, {} },
            { "system-break", TOGGLE_SYSTEM_BREAK_COMMAND, {} },
            { "page-break", TOGGLE_PAGE_BREAK_COMMAND, {} },
            { "section-break", TOGGLE_SECTION_BREAK_COMMAND, {} },
            { "apply-system-lock", APPLY_SYSTEM_LOCK_COMMAND, {} },
            { "toggle-system-lock", TOGGLE_SYSTEM_LOCK_COMMAND, {} },
            { "apply-page-lock", APPLY_PAGE_LOCK_COMMAND, {} },
            { "toggle-page-lock", TOGGLE_PAGE_LOCK_COMMAND, {} },
            { "toggle-score-lock", TOGGLE_SCORE_LOCK_COMMAND, {} },
            { "make-into-system", MAKE_INTO_SYSTEM_COMMAND, {} },
            { "make-into-page", MAKE_INTO_PAGE_COMMAND, {} },
            { "move-measure-to-prev-system", MOVE_MEASURE_TO_PREV_SYSTEM_COMMAND, {} },
            { "move-measure-to-next-system", MOVE_MEASURE_TO_NEXT_SYSTEM_COMMAND, {} },
            { "move-system-to-prev-page", MOVE_SYSTEM_TO_PREV_PAGE_COMMAND, {} },
            { "move-system-to-next-page", MOVE_SYSTEM_TO_NEXT_PAGE_COMMAND, {} },
            { "split-measure", SPLIT_MEASURE_COMMAND, {} },
            { "join-measures", JOIN_MEASURES_COMMAND, {} },
            { "insert-measure", INSERT_MEASURE_COMMAND, {} },
            { "append-measure", APPEND_MEASURE_COMMAND, {} },
            { "insert-hbox", INSERT_HBOX_COMMAND, {} },
            { "insert-vbox", INSERT_VBOX_COMMAND, {} },
            { "insert-textframe", INSERT_TEXTFRAME_COMMAND, {} },
            { "insert-fretframe", INSERT_FRETFRAME_COMMAND, {} },
            { "append-hbox", APPEND_HBOX_COMMAND, {} },
            { "append-vbox", APPEND_VBOX_COMMAND, {} },
            { "append-textframe", APPEND_TEXTFRAME_COMMAND, {} },
            { "append-fretframe", APPEND_FRETFRAME_COMMAND, {} },
            { "page-settings", OPEN_PAGE_SETTINGS_COMMAND, {} },
            { "staff-properties", OPEN_STAFF_PROPERTIES_COMMAND, {} },
            { "edit-strings", OPEN_EDIT_STRINGS_COMMAND, {} },
            { "measures-per-system", OPEN_BREAKS_COMMAND, {} },
            { "staff-text-properties", OPEN_STAFF_TEXT_PROPERTIES_COMMAND, {} },
            { "system-text-properties", OPEN_SYSTEM_TEXT_PROPERTIES_COMMAND, {} },
            { "measure-properties", OPEN_MEASURE_PROPERTIES_COMMAND, {} },
            { "transpose", OPEN_TRANSPOSE_COMMAND, {} },
            { "parts", OPEN_PARTS_COMMAND, {} },
            { "config-raster", OPEN_EDITGRIDSIZE_COMMAND, {} },
            { "realize-chord-symbols", OPEN_REALIZECHORDSYMBOLS_COMMAND, {} },
            { "load-style", LOAD_STYLE_COMMAND, {} },
            { "save-style", SAVE_STYLE_COMMAND, {} },
            { "add-fretboard-diagram", ADD_FRETBOARD_DIAGRAM_COMMAND, {} },
            { "add-ottava-8va", ADD_OTTAVA_8VA_COMMAND, {} },
            { "add-ottava-8vb", ADD_OTTAVA_8VB_COMMAND, {} },
            { "add-dynamic", ADD_DYNAMIC_COMMAND, {} },
            { "add-hairpin", ADD_HAIRPIN_COMMAND, {} },
            { "add-hairpin-reverse", ADD_HAIRPIN_REVERSE_COMMAND, {} },
            { "increase-dynamic", INCREASE_DYNAMIC_COMMAND, {} },
            { "decrease-dynamic", DECREASE_DYNAMIC_COMMAND, {} },
            { "add-noteline", ADD_NOTELINE_COMMAND, {} },
            { "add-image", ADD_IMAGE_COMMAND, {} },
            { "stretch-", STRETCH_DECREASE_COMMAND, {} },
            { "stretch+", STRETCH_INCREASE_COMMAND, {} },
            { "reset-stretch", STRETCH_RESET_COMMAND, {} },
            { "title-text", ADD_TITLE_TEXT_COMMAND, {} },
            { "subtitle-text", ADD_SUBTITLE_TEXT_COMMAND, {} },
            { "composer-text", ADD_COMPOSER_TEXT_COMMAND, {} },
            { "poet-text", ADD_LYRICIST_TEXT_COMMAND, {} },
            { "part-text", ADD_PART_TEXT_COMMAND, {} },
            { "frame-text", ADD_FRAME_TEXT_COMMAND, {} },
            { "system-text", ADD_SYSTEM_TEXT_COMMAND, {} },
            { "staff-text", ADD_STAFF_TEXT_COMMAND, {} },
            { "expression-text", ADD_EXPRESSION_TEXT_COMMAND, {} },
            { "rehearsalmark-text", ADD_REHEARSALMARK_TEXT_COMMAND, {} },
            { "instrument-change-text", ADD_INSTRUMENT_CHANGE_TEXT_COMMAND, {} },
            { "fingering-text", ADD_FINGERING_TEXT_COMMAND, {} },
            { "sticking-text", ADD_STICKING_TEXT_COMMAND, {} },
            { "chord-text", ADD_CHORD_TEXT_COMMAND, {} },
            { "roman-numeral-text", ADD_ROMAN_NUMERAL_TEXT_COMMAND, {} },
            { "nashville-number-text", ADD_NASHVILLE_NUMBER_TEXT_COMMAND, {} },
            { "lyrics", ADD_LYRICS_COMMAND, {} },
            { "tempo", ADD_TEMPO_COMMAND, {} },
            { "figured-bass", ADD_FIGURED_BASS_COMMAND, {} },
            { "reset-text-style-overrides", RESET_TEXT_STYLE_OVERRIDES_COMMAND, {} },
            { "reset-beammode", RESET_BEAMS_COMMAND, {} },
            { "reset", RESET_SHAPES_AND_POSITIONS_COMMAND, {} },
            { "reset-to-default-layout", RESET_TO_DEFAULT_LAYOUT_COMMAND, {} },
            { "show-invisible", SHOW_INVISIBLE_COMMAND, {} },
            { "show-unprintable", SHOW_UNPRINTABLE_COMMAND, {} },
            { "show-frames", SHOW_FRAMES_COMMAND, {} },
            { "show-pageborders", SHOW_PAGEBORDERS_COMMAND, {} },
            { "show-soundflags", SHOW_SOUNDFLAGS_COMMAND, {} },
            { "show-irregular", SHOW_IRREGULAR_COMMAND, {} },
            { "explode", STAFF_EXPLODE_COMMAND, {} },
            { "implode", STAFF_IMPLODE_COMMAND, {} },
            { "concert-pitch", TOGGLE_CONCERT_PITCH_COMMAND, {} },
            { "acciaccatura", ADD_ACCIACCATURA_COMMAND, {} },
            { "appoggiatura", ADD_APPOGGIATURA_COMMAND, {} },
            { "grace4", ADD_GRACE4_COMMAND, {} },
            { "grace16", ADD_GRACE16_COMMAND, {} },
            { "grace32", ADD_GRACE32_COMMAND, {} },
            { "grace8after", ADD_GRACE8_AFTER_COMMAND, {} },
            { "grace16after", ADD_GRACE16_AFTER_COMMAND, {} },
            { "grace32after", ADD_GRACE32_AFTER_COMMAND, {} },
            { "beam-auto", ADD_BEAM_AUTO_COMMAND, {} },
            { "beam-none", ADD_BEAM_NONE_COMMAND, {} },
            { "beam-break-left", ADD_BEAM_BEGIN_COMMAND, {} },
            { "beam-break-inner-8th", ADD_BEAM_BEGIN16_COMMAND, {} },
            { "beam-break-inner-16th", ADD_BEAM_BEGIN32_COMMAND, {} },
            { "beam-join", ADD_BEAM_MID_COMMAND, {} },
            { "beam-selected-range", ADD_BEAM_SELECTED_RANGE_COMMAND, {} },
            { "add-brackets", ADD_BRACKETS_COMMAND, {} },
            { "add-parentheses", ADD_PARENTHESES_COMMAND, {} },
            { "add-braces", ADD_BRACES_COMMAND, {} },
            { "add-turn", ADD_TURN_COMMAND, {} },
            { "add-turn-inverted", ADD_TURN_INVERTED_COMMAND, {} },
            { "add-turn-slash", ADD_TURN_SLASH_COMMAND, {} },
            { "add-turn-up", ADD_TURN_UP_COMMAND, {} },
            { "add-turn-inverted-up", ADD_TURN_INVERTED_UP_COMMAND, {} },
            { "add-trill", ADD_TRILL_COMMAND, {} },
            { "add-short-trill", ADD_SHORT_TRILL_COMMAND, {} },
            { "add-mordent", ADD_MORDENT_COMMAND, {} },
            { "add-haydn", ADD_HAYDN_COMMAND, {} },
            { "add-tremblement", ADD_TREMBLEMENT_COMMAND, {} },
            { "add-prall-mordent", ADD_PRALL_MORDENT_COMMAND, {} },
            { "add-shake", ADD_SHAKE_COMMAND, {} },
            { "add-shake-muffat", ADD_SHAKE_MUFFAT_COMMAND, {} },
            { "add-tremblement-couperin", ADD_TREMBLEMENT_COUPERIN_COMMAND, {} },
            { "select-next-chord", ADD_TO_SELECTION_NEXT_CHORD_COMMAND, {} },
            { "select-prev-chord", ADD_TO_SELECTION_PREV_CHORD_COMMAND, {} },
            { "select-next-measure", ADD_TO_SELECTION_NEXT_MEASURE_COMMAND, {} },
            { "select-prev-measure", ADD_TO_SELECTION_PREV_MEASURE_COMMAND, {} },
            { "select-above-staff", ADD_TO_SELECTION_ABOVE_STAFF_COMMAND, {} },
            { "select-below-staff", ADD_TO_SELECTION_BELOW_STAFF_COMMAND, {} },
            { "select-begin-line", ADD_TO_SELECTION_BEGIN_SYSTEM_COMMAND, {} },
            { "select-end-line", ADD_TO_SELECTION_END_SYSTEM_COMMAND, {} },
            { "select-begin-score", ADD_TO_SELECTION_BEGIN_SCORE_COMMAND, {} },
            { "select-end-score", ADD_TO_SELECTION_END_SCORE_COMMAND, {} },
            { "extend-to-next-note", EXTEND_TO_NEXT_NOTE_COMMAND, {} },
            { "time-delete", REMOVE_SELECTED_RANGE_COMMAND, {} },
            { "del-empty-measures", REMOVE_EMPTY_TRAILING_MEASURES_COMMAND, {} },
            { "slash-fill", SLASH_FILL_COMMAND, {} },
            { "slash-rhythm", SLASH_RHYTHM_COMMAND, {} },
            { "pitch-spell", PITCH_SPELL_COMMAND, {} },
            { "pitch-spell-sharps", PITCH_SPELL_SHARPS_COMMAND, {} },
            { "pitch-spell-flats", PITCH_SPELL_FLATS_COMMAND, {} },
            { "enh-both", ENHARMONIC_SPELL_BOTH_COMMAND, {} },
            { "enh-current", ENHARMONIC_SPELL_CURRENT_COMMAND, {} },
            { "reset-groupings", REGROUP_RHYTHMS_COMMAND, {} },
            { "resequence-rehearsal-marks", RESEQUENCE_REHEARSAL_MARKS_COMMAND, {} },
            { "unroll-repeats", UNROLL_REPEATS_COMMAND, {} },
            { "copy-lyrics-to-clipboard", COPY_LYRICS_COMMAND, {} },
            { "repeat-selection", REPEAT_SELECTION_COMMAND, {} },
            { "add-up-bow", ADD_UP_BOW_COMMAND, {} },
            { "add-down-bow", ADD_DOWN_BOW_COMMAND, {} },
            { "transpose-up", TRANSPOSE_UP_COMMAND, {} },
            { "transpose-down", TRANSPOSE_DOWN_COMMAND, {} },
            { "toggle-insert-mode", TOGGLE_INSERT_MODE_COMMAND, {} },
            { "get-location", GET_LOCATION_COMMAND, {} },
            { "toggle-mmrest", TOGGLE_MMREST_COMMAND, {} },
            { "toggle-hide-empty", TOGGLE_HIDE_EMPTY_COMMAND, {} },
            { "mirror-note", MIRROR_NOTEHEAD_COMMAND, {} },
            { "set-visible", SET_VISIBLE_COMMAND, {} },
            { "unset-visible", UNSET_VISIBLE_COMMAND, {} },
            { "toggle-autoplace", TOGGLE_AUTOPLACE_COMMAND, {} },
            { "autoplace-enabled", AUTOPLACE_ENABLED_COMMAND, {} },
            { "full-measure-rest", ADD_FULL_MEASURE_REST_COMMAND, {} },
            { "interval1", ADD_INTERVAL_PLUS_1_COMMAND, {} },
            { "interval2", ADD_INTERVAL_PLUS_2_COMMAND, {} },
            { "interval3", ADD_INTERVAL_PLUS_3_COMMAND, {} },
            { "interval4", ADD_INTERVAL_PLUS_4_COMMAND, {} },
            { "interval5", ADD_INTERVAL_PLUS_5_COMMAND, {} },
            { "interval6", ADD_INTERVAL_PLUS_6_COMMAND, {} },
            { "interval7", ADD_INTERVAL_PLUS_7_COMMAND, {} },
            { "interval8", ADD_INTERVAL_PLUS_8_COMMAND, {} },
            { "interval9", ADD_INTERVAL_PLUS_9_COMMAND, {} },
            { "interval10", ADD_INTERVAL_PLUS_10_COMMAND, {} },
            { "interval-2", ADD_INTERVAL_MINUS_2_COMMAND, {} },
            { "interval-3", ADD_INTERVAL_MINUS_3_COMMAND, {} },
            { "interval-4", ADD_INTERVAL_MINUS_4_COMMAND, {} },
            { "interval-5", ADD_INTERVAL_MINUS_5_COMMAND, {} },
            { "interval-6", ADD_INTERVAL_MINUS_6_COMMAND, {} },
            { "interval-7", ADD_INTERVAL_MINUS_7_COMMAND, {} },
            { "interval-8", ADD_INTERVAL_MINUS_8_COMMAND, {} },
            { "interval-9", ADD_INTERVAL_MINUS_9_COMMAND, {} },
            { "interval-10", ADD_INTERVAL_MINUS_10_COMMAND, {} },
            { "voice-assignment-all-in-instrument", VOICE_ASSIGNMENT_ALL_IN_INSTR_COMMAND, {} },
            { "voice-assignment-all-in-staff", VOICE_ASSIGNMENT_ALL_IN_STAFF_COMMAND, {} },
            { "pad-note-1-TAB", SET_DURATION_WHOLE_TAB_COMMAND, {} },
            { "pad-note-2-TAB", SET_DURATION_HALF_TAB_COMMAND, {} },
            { "pad-note-4-TAB", SET_DURATION_QUARTER_TAB_COMMAND, {} },
            { "pad-note-8-TAB", SET_DURATION_EIGHTH_TAB_COMMAND, {} },
            { "pad-note-16-TAB", SET_DURATION_16TH_TAB_COMMAND, {} },
            { "pad-note-32-TAB", SET_DURATION_32ND_TAB_COMMAND, {} },
            { "pad-note-64-TAB", SET_DURATION_64TH_TAB_COMMAND, {} },
            { "pad-note-128-TAB", SET_DURATION_128TH_TAB_COMMAND, {} },
            { "pad-note-256-TAB", SET_DURATION_256TH_TAB_COMMAND, {} },
            { "pad-note-512-TAB", SET_DURATION_512TH_TAB_COMMAND, {} },
            { "pad-note-1024-TAB", SET_DURATION_1024TH_TAB_COMMAND, {} },
            { "rest-TAB", ENTER_REST_TAB_COMMAND, {} },
            { "fret-0", ENTER_FRET_0_COMMAND, {} },
            { "fret-1", ENTER_FRET_1_COMMAND, {} },
            { "fret-2", ENTER_FRET_2_COMMAND, {} },
            { "fret-3", ENTER_FRET_3_COMMAND, {} },
            { "fret-4", ENTER_FRET_4_COMMAND, {} },
            { "fret-5", ENTER_FRET_5_COMMAND, {} },
            { "fret-6", ENTER_FRET_6_COMMAND, {} },
            { "fret-7", ENTER_FRET_7_COMMAND, {} },
            { "fret-8", ENTER_FRET_8_COMMAND, {} },
            { "fret-9", ENTER_FRET_9_COMMAND, {} },
            { "fret-10", ENTER_FRET_10_COMMAND, {} },
            { "fret-11", ENTER_FRET_11_COMMAND, {} },
            { "fret-12", ENTER_FRET_12_COMMAND, {} },
            { "fret-13", ENTER_FRET_13_COMMAND, {} },
            { "fret-14", ENTER_FRET_14_COMMAND, {} },
            { "standard-bend", ADD_STANDARD_BEND_COMMAND, {} },
            { "pre-bend", ADD_PRE_BEND_COMMAND, {} },
            { "grace-note-bend", ADD_GRACE_NOTE_BEND_COMMAND, {} },
            { "slight-bend", ADD_SLIGHT_BEND_COMMAND, {} },
            { "dive", ADD_DIVE_COMMAND, {} },
            { "pre-dive", ADD_PRE_DIVE_COMMAND, {} },
            { "dip", ADD_DIP_COMMAND, {} },
            { "scoop", ADD_SCOOP_COMMAND, {} },
            { "hammer-on-pull-off", ADD_HAMMER_ON_PULL_OFF_COMMAND, {} },
            { "toggle-automation", TOGGLE_AUTOMATION_COMMAND, {} },
            { "string-up", GOTO_STRING_ABOVE_COMMAND, {} },
            { "string-down", GOTO_STRING_BELOW_COMMAND, {} },
            { "move-up", MOVE_UP_COMMAND, {} },
            { "move-down", MOVE_DOWN_COMMAND, {} },
            { "move-left", SWAP_LEFT_COMMAND, {} },
            { "move-right", SWAP_RIGHT_COMMAND, {} },
            { "custom-tuplet", ADD_TUPLET_COMMAND, tupletOptions },
            { "insert-measures", INSERT_MEASURES_COMMAND, make_conv({ { "count", param<int> } }) },
            { "insert-measures-after-selection", INSERT_MEASURES_AFTER_SELECTION_COMMAND, make_conv({ { "count", param<int> } }) },
            { "insert-measures-at-start-of-score", INSERT_MEASURES_AT_START_OF_SCORE_COMMAND, make_conv({ { "count", param<int> } }) },
            { "append-measures", APPEND_MEASURES_COMMAND, make_conv({ { "count", param<int> } }) },
            { "edit-style", OPEN_EDIT_STYLE_COMMAND, make_conv({ { "page_code", param<std::string> },
                                                                   { "sub_page_code", param<std::string> } }) },
            { "zoomin", ZOOM_IN_COMMAND, {} },
            { "zoomout", ZOOM_OUT_COMMAND, {} },
            { "zoom-page-width", ZOOM_TO_PAGE_WIDTH_COMMAND, {} },
            { "zoom-whole-page", ZOOM_TO_WHOLE_PAGE_COMMAND, {} },
            { "zoom-two-pages", ZOOM_TO_TWO_PAGES_COMMAND, {} },
            { "zoom100", ZOOM_TO_100_COMMAND, {} },
            { "zoom-x-percent", ZOOM_TO_PERCENT_COMMAND, make_conv({ { "percent", param<int> } }) },
            { "view-mode-page", VIEW_MODE_PAGE_COMMAND, {} },
            { "view-mode-float", VIEW_MODE_FLOAT_COMMAND, {} },
            { "view-mode-continuous", VIEW_MODE_CONTINUOUS_COMMAND, {} },
            { "view-mode-single", VIEW_MODE_SINGLE_COMMAND, {} },
            { "scr-next", NEXT_SCREEN_COMMAND, {} },
            { "scr-prev", PREV_SCREEN_COMMAND, {} },
            { "page-next", NEXT_PAGE_COMMAND, {} },
            { "page-prev", PREV_PAGE_COMMAND, {} },
            { "page-top", TOP_OF_FIRST_PAGE_COMMAND, {} },
            { "page-end", BOTTOM_OF_LAST_PAGE_COMMAND, {} },
            { "notation-context-menu", CONTEXT_MENU_OF_SELECTION_COMMAND, {} },
            { "piano-keyboard-set-number-of-keys", PIANO_KEYBOARD_SET_NUMBER_OF_KEYS_COMMAND, make_conv({ { "keys", param<int> } }) },
            { "find", SHOW_SEARCH_COMMAND, {} },
            { "diagnostic-notationview-redraw", DIAGNOSTIC_VIEW_REDRAW_COMMAND, {} },
        };

        rcommand::registerActionToCommand(this, actionToCommand, commandDispatcher(), dispatcher());
    }

    // listen on state changes
    globalContext()->currentNotationChanged().onNotify(this, [this]() {
        auto notation = globalContext()->currentNotation();
        if (notation) {
            auto interaction = notation->interaction();

            interaction->selectionChanged().onNotify(this, [this]() {
                m_selectionChanged.notify();
            }, Asyncable::Mode::SetReplace);

            interaction->noteInput()->stateChanged().onNotify(this, [this]() {
                m_noteInputStateChanged.notify();
            }, Asyncable::Mode::SetReplace);

            interaction->textEditingStarted().onNotify(this, [this]() {
                m_textEditingChanged.send(true);
            }, Asyncable::Mode::SetReplace);
            interaction->textEditingEnded().onReceive(this, [this](TextBase*) {
                m_textEditingChanged.send(false);
            }, Asyncable::Mode::SetReplace);

            interaction->scoreConfigChanged().onReceive(this, [this](ScoreConfigType configType) {
                m_scoreConfigChanged.send(configType);
            }, Asyncable::Mode::SetReplace);

            auto undoStack = notation->undoStack();
            undoStack->stackChanged().onNotify(this, [this]() {
                m_stackChanged.notify();
            }, Asyncable::Mode::SetReplace);

            notation->style()->styleChanged().onNotify(this, [this]() {
                m_currentNotationStyleChanged.notify();
            }, Asyncable::Mode::SetReplace);

            if (const IMasterNotationPtr masterNotation = notation->masterNotation()) {
                masterNotation->automation()->automationModeEnabledChanged().onNotify(this, [this]() {
                    m_automationModeEnabledChanged.notify();
                }, Asyncable::Mode::SetReplace);
            }
        }

        m_textEditingChanged.send(isTextEditing());
        m_noteInputStateChanged.notify();
        m_currentNotationStyleChanged.notify();
    });

    globalContext()->playbackState()->playbackStatusChanged().onReceive(this, [this](muse::audio::PlaybackStatus) {
        m_isNoteInputAllowedChanged.send(isNoteInputAllowed());
    }, Asyncable::Mode::SetReplace);
}

void NotationActionController::setViewController(INotationViewController* controller)
{
    m_viewController = controller;
    LOGD() << "view controller changed";
}

INotationViewController* NotationActionController::viewController() const
{
    return m_viewController;
}

bool NotationActionController::canReceiveAction(const ActionCode& code) const
{
    // If no notation is loaded, we cannot handle any action.
    auto masterNotation = currentMasterNotation();
    if (!masterNotation) {
        return false;
    }

    if (globalContext()->playbackState()->isPlaying()) {
        if (!muse::contains(m_isAllowedDuringPlayback, code)) {
            return false;
        }
    }

    // Actions other than undo and redo can only be handled when the current
    // notation contains at least one part.
    if (!masterNotation->hasParts()) {
        return false;
    }

    auto iter = m_isEnabledMap.find(code);
    if (iter != m_isEnabledMap.end()) {
        bool enabled = iter->second();
        return enabled;
    }

    return true;
}

INotationPtr NotationActionController::currentNotation() const
{
    return globalContext()->currentNotation();
}

IMasterNotationPtr NotationActionController::currentMasterNotation() const
{
    return globalContext()->currentMasterNotation();
}

INotationInteractionPtr NotationActionController::currentNotationInteraction() const
{
    return currentNotation() ? currentNotation()->interaction() : nullptr;
}

INotationSelectionPtr NotationActionController::currentNotationSelection() const
{
    return currentNotationInteraction() ? currentNotationInteraction()->selection() : nullptr;
}

INotationElementsPtr NotationActionController::currentNotationElements() const
{
    return currentNotation() ? currentNotation()->elements() : nullptr;
}

muse::async::Notification NotationActionController::currentNotationChanged() const
{
    return globalContext()->currentNotationChanged();
}

muse::async::Notification NotationActionController::currentMasterNotationChanged() const
{
    return globalContext()->currentMasterNotationChanged();
}

INotationNoteInputPtr NotationActionController::currentNotationNoteInput() const
{
    auto interaction = currentNotationInteraction();
    if (!interaction) {
        return nullptr;
    }

    return interaction->noteInput();
}

INotationUndoStackPtr NotationActionController::currentNotationUndoStack() const
{
    auto notation = currentNotation();
    if (!notation) {
        return nullptr;
    }

    return notation->undoStack();
}

INotationMidiInputPtr NotationActionController::currentNotationMidiInput() const
{
    auto notation = currentNotation();
    if (!notation) {
        return nullptr;
    }

    return notation->midiInput();
}

mu::engraving::Score* NotationActionController::currentNotationScore() const
{
    return currentNotationElements() ? currentNotationElements()->msScore() : nullptr;
}

INotationStylePtr NotationActionController::notationStyle() const
{
    auto notation = currentNotation();
    if (!notation) {
        return nullptr;
    }

    return notation->style();
}

muse::async::Notification NotationActionController::notationStyleChanged() const
{
    return m_currentNotationStyleChanged;
}

void NotationActionController::resetState()
{
    TRACEFUNC;

    if (globalContext()->playbackState()->isPlaying()) {
        dispatcher()->dispatch("stop");
    }

    auto noteInput = currentNotationNoteInput();
    if (!noteInput) {
        return;
    }

    if (noteInput->isNoteInputMode()) {
        noteInput->endNoteInput();
        return;
    }

    auto interaction = currentNotationInteraction();
    if (!interaction) {
        return;
    }

    if (interaction->isGripEditStarted()) {
        // Exit grip edit but leave element selected
        interaction->endEditElement();
        return;
    }

    if (interaction->isTextEditingStarted()) {
        interaction->endEditElement();
        return;
    } else if (interaction->isEditingElement()) {
        interaction->endEditElement();
    }

    if (!interaction->selection()->isNone()) {
        interaction->clearSelection();
    }
}

bool NotationActionController::isNoteInputAllowed() const
{
    return !globalContext()->playbackState()->isPlaying();
}

muse::async::Channel<bool> NotationActionController::isNoteInputAllowedChanged() const
{
    return m_isNoteInputAllowedChanged;
}

muse::async::Notification NotationActionController::noteInputStateChanged() const
{
    return m_noteInputStateChanged;
}

bool NotationActionController::isNoteInputMode() const
{
    auto noteInput = currentNotationNoteInput();
    if (!noteInput) {
        return false;
    }

    return noteInput->isNoteInputMode();
}

NoteInputMethod NotationActionController::noteInputMethod() const
{
    auto noteInput = currentNotationNoteInput();
    if (!noteInput) {
        return NoteInputMethod::UNKNOWN;
    }

    return noteInput->state().noteEntryMethod();
}

void NotationActionController::toggleNoteInput(NoteInputMethod method)
{
    TRACEFUNC;

    INotationNoteInputPtr noteInput = currentNotationNoteInput();
    if (!noteInput) {
        return;
    }

    if (method == NoteInputMethod::UNKNOWN) {
        method = configuration()->defaultNoteInputMethod();
    }

    if (!noteInput->isNoteInputMode()) {
        noteInput->startNoteInput(method);
    } else if (noteInput->usingNoteInputMethod(method)) {
        noteInput->endNoteInput();
    } else {
        noteInput->setNoteInputMethod(method);
    }
}

void NotationActionController::toggleNoteInputInsert()
{
    if (!currentNotationNoteInput()->usingNoteInputMethod(NoteInputMethod::TIMEWISE)) {
        toggleNoteInput(NoteInputMethod::TIMEWISE);
    } else {
        toggleNoteInput(NoteInputMethod::BY_NOTE_NAME);
    }
}

muse::Ret NotationActionController::addNote(const muse::rcommand::Params& params)
{
    TRACEFUNC;

    NoteName note = str_conv(params.at("note").toString(), NoteName::C);
    NoteAddingMode mode = str_conv(params.at("mode").toString(), NoteAddingMode::CurrentChord);

    addNote(note, mode);
    return muse::make_ok();
}

void NotationActionController::addNote(NoteName note, NoteAddingMode mode)
{
    NoteInputParams params;
    const bool addFlag = mode == NoteAddingMode::CurrentChord;
    bool ok = mu::engraving::NoteInput::resolveNoteInputParams(currentNotationScore(), static_cast<int>(note), addFlag, params);
    if (!ok) {
        LOGE() << "Could not resolve note input params, note: " << (int)note << ", addFlag: " << addFlag;
        return;
    }

    doAddNote(params, mode);
}

muse::Ret NotationActionController::addDrumNote(const muse::rcommand::Params& params)
{
    TRACEFUNC;

    int pitch = params.at("pitch").toInt();
    NoteAddingMode mode = str_conv(params.at("mode").toString(), NoteAddingMode::CurrentChord);

    NoteInputParams noteInputParams;
    noteInputParams.drumPitch = pitch;

    doAddNote(noteInputParams, mode);

    return muse::make_ok();
}

void NotationActionController::doAddNote(const NoteInputParams& params, const NoteAddingMode& addingMode)
{
    TRACEFUNC;

    INotationNoteInputPtr noteInput = currentNotationNoteInput();
    if (!noteInput) {
        return;
    }

    startNoteInput();

    if (addingMode == NoteAddingMode::NextChord) {
        if (noteInput->usingNoteInputMethod(NoteInputMethod::BY_DURATION)) {
            noteInput->setRestMode(false);
            noteInput->setInputNote(params);

            if (configuration()->isPlayPreviewNotesInInputByDuration()) {
                const NoteInputState& state = noteInput->state();
                playbackController()->playNotes(state.notes(), state.staffIdx(), state.segment());
            }
            return;
        }
    }

    noteInput->addNote(params, addingMode);

    seekAndPlaySelectedElement();
}

void NotationActionController::setDuration(DurationType duration)
{
    TRACEFUNC;

    INotationInteractionPtr interaction = currentNotationInteraction();
    if (!interaction) {
        return;
    }

    INotationNoteInputPtr noteInput = interaction->noteInput();
    if (interaction->selection()->isNone()) {
        if (!noteInput->isNoteInputMode() && !toggleNoteInputAllowed()) {
            return;
        }

        startNoteInput();
    }

    noteInput->setDuration(duration);

    if (noteInput->usingNoteInputMethod(NoteInputMethod::BY_DURATION)
        || noteInput->usingNoteInputMethod(NoteInputMethod::RHYTHM)) {
        seekAndPlaySelectedElement();
    }
}

void NotationActionController::toggleRest()
{
    TRACEFUNC;

    INotationInteractionPtr interaction = currentNotationInteraction();
    if (!interaction) {
        return;
    }

    INotationNoteInputPtr noteInput = interaction->noteInput();
    if (interaction->selection()->isNone()) {
        if (!noteInput->isNoteInputMode() && !toggleNoteInputAllowed()) {
            return;
        }

        startNoteInput();
    }

    noteInput->toggleRest();

    if (noteInput->usingNoteInputMethod(NoteInputMethod::BY_DURATION)
        || noteInput->usingNoteInputMethod(NoteInputMethod::RHYTHM)) {
        seekSelectedElement();
    }
}

void NotationActionController::toggleDots(int dots)
{
    TRACEFUNC;

    INotationInteractionPtr interaction = currentNotationInteraction();
    if (!interaction) {
        return;
    }

    INotationNoteInputPtr noteInput = interaction->noteInput();
    if (interaction->selection()->isNone()) {
        if (!noteInput->isNoteInputMode() && !toggleNoteInputAllowed()) {
            return;
        }

        startNoteInput();
    }

    if (!noteInput->isNoteInputMode() || !configuration()->addAccidentalDotsArticulationsToNextNoteEntered()) {
        interaction->toggleDotsForSelection(dots);
        return;
    }

    noteInput->toggleDots(dots);

    if (noteInput->usingNoteInputMethod(NoteInputMethod::BY_DURATION)
        || noteInput->usingNoteInputMethod(NoteInputMethod::RHYTHM)) {
        seekAndPlaySelectedElement();
    }
}

DurationType NotationActionController::currentDurationType() const
{
    constexpr DurationType INVALID_DURATION_TYPE = DurationType::V_INVALID;

    INotationInteractionPtr interaction = currentNotationInteraction();
    if (!interaction) {
        return INVALID_DURATION_TYPE;
    }

    INotationNoteInputPtr noteInput = interaction->noteInput();
    if (!noteInput) {
        return INVALID_DURATION_TYPE;
    }

    INotationSelectionPtr selection = interaction->selection();
    if (!selection) {
        return INVALID_DURATION_TYPE;
    }

    if (noteInput->isNoteInputMode()) {
        return noteInput->state().duration().type();
    }

    if (selection->isNone() || selection->isRange()) {
        return INVALID_DURATION_TYPE;
    }

    const std::vector<EngravingItem*>& selectedElements = selection->elements();
    if (selectedElements.empty()) {
        return INVALID_DURATION_TYPE;
    }

    auto elementToChordRest = [](const EngravingItem* element) -> const ChordRest* {
        if (!element) {
            return nullptr;
        }
        if (element->isChordRest()) {
            return toChordRest(element);
        }
        if (element->isNote()) {
            return toNote(element)->chord();
        }
        if (element->isStem()) {
            return toStem(element)->chord();
        }
        if (element->isHook()) {
            return toHook(element)->chord();
        }
        return nullptr;
    };

    DurationType result = INVALID_DURATION_TYPE;
    bool isFirstElement = true;
    for (const EngravingItem* element: selectedElements) {
        const ChordRest* chordRest = elementToChordRest(element);
        if (!chordRest) {
            continue;
        }

        if (isFirstElement) {
            result = chordRest->durationType().type();
            isFirstElement = false;
        } else if (result != chordRest->durationType().type()) {
            return INVALID_DURATION_TYPE;
        }
    }

    return result;
}

int NotationActionController::currentDotCount() const
{
    INotationInteractionPtr interaction = currentNotationInteraction();
    if (!interaction) {
        return 0;
    }

    return interaction->noteInput()->state().duration().dots();
}

bool NotationActionController::currentIsRest() const
{
    INotationInteractionPtr interaction = currentNotationInteraction();
    if (!interaction) {
        return false;
    }

    INotationNoteInputPtr noteInput = interaction->noteInput();
    if (!noteInput) {
        return false;
    }

    if (noteInput->isNoteInputMode()) {
        return noteInput->state().rest();
    }

    INotationSelectionPtr selection = interaction->selection();
    if (!selection) {
        return false;
    }

    if (selection->isNone() || selection->isRange()) {
        return false;
    }

    for (const EngravingItem* element: selection->elements()) {
        if (!element->isRest()) {
            return false;
        }
    }

    return true;
}

AccidentalType NotationActionController::currentAccidentalType() const
{
    INotationInteractionPtr interaction = currentNotationInteraction();
    if (!interaction) {
        return AccidentalType::NONE;
    }

    return interaction->noteInput()->state().accidentalType();
}

std::set<SymbolId> NotationActionController::currentArticulations() const
{
    INotationInteractionPtr interaction = currentNotationInteraction();
    if (!interaction) {
        return {};
    }

    INotationNoteInputPtr noteInput = interaction->noteInput();
    if (!noteInput) {
        return {};
    }

    if (noteInput->isNoteInputMode()) {
        return mu::engraving::splitArticulations(noteInput->state().articulationIds());
    }

    INotationSelectionPtr selection = interaction->selection();
    if (!selection) {
        return {};
    }

    if (selection->isNone()) {
        return {};
    }

    auto chordArticulations = [](const Chord* chord) {
        std::set<SymbolId> result;
        for (Articulation* articulation: chord->articulations()) {
            result.insert(articulation->symId());
        }

        result = mu::engraving::flipArticulations(result, mu::engraving::PlacementV::ABOVE);
        return mu::engraving::splitArticulations(result);
    };

    std::set<SymbolId> result;
    bool isFirstNote = true;
    for (const EngravingItem* element: selection->elements()) {
        if (!element->isNote()) {
            continue;
        }

        const Note* note = toNote(element);
        if (isFirstNote) {
            result = chordArticulations(note->chord());
            isFirstNote = false;
        } else {
            std::set<SymbolId> currentNoteArticulations = chordArticulations(note->chord());
            for (auto it = result.begin(); it != result.end();) {
                if (std::find(currentNoteArticulations.begin(), currentNoteArticulations.end(),
                              *it) == currentNoteArticulations.end()) {
                    it = result.erase(it);
                } else {
                    ++it;
                }
            }
        }
    }

    return result;
}

voice_idx_t NotationActionController::currentVoice() const
{
    constexpr voice_idx_t INVALID_VOICE = muse::nidx;

    INotationInteractionPtr interaction = currentNotationInteraction();
    if (!interaction) {
        return INVALID_VOICE;
    }

    INotationNoteInputPtr noteInput = interaction->noteInput();
    if (!noteInput) {
        return INVALID_VOICE;
    }

    if (noteInput->isNoteInputMode()) {
        return noteInput->state().voice();
    }

    INotationSelectionPtr selection = interaction->selection();
    if (!selection) {
        return INVALID_VOICE;
    }

    if (selection->isNone()) {
        return INVALID_VOICE;
    }

    const std::vector<EngravingItem*>& selectedElements = selection->elements();
    if (selectedElements.empty()) {
        return INVALID_VOICE;
    }

    voice_idx_t voice = INVALID_VOICE;
    for (const EngravingItem* element : selectedElements) {
        if (element->hasVoiceAssignmentProperties()) {
            VoiceAssignment voiceAssignment = element->getProperty(Pid::VOICE_ASSIGNMENT).value<VoiceAssignment>();
            if (voiceAssignment == VoiceAssignment::ALL_VOICE_IN_INSTRUMENT || voiceAssignment == VoiceAssignment::ALL_VOICE_IN_STAFF) {
                return INVALID_VOICE;
            }
        }
        voice_idx_t elementVoice = element->voice();
        if (elementVoice != voice && voice != INVALID_VOICE) {
            return INVALID_VOICE;
        }

        voice = elementVoice;
    }

    return voice;
}

muse::async::Notification NotationActionController::selectionChanged() const
{
    return m_selectionChanged;
}

bool NotationActionController::selectionHasTie() const
{
    INotationInteractionPtr interaction = currentNotationInteraction();
    if (!interaction) {
        return false;
    }

    std::vector<Note*> tiedNotes = interaction->selection()->notes(NoteFilter::WithTie);
    if (tiedNotes.empty()) {
        return false;
    }

    bool hasTie = true;
    for (const Note* note: tiedNotes) {
        if (!note->tieFor()) {
            hasTie = false;
            break;
        }
        if (note->laissezVib()) {
            hasTie = false;
            break;
        }
    }

    return hasTie;
}

bool NotationActionController::selectionHasLaissezVib() const
{
    INotationInteractionPtr interaction = currentNotationInteraction();
    if (!interaction) {
        return false;
    }

    std::vector<Note*> tiedNotes = interaction->selection()->notes(NoteFilter::WithTie);
    if (tiedNotes.empty()) {
        return false;
    }

    bool hasLaissezVib = true;
    for (const Note* note: tiedNotes) {
        if (!note->laissezVib()) {
            hasLaissezVib = false;
            break;
        }
    }
    return hasLaissezVib;
}

bool NotationActionController::selectionHasSlur() const
{
    INotationInteractionPtr interaction = currentNotationInteraction();
    if (!interaction) {
        return false;
    }

    bool hasSlur = interaction->noteInput()->state().slur() != nullptr;
    return hasSlur;
}

muse::Ret NotationActionController::putNote(const muse::rcommand::Params& params)
{
    TRACEFUNC;

    INotationNoteInputPtr noteInput = currentNotationNoteInput();
    if (!noteInput) {
        return muse::make_ret(Ret::Code::NotSupported);
    }

    IF_ASSERT_FAILED(params.contains("pos_x") && params.contains("pos_y")) {
        return muse::make_ret(Ret::Code::BadArgs);
    }

    float posX = params.at("pos_x").toFloat();
    float posY = params.at("pos_y").toFloat();
    bool replace = params.at("replace", Val(false)).toBool();
    bool insert = params.at("insert", Val(false)).toBool();

    PointF pos = PointF(posX, posY);

    Ret ret = noteInput->putNote(pos, replace, insert);
    if (ret) {
        seekAndPlaySelectedElement();
    }

    return muse::make_ok();
}

muse::Ret NotationActionController::removeNote(const muse::rcommand::Params& params)
{
    TRACEFUNC;

    INotationNoteInputPtr noteInput = currentNotationNoteInput();
    if (!noteInput) {
        return muse::make_ret(Ret::Code::NotSupported);
    }

    IF_ASSERT_FAILED(params.contains("pos_x") && params.contains("pos_y")) {
        return muse::make_ret(Ret::Code::BadArgs);
    }

    float posX = params.at("pos_x").toFloat();
    float posY = params.at("pos_y").toFloat();

    PointF pos = PointF(posX, posY);
    noteInput->removeNote(pos);
    seekSelectedElement();

    return muse::make_ok();
}

void NotationActionController::toggleAccidental(AccidentalType type)
{
    TRACEFUNC;

    INotationInteractionPtr interaction = currentNotationInteraction();
    if (!interaction) {
        return;
    }

    INotationNoteInputPtr noteInput = interaction->noteInput();
    if (!noteInput) {
        return;
    }

    if (interaction->selection()->isNone()) {
        if (!noteInput->isNoteInputMode() && !toggleNoteInputAllowed()) {
            return;
        }

        startNoteInput();
    }

    if (noteInput->isNoteInputMode() && configuration()->addAccidentalDotsArticulationsToNextNoteEntered()) {
        noteInput->setAccidental(type);
    } else {
        interaction->toggleAccidentalForSelection(type);
        seekAndPlaySelectedElement();
    }
}

void NotationActionController::toggleArticulation(SymbolId articulationSymbolId)
{
    TRACEFUNC;

    INotationInteractionPtr interaction = currentNotationInteraction();
    if (!interaction) {
        return;
    }

    INotationNoteInputPtr noteInput = interaction->noteInput();
    if (!noteInput) {
        return;
    }

    if (interaction->selection()->isNone()) {
        if (!noteInput->isNoteInputMode() && !toggleNoteInputAllowed()) {
            return;
        }

        startNoteInput();
    }

    if (noteInput->isNoteInputMode() && configuration()->addAccidentalDotsArticulationsToNextNoteEntered()) {
        noteInput->setArticulation(articulationSymbolId);
    } else {
        interaction->toggleArticulationForSelection(articulationSymbolId);
    }
}

muse::Ret NotationActionController::putTuplet(const muse::rcommand::Params& params)
{
    TupletOptions options;
    options.ratio = engraving::Fraction::fromString(muse::String::fromStdString(params.at("ratio").toString()));
    options.numberType = engraving::str_conv(params.at("number-type").toString(), engraving::TupletNumberType::SHOW_NUMBER);
    options.bracketType = engraving::str_conv(params.at("bracket-type").toString(), engraving::TupletBracketType::AUTO_BRACKET);
    options.autoBaseLen = params.at("auto-baselen", Val(false)).toBool();

    putTuplet(options);
    return muse::make_ok();
}

void NotationActionController::putTuplet(const TupletOptions& options)
{
    TRACEFUNC;
    auto interaction = currentNotationInteraction();
    if (!interaction) {
        return;
    }

    auto noteInput = interaction->noteInput();
    if (!noteInput) {
        return;
    }

    if (!interaction->canAddTupletToSelectedChordRests()) {
        interactive()->error(muse::trc("notation", "Cannot create tuplet"), muse::trc("notation", "Note value is too short"));
        return;
    }

    if (noteInput->isNoteInputMode()) {
        noteInput->addTuplet(options);
    } else {
        interaction->addTupletToSelectedChordRests(options);
    }
}

void NotationActionController::putTuplet(int tupletCount)
{
    TRACEFUNC;

    TupletOptions options;
    options.ratio.setNumerator(tupletCount);
    options.ratio.setDenominator(2);
    options.autoBaseLen = true;
    // get the bracket type from score style settings
    if (INotationStylePtr style = notationStyle()) {
        int bracketType = style->styleValue(StyleId::tupletBracketType).toInt();
        options.bracketType = static_cast<engraving::TupletBracketType>(bracketType);
        int numberType = style->styleValue(StyleId::tupletNumberType).toInt();
        options.numberType = static_cast<engraving::TupletNumberType>(numberType);
    }

    putTuplet(options);
}

void NotationActionController::increaseDecreaseDuration(int steps, bool stepByDots)
{
    TRACEFUNC;

    INotationInteractionPtr interaction = currentNotationInteraction();
    if (!interaction) {
        return;
    }

    INotationNoteInputPtr noteInput = interaction->noteInput();

    if (noteInput->isNoteInputMode()) {
        if (steps > 0) {
            noteInput->doubleNoteInputDuration();
        } else {
            noteInput->halveNoteInputDuration();
        }
    } else {
        interaction->increaseDecreaseDuration(steps, stepByDots);
    }
}

void NotationActionController::realtimeAdvance()
{
    TRACEFUNC;

    INotationMidiInputPtr midiInput = currentNotationMidiInput();
    if (!midiInput) {
        return;
    }

    midiInput->onRealtimeAdvance();
}

bool NotationActionController::isMoveSelectionAvailable(MoveSelectionType type) const
{
    auto interaction = currentNotationInteraction();
    return interaction && interaction->moveSelectionAvailable(type);
}

bool NotationActionController::isToggleLayoutBreakAvailable() const
{
    auto interaction = currentNotationInteraction();
    return interaction && interaction->toggleLayoutBreakAvailable();
}

ScoreConfig NotationActionController::scoreConfig() const
{
    auto interaction = currentNotationInteraction();
    if (!interaction) {
        return ScoreConfig();
    }
    return interaction->scoreConfig();
}

muse::async::Channel<ScoreConfigType> NotationActionController::scoreConfigChanged() const
{
    return m_scoreConfigChanged;
}

muse::Ret NotationActionController::moveWithRet(MoveDirection direction, bool quickly)
{
    TRACEFUNC;
    auto interaction = currentNotationInteraction();
    if (!interaction) {
        return muse::make_ret(muse::Ret::Code::InternalError);
    }

    const NoteInputState& state = interaction->noteInput()->state();
    const bool previousSelectionExists = currentNotationScore() && currentNotationScore()->selection().currentCR();
    if (interaction->selection()->isNone() && previousSelectionExists && !state.beyondScore()) {
        // Try to restore the previous selection...
        interaction->moveSelectionDeprecated(direction, MoveSelectionType::EngravingItem);
        seekAndPlaySelectedElement(true);
        return muse::make_ok();
    }

    const EngravingItem* selectedElement = interaction->selection()->element();
    INotationNoteInputPtr noteInput = interaction->noteInput();
    bool playChord = false;

    switch (direction) {
    case MoveDirection::Up:
    case MoveDirection::Down:
        if (!quickly && selectedElement && selectedElement->isLyrics()) {
            interaction->moveLyrics(direction);
        } else if (selectedElement && (selectedElement->isTextBase() || selectedElement->isArticulationFamily())) {
            interaction->nudge(direction, quickly);
        } else if (selectedElement && selectedElement->hasGrips() && interaction->isGripEditStarted()) {
            interaction->nudgeAnchors(direction);
        } else if (noteInput->isNoteInputMode() && noteInput->usingNoteInputMethod(NoteInputMethod::BY_DURATION)) {
            moveInputNotes(direction == MoveDirection::Up, quickly ? PitchMode::OCTAVE : PitchMode::DIATONIC);
            return muse::make_ok();
        } else if (noteInput->isNoteInputMode() && noteInput->state().staffGroup() == mu::engraving::StaffGroup::TAB) {
            if (quickly) {
                interaction->movePitch(direction, PitchMode::OCTAVE);
            }
            interaction->moveSelectionDeprecated(direction, MoveSelectionType::String);
            return muse::make_ok();
        } else if (interaction->selection()->isNone() && !state.beyondScore()) {
            interaction->select(SelectionTarget::FirstItem);
        } else {
            interaction->movePitch(direction, quickly ? PitchMode::OCTAVE : PitchMode::CHROMATIC);
        }
        break;
    case MoveDirection::Right:
    case MoveDirection::Left:
        if (globalContext()->playbackState()->isPlaying()) {
            engraving::MeasureBeat beat = playbackController()->currentBeat();
            int targetBeatIdx = static_cast<int>(beat.beat);
            int targetMeasureIdx = beat.measureIndex;
            int increment = (direction == MoveDirection::Right ? 1 : -1);

            if (quickly) {
                targetBeatIdx = 0;
                targetMeasureIdx += increment;
                if (targetMeasureIdx > beat.maxMeasureIndex) {
                    targetMeasureIdx = beat.maxMeasureIndex;
                } else if (targetMeasureIdx < 0) {
                    targetMeasureIdx = 0;
                }
            } else {
                targetBeatIdx += increment;
                if (targetBeatIdx > beat.maxBeatIndex) {
                    targetBeatIdx = 0;
                    targetMeasureIdx += 1;
                } else if (targetBeatIdx < 0) {
                    targetMeasureIdx -= 1;

                    // Set target beat to max beat of previous bar
                    engraving::TimeSigMap* timeSigMap = currentMasterNotation()->masterScore()->sigmap();
                    int targetBarStartTick = timeSigMap->bar2tick(targetMeasureIdx, 0);
                    targetBeatIdx = timeSigMap->timesig(Fraction::fromTicks(targetBarStartTick)).timesig().numerator() - 1;
                }
            }

            playbackController()->seekBeat(targetMeasureIdx, targetBeatIdx);
            return muse::make_ok();
        }

        if (interaction->isTextEditingStarted() && textNavigationAvailable()) {
            navigateToTextElementInNearMeasure(direction);
            return muse::make_ok();
        }

        if (selectedElement && selectedElement->isTextBase()) {
            interaction->nudge(direction, quickly);
        } else if (selectedElement && selectedElement->hasGrips() && interaction->isGripEditStarted()) {
            interaction->nudgeAnchors(direction);
        } else {
            if (interaction->selection()->isNone() && !state.beyondScore()) {
                interaction->select(SelectionTarget::FirstItem);
            }
            interaction->moveSelectionDeprecated(direction, quickly ? MoveSelectionType::Measure : MoveSelectionType::Chord);
            playChord = true;
        }
        break;
    case MoveDirection::Undefined:
        break;
    }

    seekAndPlaySelectedElement(playChord);

    return muse::make_ok();
}

void NotationActionController::move(MoveDirection direction, bool quickly)
{
    moveWithRet(direction, quickly);
}

void NotationActionController::moveInputNotes(bool up, PitchMode mode)
{
    INotationInteractionPtr interaction = currentNotationInteraction();
    if (!interaction) {
        return;
    }

    interaction->noteInput()->setRestMode(false);
    interaction->noteInput()->moveInputNotes(up, mode);

    if (configuration()->isPlayPreviewNotesInInputByDuration()) {
        const NoteInputState& state = interaction->noteInput()->state();
        playbackController()->playNotes(state.notes(), state.staffIdx(), state.segment());
    }
}

void NotationActionController::movePitchDiatonic(MoveDirection direction, bool)
{
    INotationInteractionPtr interaction = currentNotationInteraction();
    if (!interaction) {
        return;
    }

    INotationNoteInputPtr noteInput = interaction->noteInput();
    if (noteInput->isNoteInputMode() && noteInput->usingNoteInputMethod(NoteInputMethod::BY_DURATION)) {
        moveInputNotes(direction == MoveDirection::Up, PitchMode::DIATONIC);
        return;
    }

    interaction->movePitch(direction, PitchMode::DIATONIC);
    seekAndPlaySelectedElement(true);
}

void NotationActionController::changeVoice(voice_idx_t voiceIndex)
{
    TRACEFUNC;

    INotationInteractionPtr interaction = currentNotationInteraction();
    if (!interaction) {
        return;
    }

    INotationNoteInputPtr noteInput = interaction->noteInput();
    if (!noteInput) {
        return;
    }

    if (interaction->selection()->isNone()) {
        if (!noteInput->isNoteInputMode() && !toggleNoteInputAllowed()) {
            return;
        }

        startNoteInput();
    }

    noteInput->setCurrentVoice(voiceIndex);

    if (!noteInput->isNoteInputMode()) {
        interaction->changeSelectedElementsVoice(voiceIndex);
    }
}

void NotationActionController::swapVoices(voice_idx_t voiceIndex1, voice_idx_t voiceIndex2)
{
    TRACEFUNC;
    auto interaction = currentNotationInteraction();
    if (!interaction) {
        return;
    }

    interaction->swapVoices(voiceIndex1, voiceIndex2);
}

void NotationActionController::cutSelection()
{
    TRACEFUNC;
    auto interaction = currentNotationInteraction();
    if (!interaction) {
        return;
    }
    interaction->copySelection();
    interaction->deleteSelection();
}

void NotationActionController::repeatSelection()
{
    auto interaction = currentNotationInteraction();
    if (!interaction) {
        return;
    }
    interaction->repeatSelection();
    seekAndPlaySelectedElement(true);
}

void NotationActionController::pasteSelection(PastingType type)
{
    TRACEFUNC;
    auto interaction = currentNotationInteraction();
    if (!interaction) {
        return;
    }

    Fraction scale = resolvePastingScale(interaction, type);
    interaction->pasteSelection(scale);

    seekAndPlaySelectedElement(DONT_PLAY_CHORD);
}

Fraction NotationActionController::resolvePastingScale(const INotationInteractionPtr& interaction, PastingType type) const
{
    TRACEFUNC;
    const Fraction DEFAULT_SCALE(1, 1);

    switch (type) {
    case PastingType::Default: return DEFAULT_SCALE;
    case PastingType::Half: return Fraction(1, 2);
    case PastingType::Double: return Fraction(2, 1);
    case PastingType::Special:
        Fraction scale = DEFAULT_SCALE;
        Fraction duration = interaction->noteInput()->state().duration().fraction();

        if (duration.isValid() && !duration.isZero()) {
            scale = duration * 4;
            scale.reduce();
        }

        return scale;
    }

    return DEFAULT_SCALE;
}

void NotationActionController::addTie()
{
    TRACEFUNC;
    auto interaction = currentNotationInteraction();
    if (!interaction) {
        return;
    }

    auto noteInput = interaction->noteInput();
    if (!noteInput) {
        return;
    }

    if (noteInput->isNoteInputMode()) {
        noteInput->addTie();
        seekAndPlaySelectedElement(true);
    } else {
        interaction->toggleTieForSelection();
    }
}

void NotationActionController::addLaissezVib()
{
    TRACEFUNC;
    auto interaction = currentNotationInteraction();
    if (!interaction) {
        return;
    }

    auto noteInput = interaction->noteInput();
    if (!noteInput) {
        return;
    }

    if (noteInput->isNoteInputMode()) {
        noteInput->addLaissezVib();
        seekAndPlaySelectedElement(true);
    } else {
        interaction->addLaissezVibToSelection();
    }
}

void NotationActionController::addSlur()
{
    TRACEFUNC;
    auto interaction = currentNotationInteraction();
    if (!interaction) {
        return;
    }

    auto noteInput = interaction->noteInput();
    if (!noteInput) {
        return;
    }

    if (noteInput->isNoteInputMode() && noteInput->state().slur()) {
        noteInput->resetSlur();
    } else {
        interaction->addSlurToSelection();
    }
}

void NotationActionController::addHammerOnPullOff()
{
    auto interaction = currentNotationInteraction();
    if (!interaction) {
        return;
    }

    interaction->addHammerOnPullOffToSelection();
}

void NotationActionController::addFret(int num)
{
    auto interaction = currentNotationInteraction();
    if (!interaction) {
        return;
    }

    interaction->addFret(num);
    seekAndPlaySelectedElement(currentNotationScore()->playChord());
}

void NotationActionController::insertClef(mu::engraving::ClefType type)
{
    INotationInteractionPtr interaction = currentNotationInteraction();
    if (!interaction || !interaction->canInsertClef(type)) {
        return;
    }

    interaction->insertClef(type);
}

void NotationActionController::addText(TextStyleType type)
{
    TRACEFUNC;

    auto interaction = currentNotationInteraction();
    if (!interaction) {
        return;
    }

    EngravingItem* item = nullptr;

    const INotationSelectionPtr sel = interaction->selection();
    if (sel->isRange()) {
        const INotationSelectionRangePtr range = sel->range();
        item = range->rangeStartSegment()->firstElementForNavigation(range->startStaffIndex());
    } else {
        item = interaction->contextItem();
    }

    if (isVerticalBoxTextStyle(type)) {
        if (!item || !item->isVBox()) {
            interaction->addTextToTopFrame(type);
            return;
        }
    }

    interaction->addTextToItem(type, item);
}

void NotationActionController::addImage()
{
    auto interaction = currentNotationInteraction();
    if (!interaction) {
        return;
    }

    EngravingItem* item = interaction->contextItem();
    if (!interaction->canAddImageToItem(item)) {
        return;
    }

    std::vector<std::string> filter
        = { muse::trc("notation", "All Supported Files") + " (*.svg *.svgz *.jpg *.jpeg *.png *.bmp *.tif *.tiff)",
            muse::trc("notation", "Scalable Vector Graphics") + " (*.svg *.svgz)",
            muse::trc("notation", "JPEG") + " (*.jpg *.jpeg)",
            muse::trc("notation", "PNG Bitmap Graphic") + " (*.png)",
            muse::trc("notation", "Bitmap") + " (*.bmp)",
            muse::trc("notation", "TIFF") + " (*.tif *.tiff)",
            muse::trc("notation", "All") + " (*)" };

    muse::io::path_t path = interactive()->selectOpeningFileSync(muse::trc("notation", "Insert Image"), "", filter);
    interaction->addImageToItem(path, item);
}

void NotationActionController::addFiguredBass()
{
    TRACEFUNC;

    auto interaction = currentNotationInteraction();
    if (!interaction) {
        return;
    }

    interaction->addFiguredBass();
}

void NotationActionController::addGuitarBend(GuitarBendType bendType)
{
    TRACEFUNC;

    auto interaction = currentNotationInteraction();
    if (!interaction) {
        return;
    }

    interaction->addGuitarBend(bendType);
}

void NotationActionController::addFretboardDiagram()
{
    TRACEFUNC;

    auto interaction = currentNotationInteraction();
    if (!interaction) {
        return;
    }

    interaction->addFretboardDiagram();
}

void NotationActionController::openSelectionMoreOptions()
{
    auto interaction = currentNotationInteraction();
    if (!interaction) {
        return;
    }

    auto item = interaction->contextItem();
    if (!item) {
        return;
    }

    bool noteSelected = item->isNote();

    if (noteSelected) {
        interactive()->open("musescore://notation/selectnote");
    } else {
        interactive()->open("musescore://notation/selectelement");
    }
}

muse::Ret NotationActionController::startEditSelectedElement(const muse::rcommand::Params& params)
{
    auto interaction = currentNotationInteraction();
    if (!interaction) {
        return muse::make_ret(Ret::Code::NotSupported);
    }

    auto selection = interaction->selection();
    if (!selection) {
        return muse::make_ret(Ret::Code::BadArgs);
    }

    mu::engraving::EngravingItem* element = selection->element();
    if (!element) {
        return muse::make_ret(Ret::Code::BadArgs);
    }

    if (element->isInstrumentName()) {
        openStaffProperties();
        return muse::make_ok();
    }

    if (elementHasPopup(element) && !interaction->textEditingAllowed(element)) {
        IF_ASSERT_FAILED(m_viewController) {
            return muse::make_ret(Ret::Code::NotSupported);
        }
        m_viewController->togglePopupForItemIfSupports(element);
        return muse::make_ok();
    }

    if (element->isText()) {
        TextStyleType styleType = mu::engraving::toText(element)->textStyleType();

        if (styleType == mu::engraving::TextStyleType::HEADER || styleType == mu::engraving::TextStyleType::FOOTER) {
            openEditStyleDialog({ { "page_code", Val("header-and-footer") } });
            return muse::make_ok();
        }
    }

    if (interaction->textEditingAllowed(element)) {
        float posX = params.at("pos_x").toFloat();
        float posY = params.at("pos_y").toFloat();
        PointF cursorPos = PointF(posX, posY);
        interaction->startEditText(element, cursorPos);
    } else {
        interaction->startEditElement(element);
    }

    return muse::make_ok();
}

muse::Ret NotationActionController::startEditSelectedText(const muse::rcommand::Params& params)
{
    auto interaction = currentNotationInteraction();
    if (!interaction) {
        return muse::make_ret(Ret::Code::NotSupported);
    }

    auto selection = interaction->selection();
    if (!selection) {
        return muse::make_ret(Ret::Code::BadArgs);
    }

    mu::engraving::EngravingItem* element = selection->element();

    if (interaction->textEditingAllowed(element)) {
        float posX = params.at("pos_x").toFloat();
        float posY = params.at("pos_y").toFloat();
        PointF cursorPos = PointF(posX, posY);
        interaction->startEditText(element, cursorPos);
    }

    return muse::make_ok();
}

muse::Ret NotationActionController::addMeasures(const muse::rcommand::Params& params, AddBoxesTarget target)
{
    if (params.contains("count")) {
        int count = params.at("count").toInt();
        addBoxes(BoxType::Measure, count, target);
    } else {
        interactive()->open("musescore://notation/selectmeasurescount")
        .onResolve(this, [this, target](const Val& v) {
            int count = v.toInt();
            addBoxes(BoxType::Measure, count, target);
        });
    }

    return muse::make_ok();
}

void NotationActionController::addBoxes(BoxType boxType, int count, AddBoxesTarget target)
{
    TRACEFUNC;
    auto interaction = currentNotationInteraction();
    if (!interaction) {
        return;
    }

    interaction->addBoxes(boxType, count, target);
}

void NotationActionController::unrollRepeats()
{
    NOT_IMPLEMENTED;
    // TODO: https://github.com/musescore/MuseScore/issues/9670
}

void NotationActionController::toggleMmrest()
{
    auto interaction = currentNotationInteraction();
    if (!interaction) {
        return;
    }

    interaction->execute(&mu::engraving::Score::cmdToggleMmrest, TranslatableString("undoableAction", "Toggle multimeasure rests"));
}

void NotationActionController::toggleHideEmpty()
{
    auto interaction = currentNotationInteraction();
    if (!interaction) {
        return;
    }

    interaction->execute(&mu::engraving::Score::cmdToggleHideEmpty, TranslatableString("undoableAction", "Toggle empty staves"));
}

void NotationActionController::addFullMeasureRest()
{
    auto interaction = currentNotationInteraction();
    if (!interaction) {
        return;
    }

    interaction->execute(&mu::engraving::Score::cmdFullMeasureRest, TranslatableString("undoableAction", "Enter full measure rest"));
}

void NotationActionController::addStretch(qreal value)
{
    TRACEFUNC;
    auto interaction = currentNotationInteraction();
    if (!interaction) {
        return;
    }

    auto selection = currentNotationSelection();
    if (!selection) {
        return;
    }

    if (!selection->isRange()) {
        return;
    }

    interaction->addStretch(value);
}

void NotationActionController::resetStretch()
{
    TRACEFUNC;
    auto interaction = currentNotationInteraction();
    if (!interaction) {
        return;
    }

    auto selection = currentNotationSelection();
    if (!selection) {
        return;
    }

    if (!selection->isRange()) {
        return;
    }

    interaction->resetStretch();
}

void NotationActionController::resetBeamMode()
{
    TRACEFUNC;
    auto interaction = currentNotationInteraction();
    if (!interaction) {
        return;
    }

    auto selection = currentNotationSelection();
    if (!selection) {
        return;
    }

    if (selection->isNone() || selection->isRange()) {
        interaction->resetBeamMode();
    }
}

muse::Ret NotationActionController::openEditStyleDialog(const muse::rcommand::Params& params)
{
    UriQuery uri("musescore://notation/style");

    if (params.contains("page_code")) {
        uri.addParam("currentPageCode", Val(params.at("page_code").toString()));

        if (params.contains("sub_page_code")) {
            uri.addParam("currentSubPageCode", Val(params.at("sub_page_code").toString()));
        }
    }

    interactive()->open(uri);
    return muse::make_ok();
}

void NotationActionController::openPageSettingsDialog()
{
    interactive()->open("musescore://notation/pagesettings");
}

void NotationActionController::openStaffProperties()
{
    interactive()->open("musescore://notation/staffproperties");
}

void NotationActionController::openEditStringsDialog()
{
    interactive()->open("musescore://notation/editstrings");
}

void NotationActionController::openBreaksDialog()
{
    interactive()->open("musescore://notation/breaks");
}

void NotationActionController::openTransposeDialog()
{
    interactive()->open("musescore://notation/transpose").onResolve(this, [this](const Val&) {
        currentNotationInteraction()->checkAndShowError();
    });
}

void NotationActionController::openPartsDialog()
{
    interactive()->open("musescore://notation/parts");
}

muse::io::path_t NotationActionController::selectStyleFile(bool forLoad)
{
    muse::io::path_t dir = configuration()->userStylesPath();
    std::string filterName = forLoad
                             ? muse::trc("notation", "MuseScore style files")
                             : muse::trc("notation", "MuseScore style file");
    std::vector<std::string> filter = { filterName + " (*.mss)" };
    return forLoad
           ? interactive()->selectOpeningFileSync(muse::trc("notation", "Load style"), dir, filter)
           : interactive()->selectSavingFileSync(muse::trc("notation", "Save style"), dir, filter);
}

void NotationActionController::loadStyle()
{
    TRACEFUNC;
    auto path = selectStyleFile(true);
    if (!path.empty()) {
        File f(path.toQString());
        if (!f.open(IODevice::ReadOnly) || !mu::engraving::MStyle::isValid(&f)) {
            interactive()->error(muse::trc("notation", "The style file could not be loaded."),
                                 f.errorString());
            return;
        }
        if (!notationStyle()->loadStyle(path.toQString(), false)) {
            auto promise = interactive()->warning(
                muse::trc("notation",
                          "Since this style file is from a different version of MuseScore Studio, your score is not guaranteed to display correctly."),
                muse::trc("notation", "Click OK to load anyway."), { IInteractive::Button::Ok, IInteractive::Button::Cancel },
                IInteractive::Button::Ok);

            promise.onResolve(this, [this, path](const IInteractive::Result& res) {
                if (res.isButton(IInteractive::Button::Ok)) {
                    notationStyle()->loadStyle(path.toQString(), true);
                }
            });
        }
    }
}

void NotationActionController::saveStyle()
{
    TRACEFUNC;
    auto path = selectStyleFile(false);
    if (!path.empty()) {
        if (!notationStyle()->saveStyle(path)) {
            interactive()->error(muse::trc("notation", "The style file could not be saved."),
                                 muse::trc("notation", "An error occurred."));
        }
    }
}

bool NotationActionController::isTextEditing() const
{
    auto interaction = currentNotationInteraction();
    if (!interaction) {
        return false;
    }

    return interaction->isTextEditingStarted();
}

bool NotationActionController::isLyricsEditing() const
{
    auto interaction = currentNotationInteraction();
    if (!interaction) {
        return false;
    }

    return interaction->isTextEditingStarted()
           && interaction->selection()->element()
           && interaction->selection()->element()->isLyrics();
}

muse::async::Channel<bool> NotationActionController::textEditingChanged() const
{
    return m_textEditingChanged;
}

bool NotationActionController::textNavigationAvailable() const
{
    return resolveTextNavigationAvailable(TextNavigationType::NearNoteOrRest);
}

bool NotationActionController::textNavigationByBeatsAvailable() const
{
    return resolveTextNavigationAvailable(TextNavigationType::NearBeat);
}

bool NotationActionController::textNavigationByFractionAvailable() const
{
    return resolveTextNavigationAvailable(TextNavigationType::Fraction);
}

bool NotationActionController::resolveTextNavigationAvailable(TextNavigationType type) const
{
    if (!isEditingText()) {
        return false;
    }

    static const QList<mu::engraving::ElementType> allowedElementsForTextNavigation {
        mu::engraving::ElementType::LYRICS,
        mu::engraving::ElementType::HARMONY,
        mu::engraving::ElementType::FIGURED_BASS,
        mu::engraving::ElementType::STICKING,
        mu::engraving::ElementType::FINGERING
    };

    static const QList<mu::engraving::ElementType> allowedElementsForBeatsNavigation {
        mu::engraving::ElementType::HARMONY
    };

    static const QList<mu::engraving::ElementType> allowedElementsForFractionNavigation {
        mu::engraving::ElementType::HARMONY,
        mu::engraving::ElementType::FIGURED_BASS
    };

    const mu::engraving::EngravingItem* element = selectedElement();
    mu::engraving::ElementType elementType = element ? element->type() : mu::engraving::ElementType::INVALID;

    switch (type) {
    case TextNavigationType::NearNoteOrRest:
        return allowedElementsForTextNavigation.contains(elementType);
    case TextNavigationType::NearBeat:
        return allowedElementsForBeatsNavigation.contains(elementType);
    case TextNavigationType::Fraction:
        return allowedElementsForFractionNavigation.contains(elementType);
    }

    return false;
}

muse::Ret NotationActionController::nextTextElement()
{
    navigateToTextElement(MoveDirection::Right, NEAR_NOTE_OR_REST);
    return muse::make_ok();
}

muse::Ret NotationActionController::prevTextElement()
{
    navigateToTextElement(MoveDirection::Left, NEAR_NOTE_OR_REST);
    return muse::make_ok();
}

muse::Ret NotationActionController::nextWord()
{
    if (!textNavigationAvailable()) {
        return muse::make_ret(Ret::Code::NotSupported);
    }

    navigateToTextElement(MoveDirection::Right, NEAR_NOTE_OR_REST, false);
    return muse::make_ok();
}

void NotationActionController::nextBeatTextElement()
{
    navigateToTextElement(MoveDirection::Right);
}

void NotationActionController::prevBeatTextElement()
{
    navigateToTextElement(MoveDirection::Left);
}

void NotationActionController::navigateToTextElement(MoveDirection direction, bool nearNoteOrRest, bool moveOnly)
{
    const mu::engraving::EngravingItem* element = selectedElement();
    if (!element) {
        return;
    }

    if (element->isLyrics()) {
        currentNotationInteraction()->navigateToLyrics(direction, moveOnly);
    } else if (element->isHarmony()) {
        const engraving::Harmony* chordSymbol = editedChordSymbol();

        // otherwise, chord symbol will be deleted when navigating away from it
        const bool canPlay = chordSymbol && !chordSymbol->harmonyName().empty();

        currentNotationInteraction()->navigateToNearHarmony(direction, nearNoteOrRest);

        if (canPlay) {
            playbackController()->playElements({ chordSymbol });
        }
    } else if (element->isFiguredBass()) {
        currentNotationInteraction()->navigateToNearFiguredBass(direction);
    } else {
        currentNotationInteraction()->navigateToNearText(direction);
    }
}

void NotationActionController::navigateToTextItemByFraction(const Fraction& fraction)
{
    const mu::engraving::EngravingItem* element = selectedElement();
    if (!element) {
        return;
    }

    if (element->isHarmony()) {
        const engraving::Harmony* chordSymbol = editedChordSymbol();

        // otherwise, chord symbol will be deleted when navigating away from it
        const bool canPlay = chordSymbol && !chordSymbol->harmonyName().empty();

        currentNotationInteraction()->navigateToHarmony(fraction);

        if (canPlay) {
            playbackController()->playElements({ chordSymbol });
        }
    } else if (element->isFiguredBass()) {
        currentNotationInteraction()->navigateToFiguredBass(fraction);
    }
}

void NotationActionController::navigateToTextElementInNearMeasure(MoveDirection direction)
{
    const mu::engraving::EngravingItem* element = selectedElement();
    if (!element) {
        return;
    }

    if (element->isHarmony()) {
        const engraving::Harmony* chordSymbol = editedChordSymbol();

        // otherwise, chord symbol will be deleted when navigating away from it
        const bool canPlay = chordSymbol && !chordSymbol->harmonyName().empty();

        currentNotationInteraction()->navigateToHarmonyInNearMeasure(direction);

        if (canPlay) {
            playbackController()->playElements({ chordSymbol });
        }
    } else if (element->isFiguredBass()) {
        currentNotationInteraction()->navigateToFiguredBassInNearMeasure(direction);
    }
}

bool NotationActionController::isEditingText() const
{
    auto interaction = currentNotationInteraction();
    if (!interaction) {
        return false;
    }

    return interaction->isTextEditingStarted();
}

void NotationActionController::openTupletOtherDialog()
{
    interactive()->open("musescore://notation/othertupletdialog");
}

void NotationActionController::openStaffTextPropertiesDialog()
{
    interactive()->open("musescore://notation/stafftextproperties");
}

void NotationActionController::openMeasurePropertiesDialog()
{
    if (currentNotationInteraction()->selectedMeasure() != nullptr) {
        interactive()->open("musescore://notation/measureproperties");
    }
}

void NotationActionController::openEditGridSizeDialog()
{
    interactive()->open("musescore://notation/editgridsize");
}

void NotationActionController::openRealizeChordSymbolsDialog()
{
    interactive()->open("musescore://notation/realizechordsymbols");
}

void NotationActionController::toggleScoreConfig(ScoreConfigType configType)
{
    TRACEFUNC;
    auto interaction = currentNotationInteraction();
    if (!interaction) {
        return;
    }

    ScoreConfig config = interaction->scoreConfig();

    bool isShow = config.isShown(configType);
    config.setShown(configType, !isShow);

    interaction->setScoreConfig(config);
}

void NotationActionController::toggleConcertPitch()
{
    TRACEFUNC;
    INotationStylePtr style = notationStyle();
    if (!style) {
        return;
    }

    bool toggle = !style->styleValue(StyleId::concertPitch).toBool();

    const TranslatableString actionName = toggle
                                          ? TranslatableString("undoableAction", "Display concert pitch")
                                          : TranslatableString("undoableAction", "Display transposed");

    currentNotationUndoStack()->prepareChanges(actionName);
    style->setStyleValue(StyleId::concertPitch, toggle);
    currentNotationUndoStack()->commitChanges();
}

void NotationActionController::seekAndPlaySelectedElement(bool playChord)
{
    seekSelectedElement();
    playSelectedElement(playChord);
}

void NotationActionController::seekSelectedElement()
{
    const IMasterNotationPtr master = currentMasterNotation();
    if (!master || master->playback()->isLoopEnabled()) {
        return;
    }

    const EngravingItem* element = selectedElement();
    if (!element) {
        return;
    }

    playbackController()->seekElement(element);
}

void NotationActionController::playSelectedElement(bool playChord)
{
    TRACEFUNC;

    const EngravingItem* element = selectedElement();
    if (!element) {
        return;
    }

    if (playChord) {
        element = element->elementBase();
    }

    playbackController()->playElements({ element });

    currentNotationScore()->setPlayChord(false);
    currentNotationScore()->setPlayNote(false);
}

bool NotationActionController::toggleNoteInputAllowed() const
{
    if (globalContext()->playbackState()->isPlaying()) {
        return false;
    }

    //! NOTE: We're more strict about starting note input mode than exiting it.
    if (!isNoteInputMode() && isEditingElement()) {
        return false;
    }

    return true;
}

void NotationActionController::startNoteInput()
{
    INotationNoteInputPtr noteInput = currentNotationNoteInput();
    if (noteInput) {
        noteInput->startNoteInput(configuration()->defaultNoteInputMethod());
    }
}

bool NotationActionController::hasSelection() const
{
    return currentNotationSelection() ? !currentNotationSelection()->isNone() : false;
}

mu::engraving::EngravingItem* NotationActionController::selectedElement() const
{
    auto selection = currentNotationSelection();
    return selection ? selection->element() : nullptr;
}

bool NotationActionController::isNoteOrRestSelected() const
{
    if (isNoteInputMode()) {
        return true;
    }

    INotationSelectionPtr selection = currentNotationInteraction() ? currentNotationInteraction()->selection() : nullptr;
    return selection && selection->elementsSelected(NOTE_REST_TYPES);
}

const mu::engraving::Harmony* NotationActionController::editedChordSymbol() const
{
    INotationInteractionPtr interaction = currentNotationInteraction();
    if (!interaction) {
        return nullptr;
    }

    const TextBase* text = interaction->editedText();
    if (!text || !text->isHarmony()) {
        return nullptr;
    }

    return toHarmony(text);
}

bool NotationActionController::elementHasPopup(const EngravingItem* e) const
{
    return AbstractElementPopupModel::hasElementEditPopup(e);
}

bool NotationActionController::canUndo() const
{
    return currentNotationUndoStack() ? currentNotationUndoStack()->canUndo() : false;
}

bool NotationActionController::canRedo() const
{
    return currentNotationUndoStack() ? currentNotationUndoStack()->canRedo() : false;
}

muse::async::Notification NotationActionController::stackChanged() const
{
    return m_stackChanged;
}

bool NotationActionController::isNotationPage() const
{
    return uiContextResolver()->matchWithCurrent(context::UiCtxProjectOpened);
}

bool NotationActionController::isTablatureStaff() const
{
    return isNotEditingElement() && currentNotationScore()->inputState().staffGroup() == mu::engraving::StaffGroup::TAB;
}

bool NotationActionController::isAutomationModeEnabled() const
{
    return currentMasterNotation() ? currentMasterNotation()->automation()->isAutomationModeEnabled() : false;
}

muse::async::Notification NotationActionController::automationModeEnabledChanged() const
{
    return m_automationModeEnabledChanged;
}

bool NotationActionController::isDebuggingCommandEnabled(const muse::rcommand::Command& command) const
{
    auto it = s_debuggingCommands.find(command);
    if (it != s_debuggingCommands.cend()) {
        return engravingConfiguration()->debuggingOptions().*(it->second);
    }

    return false;
}

muse::async::Notification NotationActionController::debuggingOptionsChanged() const
{
    return engravingConfiguration()->debuggingOptionsChanged();
}

bool NotationActionController::isEditingElement() const
{
    auto interaction = currentNotationInteraction();
    if (interaction) {
        return interaction->isEditingElement() || interaction->isDragStarted();
    }
    return false;
}

bool NotationActionController::isNotEditingElement() const
{
    return !isEditingElement();
}

void NotationActionController::checkForScoreCorruptions()
{
    project::INotationProjectPtr project = globalContext()->currentProject();
    if (!project) {
        return;
    }

    String fileName = io::filename(project->path()).toString();

    Ret ret = project->masterNotation()->masterScore()->sanityCheck();
    if (ret) {
        std::string title = muse::mtrc("project", "File “%1” seems not corrupted").arg(fileName).toStdString();
        std::string body = muse::trc("project", "This file does not seem to contain errors.");
        interactive()->info(title, body);
    } else {
        std::string title = muse::mtrc("project", "File “%1” is corrupted").arg(fileName).toStdString();
        IInteractive::Text text;
        text.text = muse::trc("project", "This file contains errors that could cause MuseScore Studio to malfunction. "
                                         "Please fix those at the earliest, to prevent crashes and further corruptions.");
        text.detailedText = ret.text();

        interactive()->warning(title, text);
    }
}

void NotationActionController::toggleAutomation()
{
    TRACEFUNC;

    IMasterNotationPtr masterNotation = currentMasterNotation();
    if (!masterNotation) {
        return;
    }

    const bool isEnabled = masterNotation->automation()->isAutomationModeEnabled();
    masterNotation->automation()->setAutomationModeEnabled(!isEnabled);
}

muse::Ret NotationActionController::selectAutomationType(const muse::rcommand::Params& params)
{
    const std::string type = params.at("type").toString();
    mu::engraving::AutomationType automationType = mu::engraving::AutomationType::Dynamics;

    if (type == "tempo") {
        automationType = mu::engraving::AutomationType::Tempo;
    } else if (type == "volume") {
        automationType = mu::engraving::AutomationType::Volume;
    } else if (type == "pan") {
        automationType = mu::engraving::AutomationType::Pan;
    }

    configuration()->setCurrentAutomationType(automationType);

    return muse::make_ok();
}

bool NotationActionController::isNoteInputActionAllowed() const
{
    if (!isNoteInputMode() && !toggleNoteInputAllowed()) {
        return false;
    }

    return !isTablatureStaff();
}

muse::Ret NotationActionController::select(const muse::rcommand::Params& params)
{
    SelectionTarget target = str_conv(params.at("target").toString(), SelectionTarget::Undefined);
    if (target == SelectionTarget::Undefined) {
        return muse::make_ret(Ret::Code::BadArgs);
    }

    PlayMode playMode = str_conv(params.at("play-mode").toString(), PlayMode::NoPlay);

    select(target, playMode);
    return muse::make_ok();
}

void NotationActionController::select(SelectionTarget target, PlayMode playMode)
{
    auto interaction = currentNotationInteraction();
    if (!interaction) {
        return;
    }

    interaction->select(target);
    seekSelectedElement();

    if (playMode != PlayMode::NoPlay) {
        playSelectedElement(playMode == PlayMode::PlayChord);
    }
}

// COMMANDS

void NotationActionController::registerCommand(const muse::rcommand::Command& command, std::function<void()> handler)
{
    registerCommand(command, handler, nullptr);
}

void NotationActionController::registerCommand(const muse::rcommand::Command& command,
                                               std::function<void()> handler,
                                               bool (NotationActionController::* enabler)() const)
{
    commandDispatcher()->onRequest(this, command, [this, command, handler, enabler]() {
        if (!commandsState()->commandState(command).enabled) {
            return muse::make_ret(Ret::Code::NotSupported);
        }

        if (enabler && !(this->*enabler)()) {
            return muse::make_ret(Ret::Code::NotSupported);
        }

        handler();
        return muse::make_ok();
    });
}

void NotationActionController::registerCommandWithParams(const muse::rcommand::Command& command,
                                                         std::function<muse::Ret(const muse::rcommand::Params&)> handler)
{
    commandDispatcher()->onRequest(this, command, [this, command, handler](const rcommand::Request& request) {
        if (!commandsState()->commandState(command).enabled) {
            return rcommand::make_response(request, muse::make_ret(Ret::Code::NotSupported));
        }

        muse::Ret ret = handler(request.params);
        return rcommand::make_response(request, ret);
    });
}

void NotationActionController::registerCommand(const muse::rcommand::Command& command, void (NotationActionController::* handler)())
{
    registerCommand(command, handler, nullptr);
}

void NotationActionController::registerCommand(const muse::rcommand::Command& command,
                                               void (NotationActionController::* handler)(),
                                               bool (NotationActionController::* enabler)() const)
{
    commandDispatcher()->onRequest(this, command, [this, command, handler, enabler]() {
        if (!commandsState()->commandState(command).enabled) {
            return muse::make_ret(Ret::Code::NotSupported);
        }

        if (enabler && !(this->*enabler)()) {
            return muse::make_ret(Ret::Code::NotSupported);
        }

        (this->*handler)();
        return muse::make_ok();
    });
}

void NotationActionController::registerCommandWithParams(const muse::rcommand::Command& command,
                                                         muse::Ret (NotationActionController::* handler)(const muse::rcommand::Params&))
{
    commandDispatcher()->onRequest(this, command, [this, command, handler](const rcommand::Request& request) {
        if (!commandsState()->commandState(command).enabled) {
            return rcommand::make_response(request, muse::make_ret(Ret::Code::NotSupported));
        }

        muse::Ret ret = (this->*handler)(request.params);
        return rcommand::make_response(request, ret);
    });
}

void NotationActionController::registerCommand(const muse::rcommand::Command& command,
                                               void (INotationInteraction::* handler)(),
                                               PlayMode playMode,
                                               bool (NotationActionController::* enabler)() const)
{
    registerCommand(command, [this, handler, playMode, enabler]()
    {
        if (enabler && !(this->*enabler)()) {
            return muse::make_ret(Ret::Code::NotSupported);
        }

        INotationPtr notation = currentNotation();
        if (notation) {
            (notation->interaction().get()->*handler)();

            seekSelectedElement();

            if (playMode != PlayMode::NoPlay) {
                playSelectedElement(playMode == PlayMode::PlayChord);
            }
        }

        return muse::make_ok();
    });
}

template<class P1>
void NotationActionController::registerCommand(const muse::rcommand::Command& command,
                                               void (INotationInteraction::* handler)(P1), P1 param1,
                                               PlayMode playMode,
                                               bool (NotationActionController::* enabler)() const)
{
    registerCommand(command, [this, handler, param1, playMode]()
    {
        INotationPtr notation = currentNotation();
        if (notation) {
            (notation->interaction().get()->*handler)(param1);

            seekSelectedElement();

            if (playMode != PlayMode::NoPlay) {
                playSelectedElement(playMode == PlayMode::PlayChord);
            }
        }
    }, enabler);
}

void NotationActionController::registerNoteInputCommand(const muse::rcommand::Command& command, NoteInputMethod method)
{
    registerCommand(command, [this, method]() { toggleNoteInput(method); }, &NotationActionController::toggleNoteInputAllowed);
}

void NotationActionController::registerNoteCommand(const muse::rcommand::Command& command,
                                                   NoteName noteName,
                                                   NoteAddingMode addingMode)
{
    registerCommand(command, [this, noteName, addingMode]()
    {
        addNote(noteName, addingMode);
    });
}

void NotationActionController::registerNavigationByFractionCommand(const muse::rcommand::Command& command, const Fraction& fraction)
{
    registerCommand(command, [this, fraction]() {
        navigateToTextItemByFraction(
            fraction);
    }, &NotationActionController::textNavigationByFractionAvailable);
}

void NotationActionController::registerViewCommand(const muse::rcommand::Command& command, void (INotationViewController::* handler)())
{
    commandDispatcher()->onRequest(this, command, [this, command, handler]() {
        if (!commandsState()->commandState(command).enabled) {
            return muse::make_ret(Ret::Code::NotSupported);
        }

        if (!m_viewController) {
            LOGE() << "command: " << command.isValid() << " NotSupported, viewController is null";
            return muse::make_ret(Ret::Code::NotSupported);
        }

        (m_viewController->*handler)();
        return muse::make_ok();
    });
}

template<typename P1>
void NotationActionController::registerViewCommand(const muse::rcommand::Command& command,
                                                   void (INotationViewController::* handler)(P1),
                                                   P1 p1)
{
    commandDispatcher()->onRequest(this, command, [this, command, handler, p1]() {
        if (!commandsState()->commandState(command).enabled) {
            return muse::make_ret(Ret::Code::NotSupported);
        }

        if (!m_viewController) {
            LOGE() << "command: " << command.isValid() << " NotSupported, viewController is null";
            return muse::make_ret(Ret::Code::NotSupported);
        }

        (m_viewController->*handler)(p1);
        return muse::make_ok();
    });
}

muse::Ret NotationActionController::zoomToPercent(const muse::rcommand::Params& params)
{
    if (!params.contains("percent")) {
        return muse::make_ret(Ret::Code::BadArgs);
    }

    if (!m_viewController) {
        LOGE() << "command: " << ZOOM_TO_PERCENT_COMMAND.toString() << " NotSupported, viewController is null";
        return muse::make_ret(Ret::Code::NotSupported);
    }

    int zoomPercentage = params.at("percent").toInt();
    m_viewController->setZoom(zoomPercentage);
    return muse::make_ok();
}

muse::Ret NotationActionController::setPianoKeyboardNumberOfKeys(const muse::rcommand::Params& params)
{
    if (!params.contains("keys")) {
        return muse::make_ret(Ret::Code::BadArgs);
    }

    int numberOfKeys = params.at("keys").toInt();
    sceneConfiguration()->setPianoKeyboardNumberOfKeys(numberOfKeys);
    return muse::make_ok();
}
