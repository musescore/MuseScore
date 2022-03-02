/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-CLA-applies
 *
 * MuseScore
 * Music Composition & Notation
 *
 * Copyright (C) 2021 MuseScore BVBA and others
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

#include "notationtypes.h"

#include "log.h"

using namespace mu::notation;
using namespace mu::actions;
using namespace mu::context;
using namespace mu::framework;

static constexpr qreal STRETCH_STEP = 0.1;
static constexpr bool NEAR_NOTE_OR_REST = true;

static const ActionCode UNDO_ACTION_CODE = "undo";
static const ActionCode REDO_ACTION_CODE = "redo";

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

void NotationActionController::init()
{
    TRACEFUNC;

    //! NOTE Just for more readable
    using Controller = NotationActionController;
    using Interaction = INotationInteraction;

    //! NOTE For historical reasons, the name of the action does not match what needs to be done
    registerAction("notation-escape", &Controller::resetState, &Controller::isNotationPage);

    registerAction("note-input", [this]() { toggleNoteInput(); });
    registerNoteInputAction("note-input-steptime", NoteInputMethod::STEPTIME);
    registerNoteInputAction("note-input-rhythm", NoteInputMethod::RHYTHM);
    registerNoteInputAction("note-input-repitch", NoteInputMethod::REPITCH);
    registerNoteInputAction("note-input-realtime-auto", NoteInputMethod::REALTIME_AUTO);
    registerNoteInputAction("note-input-realtime-manual", NoteInputMethod::REALTIME_MANUAL);
    registerNoteInputAction("note-input-timewise", NoteInputMethod::TIMEWISE);

    registerPadNoteAction("note-longa", Pad::NOTE00);
    registerPadNoteAction("note-breve", Pad::NOTE0);
    registerPadNoteAction("pad-note-1", Pad::NOTE1);
    registerPadNoteAction("pad-note-2", Pad::NOTE2);
    registerPadNoteAction("pad-note-4", Pad::NOTE4);
    registerPadNoteAction("pad-note-8", Pad::NOTE8);
    registerPadNoteAction("pad-note-16", Pad::NOTE16);
    registerPadNoteAction("pad-note-32", Pad::NOTE32);
    registerPadNoteAction("pad-note-64", Pad::NOTE64);
    registerPadNoteAction("pad-note-128", Pad::NOTE128);
    registerPadNoteAction("pad-note-256", Pad::NOTE256);
    registerPadNoteAction("pad-note-512", Pad::NOTE512);
    registerPadNoteAction("pad-note-1024", Pad::NOTE1024);
    registerPadNoteAction("pad-dot", Pad::DOT);
    registerPadNoteAction("pad-dot2", Pad::DOT2);
    registerPadNoteAction("pad-dot3", Pad::DOT3);
    registerPadNoteAction("pad-dot4", Pad::DOT4);
    registerPadNoteAction("pad-rest", Pad::REST);

    registerNoteAction("note-c", NoteName::C);
    registerNoteAction("note-d", NoteName::D);
    registerNoteAction("note-e", NoteName::E);
    registerNoteAction("note-f", NoteName::F);
    registerNoteAction("note-g", NoteName::G);
    registerNoteAction("note-a", NoteName::A);
    registerNoteAction("note-b", NoteName::B);

    registerNoteAction("chord-c", NoteName::C, NoteAddingMode::CurrentChord);
    registerNoteAction("chord-d", NoteName::D, NoteAddingMode::CurrentChord);
    registerNoteAction("chord-e", NoteName::E, NoteAddingMode::CurrentChord);
    registerNoteAction("chord-f", NoteName::F, NoteAddingMode::CurrentChord);
    registerNoteAction("chord-g", NoteName::G, NoteAddingMode::CurrentChord);
    registerNoteAction("chord-a", NoteName::A, NoteAddingMode::CurrentChord);
    registerNoteAction("chord-b", NoteName::B, NoteAddingMode::CurrentChord);

    registerNoteAction("insert-c", NoteName::C, NoteAddingMode::InsertChord);
    registerNoteAction("insert-d", NoteName::D, NoteAddingMode::InsertChord);
    registerNoteAction("insert-e", NoteName::E, NoteAddingMode::InsertChord);
    registerNoteAction("insert-f", NoteName::F, NoteAddingMode::InsertChord);
    registerNoteAction("insert-g", NoteName::G, NoteAddingMode::InsertChord);
    registerNoteAction("insert-a", NoteName::A, NoteAddingMode::InsertChord);
    registerNoteAction("insert-b", NoteName::B, NoteAddingMode::InsertChord);

    registerAction("next-text-element", &Controller::nextTextElement, &Controller::textNavigationAvailable);
    registerAction("prev-text-element", &Controller::prevTextElement, &Controller::textNavigationAvailable);
    registerAction("next-beat-TEXT", &Controller::nextBeatTextElement, &Controller::textNavigationByBeatsAvailable);
    registerAction("prev-beat-TEXT", &Controller::prevBeatTextElement, &Controller::textNavigationByBeatsAvailable);

    for (auto it = DURATIONS_FOR_TEXT_NAVIGATION.cbegin(); it != DURATIONS_FOR_TEXT_NAVIGATION.cend(); ++it) {
        registerAction(it.key(), [=]() { navigateToTextElementByFraction(it.value()); }, &Controller::textNavigationByFractionAvailable);
    }

    registerAction("next-lyric-verse", &Interaction::navigateToLyricsVerse, MoveDirection::Down, PlayMode::NoPlay,
                   &Controller::isEditingLyrics);
    registerAction("prev-lyric-verse", &Interaction::navigateToLyricsVerse, MoveDirection::Up, PlayMode::NoPlay,
                   &Controller::isEditingLyrics);
    registerAction("next-syllable", &Interaction::nagivateToNextSyllable, PlayMode::NoPlay, &Controller::isEditingLyrics);

    registerAction("add-melisma", &Interaction::addMelisma, PlayMode::NoPlay, &Controller::isEditingLyrics);
    registerAction("add-lyric-verse", &Interaction::addLyricsVerse, PlayMode::NoPlay, &Controller::isEditingLyrics);

    registerAction("flat2", [this]() { toggleAccidental(AccidentalType::FLAT2); });
    registerAction("flat", [this]() { toggleAccidental(AccidentalType::FLAT); });
    registerAction("nat", [this]() { toggleAccidental(AccidentalType::NATURAL); });
    registerAction("sharp", [this]() { toggleAccidental(AccidentalType::SHARP); });
    registerAction("sharp2", [this]() { toggleAccidental(AccidentalType::SHARP2); });

    registerAction("rest", &Interaction::putRestToSelection);
    registerAction("rest-1", &Interaction::putRest, DurationType::V_WHOLE);
    registerAction("rest-2", &Interaction::putRest, DurationType::V_HALF);
    registerAction("rest-4", &Interaction::putRest, DurationType::V_QUARTER);
    registerAction("rest-8", &Interaction::putRest, DurationType::V_EIGHTH);

    registerAction("add-marcato", [this]() { addArticulation(SymbolId::articMarcatoAbove); });
    registerAction("add-sforzato", [this]() { addArticulation(SymbolId::articAccentAbove); });
    registerAction("add-tenuto", [this]() { addArticulation(SymbolId::articTenutoAbove); });
    registerAction("add-staccato", [this]() { addArticulation(SymbolId::articStaccatoAbove); });

    registerAction("duplet", [this]() { putTuplet(2); });
    registerAction("triplet", [this]() { putTuplet(3); });
    registerAction("quadruplet", [this]() { putTuplet(4); });
    registerAction("quintuplet", [this]() { putTuplet(5); });
    registerAction("sextuplet", [this]() { putTuplet(6); });
    registerAction("septuplet", [this]() { putTuplet(7); });
    registerAction("octuplet", [this]() { putTuplet(8); });
    registerAction("nonuplet", [this]() { putTuplet(9); });
    registerAction("custom-tuplet", &Controller::putTuplet);
    registerAction("tuplet-dialog", &Controller::openTupletOtherDialog);

    registerAction("put-note", &Controller::putNote);
    registerAction("remove-note", &Controller::removeNote);

    registerAction("toggle-visible", &Interaction::toggleVisible);

    registerMoveSelectionAction("next-element", MoveSelectionType::EngravingItem, MoveDirection::Right, PlayMode::PlayNote);
    registerMoveSelectionAction("prev-element", MoveSelectionType::EngravingItem, MoveDirection::Left, PlayMode::PlayNote);
    registerMoveSelectionAction("next-track", MoveSelectionType::Track, MoveDirection::Right, PlayMode::PlayChord);
    registerMoveSelectionAction("prev-track", MoveSelectionType::Track, MoveDirection::Left, PlayMode::PlayChord);
    registerMoveSelectionAction("next-frame", MoveSelectionType::Frame, MoveDirection::Right);
    registerMoveSelectionAction("prev-frame", MoveSelectionType::Frame, MoveDirection::Left);
    registerMoveSelectionAction("next-system", MoveSelectionType::System, MoveDirection::Right);
    registerMoveSelectionAction("prev-system", MoveSelectionType::System, MoveDirection::Left);

    registerAction("notation-move-right", &Controller::move, MoveDirection::Right, false);
    registerAction("notation-move-left", &Controller::move, MoveDirection::Left, false);
    registerAction("notation-move-right-quickly", &Controller::move, MoveDirection::Right, true, &Controller::measureNavigationAvailable);
    registerAction("notation-move-left-quickly", &Controller::move, MoveDirection::Left, true, &Controller::measureNavigationAvailable);
    registerAction("pitch-up", &Controller::move, MoveDirection::Up, false);
    registerAction("pitch-down", &Controller::move, MoveDirection::Down, false);
    registerAction("pitch-up-octave", &Controller::move, MoveDirection::Up, true);
    registerAction("pitch-down-octave", &Controller::move, MoveDirection::Down, true);
    registerAction("up-chord", [this]() { moveWithinChord(MoveDirection::Up); }, &Controller::hasSelection);
    registerAction("down-chord", [this]() { moveWithinChord(MoveDirection::Down); }, &Controller::hasSelection);

    registerAction("double-duration", &Controller::doubleNoteInputDuration);
    registerAction("half-duration", &Controller::halveNoteInputDuration);
    registerAction("inc-duration-dotted", &Interaction::increaseDecreaseDuration, -1, true);
    registerAction("dec-duration-dotted", &Interaction::increaseDecreaseDuration, 1, true);

    registerAction("notation-cut", &Controller::cutSelection, &Controller::hasSelection);
    registerAction("notation-copy", &Interaction::copySelection, &Controller::hasSelection);
    registerAction("notation-paste", [this]() { pasteSelection(PastingType::Default); }, &Controller::isNotationPage);
    registerAction("notation-paste-half", [this]() { pasteSelection(PastingType::Half); });
    registerAction("notation-paste-double", [this]() { pasteSelection(PastingType::Double); });
    registerAction("notation-paste-special", [this]() { pasteSelection(PastingType::Special); });
    registerAction("notation-swap", &Interaction::swapSelection, &Controller::hasSelection);
    registerAction("notation-delete", &Interaction::deleteSelection, &Controller::hasSelection);

    registerAction("flip", &Interaction::flipSelection, &Controller::hasSelection);
    registerAction("tie", &Controller::addTie);
    registerAction("chord-tie", &Controller::chordTie);
    registerAction("add-slur", &Controller::addSlur);

    registerAction("undo", &Interaction::undo, &Controller::canUndo);
    registerAction("redo", &Interaction::redo, &Controller::canRedo);

    registerAction("select-next-chord", &Interaction::addToSelection, MoveDirection::Right, MoveSelectionType::Chord, PlayMode::NoPlay,
                   &Controller::isNotNoteInputMode);
    registerAction("select-prev-chord", &Interaction::addToSelection, MoveDirection::Left, MoveSelectionType::Chord, PlayMode::NoPlay,
                   &Controller::isNotNoteInputMode);
    registerAction("select-similar", &Controller::selectAllSimilarElements);
    registerAction("select-similar-staff", &Controller::selectAllSimilarElementsInStaff);
    registerAction("select-similar-range", &Controller::selectAllSimilarElementsInRange);
    registerAction("select-dialog", &Controller::openSelectionMoreOptions);
    registerAction("notation-select-all", &Interaction::selectAll);
    registerAction("notation-select-section", &Interaction::selectSection);
    registerAction("first-element", &Interaction::selectFirstElement, false, PlayMode::PlayChord);
    registerAction("last-element", &Interaction::selectLastElement, PlayMode::PlayChord);
    registerAction("top-chord", [this]() { selectTopOrBottomOfChord(MoveDirection::Up); }, &Controller::hasSelection);
    registerAction("bottom-chord", [this]() { selectTopOrBottomOfChord(MoveDirection::Down); }, &Controller::hasSelection);
    registerAction("move-up", &Interaction::moveChordRestToStaff, MoveDirection::Up, &Controller::hasSelection);
    registerAction("move-down", &Interaction::moveChordRestToStaff, MoveDirection::Down, &Controller::hasSelection);
    registerAction("move-left", &Interaction::swapChordRest, MoveDirection::Left, &Controller::isNoteInputMode);
    registerAction("move-right", &Interaction::swapChordRest, MoveDirection::Right, &Controller::isNoteInputMode);
    registerAction("next-segment-element", &Interaction::moveSegmentSelection, MoveDirection::Right, PlayMode::PlayNote);
    registerAction("prev-segment-element", &Interaction::moveSegmentSelection, MoveDirection::Left, PlayMode::PlayNote);

    registerAction("system-break", &Interaction::toggleLayoutBreak, LayoutBreakType::LINE, PlayMode::NoPlay,
                   &Controller::toggleLayoutBreakAvailable);
    registerAction("page-break", &Interaction::toggleLayoutBreak, LayoutBreakType::PAGE, PlayMode::NoPlay,
                   &Controller::toggleLayoutBreakAvailable);
    registerAction("section-break", &Interaction::toggleLayoutBreak, LayoutBreakType::SECTION, PlayMode::NoPlay,
                   &Controller::toggleLayoutBreakAvailable);

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
    registerAction("append-hbox", [this]() { addBoxes(BoxType::Horizontal, 1, AddBoxesTarget::AtEndOfScore); });
    registerAction("append-vbox", [this]() { addBoxes(BoxType::Vertical, 1, AddBoxesTarget::AtEndOfScore); });
    registerAction("append-textframe", [this]() { addBoxes(BoxType::Text, 1, AddBoxesTarget::AtEndOfScore); });

    registerAction("edit-style", &Controller::openEditStyleDialog);
    registerAction("page-settings", &Controller::openPageSettingsDialog);
    registerAction("staff-properties", &Controller::openStaffProperties);
    registerAction("add-remove-breaks", &Controller::openBreaksDialog);
    registerAction("edit-info", &Controller::openScoreProperties);
    registerAction("transpose", &Controller::openTransposeDialog);
    registerAction("parts", &Controller::openPartsDialog);
    registerAction("staff-text-properties", &Controller::openStaffTextPropertiesDialog);
    registerAction("system-text-properties", &Controller::openStaffTextPropertiesDialog);
    registerAction("measure-properties", &Controller::openMeasurePropertiesDialog);
    registerAction("config-raster", &Controller::openEditGridSizeDialog);
    registerAction("realize-chord-symbols", &Controller::openRealizeChordSymbolsDialog);

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
    registerAction("add-hairpin", &Interaction::addHairpinsToSelection, HairpinType::CRESC_HAIRPIN);
    registerAction("add-hairpin-reverse", &Interaction::addHairpinsToSelection, HairpinType::DECRESC_HAIRPIN);
    registerAction("add-noteline", &Interaction::addAnchoredLineToSelectedNotes);

    registerAction("title-text", [this]() { addText(TextStyleType::TITLE); });
    registerAction("subtitle-text", [this]() { addText(TextStyleType::SUBTITLE); });
    registerAction("composer-text", [this]() { addText(TextStyleType::COMPOSER); });
    registerAction("poet-text", [this]() { addText(TextStyleType::POET); });
    registerAction("part-text", [this]() { addText(TextStyleType::INSTRUMENT_EXCERPT); });

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

    registerAction("show-invisible", [this]() { toggleScoreConfig(ScoreConfigType::ShowInvisibleElements); });
    registerAction("show-unprintable", [this]() { toggleScoreConfig(ScoreConfigType::ShowUnprintableElements); });
    registerAction("show-frames", [this]() { toggleScoreConfig(ScoreConfigType::ShowFrames); });
    registerAction("show-pageborders", [this]() { toggleScoreConfig(ScoreConfigType::ShowPageMargins); });
    registerAction("show-irregular", [this]() { toggleScoreConfig(ScoreConfigType::MarkIrregularMeasures); });

    registerAction("concert-pitch", &Controller::toggleConcertPitch);

    registerAction("explode", &Interaction::explodeSelectedStaff);
    registerAction("implode", &Interaction::implodeSelectedStaff);
    registerAction("time-delete", &Interaction::removeSelectedRange);
    registerAction("del-empty-measures", &Interaction::removeEmptyTrailingMeasures);
    registerAction("slash-fill", &Interaction::fillSelectionWithSlashes);
    registerAction("slash-rhythm", &Interaction::replaceSelectedNotesWithSlashes);
    registerAction("pitch-spell", &Interaction::spellPitches);
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

    registerAction("beam-start", &Interaction::addBeamToSelectedChordRests, BeamMode::BEGIN);
    registerAction("beam-mid", &Interaction::addBeamToSelectedChordRests, BeamMode::MID);
    registerAction("no-beam", &Interaction::addBeamToSelectedChordRests, BeamMode::NONE);
    registerAction("beam-32", &Interaction::addBeamToSelectedChordRests, BeamMode::BEGIN32);
    registerAction("beam-64", &Interaction::addBeamToSelectedChordRests, BeamMode::BEGIN64);
    registerAction("auto-beam", &Interaction::addBeamToSelectedChordRests, BeamMode::AUTO);

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

    registerAction("select-next-measure", &Interaction::addToSelection, MoveDirection::Right, MoveSelectionType::Measure, PlayMode::NoPlay,
                   &Controller::isNotNoteInputMode);
    registerAction("select-prev-measure", &Interaction::addToSelection, MoveDirection::Left, MoveSelectionType::Measure, PlayMode::NoPlay,
                   &Controller::isNotNoteInputMode);
    registerAction("select-begin-line", &Interaction::expandSelection, ExpandSelectionMode::BeginSystem, PlayMode::NoPlay,
                   &Controller::isNotNoteInputMode);
    registerAction("select-end-line", &Interaction::expandSelection, ExpandSelectionMode::EndSystem, PlayMode::NoPlay,
                   &Controller::isNotNoteInputMode);
    registerAction("select-begin-score", &Interaction::expandSelection, ExpandSelectionMode::BeginScore, PlayMode::NoPlay,
                   &Controller::isNotNoteInputMode);
    registerAction("select-end-score", &Interaction::expandSelection, ExpandSelectionMode::EndScore, PlayMode::NoPlay,
                   &Controller::isNotNoteInputMode);
    registerAction("select-staff-above", &Interaction::addToSelection, MoveDirection::Up, MoveSelectionType::Track, PlayMode::NoPlay,
                   &Controller::isNotNoteInputMode);
    registerAction("select-staff-below", &Interaction::addToSelection, MoveDirection::Down, MoveSelectionType::Track, PlayMode::NoPlay,
                   &Controller::isNotNoteInputMode);
    registerAction("top-staff", &Interaction::selectTopStaff, PlayMode::PlayChord);
    registerAction("empty-trailing-measure", &Interaction::selectEmptyTrailingMeasure);
    registerAction("pitch-up-diatonic", &Interaction::movePitch, MoveDirection::Up, PitchMode::DIATONIC, PlayMode::PlayNote);
    registerAction("pitch-down-diatonic", &Interaction::movePitch, MoveDirection::Down, PitchMode::DIATONIC, PlayMode::PlayNote);

    registerAction("repeat-sel", &Interaction::repeatSelection);

    registerAction("add-trill", &Interaction::toggleArticulation, Ms::SymId::ornamentTrill);
    registerAction("add-up-bow", &Interaction::toggleArticulation, Ms::SymId::stringsUpBow);
    registerAction("add-down-bow", &Interaction::toggleArticulation, Ms::SymId::stringsDownBow);
    registerAction("transpose-up", &Interaction::transposeSemitone, 1, PlayMode::PlayNote);
    registerAction("transpose-down", &Interaction::transposeSemitone, -1, PlayMode::PlayNote);
    registerAction("toggle-insert-mode", &Interaction::toggleGlobalOrLocalInsert);

    registerAction("get-location", &Interaction::getLocation, &Controller::isNotationPage);
    registerAction("toggle-mmrest", &Interaction::execute, &Ms::Score::cmdToggleMmrest);
    registerAction("toggle-hide-empty", &Interaction::execute, &Ms::Score::cmdToggleHideEmpty);

    registerAction("mirror-note", &Interaction::execute, &Ms::Score::cmdMirrorNoteHead);
    registerAction("clef-violin", &Interaction::insertClef, Ms::ClefType::G);
    registerAction("clef-bass", &Interaction::insertClef, Ms::ClefType::F);
    registerAction("sharp2-post", &Interaction::changeAccidental, Ms::AccidentalType::SHARP2, PlayMode::PlayNote);
    registerAction("sharp-post", &Interaction::changeAccidental, Ms::AccidentalType::SHARP, PlayMode::PlayNote);
    registerAction("nat-post", &Interaction::changeAccidental, Ms::AccidentalType::NATURAL, PlayMode::PlayNote);
    registerAction("flat-post", &Interaction::changeAccidental, Ms::AccidentalType::FLAT, PlayMode::PlayNote);
    registerAction("flat2-post", &Interaction::changeAccidental, Ms::AccidentalType::FLAT2, PlayMode::PlayNote);
    registerAction("pitch-up-diatonic-alterations", &Interaction::transposeDiatonicAlterations, Ms::TransposeDirection::UP,
                   PlayMode::PlayNote);
    registerAction("pitch-down-diatonic-alterations", &Interaction::transposeDiatonicAlterations, Ms::TransposeDirection::DOWN,
                   PlayMode::PlayNote);
    registerAction("full-measure-rest", &Interaction::execute, &Ms::Score::cmdFullMeasureRest);
    registerAction("set-visible", &Interaction::execute, &Ms::Score::cmdSetVisible);
    registerAction("unset-visible", &Interaction::execute, &Ms::Score::cmdUnsetVisible);
    registerAction("toggle-autoplace", &Interaction::toggleAutoplace, false);
    registerAction("autoplace-enabled", &Interaction::toggleAutoplace, true);

    for (int i = MIN_NOTES_INTERVAL; i <= MAX_NOTES_INTERVAL; ++i) {
        if (isNotesIntervalValid(i)) {
            registerAction("interval" + std::to_string(i), &Interaction::addIntervalToSelectedNotes, i);
        }
    }

    for (int i = 0; i < Ms::VOICES; ++i) {
        registerAction("voice-" + std::to_string(i + 1), [this, i]() { changeVoice(i); });
    }

    // TAB
    registerAction("string-above", &Controller::move, MoveDirection::Up, false, &Controller::isTablatureStaff);
    registerAction("string-below", &Controller::move, MoveDirection::Down, false, &Controller::isTablatureStaff);
    registerTabPadNoteAction("pad-note-1-TAB", Pad::NOTE1);
    registerTabPadNoteAction("pad-note-2-TAB", Pad::NOTE2);
    registerTabPadNoteAction("pad-note-4-TAB", Pad::NOTE4);
    registerTabPadNoteAction("pad-note-8-TAB", Pad::NOTE8);
    registerTabPadNoteAction("pad-note-16-TAB", Pad::NOTE16);
    registerTabPadNoteAction("pad-note-32-TAB", Pad::NOTE32);
    registerTabPadNoteAction("pad-note-64-TAB", Pad::NOTE64);
    registerTabPadNoteAction("pad-note-128-TAB", Pad::NOTE128);
    registerTabPadNoteAction("pad-note-256-TAB", Pad::NOTE256);
    registerTabPadNoteAction("pad-note-512-TAB", Pad::NOTE512);
    registerTabPadNoteAction("pad-note-1024-TAB", Pad::NOTE1024);

    for (int i = 0; i < MAX_FRET; ++i) {
        registerAction("fret-" + std::to_string(i), &Interaction::addFret, i, &Controller::isTablatureStaff);
    }

    // listen on state changes
    globalContext()->currentNotationChanged().onNotify(this, [this]() {
        auto notation = globalContext()->currentNotation();
        if (notation) {
            notation->interaction()->noteInput()->stateChanged().onNotify(this, [this]() {
                m_currentNotationNoteInputChanged.notify();
            });
        }
        m_currentNotationNoteInputChanged.notify();
    });
}

