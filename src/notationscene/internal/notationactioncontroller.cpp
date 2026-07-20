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

#include <QGuiApplication>

#include "global/io/file.h"
#include "global/translation.h"

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
#include "notation/notationtypes.h"
#include "notation/inotationinteraction.h"

#include "project/inotationproject.h"

#include "qml/MuseScore/NotationScene/abstractelementpopupmodel.h"

#include "../notationcommands.h"

#include "log.h"
#include "rcommand/commandtypes.h"
#include "types/ret.h"

using namespace mu;
using namespace muse;
using namespace muse::io;
using namespace muse::ui;
using namespace mu::notation;
using namespace muse::actions;
using namespace mu::context;

static constexpr qreal STRETCH_STEP = 0.1;
static constexpr bool NEAR_NOTE_OR_REST = true;

static constexpr bool DONT_PLAY_CHORD = false;

static const QMap<ActionCode, Fraction> DURATIONS_FOR_TEXT_NAVIGATION {
    { "advance-longa", Fraction(4, 1) },
    { "advance-breve", Fraction(2, 1) },
    { "advance-1", Fraction(1, 1) },
    { "advance-2", Fraction(1, 2) },
    { "advance-4", Fraction(1, 4) },
    { "advance-8", Fraction(1, 8) },
    { "advance-16", Fraction(1, 16) },
    { "advance-32", Fraction(1, 32) },
    { "advance-64", Fraction(1, 64) },
};

using EngravingDebuggingOptions = NotationActionController::EngravingDebuggingOptions;
const std::unordered_map<ActionCode, bool EngravingDebuggingOptions::*> NotationActionController::engravingDebuggingActions {
    { "show-element-bounding-rects", &EngravingDebuggingOptions::showElementBoundingRects },
    { "color-element-shapes", &EngravingDebuggingOptions::colorElementShapes },
    { "show-segment-shapes", &EngravingDebuggingOptions::showSegmentShapes },
    { "color-segment-shapes", &EngravingDebuggingOptions::colorSegmentShapes },
    { "show-skylines", &EngravingDebuggingOptions::showSkylines },
    { "show-system-bounding-rects", &EngravingDebuggingOptions::showSystemBoundingRects },
    { "show-element-masks", &EngravingDebuggingOptions::showElementMasks },
    { "show-line-attach-points", &EngravingDebuggingOptions::showLineAttachPoints },
    { "mark-empty-staff-visibility-overrides", &EngravingDebuggingOptions::markEmptyStaffVisibilityOverrides },
    { "mark-corrupted-measures", &EngravingDebuggingOptions::markCorruptedMeasures },
    { "show-gap-rests", &EngravingDebuggingOptions::showGapRests },
    { "show-both-origin-and-combined", &EngravingDebuggingOptions::showOriginAndCombinedStaves },
};

//! NOTE Just for more readable
using Controller = NotationActionController;
using Interaction = INotationInteraction;

