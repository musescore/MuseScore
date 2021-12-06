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

static constexpr int INVALID_BOX_INDEX = -1;
static constexpr qreal STRETCH_STEP = 0.1;
static const ActionCode ESCAPE_ACTION_CODE = "escape";
static const ActionCode UNDO_ACTION_CODE = "undo";
static const ActionCode REDO_ACTION_CODE = "redo";

void NotationActionController::init()
{
    TRACEFUNC;

    //! NOTE For historical reasons, the name of the action does not match what needs to be done
    registerAction(ESCAPE_ACTION_CODE, &NotationActionController::resetState, &NotationActionController::isNotationPage);

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

    registerAction("next-lyric", &INotationInteraction::nextLyrics, MoveDirection::Right, PlayMode::NoPlay,
                   &NotationActionController::isEditingLyrics);
    registerAction("prev-lyric", &INotationInteraction::nextLyrics, MoveDirection::Left, PlayMode::NoPlay,
                   &NotationActionController::isEditingLyrics);
    registerAction("next-lyric-verse", &INotationInteraction::nextLyricsVerse, MoveDirection::Down, PlayMode::NoPlay,
                   &NotationActionController::isEditingLyrics);
    registerAction("prev-lyric-verse", &INotationInteraction::nextLyricsVerse, MoveDirection::Up, PlayMode::NoPlay,
                   &NotationActionController::isEditingLyrics);
    registerAction("next-syllable", &INotationInteraction::nextSyllable, PlayMode::NoPlay, &NotationActionController::isEditingLyrics);
    registerAction("add-melisma", &INotationInteraction::addMelisma, PlayMode::NoPlay, &NotationActionController::isEditingLyrics);
    registerAction("add-lyric-verse", &INotationInteraction::addLyricsVerse, PlayMode::NoPlay, &NotationActionController::isEditingLyrics);

    registerAction("flat2", [this]() { toggleAccidental(AccidentalType::FLAT2); });
    registerAction("flat", [this]() { toggleAccidental(AccidentalType::FLAT); });
    registerAction("nat", [this]() { toggleAccidental(AccidentalType::NATURAL); });
    registerAction("sharp", [this]() { toggleAccidental(AccidentalType::SHARP); });
    registerAction("sharp2", [this]() { toggleAccidental(AccidentalType::SHARP2); });

    registerAction("rest", &INotationInteraction::putRestToSelection);
    registerAction("rest-1", &INotationInteraction::putRest, DurationType::V_WHOLE);
    registerAction("rest-2", &INotationInteraction::putRest, DurationType::V_HALF);
    registerAction("rest-4", &INotationInteraction::putRest, DurationType::V_QUARTER);
    registerAction("rest-8", &INotationInteraction::putRest, DurationType::V_EIGHTH);

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
    registerAction("custom-tuplet", &NotationActionController::putTuplet);
    registerAction("tuplet-dialog", &NotationActionController::openTupletOtherDialog);

    registerAction("put-note", &NotationActionController::putNote);

    registerAction("toggle-visible", &INotationInteraction::toggleVisible);

    registerAction("next-element", &INotationInteraction::moveSelection, MoveDirection::Right, MoveSelectionType::EngravingItem,
                   PlayMode::PlayNote);
    registerAction("prev-element", &INotationInteraction::moveSelection, MoveDirection::Left, MoveSelectionType::EngravingItem,
                   PlayMode::PlayNote);
    registerAction("next-chord", &NotationActionController::move, MoveDirection::Right, false);
    registerAction("prev-chord", &NotationActionController::move, MoveDirection::Left, false);
    registerAction("next-measure", &NotationActionController::move, MoveDirection::Right, true);
    registerAction("prev-measure", &NotationActionController::move, MoveDirection::Left, true);
    registerAction("next-track", &INotationInteraction::moveSelection, MoveDirection::Right, MoveSelectionType::Track, PlayMode::PlayChord);
    registerAction("prev-track", &INotationInteraction::moveSelection, MoveDirection::Left, MoveSelectionType::Track, PlayMode::PlayChord);
    registerAction("pitch-up", &NotationActionController::move, MoveDirection::Up, false);
    registerAction("pitch-down", &NotationActionController::move, MoveDirection::Down, false);
    registerAction("pitch-up-octave", &NotationActionController::move, MoveDirection::Up, true);
    registerAction("pitch-down-octave", &NotationActionController::move, MoveDirection::Down, true);
    registerAction("up-chord", [this]() { moveWithinChord(MoveDirection::Up); }, &NotationActionController::hasSelection);
    registerAction("down-chord", [this]() { moveWithinChord(MoveDirection::Down); }, &NotationActionController::hasSelection);
    registerAction("double-duration", &INotationInteraction::increaseDecreaseDuration, -1, false);
    registerAction("half-duration", &INotationInteraction::increaseDecreaseDuration, 1, false);
    registerAction("inc-duration-dotted", &INotationInteraction::increaseDecreaseDuration, -1, true);
    registerAction("dec-duration-dotted", &INotationInteraction::increaseDecreaseDuration, 1, true);

    registerAction("cut", &NotationActionController::cutSelection, &NotationActionController::hasSelection);
    registerAction("copy", &INotationInteraction::copySelection, &NotationActionController::hasSelection);
    registerAction("paste", [this]() { pasteSelection(PastingType::Default); }, &NotationActionController::isNotationPage);
    registerAction("paste-half", [this]() { pasteSelection(PastingType::Half); });
    registerAction("paste-double", [this]() { pasteSelection(PastingType::Double); });
    registerAction("paste-special", [this]() { pasteSelection(PastingType::Special); });
    registerAction("swap", &INotationInteraction::swapSelection, &NotationActionController::hasSelection);
    registerAction("delete", &INotationInteraction::deleteSelection, &NotationActionController::hasSelection);
    registerAction("flip", &INotationInteraction::flipSelection, &NotationActionController::hasSelection);
    registerAction("tie", &NotationActionController::addTie);
    registerAction("chord-tie", &NotationActionController::chordTie);
    registerAction("add-slur", &NotationActionController::addSlur);

    registerAction("undo", &INotationInteraction::undo, &NotationActionController::canUndo);
    registerAction("redo", &INotationInteraction::redo, &NotationActionController::canRedo);

    registerAction("select-next-chord", &INotationInteraction::addToSelection, MoveDirection::Right, MoveSelectionType::Chord,
                   PlayMode::NoPlay, &NotationActionController::isNotNoteInputMode);
    registerAction("select-prev-chord", &INotationInteraction::addToSelection, MoveDirection::Left, MoveSelectionType::Chord,
                   PlayMode::NoPlay, &NotationActionController::isNotNoteInputMode);
    registerAction("select-similar", &NotationActionController::selectAllSimilarElements);
    registerAction("select-similar-staff", &NotationActionController::selectAllSimilarElementsInStaff);
    registerAction("select-similar-range", &NotationActionController::selectAllSimilarElementsInRange);
    registerAction("select-dialog", &NotationActionController::openSelectionMoreOptions);
    registerAction("select-all", &INotationInteraction::selectAll);
    registerAction("select-section", &INotationInteraction::selectSection);
    registerAction("first-element", &INotationInteraction::selectFirstElement, false, PlayMode::PlayChord);
    registerAction("last-element", &INotationInteraction::selectLastElement, PlayMode::PlayChord);
    registerAction("top-chord", [this]() { selectTopOrBottomOfChord(MoveDirection::Up); }, &NotationActionController::hasSelection);
    registerAction("bottom-chord", [this]() { selectTopOrBottomOfChord(MoveDirection::Down); }, &NotationActionController::hasSelection);
    registerAction("move-up", &INotationInteraction::moveChordRestToStaff, MoveDirection::Up, &NotationActionController::hasSelection);
    registerAction("move-down", &INotationInteraction::moveChordRestToStaff, MoveDirection::Down, &NotationActionController::hasSelection);
    registerAction("move-left", &INotationInteraction::swapChordRest, MoveDirection::Left, &NotationActionController::isNoteInputMode);
    registerAction("move-right", &INotationInteraction::swapChordRest, MoveDirection::Right, &NotationActionController::isNoteInputMode);
    registerAction("next-frame", &INotationInteraction::moveSelection, MoveDirection::Right, MoveSelectionType::Frame);
    registerAction("prev-frame", &INotationInteraction::moveSelection, MoveDirection::Left, MoveSelectionType::Frame);
    registerAction("next-system", &INotationInteraction::moveSelection, MoveDirection::Right, MoveSelectionType::System);
    registerAction("prev-system", &INotationInteraction::moveSelection, MoveDirection::Left, MoveSelectionType::System);
    registerAction("next-segment-element", &INotationInteraction::moveSegmentSelection, MoveDirection::Right, PlayMode::PlayNote);
    registerAction("prev-segment-element", &INotationInteraction::moveSegmentSelection, MoveDirection::Left, PlayMode::PlayNote);

    registerAction("system-break", &INotationInteraction::toggleLayoutBreak, LayoutBreakType::LINE);
    registerAction("page-break", &INotationInteraction::toggleLayoutBreak, LayoutBreakType::PAGE);
    registerAction("section-break", &INotationInteraction::toggleLayoutBreak, LayoutBreakType::SECTION);

    registerAction("split-measure", &INotationInteraction::splitSelectedMeasure);
    registerAction("join-measures", &INotationInteraction::joinSelectedMeasures);
    registerAction("insert-measures", &NotationActionController::selectMeasuresCountAndInsert);
    registerAction("append-measures", &NotationActionController::selectMeasuresCountAndAppend);
    registerAction("insert-measure", [this]() { insertBox(BoxType::Measure); });
    registerAction("append-measure", [this]() { appendBox(BoxType::Measure); });
    registerAction("insert-hbox", [this]() { insertBox(BoxType::Horizontal); });
    registerAction("insert-vbox", [this]() { insertBox(BoxType::Vertical); });
    registerAction("insert-textframe", [this]() { insertBox(BoxType::Text); });
    registerAction("append-hbox", [this]() { appendBox(BoxType::Horizontal); });
    registerAction("append-vbox", [this]() { appendBox(BoxType::Vertical); });
    registerAction("append-textframe", [this]() { appendBox(BoxType::Text); });

    registerAction("edit-style", &NotationActionController::openEditStyleDialog);
    registerAction("page-settings", &NotationActionController::openPageSettingsDialog);
    registerAction("staff-properties", &NotationActionController::openStaffProperties);
    registerAction("add-remove-breaks", &NotationActionController::openBreaksDialog);
    registerAction("edit-info", &NotationActionController::openScoreProperties);
    registerAction("transpose", &NotationActionController::openTransposeDialog);
    registerAction("parts", &NotationActionController::openPartsDialog);
    registerAction("staff-text-properties", &NotationActionController::openStaffTextPropertiesDialog);
    registerAction("system-text-properties", &NotationActionController::openStaffTextPropertiesDialog);
    registerAction("measure-properties", &NotationActionController::openMeasurePropertiesDialog);
    registerAction("config-raster", &NotationActionController::openEditGridSizeDialog);
    registerAction("load-style", &NotationActionController::loadStyle);
    registerAction("save-style", &NotationActionController::saveStyle);

    registerAction("voice-x12", &INotationInteraction::swapVoices, 0, 1);
    registerAction("voice-x13", &INotationInteraction::swapVoices, 0, 2);
    registerAction("voice-x14", &INotationInteraction::swapVoices, 0, 3);
    registerAction("voice-x23", &INotationInteraction::swapVoices, 1, 2);
    registerAction("voice-x24", &INotationInteraction::swapVoices, 1, 3);
    registerAction("voice-x34", &INotationInteraction::swapVoices, 2, 3);

    registerAction("add-8va", &INotationInteraction::addOttavaToSelection, OttavaType::OTTAVA_8VA);
    registerAction("add-8vb", &INotationInteraction::addOttavaToSelection, OttavaType::OTTAVA_8VB);
    registerAction("add-hairpin", &INotationInteraction::addHairpinToSelection, HairpinType::CRESC_HAIRPIN);
    registerAction("add-hairpin-reverse", &INotationInteraction::addHairpinToSelection, HairpinType::DECRESC_HAIRPIN);
    registerAction("add-noteline", &INotationInteraction::addAnchoredLineToSelectedNotes);

    registerAction("title-text", &INotationInteraction::addText, TextType::TITLE);
    registerAction("subtitle-text", &INotationInteraction::addText, TextType::SUBTITLE);
    registerAction("composer-text", &INotationInteraction::addText, TextType::COMPOSER);
    registerAction("poet-text", &INotationInteraction::addText, TextType::POET);
    registerAction("part-text", &INotationInteraction::addText, TextType::INSTRUMENT_EXCERPT);
    registerAction("system-text", &INotationInteraction::addText, TextType::SYSTEM);
    registerAction("staff-text", &INotationInteraction::addText, TextType::STAFF);
    registerAction("expression-text", &INotationInteraction::addText, TextType::EXPRESSION);
    registerAction("rehearsalmark-text", &INotationInteraction::addText, TextType::REHEARSAL_MARK);
    registerAction("instrument-change-text", &INotationInteraction::addText, TextType::INSTRUMENT_CHANGE);
    registerAction("fingering-text", &INotationInteraction::addText, TextType::FINGERING);
    registerAction("sticking-text", &INotationInteraction::addText, TextType::STICKING);
    registerAction("chord-text", &INotationInteraction::addText, TextType::HARMONY_A);
    registerAction("roman-numeral-text", &INotationInteraction::addText, TextType::HARMONY_ROMAN);
    registerAction("nashville-number-text", &INotationInteraction::addText, TextType::HARMONY_NASHVILLE);
    registerAction("lyrics", &INotationInteraction::addText, TextType::LYRICS_ODD);
    registerAction("figured-bass", &INotationInteraction::addFiguredBass);
    registerAction("tempo", &INotationInteraction::addText, TextType::TEMPO);

    registerAction("stretch-", [this]() { addStretch(-STRETCH_STEP); });
    registerAction("stretch+", [this]() { addStretch(STRETCH_STEP); });

    registerAction("reset-stretch", &NotationActionController::resetStretch);
    registerAction("reset-text-style-overrides", &INotationInteraction::resetTextStyleOverrides);
    registerAction("reset-beammode", &NotationActionController::resetBeamMode);
    registerAction("reset", &INotationInteraction::resetShapesAndPosition, &NotationActionController::hasSelection);

    registerAction("show-invisible", [this]() { toggleScoreConfig(ScoreConfigType::ShowInvisibleElements); });
    registerAction("show-unprintable", [this]() { toggleScoreConfig(ScoreConfigType::ShowUnprintableElements); });
    registerAction("show-frames", [this]() { toggleScoreConfig(ScoreConfigType::ShowFrames); });
    registerAction("show-pageborders", [this]() { toggleScoreConfig(ScoreConfigType::ShowPageMargins); });
    registerAction("show-irregular", [this]() { toggleScoreConfig(ScoreConfigType::MarkIrregularMeasures); });

    registerAction("concert-pitch", &NotationActionController::toggleConcertPitch);

    registerAction("explode", &INotationInteraction::explodeSelectedStaff);
    registerAction("implode", &INotationInteraction::implodeSelectedStaff);
    registerAction("realize-chord-symbols", &INotationInteraction::realizeSelectedChordSymbols);
    registerAction("time-delete", &INotationInteraction::removeSelectedRange);
    registerAction("del-empty-measures", &INotationInteraction::removeEmptyTrailingMeasures);
    registerAction("slash-fill", &INotationInteraction::fillSelectionWithSlashes);
    registerAction("slash-rhythm", &INotationInteraction::replaceSelectedNotesWithSlashes);
    registerAction("pitch-spell", &INotationInteraction::spellPitches);
    registerAction("reset-groupings", &INotationInteraction::regroupNotesAndRests);
    registerAction("resequence-rehearsal-marks", &INotationInteraction::resequenceRehearsalMarks);
    registerAction("unroll-repeats", &NotationActionController::unrollRepeats);

    registerAction("copy-lyrics-to-clipboard", &INotationInteraction::copyLyrics);
    registerAction("acciaccatura", &INotationInteraction::addGraceNotesToSelectedNotes, GraceNoteType::ACCIACCATURA);
    registerAction("appoggiatura", &INotationInteraction::addGraceNotesToSelectedNotes, GraceNoteType::APPOGGIATURA);
    registerAction("grace4", &INotationInteraction::addGraceNotesToSelectedNotes, GraceNoteType::GRACE4);
    registerAction("grace16", &INotationInteraction::addGraceNotesToSelectedNotes, GraceNoteType::GRACE16);
    registerAction("grace32", &INotationInteraction::addGraceNotesToSelectedNotes, GraceNoteType::GRACE32);
    registerAction("grace8after", &INotationInteraction::addGraceNotesToSelectedNotes, GraceNoteType::GRACE8_AFTER);
    registerAction("grace16after", &INotationInteraction::addGraceNotesToSelectedNotes, GraceNoteType::GRACE16_AFTER);
    registerAction("grace32after", &INotationInteraction::addGraceNotesToSelectedNotes, GraceNoteType::GRACE32_AFTER);

    registerAction("beam-start", &INotationInteraction::addBeamToSelectedChordRests, BeamMode::BEGIN);
    registerAction("beam-mid", &INotationInteraction::addBeamToSelectedChordRests, BeamMode::MID);
    registerAction("no-beam", &INotationInteraction::addBeamToSelectedChordRests, BeamMode::NONE);
    registerAction("beam-32", &INotationInteraction::addBeamToSelectedChordRests, BeamMode::BEGIN32);
    registerAction("beam-64", &INotationInteraction::addBeamToSelectedChordRests, BeamMode::BEGIN64);
    registerAction("auto-beam", &INotationInteraction::addBeamToSelectedChordRests, BeamMode::AUTO);

    registerAction("add-brackets", &INotationInteraction::addBracketsToSelection, BracketsType::Brackets);
    registerAction("add-parentheses", &INotationInteraction::addBracketsToSelection, BracketsType::Parentheses);
    registerAction("add-braces", &INotationInteraction::addBracketsToSelection, BracketsType::Braces);

    registerAction("enh-both", &INotationInteraction::changeEnharmonicSpelling, true);
    registerAction("enh-current", &INotationInteraction::changeEnharmonicSpelling, false);

    registerAction("edit-element", &INotationInteraction::startEditElement);

    registerAction("text-b", &INotationInteraction::toggleBold, &NotationActionController::isEditingText);
    registerAction("text-i", &INotationInteraction::toggleItalic, &NotationActionController::isEditingText);
    registerAction("text-u", &INotationInteraction::toggleUnderline, &NotationActionController::isEditingText);
    registerAction("text-s", &INotationInteraction::toggleStrike, &NotationActionController::isEditingText);

    registerAction("select-next-measure", &INotationInteraction::addToSelection, MoveDirection::Right, MoveSelectionType::Measure,
                   PlayMode::NoPlay, &NotationActionController::isNotNoteInputMode);
    registerAction("select-prev-measure", &INotationInteraction::addToSelection, MoveDirection::Left, MoveSelectionType::Measure,
                   PlayMode::NoPlay, &NotationActionController::isNotNoteInputMode);
    registerAction("select-begin-line", &INotationInteraction::expandSelection, ExpandSelectionMode::BeginSystem, PlayMode::NoPlay,
                   &NotationActionController::isNotNoteInputMode);
    registerAction("select-end-line", &INotationInteraction::expandSelection, ExpandSelectionMode::EndSystem, PlayMode::NoPlay,
                   &NotationActionController::isNotNoteInputMode);
    registerAction("select-begin-score", &INotationInteraction::expandSelection, ExpandSelectionMode::BeginScore, PlayMode::NoPlay,
                   &NotationActionController::isNotNoteInputMode);
    registerAction("select-end-score", &INotationInteraction::expandSelection, ExpandSelectionMode::EndScore, PlayMode::NoPlay,
                   &NotationActionController::isNotNoteInputMode);
    registerAction("select-staff-above", &INotationInteraction::addToSelection, MoveDirection::Up, MoveSelectionType::Track,
                   PlayMode::NoPlay, &NotationActionController::isNotNoteInputMode);
    registerAction("select-staff-below", &INotationInteraction::addToSelection, MoveDirection::Down, MoveSelectionType::Track,
                   PlayMode::NoPlay, &NotationActionController::isNotNoteInputMode);
    registerAction("top-staff", &INotationInteraction::selectTopStaff, PlayMode::PlayChord);
    registerAction("empty-trailing-measure", &INotationInteraction::selectEmptyTrailingMeasure);
    registerAction("pitch-up-diatonic", &INotationInteraction::movePitch, MoveDirection::Up, PitchMode::DIATONIC, PlayMode::PlayNote);
    registerAction("pitch-down-diatonic", &INotationInteraction::movePitch, MoveDirection::Down, PitchMode::DIATONIC, PlayMode::PlayNote);

    registerAction("repeat-sel", &INotationInteraction::repeatSelection);

    registerAction("add-trill", &INotationInteraction::toggleArticulation, Ms::SymId::ornamentTrill);
    registerAction("add-up-bow", &INotationInteraction::toggleArticulation, Ms::SymId::stringsUpBow);
    registerAction("add-down-bow", &INotationInteraction::toggleArticulation, Ms::SymId::stringsDownBow);
    registerAction("clef-violin", &INotationInteraction::insertClef, Ms::ClefType::G);
    registerAction("transpose-up", &INotationInteraction::transposeSemitone, 1, PlayMode::PlayNote);
    registerAction("transpose-down", &INotationInteraction::transposeSemitone, -1, PlayMode::PlayNote);
    registerAction("toggle-insert-mode", &INotationInteraction::toggleGlobalOrLocalInsert);
    registerAction("pad-note-decrease", &NotationActionController::halveNoteInputDuration, &NotationActionController::isNoteInputMode);
    registerAction("pad-note-increase", &NotationActionController::doubleNoteInputDuration, &NotationActionController::isNoteInputMode);
    registerAction("pad-note-decrease-TAB", &NotationActionController::halveNoteInputDuration, &NotationActionController::isNoteInputMode);
    registerAction("pad-note-increase-TAB", &NotationActionController::doubleNoteInputDuration, &NotationActionController::isNoteInputMode);
    registerAction("get-location", &INotationInteraction::getLocation, &NotationActionController::isNotationPage);
    registerAction("toggle-mmrest", &INotationInteraction::execute, &Ms::Score::cmdToggleMmrest);
    registerAction("toggle-hide-empty", &INotationInteraction::execute, &Ms::Score::cmdToggleHideEmpty);

    for (int i = MIN_NOTES_INTERVAL; i <= MAX_NOTES_INTERVAL; ++i) {
        if (isNotesIntervalValid(i)) {
            registerAction("interval" + std::to_string(i), &INotationInteraction::addIntervalToSelectedNotes, i);
        }
    }

    for (int i = 0; i < Ms::VOICES; ++i) {
        registerAction("voice-" + std::to_string(i + 1), [this, i]() { changeVoice(i); });
    }

    // TAB
    registerAction("string-above", &NotationActionController::move, MoveDirection::Up, false, &NotationActionController::isTablatureStaff);
    registerAction("string-below", &NotationActionController::move, MoveDirection::Down, false,
                   &NotationActionController::isTablatureStaff);

    for (int i = 0; i < MAX_FRET; ++i) {
        registerAction("fret-" + std::to_string(i), &INotationInteraction::addFret, i, &NotationActionController::isTablatureStaff);
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
        bool enabled = (this->*iter->second)();
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

void NotationActionController::putNote(const actions::ActionData& data)
{
    TRACEFUNC;
    auto noteInput = currentNotationNoteInput();
    if (!noteInput) {
        return;
    }

    IF_ASSERT_FAILED(data.count() > 2) {
        return;
    }

    PointF pos = data.arg<PointF>(0);
    bool replace = data.arg<bool>(1);
    bool insert = data.arg<bool>(2);

    noteInput->putNote(pos, replace, insert);

    playSelectedElement();
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

    if (!interaction->canAddTupletToSelecredChordRests()) {
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
    auto noteInput = currentNotationNoteInput();
    if (noteInput) {
        noteInput->doubleNoteInputDuration();
    }
}

void NotationActionController::halveNoteInputDuration()
{
    TRACEFUNC;
    auto noteInput = currentNotationNoteInput();
    if (noteInput) {
        noteInput->halveNoteInputDuration();
    }
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

void NotationActionController::insertBoxes(BoxType boxType, int count)
{
    TRACEFUNC;
    auto interaction = currentNotationInteraction();
    if (!interaction) {
        return;
    }

    int firstSelectedBoxIndex = this->firstSelectedBoxIndex();

    if (firstSelectedBoxIndex == INVALID_BOX_INDEX) {
        return;
    }

    interaction->addBoxes(boxType, count, firstSelectedBoxIndex);
}

void NotationActionController::insertBox(BoxType boxType)
{
    TRACEFUNC;
    insertBoxes(boxType, 1);
}

int NotationActionController::firstSelectedBoxIndex() const
{
    TRACEFUNC;
    int result = INVALID_BOX_INDEX;

    auto selection = currentNotationSelection();
    if (!selection) {
        return result;
    }

    if (selection->isRange()) {
        result = selection->range()->startMeasureIndex();
    } else if (selection->element()) {
        Measure* measure = selection->element()->findMeasure();
        result = measure ? measure->index() : INVALID_BOX_INDEX;
    }

    return result;
}

void NotationActionController::appendBoxes(BoxType boxType, int count)
{
    TRACEFUNC;
    auto interaction = currentNotationInteraction();
    if (!interaction) {
        return;
    }

    interaction->addBoxes(boxType, count);
}

void NotationActionController::appendBox(BoxType boxType)
{
    TRACEFUNC;
    appendBoxes(boxType, 1);
}

void NotationActionController::unrollRepeats()
{
    TRACEFUNC;
    auto interaction = currentNotationInteraction();
    if (!interaction) {
        return;
    }
    interaction->unrollRepeats();
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

void NotationActionController::selectMeasuresCountAndInsert()
{
    RetVal<Val> measureCount = interactive()->open("musescore://notation/selectmeasurescount?operation=insert");

    if (measureCount.ret) {
        insertBoxes(BoxType::Measure, measureCount.val.toInt());
    }
}

void NotationActionController::selectMeasuresCountAndAppend()
{
    RetVal<Val> measureCount = interactive()->open("musescore://notation/selectmeasurescount?operation=append");

    if (measureCount.ret) {
        appendBoxes(BoxType::Measure, measureCount.val.toInt());
    }
}

void NotationActionController::openEditStyleDialog()
{
    interactive()->open("musescore://notation/style");
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
    return interactive()->isOpened("musescore://notation").val;
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
    m_isEnabledMap[code] = isEnabled;
    dispatcher()->reg(this, code, handler);
}

void NotationActionController::registerAction(const mu::actions::ActionCode& code,
                                              void (NotationActionController::* handler)(const actions::ActionData& data),
                                              bool (NotationActionController::* isEnabled)() const)
{
    m_isEnabledMap[code] = isEnabled;
    dispatcher()->reg(this, code, this, handler);
}

void NotationActionController::registerAction(const mu::actions::ActionCode& code,
                                              void (NotationActionController::* handler)(),
                                              bool (NotationActionController::* isEnabled)() const)
{
    m_isEnabledMap[code] = isEnabled;
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

void NotationActionController::registerAction(const mu::actions::ActionCode& code, void (NotationActionController::* handler)(MoveDirection,
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