bool NotationActionController::canReceiveAction(const actions::ActionCode& code) const
{
    TRACEFUNC;

    //! NOTE If the notation is not loaded, we cannot process anything.
    if (!currentNotation()) {
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

mu::async::Notification NotationActionController::currentNotationChanged() const
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

mu::async::Notification NotationActionController::currentNotationNoteInputChanged() const
{
    return m_currentNotationNoteInputChanged;
}

INotationUndoStackPtr NotationActionController::currentNotationUndoStack() const
{
    auto notation = currentNotation();
    if (!notation) {
        return nullptr;
    }

    return notation->undoStack();
}

INotationStylePtr NotationActionController::currentNotationStyle() const
{
    auto notation = currentNotation();
    if (!notation) {
        return nullptr;
    }

    return notation->style();
}

mu::async::Notification NotationActionController::currentNotationStyleChanged() const
{
    return currentNotationStyle() ? currentNotationStyle()->styleChanged() : async::Notification();
}

void NotationActionController::resetState()
{
    TRACEFUNC;

    if (playbackController()->isPlaying()) {
        playbackController()->reset();
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

    if (interaction->isElementEditStarted()) {
        interaction->endEditElement();
        return;
    }

    if (!interaction->selection()->isNone()) {
        interaction->clearSelection();
    }
}

void NotationActionController::toggleNoteInput()
{
    TRACEFUNC;
    auto noteInput = currentNotationNoteInput();
    if (!noteInput) {
        return;
    }

    if (noteInput->isNoteInputMode()) {
        noteInput->endNoteInput();
    } else {
        noteInput->startNoteInput();
    }
}

void NotationActionController::toggleNoteInputMethod(NoteInputMethod method)
{
    TRACEFUNC;
    auto noteInput = currentNotationNoteInput();
    if (!noteInput) {
        return;
    }

    if (!noteInput->isNoteInputMode()) {
        noteInput->startNoteInput();
    } else if (noteInput->state().method == method) {
        noteInput->endNoteInput();
        return;
    }

    noteInput->toggleNoteInputMethod(method);
}

void NotationActionController::addNote(NoteName note, NoteAddingMode addingMode)
{
    TRACEFUNC;
    auto noteInput = currentNotationNoteInput();
    if (!noteInput) {
        return;
    }

    if (!noteInput->isNoteInputMode()) {
        noteInput->startNoteInput();
    }

    noteInput->addNote(note, addingMode);

    playSelectedElement();
}

void NotationActionController::padNote(const Pad& pad)
{
    TRACEFUNC;
    auto noteInput = currentNotationNoteInput();
    if (!noteInput) {
        return;
    }

    startNoteInputIfNeed();

    noteInput->padNote(pad);
}

void NotationActionController::putNote(const actions::ActionData& args)
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

    noteInput->putNote(pos, replace, insert);

    playSelectedElement();
}

void NotationActionController::removeNote(const actions::ActionData& args)
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
}

void NotationActionController::toggleAccidental(AccidentalType type)
{
    TRACEFUNC;
    auto interaction = currentNotationInteraction();
    if (!interaction) {
        return;
    }

    auto noteInput = currentNotationNoteInput();
    if (!noteInput) {
        return;
    }

    startNoteInputIfNeed();

    if (noteInput->isNoteInputMode()) {
        noteInput->setAccidental(type);
    } else {
        interaction->addAccidentalToSelection(type);
    }
}

void NotationActionController::addArticulation(SymbolId articulationSymbolId)
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

    startNoteInputIfNeed();

    if (noteInput->isNoteInputMode()) {
        noteInput->setArticulation(articulationSymbolId);
    } else {
        interaction->changeSelectedNotesArticulation(articulationSymbolId);
    }
}