void NotationActionController::init()
{
    TRACEFUNC;

    // global commands
    registerCommand(CANCEL_COMMAND, &Controller::resetState);
    registerCommand(UNDO_COMMAND, &Interaction::undo);
    registerCommand(REDO_COMMAND, &Interaction::redo);

    // navigation and selection commands
    static const rcommand::CommandQuery SELECT_QUERY(SELECT_COMMAND);

    static const std::map<rcommand::Command, rcommand::CommandQuery> SELECTION_ALIASES {
        { GOTO_FIRST_ELEMENT_COMMAND, SELECT_QUERY.set("target", "first-item").set("play-mode", "chord") },
        { GOTO_LAST_ELEMENT_COMMAND, SELECT_QUERY.set("target", "last-item").set("play-mode", "chord") },
        { GOTO_NEXT_ELEMENT_COMMAND, SELECT_QUERY.set("target", "next-item").set("play-mode", "note") },
        { GOTO_PREV_ELEMENT_COMMAND, SELECT_QUERY.set("target", "prev-item").set("play-mode", "note") },
        { GOTO_NEXT_TRACK_COMMAND, SELECT_QUERY.set("target", "next-track").set("play-mode", "chord") },
        { GOTO_PREV_TRACK_COMMAND, SELECT_QUERY.set("target", "prev-track").set("play-mode", "chord") },
        { GOTO_NEXT_FRAME_COMMAND, SELECT_QUERY.set("target", "next-frame") },
        { GOTO_PREV_FRAME_COMMAND, SELECT_QUERY.set("target", "prev-frame") },
        { GOTO_NEXT_SYSTEM_COMMAND, SELECT_QUERY.set("target", "next-system") },
        { GOTO_PREV_SYSTEM_COMMAND, SELECT_QUERY.set("target", "prev-system") },
        { GOTO_UPNOTE_IN_CHORD_COMMAND, SELECT_QUERY.set("target", "up-note-in-chord") },
        { GOTO_DOWNNOTE_IN_CHORD_COMMAND, SELECT_QUERY.set("target", "down-note-in-chord") },
        { GOTO_TOPNOTE_IN_CHORD_COMMAND, SELECT_QUERY.set("target", "top-note-in-chord") },
        { GOTO_BOTTOMNOTE_IN_CHORD_COMMAND, SELECT_QUERY.set("target", "bottom-note-in-chord") },
        { SELECT_SIMILAR_COMMAND, SELECT_QUERY.set("target", "similar") },
        { SELECT_SIMILAR_IN_STAFF_COMMAND, SELECT_QUERY.set("target", "similar-in-staff") },
        { SELECT_SIMILAR_IN_RANGE_COMMAND, SELECT_QUERY.set("target", "similar-in-range") },
        { SELECT_NOTES_IN_CHORD_COMMAND, SELECT_QUERY.set("target", "notes-in-chord") },
        { SELECT_ALL_COMMAND, SELECT_QUERY.set("target", "all") },
        { SELECT_SECTION_COMMAND, SELECT_QUERY.set("target", "section") },
    };

    registerCommand(SELECT_COMMAND, &Controller::select);
    registerAliases(SELECTION_ALIASES, &Controller::select);

    //! TODO remove, for example
    // registerSelectionCommand(GOTO_FIRST_ELEMENT_COMMAND, SelectionTarget::FirstItem, PlayMode::PlayChord);
    // registerSelectionCommand(GOTO_LAST_ELEMENT_COMMAND, SelectionTarget::LastItem, PlayMode::PlayChord);
    // registerSelectionCommand(GOTO_NEXT_ELEMENT_COMMAND, SelectionTarget::NextItem, PlayMode::PlayNote);
    // registerSelectionCommand(GOTO_PREV_ELEMENT_COMMAND, SelectionTarget::PrevItem, PlayMode::PlayNote);
    // registerSelectionCommand(GOTO_NEXT_TRACK_COMMAND, SelectionTarget::NextTrack, PlayMode::PlayChord);
    // registerSelectionCommand(GOTO_PREV_TRACK_COMMAND, SelectionTarget::PrevTrack, PlayMode::PlayChord);
    // registerSelectionCommand(GOTO_NEXT_FRAME_COMMAND, SelectionTarget::NextFrame);
    // registerSelectionCommand(GOTO_PREV_FRAME_COMMAND, SelectionTarget::PrevFrame);
    // registerSelectionCommand(GOTO_NEXT_SYSTEM_COMMAND, SelectionTarget::NextSystem);
    // registerSelectionCommand(GOTO_PREV_SYSTEM_COMMAND, SelectionTarget::PrevSystem);
    // registerSelectionCommand(GOTO_UPNOTE_IN_CHORD_COMMAND, SelectionTarget::UpNoteInChord);
    // registerSelectionCommand(GOTO_DOWNNOTE_IN_CHORD_COMMAND, SelectionTarget::DownNoteInChord);
    // registerSelectionCommand(GOTO_TOPNOTE_IN_CHORD_COMMAND, SelectionTarget::TopNoteInChord);
    // registerSelectionCommand(GOTO_BOTTOMNOTE_IN_CHORD_COMMAND, SelectionTarget::BottomNoteInChord);

    // registerSelectionCommand(SELECT_SIMILAR_COMMAND, SelectionTarget::Similar);
    // registerSelectionCommand(SELECT_SIMILAR_IN_STAFF_COMMAND, SelectionTarget::SimilarInStaff);
    // registerSelectionCommand(SELECT_SIMILAR_IN_RANGE_COMMAND, SelectionTarget::SimilarInRange);
    // registerSelectionCommand(SELECT_NOTES_IN_CHORD_COMMAND, SelectionTarget::NotesInChord);
    // registerSelectionCommand(SELECT_ALL_COMMAND, SelectionTarget::All);
    // registerSelectionCommand(SELECT_SECTION_COMMAND, SelectionTarget::Section);

    registerAction("select-dialog", &Controller::openSelectionMoreOptions, &Controller::hasSelection);

    registerCommand(EDIT_NEXT_WORD_COMMAND, [this]() { nextWord(); });
    registerCommand(EDIT_NEXT_TEXT_ELEMENT_COMMAND, [this]() { nextTextElement(); });
    registerCommand(EDIT_PREV_TEXT_ELEMENT_COMMAND, [this]() { prevTextElement(); });

    // note input commands

    registerNoteInputCommand(TOGGLE_NOTE_INPUT_COMMAND, NoteInputMethod::UNKNOWN /*default*/);
    registerNoteInputCommand(TOGGLE_NOTE_INPUT_BY_NOTE_NAME_COMMAND, NoteInputMethod::BY_NOTE_NAME);
    registerNoteInputCommand(TOGGLE_NOTE_INPUT_BY_DURATION_COMMAND, NoteInputMethod::BY_DURATION);
    registerNoteInputCommand(TOGGLE_NOTE_INPUT_RHYTHM_COMMAND, NoteInputMethod::RHYTHM);
    registerNoteInputCommand(TOGGLE_NOTE_INPUT_REPITCH_COMMAND, NoteInputMethod::REPITCH);
    registerNoteInputCommand(TOGGLE_NOTE_INPUT_REALTIME_AUTO_COMMAND, NoteInputMethod::REALTIME_AUTO);
    registerNoteInputCommand(TOGGLE_NOTE_INPUT_REALTIME_MANUAL_COMMAND, NoteInputMethod::REALTIME_MANUAL);
    registerNoteInputCommand(TOGGLE_NOTE_INPUT_TIMEWISE_COMMAND, NoteInputMethod::TIMEWISE);

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

    registerCommand(ADD_TIE_COMMAND, &Controller::addTie);
    registerCommand(ADD_SLUR_COMMAND, &Controller::addSlur);
    registerCommand(ADD_LV_COMMAND, &Controller::addLaissezVib);
    registerCommand(ADD_MARCATO_COMMAND, [this]() { toggleArticulation(SymbolId::articMarcatoAbove); });
    registerCommand(ADD_SFORZATO_COMMAND, [this]() { toggleArticulation(SymbolId::articAccentAbove); });
    registerCommand(ADD_TENUTO_COMMAND, [this]() { toggleArticulation(SymbolId::articTenutoAbove); });
    registerCommand(ADD_STACCATO_COMMAND, [this]() { toggleArticulation(SymbolId::articStaccatoAbove); });

    registerCommand(USE_VOICE_1_COMMAND, [this]() { changeVoice(0); });
    registerCommand(USE_VOICE_2_COMMAND, [this]() { changeVoice(1); });
    registerCommand(USE_VOICE_3_COMMAND, [this]() { changeVoice(2); });
    registerCommand(USE_VOICE_4_COMMAND, [this]() { changeVoice(3); });

    registerCommand(FLIP_COMMAND, &Interaction::flipSelection);
    registerCommand(FLIP_HORIZONTALLY_COMMAND, &Interaction::flipSelectionHorizontally);

    registerCommand(ADD_NOTE_COMMAND, &Controller::handleNoteAction);
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

    registerCommand(SHOW_TUPLET_CONFIGURE_COMMAND, [this]() { openTupletOtherDialog(); });
    registerCommand(ADD_TUPLET_COMMAND, &Controller::putTuplet);
    registerCommand(ADD_DUPLET_COMMAND, [this]() { putTuplet(2); });
    registerCommand(ADD_TRIPLET_COMMAND, [this]() { putTuplet(3); });
    registerCommand(ADD_QUADRUPLET_COMMAND, [this]() { putTuplet(4); });
    registerCommand(ADD_QUINTUPLET_COMMAND, [this]() { putTuplet(5); });
    registerCommand(ADD_SEXTUPLET_COMMAND, [this]() { putTuplet(6); });
    registerCommand(ADD_SEPTUPLET_COMMAND, [this]() { putTuplet(7); });
    registerCommand(ADD_OCTUPLET_COMMAND, [this]() { putTuplet(8); });
    registerCommand(ADD_NONUPLET_COMMAND, [this]() { putTuplet(9); });

    // editing commands
    registerCommand(COPY_COMMAND, &Interaction::copySelection);
    registerCommand(CUT_COMMAND, &Controller::cutSelection);
    registerCommand(PASTE_COMMAND, [this]() { pasteSelection(PastingType::Default); });
    registerCommand(DELETE_COMMAND, &Interaction::deleteSelection);

    // move commands
    registerCommand(MOVE_RIGHT_COMMAND, [this]() { move(MoveDirection::Right, false); });
    registerCommand(MOVE_LEFT_COMMAND, [this]() { move(MoveDirection::Left, false); });
    registerCommand(MOVE_RIGHT_QUICKLY_COMMAND, [this]() { move(MoveDirection::Right, true); });
    registerCommand(MOVE_LEFT_QUICKLY_COMMAND, [this]() { move(MoveDirection::Left, true); });

    registerCommand(PITCH_UP_COMMAND, [this]() { move(MoveDirection::Up, false); });
    registerCommand(PITCH_DOWN_COMMAND, [this]() { move(MoveDirection::Down, false); });
    registerCommand(PITCH_UP_OCTAVE_COMMAND, [this]() { move(MoveDirection::Up, true); });
    registerCommand(PITCH_DOWN_OCTAVE_COMMAND, [this]() { move(MoveDirection::Down, true); });

    // --------------------

    m_isAllowedDuringPlayback.insert("action://notation/cancel");

    registerAction("note-action", &Controller::handleNoteAction); // used for drums

    registerAction("next-beat-TEXT", &Controller::nextBeatTextElement, &Controller::textNavigationByBeatsAvailable);
    registerAction("prev-beat-TEXT", &Controller::prevBeatTextElement, &Controller::textNavigationByBeatsAvailable);

    for (auto it = DURATIONS_FOR_TEXT_NAVIGATION.cbegin(); it != DURATIONS_FOR_TEXT_NAVIGATION.cend(); ++it) {
        registerAction(it.key(), [this, fraction = it.value()]() {
            navigateToTextElementByFraction(
                fraction);
        }, &Controller::textNavigationByFractionAvailable);
    }

    registerAction("next-lyric-verse", &Interaction::navigateToLyricsVerse, MoveDirection::Down, PlayMode::NoPlay,
                   &Controller::isEditingLyrics);
    registerAction("prev-lyric-verse", &Interaction::navigateToLyricsVerse, MoveDirection::Up, PlayMode::NoPlay,
                   &Controller::isEditingLyrics);
    registerAction("next-syllable", &Interaction::navigateToNextSyllable, PlayMode::NoPlay, &Controller::isEditingLyrics);

    registerAction("add-melisma", &Interaction::addMelisma, PlayMode::NoPlay, &Controller::isEditingLyrics);
    registerAction("add-lyric-verse", &Interaction::addLyricsVerse, PlayMode::NoPlay, &Controller::isEditingLyrics);

    registerAction("rest", &Interaction::putRestToSelection);

    registerAction("put-note", &Controller::putNote);
    registerAction("remove-note", &Controller::removeNote);

    registerAction("toggle-visible", &Interaction::toggleVisible, &Controller::isToggleVisibleAllowed);

    m_isAllowedDuringPlayback.insert({
        "notation-move-right", "notation-move-left",
        "notation-move-right-quickly", "notation-move-left-quickly",
    });

    registerAction("double-duration", &Controller::doubleNoteInputDuration);
    registerAction("half-duration", &Controller::halveNoteInputDuration);
    registerAction("inc-duration-dotted", &Interaction::increaseDecreaseDuration, -1, true);
    registerAction("dec-duration-dotted", &Interaction::increaseDecreaseDuration, 1, true);

    registerAction("notation-paste-half", [this]() { pasteSelection(PastingType::Half); });
    registerAction("notation-paste-double", [this]() { pasteSelection(PastingType::Double); });
    registerAction("notation-paste-special", [this]() { pasteSelection(PastingType::Special); });
    registerAction("notation-swap", &Interaction::swapSelection, &Controller::hasSelection);

    registerAction("chord-tie", &Controller::chordTie);

    registerAction("hammer-on-pull-off", &Controller::addHammerOnPullOff);

    registerAction("move-up", &Interaction::moveChordRestToStaff, MoveDirection::Up, &Controller::hasSelection);
    registerAction("move-down", &Interaction::moveChordRestToStaff, MoveDirection::Down, &Controller::hasSelection);
    registerAction("move-left", &Interaction::swapChordRest, MoveDirection::Left, &Controller::isNoteInputMode);
    registerAction("move-right", &Interaction::swapChordRest, MoveDirection::Right, &Controller::isNoteInputMode);
    registerAction("toggle-snap-to-previous", &Interaction::toggleSnapToPrevious, &Controller::hasSelection);
    registerAction("toggle-snap-to-next", &Interaction::toggleSnapToNext, &Controller::hasSelection);
    registerAction("next-segment-element", &Interaction::moveSegmentSelection, MoveDirection::Right, PlayMode::PlayNote);
    registerAction("prev-segment-element", &Interaction::moveSegmentSelection, MoveDirection::Left, PlayMode::PlayNote);

    registerAction("system-break", &Interaction::toggleLayoutBreak, LayoutBreakType::LINE, PlayMode::NoPlay,
                   &Controller::toggleLayoutBreakAvailable);
    registerAction("page-break", &Interaction::toggleLayoutBreak, LayoutBreakType::PAGE, PlayMode::NoPlay,
                   &Controller::toggleLayoutBreakAvailable);
    registerAction("section-break", &Interaction::toggleLayoutBreak, LayoutBreakType::SECTION, PlayMode::NoPlay,
                   &Controller::toggleLayoutBreakAvailable);

    registerAction("apply-system-lock", &Interaction::applySystemLock);
    registerAction("move-measure-to-prev-system", &Interaction::moveMeasureToPrevSystem);
    registerAction("move-measure-to-next-system", &Interaction::moveMeasureToNextSystem);
    registerAction("toggle-system-lock", &Interaction::toggleSystemLock);
    registerAction("toggle-score-lock", &Interaction::toggleScoreLock);
    registerAction("make-into-system", &Interaction::makeIntoSystem);
    registerAction("apply-page-lock", &Interaction::applyPageLock);
    registerAction("move-system-to-prev-page", &Interaction::moveSystemToPrevPage);
    registerAction("move-system-to-next-page", &Interaction::moveSystemToNextPage);
    registerAction("toggle-page-lock", &Interaction::togglePageLock);
    registerAction("make-into-page", &Interaction::makeIntoPage);

    registerAction("split-measure", &Interaction::splitSelectedMeasure);
    registerAction("join-measures", &Interaction::joinSelectedMeasures);

    registerAction("insert-measure", [this]() {
        addBoxes(BoxType::Measure, 1, AddBoxesTarget::BeforeSelection);
    }, &Controller::hasSelection);
    registerAction("insert-measures", [this](const ActionData& actionData) {
        addMeasures(actionData, AddBoxesTarget::BeforeSelection);
    }, &Controller::hasSelection);
    registerAction("insert-measures-after-selection", [this](const ActionData& actionData) {
        addMeasures(actionData, AddBoxesTarget::AfterSelection);
    }, &Controller::hasSelection);
    registerAction("insert-measures-at-start-of-score", [this](const ActionData& actionData) {
        addMeasures(actionData, AddBoxesTarget::AtStartOfScore);
    });
    registerAction("append-measure", [this]() {
        addBoxes(BoxType::Measure, 1, AddBoxesTarget::AtEndOfScore);
    });
    registerAction("append-measures", [this](const ActionData& actionData) {
        addMeasures(actionData, AddBoxesTarget::AtEndOfScore);
    });

    registerAction("insert-hbox", [this]() { addBoxes(BoxType::Horizontal, 1, AddBoxesTarget::BeforeSelection); });
    registerAction("insert-vbox", [this]() { addBoxes(BoxType::Vertical, 1, AddBoxesTarget::BeforeSelection); });
    registerAction("insert-textframe", [this]() { addBoxes(BoxType::Text, 1, AddBoxesTarget::BeforeSelection); });
    registerAction("insert-fretframe", [this]() { addBoxes(BoxType::Fret, 1, AddBoxesTarget::BeforeSelection); });
    registerAction("append-hbox", [this]() { addBoxes(BoxType::Horizontal, 1, AddBoxesTarget::AtEndOfScore); });
    registerAction("append-vbox", [this]() { addBoxes(BoxType::Vertical, 1, AddBoxesTarget::AtEndOfScore); });
    registerAction("append-textframe", [this]() { addBoxes(BoxType::Text, 1, AddBoxesTarget::AtEndOfScore); });
    registerAction("append-fretframe", [this]() { addBoxes(BoxType::Fret, 1, AddBoxesTarget::AtEndOfScore); });

    registerAction("edit-style", &Controller::openEditStyleDialog);
    registerAction("page-settings", &Controller::openPageSettingsDialog);
    registerAction("staff-properties", &Controller::openStaffProperties);
    registerAction("edit-strings", &Controller::openEditStringsDialog);
    registerAction("measures-per-system", &Controller::openBreaksDialog);
    registerAction("transpose", &Controller::openTransposeDialog);
    registerAction("parts", &Controller::openPartsDialog);
    registerAction("staff-text-properties", &Controller::openStaffTextPropertiesDialog);
    registerAction("system-text-properties", &Controller::openStaffTextPropertiesDialog);
    registerAction("measure-properties", &Controller::openMeasurePropertiesDialog);
    registerAction("config-raster", &Controller::openEditGridSizeDialog);
    registerAction("realize-chord-symbols", &Controller::openRealizeChordSymbolsDialog);
    registerAction("add-fretboard-diagram", &Controller::addFretboardDiagram);

    registerAction("load-style", &Controller::loadStyle);
    registerAction("save-style", &Controller::saveStyle);

    registerAction("voice-x12", &Interaction::swapVoices, 0, 1);
    registerAction("voice-x13", &Interaction::swapVoices, 0, 2);
    registerAction("voice-x14", &Interaction::swapVoices, 0, 3);
    registerAction("voice-x23", &Interaction::swapVoices, 1, 2);
    registerAction("voice-x24", &Interaction::swapVoices, 1, 3);
    registerAction("voice-x34", &Interaction::swapVoices, 2, 3);

    registerAction("add-8va", &Interaction::addOttavaToSelection, OttavaType::OTTAVA_8VA);
    registerAction("add-8vb", &Interaction::addOttavaToSelection, OttavaType::OTTAVA_8VB);
    registerAction("add-dynamic", &Interaction::toggleDynamicPopup, &Controller::isNoteOrRestSelected);
    registerAction("add-hairpin", &Interaction::addHairpinsToSelection, HairpinType::CRESC_HAIRPIN, &Controller::isNoteOrRestSelected);
    registerAction("add-hairpin-reverse", &Interaction::addHairpinsToSelection, HairpinType::DIM_HAIRPIN,
                   &Controller::isNoteOrRestSelected);
    registerAction("add-noteline", &Interaction::addAnchoredLineToSelectedNotes);

    registerAction("add-image", [this]() { addImage(); });

    registerAction("title-text", [this]() { addText(TextStyleType::TITLE); });
    registerAction("subtitle-text", [this]() { addText(TextStyleType::SUBTITLE); });
    registerAction("composer-text", [this]() { addText(TextStyleType::COMPOSER); });
    registerAction("poet-text", [this]() { addText(TextStyleType::LYRICIST); });
    registerAction("part-text", [this]() { addText(TextStyleType::INSTRUMENT_EXCERPT); });
    registerAction("frame-text", [this]() { addText(TextStyleType::FRAME); });

    registerAction("system-text", [this]() { addText(TextStyleType::SYSTEM); });
    registerAction("staff-text", [this]() { addText(TextStyleType::STAFF); });
    registerAction("expression-text", [this]() { addText(TextStyleType::EXPRESSION); });
    registerAction("rehearsalmark-text", [this]() { addText(TextStyleType::REHEARSAL_MARK); });
    registerAction("instrument-change-text", [this]() { addText(TextStyleType::INSTRUMENT_CHANGE); });
    registerAction("fingering-text", [this]() { addText(TextStyleType::FINGERING); });
    registerAction("sticking-text", [this]() { addText(TextStyleType::STICKING); });
    registerAction("chord-text", [this]() { addText(TextStyleType::HARMONY_A); });
    registerAction("roman-numeral-text", [this]() { addText(TextStyleType::HARMONY_ROMAN); });
    registerAction("nashville-number-text", [this]() { addText(TextStyleType::HARMONY_NASHVILLE); });
    registerAction("lyrics", [this]() { addText(TextStyleType::LYRICS_ODD); });
    registerAction("tempo", [this]() { addText(TextStyleType::TEMPO); });

    registerAction("figured-bass", [this]() { addFiguredBass(); });

    registerAction("stretch-", [this]() { addStretch(-STRETCH_STEP); });
    registerAction("stretch+", [this]() { addStretch(STRETCH_STEP); });

    registerAction("reset-stretch", &Controller::resetStretch);
    registerAction("reset-text-style-overrides", &Interaction::resetTextStyleOverrides);
    registerAction("reset-beammode", &Controller::resetBeamMode);
    registerAction("reset", &Interaction::resetShapesAndPosition, &Controller::hasSelection);
    registerAction("reset-to-default-layout", &Interaction::resetToDefaultLayout);

    registerAction("show-invisible", [this]() { toggleScoreConfig(ScoreConfigType::ShowInvisibleElements); });
    registerAction("show-unprintable", [this]() { toggleScoreConfig(ScoreConfigType::ShowUnprintableElements); });
    registerAction("show-frames", [this]() { toggleScoreConfig(ScoreConfigType::ShowFrames); });
    registerAction("show-pageborders", [this]() { toggleScoreConfig(ScoreConfigType::ShowPageMargins); });
    registerAction("show-soundflags", [this]() { toggleScoreConfig(ScoreConfigType::ShowSoundFlags); });
    registerAction("show-irregular", [this]() { toggleScoreConfig(ScoreConfigType::MarkIrregularMeasures); });

    registerAction("concert-pitch", &Controller::toggleConcertPitch);

    registerAction("explode", &Interaction::explodeSelectedStaff);
    registerAction("implode", &Interaction::implodeSelectedStaff);
    registerAction("extend-to-next-note", &Interaction::extendToNextNote);
    registerAction("time-delete", &Interaction::removeSelectedRange);
    registerAction("del-empty-measures", &Interaction::removeEmptyTrailingMeasures);
    registerAction("slash-fill", &Interaction::fillSelectionWithSlashes);
    registerAction("slash-rhythm", &Interaction::replaceSelectedNotesWithSlashes);
    registerAction("pitch-spell", &Interaction::spellPitches);
    registerAction("pitch-spell-sharps", &Interaction::spellPitchesWithSharps);
    registerAction("pitch-spell-flats", &Interaction::spellPitchesWithFlats);
    registerAction("reset-groupings", &Interaction::regroupNotesAndRests);
    registerAction("resequence-rehearsal-marks", &Interaction::resequenceRehearsalMarks);

    registerAction("unroll-repeats", &Controller::unrollRepeats);

    registerAction("copy-lyrics-to-clipboard", &Interaction::copyLyrics);
    registerAction("acciaccatura", &Interaction::addGraceNotesToSelectedNotes, GraceNoteType::ACCIACCATURA);
    registerAction("appoggiatura", &Interaction::addGraceNotesToSelectedNotes, GraceNoteType::APPOGGIATURA);
    registerAction("grace4", &Interaction::addGraceNotesToSelectedNotes, GraceNoteType::GRACE4);
    registerAction("grace16", &Interaction::addGraceNotesToSelectedNotes, GraceNoteType::GRACE16);
    registerAction("grace32", &Interaction::addGraceNotesToSelectedNotes, GraceNoteType::GRACE32);
    registerAction("grace8after", &Interaction::addGraceNotesToSelectedNotes, GraceNoteType::GRACE8_AFTER);
    registerAction("grace16after", &Interaction::addGraceNotesToSelectedNotes, GraceNoteType::GRACE16_AFTER);
    registerAction("grace32after", &Interaction::addGraceNotesToSelectedNotes, GraceNoteType::GRACE32_AFTER);

    registerAction("beam-auto", &Interaction::addBeamToSelectedChordRests, BeamMode::AUTO);
    registerAction("beam-none", &Interaction::addBeamToSelectedChordRests, BeamMode::NONE);
    registerAction("beam-break-left", &Interaction::addBeamToSelectedChordRests, BeamMode::BEGIN);
    registerAction("beam-break-inner-8th", &Interaction::addBeamToSelectedChordRests, BeamMode::BEGIN16);
    registerAction("beam-break-inner-16th", &Interaction::addBeamToSelectedChordRests, BeamMode::BEGIN32);
    registerAction("beam-join", &Interaction::addBeamToSelectedChordRests, BeamMode::MID);
    registerAction("beam-selected-range", &Interaction::beamSelectedRange);

    registerAction("add-brackets", &Interaction::addBracketsToSelection, BracketsType::Brackets);
    registerAction("add-parentheses", &Interaction::addBracketsToSelection, BracketsType::Parentheses);
    registerAction("add-braces", &Interaction::addBracketsToSelection, BracketsType::Braces);

    registerAction("enh-both", &Interaction::changeEnharmonicSpelling, true);
    registerAction("enh-current", &Interaction::changeEnharmonicSpelling, false);

    registerAction("edit-element", &Controller::startEditSelectedElement);
    registerAction("edit-text", &Controller::startEditSelectedText);

    registerAction("text-b", &Interaction::toggleBold, &Controller::isEditingText);
    registerAction("text-i", &Interaction::toggleItalic, &Controller::isEditingText);
    registerAction("text-u", &Interaction::toggleUnderline, &Controller::isEditingText);
    registerAction("text-s", &Interaction::toggleStrike, &Controller::isEditingText);
    registerAction("text-sub", &Interaction::toggleSubScript, &Controller::isEditingText);
    registerAction("text-sup", &Interaction::toggleSuperScript, &Controller::isEditingText);

    registerAddToSelectionAction("select-next-chord", MoveSelectionType::Chord, MoveDirection::Right);
    registerAddToSelectionAction("select-prev-chord", MoveSelectionType::Chord, MoveDirection::Left);
    registerAddToSelectionAction("select-next-measure", MoveSelectionType::Measure, MoveDirection::Right);
    registerAddToSelectionAction("select-prev-measure", MoveSelectionType::Measure, MoveDirection::Left);
    registerAddToSelectionAction("select-staff-above", MoveSelectionType::Track, MoveDirection::Up);
    registerAddToSelectionAction("select-staff-below", MoveSelectionType::Track, MoveDirection::Down);

    registerExpandSelectionAction("select-begin-line", ExpandSelectionMode::BeginSystem);
    registerExpandSelectionAction("select-end-line", ExpandSelectionMode::EndSystem);
    registerExpandSelectionAction("select-begin-score", ExpandSelectionMode::BeginScore);
    registerExpandSelectionAction("select-end-score", ExpandSelectionMode::EndScore);

    registerAction("top-staff", &Interaction::selectTopStaff, PlayMode::PlayChord);
    registerAction("empty-trailing-measure", &Interaction::selectEmptyTrailingMeasure);
    m_isAllowedDuringPlayback.insert({ "top-staff", "empty-trailing-measure" });

    registerAction("pitch-up-diatonic", &Controller::movePitchDiatonic, MoveDirection::Up, false);
    registerAction("pitch-down-diatonic", &Controller::movePitchDiatonic, MoveDirection::Down, false);

    registerAction("repeat-sel", &Controller::repeatSelection);

    registerAction("add-turn", &Interaction::toggleOrnament, mu::engraving::SymId::ornamentTurn);
    registerAction("add-turn-inverted", &Interaction::toggleOrnament, mu::engraving::SymId::ornamentTurnInverted);
    registerAction("add-turn-slash", &Interaction::toggleOrnament, mu::engraving::SymId::ornamentTurnSlash);
    registerAction("add-turn-up", &Interaction::toggleOrnament, mu::engraving::SymId::ornamentTurnUp);
    registerAction("add-turn-inverted-up", &Interaction::toggleOrnament, mu::engraving::SymId::ornamentTurnUpS);
    registerAction("add-trill", &Interaction::toggleOrnament, mu::engraving::SymId::ornamentTrill);
    registerAction("add-short-trill", &Interaction::toggleOrnament, mu::engraving::SymId::ornamentShortTrill);
    registerAction("add-mordent", &Interaction::toggleOrnament, mu::engraving::SymId::ornamentMordent);
    registerAction("add-haydn", &Interaction::toggleOrnament, mu::engraving::SymId::ornamentHaydn);
    registerAction("add-tremblement", &Interaction::toggleOrnament, mu::engraving::SymId::ornamentTremblement);
    registerAction("add-prall-mordent", &Interaction::toggleOrnament, mu::engraving::SymId::ornamentPrallMordent);
    registerAction("add-shake", &Interaction::toggleOrnament, mu::engraving::SymId::ornamentShake3);
    registerAction("add-shake-muffat", &Interaction::toggleOrnament, mu::engraving::SymId::ornamentShakeMuffat1);
    registerAction("add-tremblement-couperin", &Interaction::toggleOrnament, mu::engraving::SymId::ornamentTremblementCouperin);

    registerAction("add-up-bow", &Interaction::toggleArticulation, mu::engraving::SymId::stringsUpBow);
    registerAction("add-down-bow", &Interaction::toggleArticulation, mu::engraving::SymId::stringsDownBow);
    registerAction("transpose-up", &Interaction::transposeSemitone, 1, PlayMode::PlayNote);
    registerAction("transpose-down", &Interaction::transposeSemitone, -1, PlayMode::PlayNote);
    registerAction("toggle-insert-mode", [this]() { toggleNoteInputInsert(); }, &NotationActionController::isNotEditingElement);

    registerAction("get-location", &Interaction::getLocation, &Controller::isNotationPage);
    registerAction("toggle-mmrest", &Interaction::execute, &mu::engraving::Score::cmdToggleMmrest,
                   TranslatableString("undoableAction", "Toggle multimeasure rests"));
    registerAction("toggle-hide-empty", &Interaction::execute, &mu::engraving::Score::cmdToggleHideEmpty,
                   TranslatableString("undoableAction", "Toggle empty staves"));

    registerAction("mirror-note", &Interaction::mirrorNotes, &Controller::hasSelection);

    registerAction("clef-violin", [this]() { insertClef(mu::engraving::ClefType::G); });
    registerAction("clef-bass", [this]() { insertClef(mu::engraving::ClefType::F); });

    registerAction("sharp2-post", &Interaction::changeAccidental, mu::engraving::AccidentalType::SHARP2, PlayMode::PlayNote);
    registerAction("sharp-post", &Interaction::changeAccidental, mu::engraving::AccidentalType::SHARP, PlayMode::PlayNote);
    registerAction("nat-post", &Interaction::changeAccidental, mu::engraving::AccidentalType::NATURAL, PlayMode::PlayNote);
    registerAction("flat-post", &Interaction::changeAccidental, mu::engraving::AccidentalType::FLAT, PlayMode::PlayNote);
    registerAction("flat2-post", &Interaction::changeAccidental, mu::engraving::AccidentalType::FLAT2, PlayMode::PlayNote);
    registerAction("pitch-up-diatonic-alterations", &Interaction::transposeDiatonicAlterations, mu::engraving::TransposeDirection::UP,
                   PlayMode::PlayNote);
    registerAction("pitch-down-diatonic-alterations", &Interaction::transposeDiatonicAlterations, mu::engraving::TransposeDirection::DOWN,
                   PlayMode::PlayNote);
    registerAction("full-measure-rest", &Interaction::execute, &mu::engraving::Score::cmdFullMeasureRest,
                   TranslatableString("undoableAction", "Enter full-measure rest"));
    registerAction("set-visible", &Interaction::setSelectionVisible, true);
    registerAction("unset-visible", &Interaction::setSelectionVisible, false);
    registerAction("toggle-autoplace", &Interaction::toggleAutoplace, false);
    registerAction("autoplace-enabled", &Interaction::toggleAutoplace, true);

    for (int i = MIN_NOTES_INTERVAL; i <= MAX_NOTES_INTERVAL; ++i) {
        if (isNotesIntervalValid(i)) {
            registerAction("interval" + std::to_string(i), &Interaction::addIntervalToSelectedNotes, i, PlayMode::PlayChord);
        }
    }

    registerAction("voice-assignment-all-in-instrument", &Interaction::changeSelectedElementsVoiceAssignment,
                   VoiceAssignment::ALL_VOICE_IN_INSTRUMENT);
    registerAction("voice-assignment-all-in-staff", &Interaction::changeSelectedElementsVoiceAssignment,
                   VoiceAssignment::ALL_VOICE_IN_STAFF);

    // TAB
    registerAction("string-above", &Controller::move, MoveDirection::Up, false, &Controller::isTablatureStaff);
    registerAction("string-below", &Controller::move, MoveDirection::Down, false, &Controller::isTablatureStaff);
    registerAction("pad-note-1-TAB", [this]() { setDuration(DurationType::V_WHOLE); }, &NotationActionController::isTablatureStaff);
    registerAction("pad-note-2-TAB", [this]() { setDuration(DurationType::V_HALF); }, &NotationActionController::isTablatureStaff);
    registerAction("pad-note-4-TAB", [this]() { setDuration(DurationType::V_QUARTER); }, &NotationActionController::isTablatureStaff);
    registerAction("pad-note-8-TAB", [this]() { setDuration(DurationType::V_EIGHTH); }, &NotationActionController::isTablatureStaff);
    registerAction("pad-note-16-TAB", [this]() { setDuration(DurationType::V_16TH); }, &NotationActionController::isTablatureStaff);
    registerAction("pad-note-32-TAB", [this]() { setDuration(DurationType::V_32ND); }, &NotationActionController::isTablatureStaff);
    registerAction("pad-note-64-TAB", [this]() { setDuration(DurationType::V_64TH); }, &NotationActionController::isTablatureStaff);
    registerAction("pad-note-128-TAB", [this]() { setDuration(DurationType::V_128TH); }, &NotationActionController::isTablatureStaff);
    registerAction("pad-note-256-TAB", [this]() { setDuration(DurationType::V_256TH); }, &NotationActionController::isTablatureStaff);
    registerAction("pad-note-512-TAB", [this]() { setDuration(DurationType::V_512TH); }, &NotationActionController::isTablatureStaff);
    registerAction("pad-note-1024-TAB", [this]() { setDuration(DurationType::V_1024TH); }, &NotationActionController::isTablatureStaff);
    registerAction("rest-TAB", &Interaction::putRestToSelection);

    registerAction("standard-bend", [this]() { addGuitarBend(GuitarBendType::BEND); });
    registerAction("pre-bend",  [this]() { addGuitarBend(GuitarBendType::PRE_BEND); });
    registerAction("grace-note-bend",  [this]() { addGuitarBend(GuitarBendType::GRACE_NOTE_BEND); });
    registerAction("slight-bend",  [this]() { addGuitarBend(GuitarBendType::SLIGHT_BEND); });

    registerAction("dive", [this]() { addGuitarBend(GuitarBendType::DIVE); });
    registerAction("pre-dive",  [this]() { addGuitarBend(GuitarBendType::PRE_DIVE); });
    registerAction("dip",  [this]() { addGuitarBend(GuitarBendType::DIP); });
    registerAction("scoop",  [this]() { addGuitarBend(GuitarBendType::SCOOP); });

    for (int i = 0; i < MAX_FRET; ++i) {
        registerAction("fret-" + std::to_string(i), [i, this]() { addFret(i); }, &Controller::isTablatureStaff);
    }

    registerAction("toggle-automation", &Controller::toggleAutomation);
    m_isAllowedDuringPlayback.insert("toggle-automation");

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

            auto undoStack = notation->undoStack();
            undoStack->stackChanged().onNotify(this, [this]() {
                m_stackChanged.notify();
            }, Asyncable::Mode::SetReplace);
        }
        m_textEditingChanged.send(isTextEditing());
        m_noteInputStateChanged.notify();
    });

    globalContext()->playbackState()->playbackStatusChanged().onReceive(this, [this](muse::audio::PlaybackStatus) {
        m_isNoteInputAllowedChanged.send(isNoteInputAllowed());
    }, Asyncable::Mode::SetReplace);

    // Register engraving debugging options actions
    for (auto& [code, member] : engravingDebuggingActions) {
        dispatcher()->reg(this, code, [this, member = member]() {
            EngravingDebuggingOptions options = engravingConfiguration()->debuggingOptions();
            bool showGapRests = options.showGapRests;
            options.*member = !(options.*member);
            engravingConfiguration()->setDebuggingOptions(options);
            if (options.showGapRests != showGapRests) {
                currentNotation()->interaction()->toggleDebugShowGapRests();
            }
        });
    }
    dispatcher()->reg(this, "check-for-score-corruptions", [this] { checkForScoreCorruptions(); });

    // compat
    {
        static std::map<ActionCode, rcommand::Command> actionToCommand = {
            { "action://notation/copy", COPY_COMMAND },
            { "action://notation/cut", CUT_COMMAND },
            { "action://notation/paste", PASTE_COMMAND },
            { "action://notation/delete", DELETE_COMMAND },
            { "action://notation/cancel", CANCEL_COMMAND },
            { "action://notation/undo", UNDO_COMMAND },
            { "action://notation/redo", REDO_COMMAND },
            { "action://copy", COPY_COMMAND },
            { "action://cut", CUT_COMMAND },
            { "action://paste", PASTE_COMMAND },
            { "action://delete", DELETE_COMMAND },
            { "action://cancel", CANCEL_COMMAND },
            { "action://undo", UNDO_COMMAND },
            { "action://redo", REDO_COMMAND },
            { "notation-move-right", MOVE_RIGHT_COMMAND },
            { "notation-move-left", MOVE_LEFT_COMMAND },
            { "notation-move-right-quickly", MOVE_RIGHT_QUICKLY_COMMAND },
            { "notation-move-left-quickly", MOVE_LEFT_QUICKLY_COMMAND },
            { "pitch-up", PITCH_UP_COMMAND },
            { "pitch-down", PITCH_DOWN_COMMAND },
            { "pitch-up-octave", PITCH_UP_OCTAVE_COMMAND },
            { "pitch-down-octave", PITCH_DOWN_OCTAVE_COMMAND },
            { "next-word", EDIT_NEXT_WORD_COMMAND },
            { "next-text-element", EDIT_NEXT_TEXT_ELEMENT_COMMAND },
            { "prev-text-element", EDIT_PREV_TEXT_ELEMENT_COMMAND },
            { "note-input", TOGGLE_NOTE_INPUT_COMMAND },
            { "note-input-by-note-name", TOGGLE_NOTE_INPUT_BY_NOTE_NAME_COMMAND },
            { "note-input-by-duration", TOGGLE_NOTE_INPUT_BY_DURATION_COMMAND },
            { "note-input-rhythm", TOGGLE_NOTE_INPUT_RHYTHM_COMMAND },
            { "note-input-repitch", TOGGLE_NOTE_INPUT_REPITCH_COMMAND },
            { "note-input-realtime-auto", TOGGLE_NOTE_INPUT_REALTIME_AUTO_COMMAND },
            { "note-input-realtime-manual", TOGGLE_NOTE_INPUT_REALTIME_MANUAL_COMMAND },
            { "note-input-timewise", TOGGLE_NOTE_INPUT_TIMEWISE_COMMAND },
            { "realtime-advance", REALTIME_ADVANCE_COMMAND },
            { "note-longa", SET_DURATION_LONGA_COMMAND },
            { "note-breve", SET_DURATION_BREVE_COMMAND },
            { "pad-note-1", SET_DURATION_WHOLE_COMMAND },
            { "pad-note-2", SET_DURATION_HALF_COMMAND },
            { "pad-note-4", SET_DURATION_QUARTER_COMMAND },
            { "pad-note-8", SET_DURATION_EIGHTH_COMMAND },
            { "pad-note-16", SET_DURATION_16TH_COMMAND },
            { "pad-note-32", SET_DURATION_32ND_COMMAND },
            { "pad-note-64", SET_DURATION_64TH_COMMAND },
            { "pad-note-128", SET_DURATION_128TH_COMMAND },
            { "pad-note-256", SET_DURATION_256TH_COMMAND },
            { "pad-note-512", SET_DURATION_512TH_COMMAND },
            { "pad-note-1024", SET_DURATION_1024TH_COMMAND },
            { "pad-dot", TOGGLE_DOT_COMMAND },
            { "pad-dot2", TOGGLE_DOT2_COMMAND },
            { "pad-dot3", TOGGLE_DOT3_COMMAND },
            { "pad-dot4", TOGGLE_DOT4_COMMAND },
            { "pad-rest", TOGGLE_REST_COMMAND },
            { "flat2", TOGGLE_FLAT2_COMMAND },
            { "flat", TOGGLE_FLAT_COMMAND },
            { "nat", TOGGLE_NAT_COMMAND },
            { "sharp", TOGGLE_SHARP_COMMAND },
            { "sharp2", TOGGLE_SHARP2_COMMAND },
            { "tie", ADD_TIE_COMMAND },
            { "lv", ADD_LV_COMMAND },
            { "add-slur", ADD_SLUR_COMMAND },
            { "add-marcato", ADD_MARCATO_COMMAND },
            { "add-sforzato", ADD_SFORZATO_COMMAND },
            { "add-tenuto", ADD_TENUTO_COMMAND },
            { "add-staccato", ADD_STACCATO_COMMAND },
            { "voice-1", USE_VOICE_1_COMMAND },
            { "voice-2", USE_VOICE_2_COMMAND },
            { "voice-3", USE_VOICE_3_COMMAND },
            { "voice-4", USE_VOICE_4_COMMAND },
            { "flip", FLIP_COMMAND },
            { "flip-horizontally", FLIP_HORIZONTALLY_COMMAND },
            { "note-c", ENTER_NOTE_C_COMMAND },
            { "note-d", ENTER_NOTE_D_COMMAND },
            { "note-e", ENTER_NOTE_E_COMMAND },
            { "note-f", ENTER_NOTE_F_COMMAND },
            { "note-g", ENTER_NOTE_G_COMMAND },
            { "note-a", ENTER_NOTE_A_COMMAND },
            { "note-b", ENTER_NOTE_B_COMMAND },
            { "chord-c", ADD_NOTE_C_COMMAND },
            { "chord-d", ADD_NOTE_D_COMMAND },
            { "chord-e", ADD_NOTE_E_COMMAND },
            { "chord-f", ADD_NOTE_F_COMMAND },
            { "chord-g", ADD_NOTE_G_COMMAND },
            { "chord-a", ADD_NOTE_A_COMMAND },
            { "chord-b", ADD_NOTE_B_COMMAND },
            { "insert-c", INSERT_NOTE_C_COMMAND },
            { "insert-d", INSERT_NOTE_D_COMMAND },
            { "insert-e", INSERT_NOTE_E_COMMAND },
            { "insert-f", INSERT_NOTE_F_COMMAND },
            { "insert-g", INSERT_NOTE_G_COMMAND },
            { "insert-a", INSERT_NOTE_A_COMMAND },
            { "insert-b", INSERT_NOTE_B_COMMAND },
            { "duplet", ADD_DUPLET_COMMAND },
            { "triplet", ADD_TRIPLET_COMMAND },
            { "quadruplet", ADD_QUADRUPLET_COMMAND },
            { "quintuplet", ADD_QUINTUPLET_COMMAND },
            { "sextuplet", ADD_SEXTUPLET_COMMAND },
            { "septuplet", ADD_SEPTUPLET_COMMAND },
            { "octuplet", ADD_OCTUPLET_COMMAND },
            { "nonuplet", ADD_NONUPLET_COMMAND },
            { "tuplet-dialog", SHOW_TUPLET_CONFIGURE_COMMAND },
            { "first-element", GOTO_FIRST_ELEMENT_COMMAND },
            { "last-element", GOTO_LAST_ELEMENT_COMMAND },
            { "next-element", GOTO_NEXT_ELEMENT_COMMAND },
            { "prev-element", GOTO_PREV_ELEMENT_COMMAND },
            { "next-track", GOTO_NEXT_TRACK_COMMAND },
            { "prev-track", GOTO_PREV_TRACK_COMMAND },
            { "next-frame", GOTO_NEXT_FRAME_COMMAND },
            { "prev-frame", GOTO_PREV_FRAME_COMMAND },
            { "next-system", GOTO_NEXT_SYSTEM_COMMAND },
            { "prev-system", GOTO_PREV_SYSTEM_COMMAND },
            { "up-chord", GOTO_UPNOTE_IN_CHORD_COMMAND },
            { "down-chord", GOTO_DOWNNOTE_IN_CHORD_COMMAND },
            { "top-chord", GOTO_TOPNOTE_IN_CHORD_COMMAND },
            { "bottom-chord", GOTO_BOTTOMNOTE_IN_CHORD_COMMAND },
            { "select-similar", SELECT_SIMILAR_COMMAND },
            { "select-similar-staff", SELECT_SIMILAR_IN_STAFF_COMMAND },
            { "select-similar-range", SELECT_SIMILAR_IN_RANGE_COMMAND },
            { "select-notes-in-chord", SELECT_NOTES_IN_CHORD_COMMAND },
            { "notation-select-all", SELECT_ALL_COMMAND },
            { "notation-select-section", SELECT_SECTION_COMMAND },
        };

        auto ad = dispatcher();
        auto d = commandDispatcher();
        for (const auto& [actionCode, command] : actionToCommand) {
            ad->reg(this, actionCode, [d, command]() { return d->dispatch(command); });
        }

        ad->reg(this, "custom-tuplet", [d](const ActionData& args) {
            IF_ASSERT_FAILED(args.count() > 0) {
                return;
            }

            TupletOptions options = args.arg<TupletOptions>(0);

            rcommand::CommandQuery query(ADD_TUPLET_COMMAND);
            query.addParam("ratio", Val(options.ratio.toString().toStdString()));
            query.addParam("number-type", Val(engraving::str_conv(options.numberType)));
            query.addParam("bracket-type", Val(engraving::str_conv(options.bracketType)));
            query.addParam("auto-baselen", Val(options.autoBaseLen));
            d->dispatch(query);
        });
    }
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

