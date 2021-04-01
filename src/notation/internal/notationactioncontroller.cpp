//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2020 MuseScore BVBA and others
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
//=============================================================================
#include "notationactioncontroller.h"

#include <QPoint>

#include "log.h"
#include "notationtypes.h"

using namespace mu::notation;
using namespace mu::actions;
using namespace mu::context;

static constexpr int INVALID_BOX_INDEX = -1;
static constexpr qreal STRETCH_STEP = 0.1;
static const ActionCode ESCAPE_ACTION_CODE = "escape";
static const ActionCode UNDO_ACTION_CODE = "undo";
static const ActionCode REDO_ACTION_CODE = "redo";

void NotationActionController::init()
{
    //! NOTE For historical reasons, the name of the action does not match what needs to be done
    dispatcher()->reg(this, ESCAPE_ACTION_CODE, this, &NotationActionController::resetState);

    dispatcher()->reg(this, "note-input", [this]() { toggleNoteInput(); });
    dispatcher()->reg(this, "note-input-rhythm", [this]() { toggleNoteInputMethod(NoteInputMethod::RHYTHM); });
    dispatcher()->reg(this, "note-input-repitch", [this]() { toggleNoteInputMethod(NoteInputMethod::REPITCH); });
    dispatcher()->reg(this, "note-input-realtime-auto", [this]() { toggleNoteInputMethod(NoteInputMethod::REALTIME_AUTO); });
    dispatcher()->reg(this, "note-input-realtime-manual", [this]() { toggleNoteInputMethod(NoteInputMethod::REALTIME_MANUAL); });
    dispatcher()->reg(this, "note-input-timewise", [this]() { toggleNoteInputMethod(NoteInputMethod::TIMEWISE); });

    dispatcher()->reg(this, "note-longa", [this]() { padNote(Pad::NOTE00); });
    dispatcher()->reg(this, "note-breve", [this]() { padNote(Pad::NOTE0); });
    dispatcher()->reg(this, "pad-note-1", [this]() { padNote(Pad::NOTE1); });
    dispatcher()->reg(this, "pad-note-2", [this]() { padNote(Pad::NOTE2); });
    dispatcher()->reg(this, "pad-note-4", [this]() { padNote(Pad::NOTE4); });
    dispatcher()->reg(this, "pad-note-8", [this]() { padNote(Pad::NOTE8); });
    dispatcher()->reg(this, "pad-note-16", [this]() { padNote(Pad::NOTE16); });
    dispatcher()->reg(this, "pad-note-32", [this]() { padNote(Pad::NOTE32); });
    dispatcher()->reg(this, "pad-note-64", [this]() { padNote(Pad::NOTE64); });
    dispatcher()->reg(this, "pad-note-128", [this]() { padNote(Pad::NOTE128); });
    dispatcher()->reg(this, "pad-note-256", [this]() { padNote(Pad::NOTE256); });
    dispatcher()->reg(this, "pad-note-512", [this]() { padNote(Pad::NOTE512); });
    dispatcher()->reg(this, "pad-note-1024", [this]() { padNote(Pad::NOTE1024); });
    dispatcher()->reg(this, "pad-dot", [this]() { padNote(Pad::DOT); });
    dispatcher()->reg(this, "pad-dotdot", [this]() { padNote(Pad::DOTDOT); });
    dispatcher()->reg(this, "pad-dot3", [this]() { padNote(Pad::DOT3); });
    dispatcher()->reg(this, "pad-dot4", [this]() { padNote(Pad::DOT4); });
    dispatcher()->reg(this, "pad-rest", [this]() { padNote(Pad::REST); });

    dispatcher()->reg(this, "note-c", [this]() { addNote(NoteName::C, NoteAddingMode::NextChord); });
    dispatcher()->reg(this, "note-d", [this]() { addNote(NoteName::D, NoteAddingMode::NextChord); });
    dispatcher()->reg(this, "note-e", [this]() { addNote(NoteName::E, NoteAddingMode::NextChord); });
    dispatcher()->reg(this, "note-f", [this]() { addNote(NoteName::F, NoteAddingMode::NextChord); });
    dispatcher()->reg(this, "note-g", [this]() { addNote(NoteName::G, NoteAddingMode::NextChord); });
    dispatcher()->reg(this, "note-a", [this]() { addNote(NoteName::A, NoteAddingMode::NextChord); });
    dispatcher()->reg(this, "note-b", [this]() { addNote(NoteName::B, NoteAddingMode::NextChord); });

    dispatcher()->reg(this, "chord-c", [this]() { addNote(NoteName::C, NoteAddingMode::CurrentChord); });
    dispatcher()->reg(this, "chord-d", [this]() { addNote(NoteName::D, NoteAddingMode::CurrentChord); });
    dispatcher()->reg(this, "chord-e", [this]() { addNote(NoteName::E, NoteAddingMode::CurrentChord); });
    dispatcher()->reg(this, "chord-f", [this]() { addNote(NoteName::F, NoteAddingMode::CurrentChord); });
    dispatcher()->reg(this, "chord-g", [this]() { addNote(NoteName::G, NoteAddingMode::CurrentChord); });
    dispatcher()->reg(this, "chord-a", [this]() { addNote(NoteName::A, NoteAddingMode::CurrentChord); });
    dispatcher()->reg(this, "chord-b", [this]() { addNote(NoteName::B, NoteAddingMode::CurrentChord); });

    dispatcher()->reg(this, "insert-c", [this]() { addNote(NoteName::C, NoteAddingMode::InsertChord); });
    dispatcher()->reg(this, "insert-d", [this]() { addNote(NoteName::D, NoteAddingMode::InsertChord); });
    dispatcher()->reg(this, "insert-e", [this]() { addNote(NoteName::E, NoteAddingMode::InsertChord); });
    dispatcher()->reg(this, "insert-f", [this]() { addNote(NoteName::F, NoteAddingMode::InsertChord); });
    dispatcher()->reg(this, "insert-g", [this]() { addNote(NoteName::G, NoteAddingMode::InsertChord); });
    dispatcher()->reg(this, "insert-a", [this]() { addNote(NoteName::A, NoteAddingMode::InsertChord); });
    dispatcher()->reg(this, "insert-b", [this]() { addNote(NoteName::B, NoteAddingMode::InsertChord); });

    dispatcher()->reg(this, "flat2", [this]() { toggleAccidental(AccidentalType::FLAT2); });
    dispatcher()->reg(this, "flat", [this]() { toggleAccidental(AccidentalType::FLAT); });
    dispatcher()->reg(this, "nat", [this]() { toggleAccidental(AccidentalType::NATURAL); });
    dispatcher()->reg(this, "sharp", [this]() { toggleAccidental(AccidentalType::SHARP); });
    dispatcher()->reg(this, "sharp2", [this]() { toggleAccidental(AccidentalType::SHARP2); });

    dispatcher()->reg(this, "add-marcato", [this]() { addArticulation(SymbolId::articMarcatoAbove); });
    dispatcher()->reg(this, "add-sforzato", [this]() { addArticulation(SymbolId::articAccentAbove); });
    dispatcher()->reg(this, "add-tenuto", [this]() { addArticulation(SymbolId::articTenutoAbove); });
    dispatcher()->reg(this, "add-staccato", [this]() { addArticulation(SymbolId::articStaccatoAbove); });

    dispatcher()->reg(this, "duplet", [this]() { putTuplet(2); });
    dispatcher()->reg(this, "triplet", [this]() { putTuplet(3); });
    dispatcher()->reg(this, "quadruplet", [this]() { putTuplet(4); });
    dispatcher()->reg(this, "quintuplet", [this]() { putTuplet(5); });
    dispatcher()->reg(this, "sextuplet", [this]() { putTuplet(6); });
    dispatcher()->reg(this, "septuplet", [this]() { putTuplet(7); });
    dispatcher()->reg(this, "octuplet", [this]() { putTuplet(8); });
    dispatcher()->reg(this, "nonuplet", [this]() { putTuplet(9); });
    dispatcher()->reg(this, "tuplet-dialog", this, &NotationActionController::openTupletOtherDialog);

    dispatcher()->reg(this, "put-note", this, &NotationActionController::putNote);

    dispatcher()->reg(this, "next-element", [this](const ActionCode& actionCode) { moveAction(actionCode); });
    dispatcher()->reg(this, "prev-element", [this](const ActionCode& actionCode) { moveAction(actionCode); });
    dispatcher()->reg(this, "next-chord", [this](const ActionCode& actionCode) { moveAction(actionCode); });
    dispatcher()->reg(this, "prev-chord", [this](const ActionCode& actionCode) { moveAction(actionCode); });
    dispatcher()->reg(this, "next-measure", [this](const ActionCode& actionCode) { moveAction(actionCode); });
    dispatcher()->reg(this, "prev-measure", [this](const ActionCode& actionCode) { moveAction(actionCode); });
    dispatcher()->reg(this, "next-track", [this](const ActionCode& actionCode) { moveAction(actionCode); });
    dispatcher()->reg(this, "prev-track", [this](const ActionCode& actionCode) { moveAction(actionCode); });
    dispatcher()->reg(this, "pitch-up", [this](const ActionCode& actionCode) { moveAction(actionCode); });
    dispatcher()->reg(this, "pitch-down", [this](const ActionCode& actionCode) { moveAction(actionCode); });
    dispatcher()->reg(this, "pitch-up-octave", [this](const ActionCode& actionCode) { moveAction(actionCode); });
    dispatcher()->reg(this, "pitch-down-octave", [this](const ActionCode& actionCode) { moveAction(actionCode); });

    dispatcher()->reg(this, "cut", this, &NotationActionController::cutSelection);
    dispatcher()->reg(this, "copy", this, &NotationActionController::copySelection);
    dispatcher()->reg(this, "paste", [this]() { pasteSelection(PastingType::Default); });
    dispatcher()->reg(this, "paste-half", [this]() { pasteSelection(PastingType::Half); });
    dispatcher()->reg(this, "paste-double", [this]() { pasteSelection(PastingType::Double); });
    dispatcher()->reg(this, "paste-special", [this]() { pasteSelection(PastingType::Special); });
    dispatcher()->reg(this, "swap", this, &NotationActionController::swapSelection);
    dispatcher()->reg(this, "delete", this, &NotationActionController::deleteSelection);
    dispatcher()->reg(this, "flip", this, &NotationActionController::flipSelection);
    dispatcher()->reg(this, "tie", this, &NotationActionController::addTie);
    dispatcher()->reg(this, "add-slur", this, &NotationActionController::addSlur);

    dispatcher()->reg(this, UNDO_ACTION_CODE, this, &NotationActionController::undo);
    dispatcher()->reg(this, REDO_ACTION_CODE, this, &NotationActionController::redo);

    dispatcher()->reg(this, "select-next-chord", [this]() { addChordToSelection(MoveDirection::Right); });
    dispatcher()->reg(this, "select-prev-chord", [this]() { addChordToSelection(MoveDirection::Left); });
    dispatcher()->reg(this, "select-similar", this, &NotationActionController::selectAllSimilarElements);
    dispatcher()->reg(this, "select-similar-staff", this, &NotationActionController::selectAllSimilarElementsInStaff);
    dispatcher()->reg(this, "select-similar-range", this, &NotationActionController::selectAllSimilarElementsInRange);
    dispatcher()->reg(this, "select-dialog", this, &NotationActionController::openSelectionMoreOptions);
    dispatcher()->reg(this, "select-all", this, &NotationActionController::selectAll);
    dispatcher()->reg(this, "select-section", this, &NotationActionController::selectSection);

    dispatcher()->reg(this, "split-measure", this, &NotationActionController::splitMeasure);
    dispatcher()->reg(this, "join-measures", this, &NotationActionController::joinSelectedMeasures);
    dispatcher()->reg(this, "insert-measures", this, &NotationActionController::selectMeasuresCountAndInsert);
    dispatcher()->reg(this, "append-measures", this, &NotationActionController::selectMeasuresCountAndAppend);
    dispatcher()->reg(this, "insert-measure", [this]() { insertBox(BoxType::Measure); });
    dispatcher()->reg(this, "append-measure", [this]() { appendBox(BoxType::Measure); });
    dispatcher()->reg(this, "insert-hbox", [this]() { insertBox(BoxType::Horizontal); });
    dispatcher()->reg(this, "insert-vbox", [this]() { insertBox(BoxType::Vertical); });
    dispatcher()->reg(this, "insert-textframe", [this]() { insertBox(BoxType::Text); });
    dispatcher()->reg(this, "append-hbox", [this]() { appendBox(BoxType::Horizontal); });
    dispatcher()->reg(this, "append-vbox", [this]() { appendBox(BoxType::Vertical); });
    dispatcher()->reg(this, "append-textframe", [this]() { appendBox(BoxType::Text); });

    dispatcher()->reg(this, "edit-style", this, &NotationActionController::openPageStyle);
    dispatcher()->reg(this, "staff-properties", this, &NotationActionController::openStaffProperties);
    dispatcher()->reg(this, "add-remove-breaks", this, &NotationActionController::openBreaksDialog);
    dispatcher()->reg(this, "edit-info", this, &NotationActionController::openScoreProperties);
    dispatcher()->reg(this, "transpose", this, &NotationActionController::openTransposeDialog);
    dispatcher()->reg(this, "parts", this, &NotationActionController::openPartsDialog);

    dispatcher()->reg(this, "voice-x12", [this]() { swapVoices(0, 1); });
    dispatcher()->reg(this, "voice-x13", [this]() { swapVoices(0, 2); });
    dispatcher()->reg(this, "voice-x14", [this]() { swapVoices(0, 3); });
    dispatcher()->reg(this, "voice-x23", [this]() { swapVoices(1, 2); });
    dispatcher()->reg(this, "voice-x24", [this]() { swapVoices(1, 3); });
    dispatcher()->reg(this, "voice-x34", [this]() { swapVoices(2, 3); });

    dispatcher()->reg(this, "add-8va", [this]() { addOttava(OttavaType::OTTAVA_8VA); });
    dispatcher()->reg(this, "add-8vb", [this]() { addOttava(OttavaType::OTTAVA_8VB); });
    dispatcher()->reg(this, "add-hairpin", [this]() { addHairpin(HairpinType::CRESC_HAIRPIN); });
    dispatcher()->reg(this, "add-hairpin-reverse", [this]() { addHairpin(HairpinType::DECRESC_HAIRPIN); });
    dispatcher()->reg(this, "add-noteline", this, &NotationActionController::addAnchoredNoteLine);

    dispatcher()->reg(this, "title-text", [this]() { addText(TextType::TITLE); });
    dispatcher()->reg(this, "subtitle-text", [this]() { addText(TextType::SUBTITLE); });
    dispatcher()->reg(this, "composer-text", [this]() { addText(TextType::COMPOSER); });
    dispatcher()->reg(this, "poet-text", [this]() { addText(TextType::POET); });
    dispatcher()->reg(this, "part-text", [this]() { addText(TextType::INSTRUMENT_EXCERPT); });
    dispatcher()->reg(this, "system-text", [this]() { addText(TextType::SYSTEM); });
    dispatcher()->reg(this, "staff-text", [this]() { addText(TextType::STAFF); });
    dispatcher()->reg(this, "expression-text", [this]() { addText(TextType::EXPRESSION); });
    dispatcher()->reg(this, "rehearsalmark-text", [this]() { addText(TextType::REHEARSAL_MARK); });
    dispatcher()->reg(this, "instrument-change-text", [this]() { addText(TextType::INSTRUMENT_CHANGE); });
    dispatcher()->reg(this, "fingering-text", [this]() { addText(TextType::FINGERING); });
    dispatcher()->reg(this, "sticking-text", [this]() { addText(TextType::STICKING); });
    dispatcher()->reg(this, "chord-text", [this]() { addText(TextType::HARMONY_A); });
    dispatcher()->reg(this, "roman-numeral-text", [this]() { addText(TextType::HARMONY_ROMAN); });
    dispatcher()->reg(this, "nashville-number-text", [this]() { addText(TextType::HARMONY_NASHVILLE); });
    dispatcher()->reg(this, "lyrics", [this]() { addText(TextType::LYRICS_ODD); });
    dispatcher()->reg(this, "figured-bass", [this]() { addFiguredBass(); });
    dispatcher()->reg(this, "tempo", [this]() { addText(TextType::TEMPO); });

    dispatcher()->reg(this, "stretch-", [this]() { addStretch(-STRETCH_STEP); });
    dispatcher()->reg(this, "stretch+", [this]() { addStretch(STRETCH_STEP); });

    dispatcher()->reg(this, "reset-stretch", this, &NotationActionController::resetStretch);
    dispatcher()->reg(this, "reset-text-style-overrides", this, &NotationActionController::resetTextStyleOverrides);
    dispatcher()->reg(this, "reset-beammode", this, &NotationActionController::resetBeamMode);
    dispatcher()->reg(this, "reset", this, &NotationActionController::resetShapesAndPosition);

    dispatcher()->reg(this, "show-invisible", [this]() { toggleScoreConfig(ScoreConfigType::ShowInvisibleElements); });
    dispatcher()->reg(this, "show-unprintable", [this]() { toggleScoreConfig(ScoreConfigType::ShowUnprintableElements); });
    dispatcher()->reg(this, "show-frames", [this]() { toggleScoreConfig(ScoreConfigType::ShowFrames); });
    dispatcher()->reg(this, "show-pageborders", [this]() { toggleScoreConfig(ScoreConfigType::ShowPageMargins); });
    dispatcher()->reg(this, "show-irregular", [this]() { toggleScoreConfig(ScoreConfigType::MarkIrregularMeasures); });

    dispatcher()->reg(this, "explode", this, &NotationActionController::explodeSelectedStaff);
    dispatcher()->reg(this, "implode", this, &NotationActionController::implodeSelectedStaff);
    dispatcher()->reg(this, "realize-chord-symbols", this, &NotationActionController::realizeSelectedChordSymbols);
    dispatcher()->reg(this, "time-delete", this, &NotationActionController::removeSelectedRange);
    dispatcher()->reg(this, "del-empty-measures", this, &NotationActionController::removeEmptyTrailingMeasures);
    dispatcher()->reg(this, "slash-fill", this, &NotationActionController::fillSelectionWithSlashes);
    dispatcher()->reg(this, "slash-rhythm", this, &NotationActionController::replaceSelectedNotesWithSlashes);
    dispatcher()->reg(this, "pitch-spell", this, &NotationActionController::spellPitches);
    dispatcher()->reg(this, "reset-groupings", this, &NotationActionController::regroupNotesAndRests);
    dispatcher()->reg(this, "resequence-rehearsal-marks", this, &NotationActionController::resequenceRehearsalMarks);
    dispatcher()->reg(this, "unroll-repeats", this, &NotationActionController::unrollRepeats);
    dispatcher()->reg(this, "copy-lyrics-to-clipboard", this, &NotationActionController::copyLyrics);

    dispatcher()->reg(this, "acciaccatura", [this]() { addGraceNotesToSelectedNotes(GraceNoteType::ACCIACCATURA); });
    dispatcher()->reg(this, "appoggiatura", [this]() { addGraceNotesToSelectedNotes(GraceNoteType::APPOGGIATURA); });
    dispatcher()->reg(this, "grace4", [this]() { addGraceNotesToSelectedNotes(GraceNoteType::GRACE4); });
    dispatcher()->reg(this, "grace16", [this]() { addGraceNotesToSelectedNotes(GraceNoteType::GRACE16); });
    dispatcher()->reg(this, "grace32", [this]() { addGraceNotesToSelectedNotes(GraceNoteType::GRACE32); });
    dispatcher()->reg(this, "grace8after", [this]() { addGraceNotesToSelectedNotes(GraceNoteType::GRACE8_AFTER); });
    dispatcher()->reg(this, "grace16after", [this]() { addGraceNotesToSelectedNotes(GraceNoteType::GRACE16_AFTER); });
    dispatcher()->reg(this, "grace32after", [this]() { addGraceNotesToSelectedNotes(GraceNoteType::GRACE32_AFTER); });

    dispatcher()->reg(this, "beam-start", [this]() { addBeamToSelectedChordRests(BeamMode::BEGIN); });
    dispatcher()->reg(this, "beam-mid", [this]() { addBeamToSelectedChordRests(BeamMode::MID); });
    dispatcher()->reg(this, "no-beam", [this]() { addBeamToSelectedChordRests(BeamMode::NONE); });
    dispatcher()->reg(this, "beam-32", [this]() { addBeamToSelectedChordRests(BeamMode::BEGIN32); });
    dispatcher()->reg(this, "beam-64", [this]() { addBeamToSelectedChordRests(BeamMode::BEGIN64); });
    dispatcher()->reg(this, "auto-beam", [this]() { addBeamToSelectedChordRests(BeamMode::AUTO); });

    dispatcher()->reg(this, "add-brackets", [this]() { addBracketsToSelection(BracketsType::Brackets); });
    dispatcher()->reg(this, "add-parentheses", [this]() { addBracketsToSelection(BracketsType::Parentheses); });
    dispatcher()->reg(this, "add-braces", [this]() { addBracketsToSelection(BracketsType::Braces); });

    for (int i = MIN_NOTES_INTERVAL; i <= MAX_NOTES_INTERVAL; ++i) {
        if (isNotesIntervalValid(i)) {
            dispatcher()->reg(this, "interval" + std::to_string(i), [this, i]() { addInterval(i); });
        }
    }

    for (int i = 0; i < VOICES; ++i) {
        dispatcher()->reg(this, "voice-" + std::to_string(i + 1), [this, i]() { changeVoice(i); });
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
    //! NOTE If the notation is not loaded, we cannot process anything.
    if (!currentNotation()) {
        return false;
    }

    //! NOTE At the moment, if we are in the text editing mode, we can only process the escape
    if (isTextEditting()) {
        return code == ESCAPE_ACTION_CODE;
    }

    if (code == UNDO_ACTION_CODE) {
        return canUndo();
    }

    if (code == REDO_ACTION_CODE) {
        return canRedo();
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

void NotationActionController::toggleNoteInput()
{
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

void NotationActionController::resetState()
{
    auto isAudioPlaying = [this]() {
        return sequencer()->status() == audio::ISequencer::PLAYING;
    };

    if (isAudioPlaying()) {
        sequencer()->stop();
        return;
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

    if (interaction->isDragStarted()) {
        interaction->endDrag();
        return;
    }

    if (interaction->isTextEditingStarted()) {
        interaction->endEditText();
        return;
    }

    if (!interaction->selection()->isNone()) {
        interaction->clearSelection();
    }
}

void NotationActionController::toggleNoteInputMethod(NoteInputMethod method)
{
    auto noteInput = currentNotationNoteInput();
    if (!noteInput) {
        return;
    }

    if (!noteInput->isNoteInputMode()) {
        noteInput->startNoteInput();
    }

    noteInput->toggleNoteInputMethod(method);
}

void NotationActionController::addNote(NoteName note, NoteAddingMode addingMode)
{
    auto noteInput = currentNotationNoteInput();
    if (!noteInput) {
        return;
    }

    if (!noteInput->isNoteInputMode()) {
        noteInput->startNoteInput();
    }

    noteInput->addNote(note, addingMode);
}

void NotationActionController::addText(TextType type)
{
    auto interaction = currentNotationInteraction();
    if (!interaction) {
        return;
    }

    interaction->addText(type);
}

void NotationActionController::addFiguredBass()
{
    auto interaction = currentNotationInteraction();
    if (!interaction) {
        return;
    }

    interaction->addFiguredBass();
}

void NotationActionController::padNote(const Pad& pad)
{
    auto noteInput = currentNotationNoteInput();
    if (!noteInput) {
        return;
    }

    startNoteInputIfNeed();

    noteInput->padNote(pad);
}

void NotationActionController::putNote(const actions::ActionData& data)
{
    auto noteInput = currentNotationNoteInput();
    if (!noteInput) {
        return;
    }

    IF_ASSERT_FAILED(data.count() > 2) {
        return;
    }

    QPoint pos = data.arg<QPoint>(0);
    bool replace = data.arg<bool>(1);
    bool insert = data.arg<bool>(2);

    noteInput->putNote(pos, replace, insert);
}

void NotationActionController::toggleAccidental(AccidentalType type)
{
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

void NotationActionController::putTuplet(int tupletCount)
{
    auto interaction = currentNotationInteraction();
    if (!interaction) {
        return;
    }

    auto noteInput = interaction->noteInput();
    if (!noteInput) {
        return;
    }

    TupletOptions options;
    options.ratio.setNumerator(tupletCount);

    if (noteInput->isNoteInputMode()) {
        noteInput->addTuplet(options);
    } else {
        interaction->addTupletToSelectedChordRests(options);
    }
}

void NotationActionController::addBeamToSelectedChordRests(BeamMode mode)
{
    auto interaction = currentNotationInteraction();
    if (!interaction) {
        return;
    }

    interaction->addBeamToSelectedChordRests(mode);
}

void NotationActionController::addBracketsToSelection(BracketsType type)
{
    auto interaction = currentNotationInteraction();
    if (!interaction) {
        return;
    }

    interaction->addBracketsToSelection(type);
}

void NotationActionController::moveAction(const actions::ActionCode& actionCode)
{
    auto interaction = currentNotationInteraction();
    if (!interaction) {
        return;
    }

    Element* element = interaction->selection()->element();
    if (!element) {
        LOGW() << "no selection element";
        return;
    }

    if (element->isLyrics()) {
        NOT_IMPLEMENTED;
    } else if (element->isTextBase()) {
        moveText(interaction, actionCode);
    } else {
        if ("pitch-up" == actionCode) {
            if (element->isRest()) {
                NOT_IMPLEMENTED << actionCode << ", element->isRest";
            } else {
                interaction->movePitch(MoveDirection::Up, PitchMode::CHROMATIC);
            }
        } else if ("pitch-down" == actionCode) {
            if (element->isRest()) {
                NOT_IMPLEMENTED << actionCode << ", element->isRest";
            } else {
                interaction->movePitch(MoveDirection::Down, PitchMode::CHROMATIC);
            }
        } else if ("pitch-up-octave" == actionCode) {
            interaction->movePitch(MoveDirection::Up, PitchMode::OCTAVE);
        } else if ("pitch-down-octave" == actionCode) {
            interaction->movePitch(MoveDirection::Down, PitchMode::OCTAVE);
        } else if ("next-element" == actionCode) {
            interaction->moveSelection(MoveDirection::Right, MoveSelectionType::Element);
        } else if ("prev-element" == actionCode) {
            interaction->moveSelection(MoveDirection::Left, MoveSelectionType::Element);
        } else if ("next-chord" == actionCode) {
            interaction->moveSelection(MoveDirection::Right, MoveSelectionType::Chord);
        } else if ("prev-chord" == actionCode) {
            interaction->moveSelection(MoveDirection::Left, MoveSelectionType::Chord);
        } else if ("next-measure" == actionCode) {
            interaction->moveSelection(MoveDirection::Right, MoveSelectionType::Measure);
        } else if ("prev-measure" == actionCode) {
            interaction->moveSelection(MoveDirection::Left, MoveSelectionType::Measure);
        } else if ("next-track" == actionCode) {
            interaction->moveSelection(MoveDirection::Right, MoveSelectionType::Track);
        } else if ("prev-track" == actionCode) {
            interaction->moveSelection(MoveDirection::Left, MoveSelectionType::Track);
        } else {
            NOT_SUPPORTED << actionCode;
        }
    }
}

void NotationActionController::moveText(INotationInteractionPtr interaction, const actions::ActionCode& actionCode)
{
    MoveDirection direction = MoveDirection::Undefined;
    bool quickly = false;

    if ("next-chord" == actionCode) {
        direction = MoveDirection::Right;
    } else if ("next-measure" == actionCode) {
        direction = MoveDirection::Right;
        quickly = true;
    } else if ("prev-chord" == actionCode) {
        direction = MoveDirection::Left;
    } else if ("prev-measure" == actionCode) {
        direction = MoveDirection::Left;
        quickly = true;
    } else if ("pitch-up" == actionCode) {
        direction = MoveDirection::Up;
    } else if ("pitch-down" == actionCode) {
        direction = MoveDirection::Down;
    } else if ("pitch-up-octave" == actionCode) {
        direction = MoveDirection::Up;
        quickly = true;
    } else if ("pitch-down-octave" == actionCode) {
        direction = MoveDirection::Down;
        quickly = true;
    } else {
        NOT_SUPPORTED << actionCode;
        return;
    }

    interaction->moveText(direction, quickly);
}

void NotationActionController::swapVoices(int voiceIndex1, int voiceIndex2)
{
    auto interaction = currentNotationInteraction();
    if (!interaction) {
        return;
    }

    interaction->swapVoices(voiceIndex1, voiceIndex2);
}

void NotationActionController::changeVoice(int voiceIndex)
{
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
    copySelection();
    deleteSelection();
}

void NotationActionController::copySelection()
{
    auto interaction = currentNotationInteraction();
    if (!interaction) {
        return;
    }

    interaction->copySelection();
}

void NotationActionController::pasteSelection(PastingType type)
{
    auto interaction = currentNotationInteraction();
    if (!interaction) {
        return;
    }

    Fraction scale = resolvePastingScale(interaction, type);
    interaction->pasteSelection(scale);
}

Fraction NotationActionController::resolvePastingScale(const INotationInteractionPtr& interaction, PastingType type) const
{
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

void NotationActionController::deleteSelection()
{
    auto interaction = currentNotationInteraction();
    if (!interaction) {
        return;
    }

    interaction->deleteSelection();
}

void NotationActionController::swapSelection()
{
    auto interaction = currentNotationInteraction();
    if (!interaction) {
        return;
    }

    interaction->swapSelection();
}

void NotationActionController::flipSelection()
{
    auto interaction = currentNotationInteraction();
    if (!interaction) {
        return;
    }

    interaction->flipSelection();
}

void NotationActionController::addTie()
{
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

void NotationActionController::addSlur()
{
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

void NotationActionController::addInterval(int interval)
{
    auto interaction = currentNotationInteraction();
    if (!interaction) {
        return;
    }

    interaction->addIntervalToSelectedNotes(interval);
}

void NotationActionController::undo()
{
    auto notation = currentNotation();
    if (!notation) {
        return;
    }

    notation->undoStack()->undo();
}

void NotationActionController::redo()
{
    auto notation = currentNotation();
    if (!notation) {
        return;
    }

    notation->undoStack()->redo();
}

void NotationActionController::addChordToSelection(MoveDirection direction)
{
    auto interaction = currentNotationInteraction();
    if (!interaction) {
        return;
    }

    if (interaction->selection()->elements().size() == 1
        && interaction->selection()->elements().front()->type() == ElementType::SLUR) {
        return;
    }

    interaction->addChordToSelection(direction);
}

void NotationActionController::selectAllSimilarElements()
{
    auto notationElements = currentNotationElements();
    auto interaction = currentNotationInteraction();
    if (!notationElements || !interaction) {
        return;
    }

    Element* selectedElement = interaction->selection()->element();
    if (!selectedElement) {
        return;
    }

    FilterElementsOptions options = elementsFilterOptions(selectedElement);
    std::vector<Element*> elements = notationElements->elements(options);
    if (elements.empty()) {
        return;
    }

    interaction->clearSelection();

    interaction->select(elements, SelectType::ADD);
}

void NotationActionController::selectAllSimilarElementsInStaff()
{
    auto notationElements = currentNotationElements();
    auto interaction = currentNotationInteraction();
    if (!notationElements || !interaction) {
        return;
    }

    Element* selectedElement = interaction->selection()->element();
    if (!selectedElement) {
        return;
    }

    FilterElementsOptions options = elementsFilterOptions(selectedElement);
    options.staffStart = selectedElement->staffIdx();
    options.staffEnd = options.staffStart + 1;

    std::vector<Element*> elements = notationElements->elements(options);
    if (elements.empty()) {
        return;
    }

    interaction->clearSelection();

    interaction->select(elements, SelectType::ADD);
}

void NotationActionController::selectAllSimilarElementsInRange()
{
    NOT_IMPLEMENTED;
}

void NotationActionController::selectSection()
{
    auto interaction = currentNotationInteraction();
    if (!interaction) {
        return;
    }

    interaction->selectSection();
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

void NotationActionController::selectAll()
{
    auto interaction = currentNotationInteraction();
    if (!interaction) {
        return;
    }

    interaction->selectAll();
}

void NotationActionController::splitMeasure()
{
    auto interaction = currentNotationInteraction();
    if (!interaction) {
        return;
    }

    interaction->splitSelectedMeasure();
}

void NotationActionController::joinSelectedMeasures()
{
    auto interaction = currentNotationInteraction();
    if (!interaction) {
        return;
    }

    interaction->joinSelectedMeasures();
}

void NotationActionController::insertBoxes(BoxType boxType, int count)
{
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
    insertBoxes(boxType, 1);
}

int NotationActionController::firstSelectedBoxIndex() const
{
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
    auto interaction = currentNotationInteraction();
    if (!interaction) {
        return;
    }

    interaction->addBoxes(boxType, count);
}

void NotationActionController::appendBox(BoxType boxType)
{
    appendBoxes(boxType, 1);
}

void NotationActionController::addOttava(OttavaType type)
{
    auto interaction = currentNotationInteraction();
    if (!interaction) {
        return;
    }

    interaction->addOttavaToSelection(type);
}

void NotationActionController::addHairpin(HairpinType type)
{
    auto interaction = currentNotationInteraction();
    if (!interaction) {
        return;
    }

    interaction->addHairpinToSelection(type);
}

void NotationActionController::addAnchoredNoteLine()
{
    auto interaction = currentNotationInteraction();
    if (!interaction) {
        return;
    }

    interaction->addAnchoredLineToSelectedNotes();
}

void NotationActionController::explodeSelectedStaff()
{
    auto interaction = currentNotationInteraction();
    if (!interaction) {
        return;
    }

    interaction->explodeSelectedStaff();
}

void NotationActionController::implodeSelectedStaff()
{
    auto interaction = currentNotationInteraction();
    if (!interaction) {
        return;
    }

    interaction->implodeSelectedStaff();
}

void NotationActionController::realizeSelectedChordSymbols()
{
    auto interaction = currentNotationInteraction();
    if (!interaction) {
        return;
    }

    interaction->realizeSelectedChordSymbols();
}

void NotationActionController::removeSelectedRange()
{
    auto interaction = currentNotationInteraction();
    if (!interaction) {
        return;
    }

    interaction->removeSelectedRange();
}

void NotationActionController::removeEmptyTrailingMeasures()
{
    auto interaction = currentNotationInteraction();
    if (!interaction) {
        return;
    }

    interaction->removeEmptyTrailingMeasures();
}

void NotationActionController::fillSelectionWithSlashes()
{
    auto interaction = currentNotationInteraction();
    if (!interaction) {
        return;
    }

    interaction->fillSelectionWithSlashes();
}

void NotationActionController::replaceSelectedNotesWithSlashes()
{
    auto interaction = currentNotationInteraction();
    if (!interaction) {
        return;
    }

    interaction->replaceSelectedNotesWithSlashes();
}

void NotationActionController::spellPitches()
{
    auto interaction = currentNotationInteraction();
    if (!interaction) {
        return;
    }

    interaction->spellPitches();
}

void NotationActionController::regroupNotesAndRests()
{
    auto interaction = currentNotationInteraction();
    if (!interaction) {
        return;
    }

    interaction->regroupNotesAndRests();
}

void NotationActionController::resequenceRehearsalMarks()
{
    auto interaction = currentNotationInteraction();
    if (!interaction) {
        return;
    }

    interaction->resequenceRehearsalMarks();
}

void NotationActionController::unrollRepeats()
{
    auto interaction = currentNotationInteraction();
    if (!interaction) {
        return;
    }

    interaction->unrollRepeats();
}

void NotationActionController::copyLyrics()
{
    auto interaction = currentNotationInteraction();
    if (!interaction) {
        return;
    }

    interaction->copyLyrics();
}

void NotationActionController::addGraceNotesToSelectedNotes(GraceNoteType type)
{
    auto interaction = currentNotationInteraction();
    if (!interaction) {
        return;
    }

    interaction->addGraceNotesToSelectedNotes(type);
}

void NotationActionController::addStretch(qreal value)
{
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

    interaction->resetToDefault(ResettableValueType::Stretch);
}

void NotationActionController::resetTextStyleOverrides()
{
    auto interaction = currentNotationInteraction();
    if (!interaction) {
        return;
    }

    interaction->resetToDefault(ResettableValueType::TextStyleOverriders);
}

void NotationActionController::resetBeamMode()
{
    auto interaction = currentNotationInteraction();
    if (!interaction) {
        return;
    }

    auto selection = currentNotationSelection();
    if (!selection) {
        return;
    }

    if (selection->isNone() || selection->isRange()) {
        interaction->resetToDefault(ResettableValueType::BeamMode);
    }
}

void NotationActionController::resetShapesAndPosition()
{
    auto interaction = currentNotationInteraction();
    if (!interaction) {
        return;
    }

    auto selection = currentNotationSelection();
    if (!selection) {
        return;
    }

    if (selection->isNone()) {
        return;
    }

    interaction->resetToDefault(ResettableValueType::TextStyleOverriders);
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

void NotationActionController::openPageStyle()
{
    interactive()->open("musescore://notation/style");
}

void NotationActionController::openStaffProperties()
{
    auto interaction = currentNotationInteraction();
    if (!interaction) {
        return;
    }

    int staffIdx = interaction->selection()->range()->startStaffIndex();
    interactive()->open("musescore://notation/staffproperties?staffIdx=" + QString::number(staffIdx).toStdString());
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

FilterElementsOptions NotationActionController::elementsFilterOptions(const Element* element) const
{
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

bool NotationActionController::isTextEditting() const
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

void NotationActionController::toggleScoreConfig(ScoreConfigType configType)
{
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
}

void NotationActionController::startNoteInputIfNeed()
{
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