void NotationActionController::putTuplet(const ActionData& data)
{
    IF_ASSERT_FAILED(data.count() == 1) {
        return;
    }

    TupletOptions options = data.arg<TupletOptions>(0);

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
        interactive()->error(trc("notation", "Cannot create tuplet"), trc("notation", "Note value is too short"),
                             { IInteractive::Button::Ok });
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

bool NotationActionController::moveSelectionAvailable(MoveSelectionType type) const
{
    auto interaction = currentNotationInteraction();
    return interaction && interaction->moveSelectionAvailable(type);
}

void NotationActionController::moveSelection(MoveSelectionType type, MoveDirection direction)
{
    auto interaction = currentNotationInteraction();
    if (!interaction) {
        return;
    }

    interaction->moveSelection(direction, type);
}

void NotationActionController::move(MoveDirection direction, bool quickly)
{
    TRACEFUNC;
    auto interaction = currentNotationInteraction();
    if (!interaction) {
        return;
    }

    EngravingItem* selectedElement = interaction->selection()->element();

    switch (direction) {
    case MoveDirection::Up:
    case MoveDirection::Down:
        if (!quickly && selectedElement && selectedElement->isLyrics()) {
            interaction->moveLyrics(direction);
        } else if (selectedElement && (selectedElement->isTextBase() || selectedElement->isArticulation())) {
            interaction->nudge(direction, quickly);
        } else if (interaction->noteInput()->isNoteInputMode() && interaction->noteInput()->state().staffGroup == Ms::StaffGroup::TAB) {
            interaction->moveSelection(direction, MoveSelectionType::String);
        } else {
            interaction->movePitch(direction, quickly ? PitchMode::OCTAVE : PitchMode::CHROMATIC);
        }
        break;
    case MoveDirection::Right:
    case MoveDirection::Left:
        if (interaction->isTextEditingStarted() && textNavigationAvailable()) {
            navigateToTextElementInNearMeasure(direction);
            break;
        }

        if (selectedElement && selectedElement->isTextBase()) {
            interaction->nudge(direction, quickly);
        } else {
            interaction->moveSelection(direction, quickly ? MoveSelectionType::Measure : MoveSelectionType::Chord);
        }
        break;
    case MoveDirection::Undefined:
        break;
    }
    playSelectedElement(false);
}

void NotationActionController::moveWithinChord(MoveDirection direction)
{
    TRACEFUNC;
    auto interaction = currentNotationInteraction();
    if (!interaction) {
        return;
    }

    interaction->moveChordNoteSelection(direction);

    playSelectedElement(false);
}

void NotationActionController::selectTopOrBottomOfChord(MoveDirection direction)
{
    TRACEFUNC;
    auto interaction = currentNotationInteraction();
    if (!interaction) {
        return;
    }

    interaction->selectTopOrBottomOfChord(direction);

    playSelectedElement(false);
}

void NotationActionController::changeVoice(int voiceIndex)
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

    startNoteInputIfNeed();

    noteInput->setCurrentVoiceIndex(voiceIndex);

    if (!noteInput->isNoteInputMode()) {
        interaction->changeSelectedNotesVoice(voiceIndex);
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

void NotationActionController::pasteSelection(PastingType type)
{
    TRACEFUNC;
    auto interaction = currentNotationInteraction();
    if (!interaction) {
        return;
    }

    Fraction scale = resolvePastingScale(interaction, type);
    interaction->pasteSelection(scale);
    playSelectedElement(false);
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
        Fraction duration = interaction->noteInput()->state().duration.fraction();

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
    } else {
        interaction->addTiedNoteToChord();
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

    if (noteInput->isNoteInputMode() && noteInput->state().withSlur) {
        noteInput->resetSlur();
    } else {
        interaction->addSlurToSelection();
    }
}

IInteractive::Result NotationActionController::showErrorMessage(const std::string& message) const
{
    return interactive()->info(message, "", {}, 0, IInteractive::Option::WithIcon | IInteractive::Option::WithShowAgain);
}

void NotationActionController::addText(TextStyleType type)
{
    TRACEFUNC;

    auto interaction = currentNotationInteraction();
    if (!interaction) {
        return;
    }

    Ret ret = interaction->canAddText(type);
    if (!ret) {
        if (configuration()->needToShowAddTextErrorMessage()) {
            IInteractive::Result result = showErrorMessage(ret.text());
            if (!result.showAgain()) {
                configuration()->setNeedToShowAddTextErrorMessage(false);
            }
        }

        return;
    }

    interaction->addText(type);
}

void NotationActionController::addFiguredBass()
{
    TRACEFUNC;

    auto interaction = currentNotationInteraction();
    if (!interaction) {
        return;
    }

    Ret ret = interaction->canAddFiguredBass();
    if (!ret) {
        if (configuration()->needToShowAddFiguredBassErrorMessage()) {
            IInteractive::Result result = showErrorMessage(ret.text());
            if (!result.showAgain()) {
                configuration()->setNeedToShowAddFiguredBassErrorMessage(false);
            }
        }

        return;
    }

    interaction->addFiguredBass();
}

void NotationActionController::selectAllSimilarElements()
{
    TRACEFUNC;
    auto notationElements = currentNotationElements();
    auto interaction = currentNotationInteraction();
    if (!notationElements || !interaction) {
        return;
    }

    EngravingItem* selectedElement = interaction->selection()->element();
    if (!selectedElement) {
        return;
    }

    FilterElementsOptions options = elementsFilterOptions(selectedElement);
    std::vector<EngravingItem*> elements = notationElements->elements(options);
    if (elements.empty()) {
        return;
    }

    interaction->clearSelection();

    interaction->select(elements, SelectType::ADD);
}

void NotationActionController::selectAllSimilarElementsInStaff()
{
    TRACEFUNC;
    auto notationElements = currentNotationElements();
    auto interaction = currentNotationInteraction();
    if (!notationElements || !interaction) {
        return;
    }

    EngravingItem* selectedElement = interaction->selection()->element();
    if (!selectedElement) {
        return;
    }

    FilterElementsOptions options = elementsFilterOptions(selectedElement);
    options.staffStart = selectedElement->staffIdx();
    options.staffEnd = options.staffStart + 1;

    std::vector<EngravingItem*> elements = notationElements->elements(options);
    if (elements.empty()) {
        return;
    }

    interaction->clearSelection();

    interaction->select(elements, SelectType::ADD);
}

void NotationActionController::selectAllSimilarElementsInRange()
{
    auto elements = currentNotationElements();
    Ms::EngravingItem* lastHit = currentNotationSelection()->lastElementHit();
    if (!elements || !lastHit) {
        return;
    }

    Ms::Score* score = elements->msScore();
    score->selectSimilarInRange(lastHit);
    if (score->selectionChanged()) {
        currentNotationInteraction()->selectionChanged().notify();
    }
}

void NotationActionController::openSelectionMoreOptions()
{
    auto interaction = currentNotationInteraction();
    if (!interaction) {
        return;
    }

    auto selection = interaction->selection();
    if (!selection) {
        return;
    }

    bool noteSelected = selection->element() && selection->element()->isNote();

    if (noteSelected) {
        interactive()->open("musescore://notation/selectnote");
    } else {
        interactive()->open("musescore://notation/selectelement");
    }
}

void NotationActionController::startEditSelectedElement()
{
    auto interaction = currentNotationInteraction();
    if (!interaction) {
        return;
    }

    auto selection = interaction->selection();
    if (!selection) {
        return;
    }

    Ms::EngravingItem* element = selection->element();
    if (!element) {
        return;
    }

    if (element->isInstrumentName()) {
        openStaffProperties();
        return;
    }

    if (interaction->textEditingAllowed(element)) {
        interaction->startEditText(element);
    } else {
        interaction->startEditElement(element);
    }
}

void NotationActionController::startEditSelectedText()
{
    auto interaction = currentNotationInteraction();
    if (!interaction) {
        return;
    }

    auto selection = interaction->selection();
    if (!selection) {
        return;
    }

    Ms::EngravingItem* element = selection->element();

    if (interaction->textEditingAllowed(element)) {
        interaction->startEditText(element);
    }
}

void NotationActionController::addMeasures(const actions::ActionData& actionData, AddBoxesTarget target)
{
    TRACEFUNC;
    int count = 1;

    if (actionData.empty()) {
        RetVal<Val> result = interactive()->open("musescore://notation/selectmeasurescount");

        if (result.ret) {
            count = result.val.toInt();
        } else {
            return;
        }
    } else {
        count = actionData.arg<int>();
    }

    addBoxes(BoxType::Measure, count, target);
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

void NotationActionController::openBreaksDialog()
{
    interactive()->open("musescore://notation/breaks");
}

void NotationActionController::openScoreProperties()
{
    interactive()->open("musescore://notation/properties");
}

void NotationActionController::openTransposeDialog()
{
    interactive()->open("musescore://notation/transpose");
}

void NotationActionController::openPartsDialog()
{
    interactive()->open("musescore://notation/parts");
}

mu::io::path NotationActionController::selectStyleFile(bool forLoad)
{
    mu::io::path dir = configuration()->userStylesPath();
    QString filter = qtrc("notation", "MuseScore Styles") + " (*.mss)";
    return forLoad
           ? interactive()->selectOpeningFile(qtrc("notation", "Load Style"), dir, filter)
           : interactive()->selectSavingFile(qtrc("notation", "Save Style"), dir, filter);
}

void NotationActionController::loadStyle()
{
    TRACEFUNC;
    auto path = selectStyleFile(true);
    if (!path.empty()) {
        QFile f(path.toQString());
        if (!f.open(QIODevice::ReadOnly) || !Ms::MStyle::isValid(&f)) {
            interactive()->error(trc("notation", "The style file could not be loaded."),
                                 f.errorString().toStdString(), { IInteractive::Button::Ok },
                                 IInteractive::Button::Ok, IInteractive::Option::WithIcon);
            return;
        }
        if (!currentNotationStyle()->loadStyle(path.toQString(), false) && interactive()->warning(
                trc("notation",
                    "Since this style file is from a different version of MuseScore, your score is not guaranteed to display correctly."),
                trc("notation", "Click OK to load anyway."), { IInteractive::Button::Ok, IInteractive::Button::Cancel },
                IInteractive::Button::Ok, IInteractive::Option::WithIcon).standardButton()
            == IInteractive::Button::Ok) {
            currentNotationStyle()->loadStyle(path.toQString(), true);
        }
    }
}

void NotationActionController::saveStyle()
{
    TRACEFUNC;
    auto path = selectStyleFile(false);
    if (!path.empty()) {
        if (!currentNotationStyle()->saveStyle(path)) {
            interactive()->error(trc("notation", "The style file could not be saved."),
                                 Ms::MScore::lastError.toStdString(), { IInteractive::Button::Ok },
                                 IInteractive::Button::Ok, IInteractive::Option::WithIcon);
        }
    }
}

FilterElementsOptions NotationActionController::elementsFilterOptions(const EngravingItem* element) const
{
    TRACEFUNC;
    FilterElementsOptions options;
    options.elementType = element->type();

    if (element->type() == ElementType::NOTE) {
        const Ms::Note* note = dynamic_cast<const Ms::Note*>(element);
        if (note->chord()->isGrace()) {
            options.subtype = -1;
        } else {
            options.subtype = element->subtype();
        }
    }

    return options;
}

bool NotationActionController::measureNavigationAvailable() const
{
    return isNotEditingElement() || textNavigationAvailable();
}

bool NotationActionController::toggleLayoutBreakAvailable() const
{
    INotationInteractionPtr interaction = currentNotationInteraction();
    return interaction && interaction->toggleLayoutBreakAvailable();
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

    static const QList<Ms::ElementType> allowedElementsForTextNavigation {
        Ms::ElementType::LYRICS,
        Ms::ElementType::HARMONY,
        Ms::ElementType::FIGURED_BASS,
        Ms::ElementType::STICKING,
        Ms::ElementType::FINGERING
    };

    static const QList<Ms::ElementType> allowedElementsForBeatsNavigation {
        Ms::ElementType::HARMONY
    };

    static const QList<Ms::ElementType> allowedElementsForFractionNavigation {
        Ms::ElementType::HARMONY,
        Ms::ElementType::FIGURED_BASS
    };

    const Ms::EngravingItem* element = selectedElement();
    Ms::ElementType elementType = element ? element->type() : Ms::ElementType::INVALID;

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

void NotationActionController::nextTextElement()
{
    navigateToTextElement(MoveDirection::Right, NEAR_NOTE_OR_REST);
}

void NotationActionController::prevTextElement()
{
    navigateToTextElement(MoveDirection::Left, NEAR_NOTE_OR_REST);
}

void NotationActionController::nextBeatTextElement()
{
    navigateToTextElement(MoveDirection::Right);
}

void NotationActionController::prevBeatTextElement()
{
    navigateToTextElement(MoveDirection::Left);
}

void NotationActionController::navigateToTextElement(MoveDirection direction, bool nearNoteOrRest)
{
    const Ms::EngravingItem* element = selectedElement();
    if (!element) {
        return;
    }

    if (element->isLyrics()) {
        currentNotationInteraction()->navigateToLyrics(direction);
    } else if (element->isHarmony()) {
        currentNotationInteraction()->navigateToNearHarmony(direction, nearNoteOrRest);
    } else if (element->isFiguredBass()) {
        currentNotationInteraction()->navigateToNearFiguredBass(direction);
    } else {
        currentNotationInteraction()->navigateToNearText(direction);
    }
}

void NotationActionController::navigateToTextElementByFraction(const Fraction& fraction)
{
    const Ms::EngravingItem* element = selectedElement();
    if (!element) {
        return;
    }

    if (element->isHarmony()) {
        currentNotationInteraction()->navigateToHarmony(fraction);
    } else if (element->isFiguredBass()) {
        currentNotationInteraction()->navigateToFiguredBass(fraction);
    }
}

void NotationActionController::navigateToTextElementInNearMeasure(MoveDirection direction)
{
    const Ms::EngravingItem* element = selectedElement();
    if (!element) {
        return;
    }

    if (element->isHarmony()) {
        currentNotationInteraction()->navigateToHarmonyInNearMeasure(direction);
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

bool NotationActionController::isNoteInputMode() const
{
    auto interaction = currentNotationInteraction();
    if (!interaction) {
        return false;
    }

    auto noteInput = interaction->noteInput();
    if (!noteInput) {
        return false;
    }

    return noteInput->isNoteInputMode();
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
    interactive()->open("musescore://notation/measureproperties");
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

    currentNotationUndoStack()->prepareChanges();
    bool enabled = style->styleValue(StyleId::concertPitch).toBool();
    style->setStyleValue(StyleId::concertPitch, !enabled);
    currentNotationUndoStack()->commitChanges();
}

void NotationActionController::playSelectedElement(bool playChord)
{
    TRACEFUNC;
    auto interaction = currentNotationInteraction();
    if (!interaction) {
        return;
    }

    EngravingItem* element = interaction->selection()->element();
    if (!element || !element->isNote()) {
        return;
    }

    if (playChord && playbackConfiguration()->playChordWhenEditing()) {
        element = element->elementBase();
    }

    playbackController()->playElement(element);
}

void NotationActionController::startNoteInputIfNeed()
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

    if (interaction->selection()->isNone() && !noteInput->isNoteInputMode()) {
        noteInput->startNoteInput();
    }
}

bool NotationActionController::hasSelection() const
{
    return currentNotationSelection() ? !currentNotationSelection()->isNone() : false;
}

Ms::EngravingItem* NotationActionController::selectedElement() const
{
    auto selection = currentNotationSelection();
    return selection ? selection->element() : nullptr;
}

bool NotationActionController::canUndo() const
{
    return currentNotationUndoStack() ? currentNotationUndoStack()->canUndo() : false;
}

bool NotationActionController::canRedo() const
{
    return currentNotationUndoStack() ? currentNotationUndoStack()->canRedo() : false;
}

bool NotationActionController::isNotationPage() const
{
    return uiContextResolver()->matchWithCurrent(context::UiCtxNotationOpened);
}

bool NotationActionController::isStandardStaff() const
{
    return isNotEditingElement() && !isTablatureStaff();
}

bool NotationActionController::isTablatureStaff() const
{
    return isNotEditingElement() && currentNotationElements()->msScore()->inputState().staffGroup() == Ms::StaffGroup::TAB;
}

bool NotationActionController::isEditingElement() const
{
    auto interaction = currentNotationInteraction();
    if (interaction) {
        return interaction->isElementEditStarted() || interaction->isDragStarted();
    }
    return false;
}

bool NotationActionController::isNotEditingElement() const
{
    return !isEditingElement();
}

void NotationActionController::registerAction(const mu::actions::ActionCode& code,
                                              std::function<void()> handler, bool (NotationActionController::* isEnabled)() const)
{
    m_isEnabledMap[code] = std::bind(isEnabled, this);
    dispatcher()->reg(this, code, handler);
}

void NotationActionController::registerAction(const mu::actions::ActionCode& code,
                                              std::function<void(const actions::ActionData&)> handler,
                                              bool (NotationActionController::* isEnabled)() const)
{
    m_isEnabledMap[code] = std::bind(isEnabled, this);
    dispatcher()->reg(this, code, handler);
}

void NotationActionController::registerAction(const mu::actions::ActionCode& code,
                                              void (NotationActionController::* handler)(const actions::ActionData& data),
                                              bool (NotationActionController::* isEnabled)() const)
{
    m_isEnabledMap[code] = std::bind(isEnabled, this);
    dispatcher()->reg(this, code, this, handler);
}

void NotationActionController::registerAction(const mu::actions::ActionCode& code,
                                              void (NotationActionController::* handler)(),
                                              bool (NotationActionController::* isEnabled)() const)
{
    m_isEnabledMap[code] = std::bind(isEnabled, this);
    dispatcher()->reg(this, code, this, handler);
}

void NotationActionController::registerNoteInputAction(const mu::actions::ActionCode& code, NoteInputMethod inputMethod)
{
    registerAction(code, [this, inputMethod]() { toggleNoteInputMethod(inputMethod); }, &NotationActionController::isNotEditingElement);
}

void NotationActionController::registerNoteAction(const mu::actions::ActionCode& code, NoteName noteName, NoteAddingMode addingMode)
{
    registerAction(code, [this, noteName, addingMode]() { addNote(noteName, addingMode); }, &NotationActionController::isStandardStaff);
}

void NotationActionController::registerPadNoteAction(const mu::actions::ActionCode& code, Pad padding)
{
    registerAction(code, [this, padding]() { padNote(padding); });
}

void NotationActionController::registerTabPadNoteAction(const mu::actions::ActionCode& code, Pad padding)
{
    registerAction(code, [this, padding]() { padNote(padding); }, &NotationActionController::isTablatureStaff);
}

void NotationActionController::registerMoveSelectionAction(const mu::actions::ActionCode& code, MoveSelectionType type,
                                                           MoveDirection direction, PlayMode playMode)
{
    auto moveSelectionFunc = [this, type, direction, playMode]() {
        moveSelection(type, direction);

        if (playMode != PlayMode::NoPlay) {
            playSelectedElement(playMode == PlayMode::PlayChord);
        }
    };

    auto moveSelectionAvailableFunc = [this, type]() {
        return moveSelectionAvailable(type);
    };

    m_isEnabledMap[code] = moveSelectionAvailableFunc;
    dispatcher()->reg(this, code, moveSelectionFunc);
}

void NotationActionController::registerAction(const mu::actions::ActionCode& code,
                                              void (INotationInteraction::* handler)(), PlayMode playMode,
                                              bool (NotationActionController::* enabler)() const)
{
    registerAction(code, [this, handler, playMode]()
    {
        auto interaction = currentNotationInteraction().get();
        if (interaction) {
            (interaction->*handler)();
            if (playMode != PlayMode::NoPlay) {
                playSelectedElement(playMode == PlayMode::PlayChord);
            }
        }
    }, enabler);
}

void NotationActionController::registerAction(const mu::actions::ActionCode& code,
                                              void (NotationActionController::* handler)(MoveDirection,
                                                                                         bool), MoveDirection direction, bool quickly,
                                              bool (NotationActionController::* enabler)() const)
{
    registerAction(code, [this, handler, direction, quickly]() { (this->*handler)(direction, quickly); }, enabler);
}

void NotationActionController::registerAction(const mu::actions::ActionCode& code,
                                              void (INotationInteraction::* handler)(), bool (NotationActionController::* enabler)() const)
{
    registerAction(code, handler, PlayMode::NoPlay, enabler);
}

template<class P1>
void NotationActionController::registerAction(const mu::actions::ActionCode& code, void (INotationInteraction::* handler)(P1),
                                              P1 param1, PlayMode playMode, bool (NotationActionController::* enabler)() const)
{
    registerAction(code, [this, handler, param1, playMode]()
    {
        auto interaction = currentNotationInteraction().get();
        if (interaction) {
            (interaction->*handler)(param1);
            if (playMode != PlayMode::NoPlay) {
                playSelectedElement(playMode == PlayMode::PlayChord);
            }
        }
    }, enabler);
}

template<class P1>
void NotationActionController::registerAction(const mu::actions::ActionCode& code,
                                              void (INotationInteraction::* handler)(
                                                  P1), P1 param1, bool (NotationActionController::* enabler)() const)
{
    registerAction(code, handler, param1, PlayMode::NoPlay, enabler);
}

template<typename P1, typename P2>
void NotationActionController::registerAction(const mu::actions::ActionCode& code, void (INotationInteraction::* handler)(P1, P2),
                                              P1 param1, P2 param2, PlayMode playMode, bool (NotationActionController::* enabler)() const)
{
    registerAction(code, [this, handler, param1, param2, playMode]()
    {
        auto interaction = currentNotationInteraction().get();
        if (interaction) {
            (interaction->*handler)(param1, param2);
            if (playMode != PlayMode::NoPlay) {
                playSelectedElement(playMode == PlayMode::PlayChord);
            }
        }
    }, enabler);
}