INotationStylePtr NotationActionController::currentNotationStyle() const
{
    auto notation = currentNotation();
    if (!notation) {
        return nullptr;
    }

    return notation->style();
}

muse::async::Notification NotationActionController::currentNotationStyleChanged() const
{
    return currentNotationStyle() ? currentNotationStyle()->styleChanged() : muse::async::Notification();
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

void NotationActionController::handleNoteAction(NoteName note, NoteAddingMode addingMode)
{
    NoteInputParams params;
    const bool addFlag = addingMode == NoteAddingMode::CurrentChord;
    bool ok = mu::engraving::NoteInput::resolveNoteInputParams(currentNotationScore(), static_cast<int>(note), addFlag, params);
    if (!ok) {
        LOGE() << "Could not resolve note input params, note: " << (int)note << ", addFlag: " << addFlag;
        return;
    }

    handleNoteAction(ActionData::make_arg2<NoteInputParams, NoteAddingMode>(params, addingMode));
}

void NotationActionController::handleNoteAction(const muse::actions::ActionData& args)
{
    handleNoteAction(args.arg<NoteInputParams>(0), args.arg<NoteAddingMode>(1));
}

void NotationActionController::handleNoteAction(const muse::rcommand::CommandQuery& query)
{
    TRACEFUNC;

    NoteName note = str_conv(query.param("note").toString(), NoteName::C);
    NoteAddingMode mode = str_conv(query.param("mode").toString(), NoteAddingMode::CurrentChord);

    handleNoteAction(note, mode);
}

void NotationActionController::handleNoteAction(const NoteInputParams& params, const NoteAddingMode& addingMode)
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

void NotationActionController::putNote(const ActionData& args)
{
    TRACEFUNC;

    INotationNoteInputPtr noteInput = currentNotationNoteInput();
    if (!noteInput) {
        return;
    }

    IF_ASSERT_FAILED(args.count() > 2) {
        return;
    }

    PointF pos = args.arg<PointF>(0);
    bool replace = args.arg<bool>(1);
    bool insert = args.arg<bool>(2);

    Ret ret = noteInput->putNote(pos, replace, insert);
    if (ret) {
        seekAndPlaySelectedElement();
    }
}

void NotationActionController::removeNote(const ActionData& args)
{
    TRACEFUNC;

    INotationNoteInputPtr noteInput = currentNotationNoteInput();
    if (!noteInput) {
        return;
    }

    IF_ASSERT_FAILED(args.count() == 1) {
        return;
    }

    PointF pos = args.arg<PointF>(0);
    noteInput->removeNote(pos);
    seekSelectedElement();
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

void NotationActionController::putTuplet(const muse::rcommand::CommandQuery& query)
{
    TupletOptions options;
    options.ratio = engraving::Fraction::fromString(muse::String::fromStdString(query.param("ratio").toString()));
    options.numberType = engraving::str_conv(query.param("number-type").toString(), engraving::TupletNumberType::SHOW_NUMBER);
    options.bracketType = engraving::str_conv(query.param("bracket-type").toString(), engraving::TupletBracketType::AUTO_BRACKET);
    options.autoBaseLen = query.param("auto-baselen", Val(false)).toBool();

    putTuplet(options);
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
    if (INotationStylePtr style = currentNotationStyle()) {
        int bracketType = style->styleValue(StyleId::tupletBracketType).toInt();
        options.bracketType = static_cast<engraving::TupletBracketType>(bracketType);
        int numberType = style->styleValue(StyleId::tupletNumberType).toInt();
        options.numberType = static_cast<engraving::TupletNumberType>(numberType);
    }

    putTuplet(options);
}

void NotationActionController::doubleNoteInputDuration()
{
    TRACEFUNC;

    INotationInteractionPtr interaction = currentNotationInteraction();
    if (!interaction) {
        return;
    }

    INotationNoteInputPtr noteInput = interaction->noteInput();

    if (noteInput->isNoteInputMode()) {
        noteInput->doubleNoteInputDuration();
    } else {
        interaction->increaseDecreaseDuration(-1, false);
    }
}

void NotationActionController::halveNoteInputDuration()
{
    TRACEFUNC;

    INotationInteractionPtr interaction = currentNotationInteraction();
    if (!interaction) {
        return;
    }

    INotationNoteInputPtr noteInput = interaction->noteInput();

    if (noteInput->isNoteInputMode()) {
        noteInput->halveNoteInputDuration();
    } else {
        interaction->increaseDecreaseDuration(1, false);
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

void NotationActionController::select(SelectionTarget target)
{
    auto interaction = currentNotationInteraction();
    if (!interaction) {
        return;
    }

    interaction->select(target);
    seekSelectedElement();
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
        interaction->addTieToSelection();
    }
}

void NotationActionController::chordTie()
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
        interaction->addTiedNoteToChord();
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

void NotationActionController::startEditSelectedElement(const ActionData& args)
{
    auto interaction = currentNotationInteraction();
    if (!interaction) {
        return;
    }

    auto selection = interaction->selection();
    if (!selection) {
        return;
    }

    mu::engraving::EngravingItem* element = selection->element();
    if (!element) {
        return;
    }

    if (element->isInstrumentName()) {
        openStaffProperties();
        return;
    }

    if (elementHasPopup(element) && !interaction->textEditingAllowed(element)) {
        dispatcher()->dispatch("notation-popup-menu", ActionData::make_arg1<EngravingItem*>(element));
        return;
    }

    if (element->isText()) {
        TextStyleType styleType = mu::engraving::toText(element)->textStyleType();

        if (styleType == mu::engraving::TextStyleType::HEADER || styleType == mu::engraving::TextStyleType::FOOTER) {
            openEditStyleDialog(ActionData::make_arg1<QString>("header-and-footer"));
            return;
        }
    }

    if (interaction->textEditingAllowed(element)) {
        PointF cursorPos = !args.empty() ? args.arg<PointF>(0) : PointF();
        interaction->startEditText(element, cursorPos);
    } else {
        interaction->startEditElement(element);
    }
}

void NotationActionController::startEditSelectedText(const ActionData& args)
{
    auto interaction = currentNotationInteraction();
    if (!interaction) {
        return;
    }

    auto selection = interaction->selection();
    if (!selection) {
        return;
    }

    mu::engraving::EngravingItem* element = selection->element();

    if (interaction->textEditingAllowed(element)) {
        PointF cursorPos = !args.empty() ? args.arg<PointF>(0) : PointF();
        interaction->startEditText(element, cursorPos);
    }
}

void NotationActionController::addMeasures(const ActionData& actionData, AddBoxesTarget target)
{
    TRACEFUNC;

    if (!actionData.empty()) {
        int count = actionData.arg<int>();
        addBoxes(BoxType::Measure, count, target);
    } else {
        interactive()->open("musescore://notation/selectmeasurescount")
        .onResolve(this, [this, target](const Val& v) {
            int count = v.toInt();
            addBoxes(BoxType::Measure, count, target);
        });
    }
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

void NotationActionController::openEditStyleDialog(const ActionData& args)
{
    UriQuery uri("musescore://notation/style");

    if (args.count() > 0) {
        uri.addParam("currentPageCode", Val(args.arg<QString>(0)));
    }

    if (args.count() > 1) {
        uri.addParam("currentSubPageCode", Val(args.arg<QString>(1)));
    }

    interactive()->open(uri);
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
        if (!currentNotationStyle()->loadStyle(path.toQString(), false)) {
            auto promise = interactive()->warning(
                muse::trc("notation",
                          "Since this style file is from a different version of MuseScore Studio, your score is not guaranteed to display correctly."),
                muse::trc("notation", "Click OK to load anyway."), { IInteractive::Button::Ok, IInteractive::Button::Cancel },
                IInteractive::Button::Ok);

            promise.onResolve(this, [this, path](const IInteractive::Result& res) {
                if (res.isButton(IInteractive::Button::Ok)) {
                    currentNotationStyle()->loadStyle(path.toQString(), true);
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
        if (!currentNotationStyle()->saveStyle(path)) {
            interactive()->error(muse::trc("notation", "The style file could not be saved."),
                                 muse::trc("notation", "An error occurred."));
        }
    }
}

bool NotationActionController::measureNavigationAvailable() const
{
    return isNotEditingOrHasPopup() || textNavigationAvailable();
}

bool NotationActionController::toggleLayoutBreakAvailable() const
{
    INotationInteractionPtr interaction = currentNotationInteraction();
    return interaction && interaction->toggleLayoutBreakAvailable();
}

bool NotationActionController::isTextEditing() const
{
    auto interaction = currentNotationInteraction();
    if (!interaction) {
        return false;
    }

    return interaction->isTextEditingStarted();
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

void NotationActionController::navigateToTextElementByFraction(const Fraction& fraction)
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

bool NotationActionController::isEditingLyrics() const
{
    auto interaction = currentNotationInteraction();
    if (!interaction) {
        return false;
    }

    return interaction->isTextEditingStarted() && interaction->selection()->element()
           && interaction->selection()->element()->isLyrics();
}

bool NotationActionController::isNotNoteInputMode() const
{
    return currentNotationInteraction() && !isNoteInputMode();
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

    switch (configType) {
    case ScoreConfigType::ShowInvisibleElements:
        config.isShowInvisibleElements = !config.isShowInvisibleElements;
        break;
    case ScoreConfigType::ShowUnprintableElements:
        config.isShowUnprintableElements = !config.isShowUnprintableElements;
        break;
    case ScoreConfigType::ShowFrames:
        config.isShowFrames = !config.isShowFrames;
        break;
    case ScoreConfigType::ShowPageMargins:
        config.isShowPageMargins = !config.isShowPageMargins;
        break;
    case ScoreConfigType::ShowSoundFlags:
        config.isShowSoundFlags = !config.isShowSoundFlags;
        break;
    case ScoreConfigType::MarkIrregularMeasures:
        config.isMarkIrregularMeasures = !config.isMarkIrregularMeasures;
        break;
    }

    interaction->setScoreConfig(config);
    interaction->scoreConfigChanged().send(configType);
}

void NotationActionController::toggleConcertPitch()
{
    TRACEFUNC;
    INotationStylePtr style = currentNotationStyle();
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

bool NotationActionController::isNotEditingOrHasPopup() const
{
    const EngravingItem* element = selectedElement();

    if (!element) {
        return isNotEditingElement();
    }

    return elementHasPopup(element) || isNotEditingElement();
}

bool NotationActionController::isToggleVisibleAllowed() const
{
    auto interaction = currentNotationInteraction();
    if (interaction) {
        return !(interaction->isTextEditingStarted());
    }
    return false;
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

void NotationActionController::registerAction(const ActionCode& code,
                                              std::function<void()> handler, bool (NotationActionController::* isEnabled)() const)
{
    m_isEnabledMap[code] = std::bind(isEnabled, this);
    dispatcher()->reg(this, code, handler);
}

void NotationActionController::registerAction(const ActionCode& code,
                                              std::function<void(const ActionData&)> handler,
                                              bool (NotationActionController::* isEnabled)() const)
{
    m_isEnabledMap[code] = std::bind(isEnabled, this);
    dispatcher()->reg(this, code, handler);
}

void NotationActionController::registerAction(const ActionCode& code,
                                              void (NotationActionController::* handler)(const ActionData& data),
                                              bool (NotationActionController::* isEnabled)() const)
{
    m_isEnabledMap[code] = std::bind(isEnabled, this);
    dispatcher()->reg(this, code, this, handler);
}

void NotationActionController::registerAction(const ActionCode& code,
                                              void (NotationActionController::* handler)(),
                                              bool (NotationActionController::* isEnabled)() const)
{
    m_isEnabledMap[code] = std::bind(isEnabled, this);
    dispatcher()->reg(this, code, this, handler);
}

void NotationActionController::registerNoteInputAction(const ActionCode& code, NoteInputMethod inputMethod)
{
    registerAction(code, [this, inputMethod]() { toggleNoteInput(inputMethod); }, &Controller::toggleNoteInputAllowed);
}

bool NotationActionController::isNoteInputActionAllowed() const
{
    if (!isNoteInputMode() && !toggleNoteInputAllowed()) {
        return false;
    }

    return !isTablatureStaff();
}

void NotationActionController::select(const muse::rcommand::CommandQuery& query)
{
    LOGDA() << query.toString();

    SelectionTarget target = str_conv(query.param("target").toString(), SelectionTarget::Undefined);
    if (target == SelectionTarget::Undefined) {
        return;
    }

    select(target);

    PlayMode playMode = str_conv(query.param("play-mode").toString(), PlayMode::NoPlay);
    if (playMode != PlayMode::NoPlay) {
        seekSelectedElement();
        playSelectedElement(playMode == PlayMode::PlayChord);
    }
}

void NotationActionController::registerSelectionCommand(const muse::rcommand::Command& command,
                                                        SelectionTarget target, PlayMode playMode)
{
    registerCommand(command, [this, target, playMode]() {
        select(target);

        if (playMode != PlayMode::NoPlay) {
            seekSelectedElement();
            playSelectedElement(playMode == PlayMode::PlayChord);
        }
    });
}

void NotationActionController::registerAddToSelectionAction(const ActionCode& code, MoveSelectionType type, MoveDirection direction)
{
    registerAction(code, &Interaction::addToSelection, direction, type, PlayMode::NoPlay, &Controller::isNotNoteInputMode);
    m_isAllowedDuringPlayback.insert(code);
}

void NotationActionController::registerExpandSelectionAction(const ActionCode& code, ExpandSelectionMode mode)
{
    registerAction(code, &Interaction::expandSelection, mode, PlayMode::NoPlay, &Controller::isNotNoteInputMode);
    m_isAllowedDuringPlayback.insert(code);
}

void NotationActionController::registerAction(const ActionCode& code,
                                              void (INotationInteraction::* handler)(), PlayMode playMode,
                                              bool (NotationActionController::* enabler)() const)
{
    registerAction(code, [this, handler, playMode]()
    {
        INotationPtr notation = currentNotation();
        if (notation) {
            (notation->interaction().get()->*handler)();

            seekSelectedElement();

            if (playMode != PlayMode::NoPlay) {
                playSelectedElement(playMode == PlayMode::PlayChord);
            }
        }
    }, enabler);
}

void NotationActionController::registerAction(const ActionCode& code,
                                              void (NotationActionController::* handler)(MoveDirection,
                                                                                         bool), MoveDirection direction, bool quickly,
                                              bool (NotationActionController::* enabler)() const)
{
    registerAction(code, [this, handler, direction, quickly]() { (this->*handler)(direction, quickly); }, enabler);
}

void NotationActionController::registerAction(const muse::actions::ActionCode& code,
                                              void (NotationActionController::* handler)(), Ret (INotationInteraction::*enabler)() const)
{
    auto _enabler = [this, enabler]() {
        INotationPtr notation = currentNotation();
        if (notation) {
            return ((*notation->interaction()).*enabler)().success();
        }

        return false;
    };

    m_isEnabledMap[code] = _enabler;
    dispatcher()->reg(this, code, this, handler);
}

void NotationActionController::registerAction(const muse::actions::ActionCode& code, std::function<void()> handler,
                                              Ret (INotationInteraction::*enabler)() const)
{
    auto _enabler = [this, enabler]() {
        INotationPtr notation = currentNotation();
        if (notation) {
            return ((*notation->interaction()).*enabler)().success();
        }

        return false;
    };

    m_isEnabledMap[code] = _enabler;
    dispatcher()->reg(this, code, handler);
}

void NotationActionController::registerAction(const ActionCode& code,
                                              void (INotationInteraction::* handler)(), bool (NotationActionController::* enabler)() const)
{
    registerAction(code, handler, PlayMode::NoPlay, enabler);
}

template<class P1>
void NotationActionController::registerAction(const ActionCode& code, void (INotationInteraction::* handler)(P1),
                                              P1 param1, PlayMode playMode, bool (NotationActionController::* enabler)() const)
{
    registerAction(code, [this, handler, param1, playMode]()
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

template<class P1>
void NotationActionController::registerAction(const ActionCode& code,
                                              void (INotationInteraction::* handler)(
                                                  P1), P1 param1, bool (NotationActionController::* enabler)() const)
{
    registerAction(code, handler, param1, PlayMode::NoPlay, enabler);
}

template<typename P1, typename P2, typename Q1, typename Q2>
void NotationActionController::registerAction(const ActionCode& code, void (INotationInteraction::* handler)(P1, P2),
                                              Q1 param1, Q2 param2, PlayMode playMode, bool (NotationActionController::* enabler)() const)
{
    registerAction(code, [this, handler, param1, param2, playMode]()
    {
        INotationPtr notation = currentNotation();
        if (notation) {
            (notation->interaction().get()->*handler)(param1, param2);

            seekSelectedElement();

            if (playMode != PlayMode::NoPlay) {
                playSelectedElement(playMode == PlayMode::PlayChord);
            }
        }
    }, enabler);
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

void NotationActionController::registerCommand(const muse::rcommand::Command& command,
                                               void (NotationActionController::* handler)(const muse::rcommand::CommandQuery&))
{
    commandDispatcher()->onRequest(this, command, [this, command, handler](const rcommand::Request& request) {
        if (!commandsState()->commandState(command).enabled) {
            return rcommand::make_response(request, muse::make_ret(Ret::Code::NotSupported));
        }

        (this->*handler)(request.query);

        return rcommand::make_response(request, muse::make_ok());
    });
}

void NotationActionController::registerCommand(const muse::rcommand::Command& command,
                                               void (INotationInteraction::* handler)(), PlayMode playMode)
{
    registerCommand(command, [this, handler, playMode]()
    {
        INotationPtr notation = currentNotation();
        if (notation) {
            (notation->interaction().get()->*handler)();

            seekSelectedElement();

            if (playMode != PlayMode::NoPlay) {
                playSelectedElement(playMode == PlayMode::PlayChord);
            }
        }
    });
}

void NotationActionController::registerAliases(const std::map<muse::rcommand::Command, muse::rcommand::CommandQuery>& aliases,
                                               void (NotationActionController::*handler)(const muse::rcommand::CommandQuery&))
{
    for (auto it = aliases.cbegin(); it != aliases.cend(); ++it) {
        auto alias = it->first;
        auto query = it->second;

        commandDispatcher()->onRequest(this, alias, [this, query, handler]() {
            if (!commandsState()->commandState(query.uri()).enabled) {
                return muse::make_ret(Ret::Code::NotSupported);
            }

            (this->*handler)(query);
            return muse::make_ok();
        });
    }
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
        handleNoteAction(noteName, addingMode);
    });
}
