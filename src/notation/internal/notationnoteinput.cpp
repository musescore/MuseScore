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
#include "notationnoteinput.h"

#include "libmscore/masterscore.h"
#include "libmscore/input.h"
#include "libmscore/staff.h"
#include "libmscore/note.h"
#include "libmscore/chord.h"
#include "libmscore/slur.h"
#include "libmscore/articulation.h"
#include "libmscore/system.h"
#include "libmscore/stafftype.h"

#include "scorecallbacks.h"

#include "log.h"

using namespace mu::notation;
using namespace mu::async;

NotationNoteInput::NotationNoteInput(const IGetScore* getScore, INotationInteraction* interaction, INotationUndoStackPtr undoStack)
    : m_getScore(getScore), m_interaction(interaction), m_undoStack(undoStack)
{
    m_scoreCallbacks = new ScoreCallbacks();
    m_scoreCallbacks->setGetScore(getScore);
    m_scoreCallbacks->setNotationInteraction(interaction);

    m_interaction->selectionChanged().onNotify(this, [this]() {
        if (!isNoteInputMode()) {
            updateInputState();
        }
    });
}

NotationNoteInput::~NotationNoteInput()
{
    delete m_scoreCallbacks;
}

bool NotationNoteInput::isNoteInputMode() const
{
    return score()->inputState().noteEntryMode();
}

NoteInputState NotationNoteInput::state() const
{
    const Ms::InputState& inputState = score()->inputState();

    NoteInputState noteInputState;
    noteInputState.method = inputState.noteEntryMethod();
    noteInputState.duration = inputState.duration();
    noteInputState.accidentalType = inputState.accidentalType();
    noteInputState.articulationIds = articulationIds();
    noteInputState.withSlur = inputState.slur() != nullptr;
    noteInputState.currentVoiceIndex = inputState.voice();
    noteInputState.currentTrack = inputState.track();
    noteInputState.drumset = inputState.drumset();
    noteInputState.isRest = inputState.rest();
    noteInputState.staffGroup = inputState.staffGroup();

    return noteInputState;
}

//! NOTE Coped from `void ScoreView::startNoteEntry()`
void NotationNoteInput::startNoteInput()
{
    TRACEFUNC;

    if (isNoteInputMode()) {
        return;
    }

    Ms::EngravingItem* el = resolveNoteInputStartPosition();
    if (!el) {
        return;
    }

    m_interaction->select({ el }, SelectType::SINGLE, 0);

    Ms::InputState& is = score()->inputState();

    // Not strictly necessary, just for safety
    if (is.noteEntryMethod() == Ms::NoteEntryMethod::UNKNOWN) {
        is.setNoteEntryMethod(Ms::NoteEntryMethod::STEPTIME);
    }

    Duration d(is.duration());
    if (!d.isValid() || d.isZero() || d.type() == DurationType::V_MEASURE) {
        is.setDuration(Duration(DurationType::V_QUARTER));
    }
    is.setAccidentalType(Ms::AccidentalType::NONE);

    is.setRest(false);
    is.setNoteEntryMode(true);

    //! TODO Find out why.
    score()->setUpdateAll();
    score()->update();
    //! ---

    const Staff* staff = score()->staff(is.track() / Ms::VOICES);
    switch (staff->staffType(is.tick())->group()) {
    case Ms::StaffGroup::STANDARD:
        break;
    case Ms::StaffGroup::TAB: {
        int strg = 0;                           // assume topmost string as current string
        // if entering note entry with a note selected and the note has a string
        // set InputState::_string to note physical string
        if (el->type() == ElementType::NOTE) {
            strg = (static_cast<Ms::Note*>(el))->string();
        }
        is.setString(strg);
        break;
    }
    case Ms::StaffGroup::PERCUSSION:
        break;
    }

    notifyAboutStateChanged();

    m_scoreCallbacks->adjustCanvasPosition(el, false);
}

Ms::EngravingItem* NotationNoteInput::resolveNoteInputStartPosition() const
{
    EngravingItem* el = score()->selection().element();
    if (!el) {
        el = score()->selection().firstChordRest();
    }

    const Ms::InputState& is = score()->inputState();

    if (!el) {
        if (const Ms::Segment* segment = is.lastSegment()) {
            el = segment->element(is.track());
        }
    }

    if (el == nullptr
        || (el->type() != ElementType::CHORD && el->type() != ElementType::REST && el->type() != ElementType::NOTE)) {
        // if no note/rest is selected, start with voice 0
        int track = is.track() == -1 ? 0 : (is.track() / Ms::VOICES) * Ms::VOICES;
        // try to find an appropriate measure to start in
        Fraction tick = el ? el->tick() : Fraction(0, 1);
        el = score()->searchNote(tick, track);
        if (!el) {
            el = score()->searchNote(Fraction(0, 1), track);
        }
    }

    if (!el) {
        return nullptr;
    }

    if (el->type() == ElementType::CHORD) {
        Ms::Chord* c = static_cast<Ms::Chord*>(el);
        Ms::Note* note = c->selectedNote();
        if (note == 0) {
            note = c->upNote();
        }
        el = note;
    }

    return el;
}

void NotationNoteInput::endNoteInput()
{
    TRACEFUNC;

    if (!isNoteInputMode()) {
        return;
    }

    Ms::InputState& is = score()->inputState();
    is.setNoteEntryMode(false);

    if (is.slur()) {
        const std::vector<Ms::SpannerSegment*>& el = is.slur()->spannerSegments();
        if (!el.empty()) {
            el.front()->setSelected(false);
        }
        is.setSlur(0);
    }

    updateInputState();
}

void NotationNoteInput::toggleNoteInputMethod(NoteInputMethod method)
{
    TRACEFUNC;

    score()->inputState().setNoteEntryMethod(method);

    notifyAboutStateChanged();
}

void NotationNoteInput::addNote(NoteName noteName, NoteAddingMode addingMode)
{
    TRACEFUNC;

    Ms::EditData editData(m_scoreCallbacks);

    startEdit();
    int inote = static_cast<int>(noteName);
    bool addToUpOnCurrentChord = addingMode == NoteAddingMode::CurrentChord;
    bool insertNewChord = addingMode == NoteAddingMode::InsertChord;
    score()->cmdAddPitch(editData, inote, addToUpOnCurrentChord, insertNewChord);
    apply();

    notifyNoteAddedChanged();
    notifyAboutStateChanged();
}

void NotationNoteInput::padNote(const Pad& pad)
{
    TRACEFUNC;

    Ms::EditData editData(m_scoreCallbacks);

    startEdit();
    score()->padToggle(pad, editData);
    apply();

    notifyAboutStateChanged();
}

void NotationNoteInput::putNote(const PointF& pos, bool replace, bool insert)
{
    TRACEFUNC;

    startEdit();
    score()->putNote(pos, replace, insert);
    apply();

    notifyNoteAddedChanged();
    notifyAboutStateChanged();

    if (Ms::ChordRest* chordRest = score()->inputState().cr()) {
        m_scoreCallbacks->adjustCanvasPosition(chordRest, false);
    }
}

void NotationNoteInput::removeNote(const PointF& pos)
{
    TRACEFUNC;

    Ms::InputState& inputState = score()->inputState();
    bool restMode = inputState.rest();

    startEdit();
    inputState.setRest(!restMode);
    score()->putNote(pos, false, false);
    inputState.setRest(restMode);
    apply();

    notifyAboutStateChanged();
}

void NotationNoteInput::setAccidental(AccidentalType accidentalType)
{
    TRACEFUNC;

    Ms::EditData editData(m_scoreCallbacks);

    score()->toggleAccidental(accidentalType, editData);

    notifyAboutStateChanged();
}

void NotationNoteInput::setArticulation(SymbolId articulationSymbolId)
{
    TRACEFUNC;

    Ms::InputState& inputState = score()->inputState();

    std::set<SymbolId> articulations = Ms::updateArticulations(
        inputState.articulationIds(), articulationSymbolId, Ms::ArticulationsUpdateMode::Remove);
    inputState.setArticulationIds(articulations);

    notifyAboutStateChanged();
}

void NotationNoteInput::setDrumNote(int note)
{
    TRACEFUNC;

    score()->inputState().setDrumNote(note);
    notifyAboutStateChanged();
}

void NotationNoteInput::setCurrentVoiceIndex(int voiceIndex)
{
    TRACEFUNC;

    if (!isVoiceIndexValid(voiceIndex)) {
        return;
    }

    Ms::InputState& inputState = score()->inputState();
    inputState.setVoice(voiceIndex);

    if (inputState.segment()) {
        Ms::Segment* segment = inputState.segment()->measure()->first(Ms::SegmentType::ChordRest);
        inputState.setSegment(segment);
    }

    notifyAboutStateChanged();
}

void NotationNoteInput::resetInputPosition()
{
    Ms::InputState& inputState = score()->inputState();

    inputState.setTrack(-1);
    inputState.setString(-1);
    inputState.setSegment(nullptr);

    notifyAboutStateChanged();
}

void NotationNoteInput::addTuplet(const TupletOptions& options)
{
    TRACEFUNC;

    const Ms::InputState& inputState = score()->inputState();

    startEdit();
    score()->expandVoice();
    Ms::ChordRest* chordRest = inputState.cr();
    if (chordRest) {
        score()->changeCRlen(chordRest, inputState.duration());
        score()->addTuplet(chordRest, options.ratio, options.numberType, options.bracketType);
    }
    apply();

    notifyAboutStateChanged();
}

mu::RectF NotationNoteInput::cursorRect() const
{
    TRACEFUNC;

    if (!isNoteInputMode()) {
        return {};
    }

    const Ms::InputState& inputState = score()->inputState();
    const Ms::Segment* segment = inputState.segment();
    if (!segment) {
        return {};
    }

    Ms::System* system = segment->measure()->system();
    if (!system) {
        return {};
    }

    int track = inputState.track() == -1 ? 0 : inputState.track();
    int staffIdx = track / Ms::VOICES;

    const Staff* staff = score()->staff(staffIdx);
    if (!staff) {
        return {};
    }

    constexpr int sideMargin = 4;
    constexpr int skylineMargin = 20;

    RectF segmentContentRect = segment->contentRect();
    double x = segmentContentRect.translated(segment->pagePos()).x() - sideMargin;
    double y = system->staffYpage(staffIdx) + system->page()->pos().y();
    double w = segmentContentRect.width() + 2 * sideMargin;
    double h = 0.0;

    const Ms::StaffType* staffType = staff->staffType(inputState.tick());
    double spatium = score()->spatium();
    double lineDist = staffType->lineDistance().val() * spatium;
    int lines = staffType->lines();
    int inputStateStringsCount = inputState.string();

    int instrumentStringsCount = staff->part()->instrument()->stringData()->strings();
    if (staff->isTabStaff(inputState.tick()) && inputStateStringsCount >= 0 && inputStateStringsCount <= instrumentStringsCount) {
        h = lineDist;
        y += staffType->physStringToYOffset(inputStateStringsCount) * spatium;
        y -= (staffType->onLines() ? lineDist * 0.5 : lineDist);
    } else {
        h = (lines - 1) * lineDist + 2 * skylineMargin;
        y -= skylineMargin;
    }

    RectF result = RectF(x, y, w, h);

    if (configuration()->canvasOrientation().val == framework::Orientation::Horizontal) {
        result.translate(system->page()->pos());
    }

    return result;
}

void NotationNoteInput::addSlur(Ms::Slur* slur)
{
    TRACEFUNC;

    Ms::InputState& inputState = score()->inputState();
    inputState.setSlur(slur);

    if (slur) {
        std::vector<Ms::SpannerSegment*> slurSpannerSegments = slur->spannerSegments();
        if (!slurSpannerSegments.empty()) {
            slurSpannerSegments.front()->setSelected(true);
        }
    }

    notifyAboutStateChanged();
}

void NotationNoteInput::resetSlur()
{
    TRACEFUNC;

    Ms::InputState& inputState = score()->inputState();
    Ms::Slur* slur = inputState.slur();
    if (!slur) {
        return;
    }

    score()->deselect(slur);

    addSlur(nullptr);
}

void NotationNoteInput::addTie()
{
    TRACEFUNC;

    startEdit();
    score()->cmdAddTie();
    apply();

    notifyAboutStateChanged();
}

Notification NotationNoteInput::noteAdded() const
{
    return m_noteAdded;
}

Notification NotationNoteInput::stateChanged() const
{
    return m_stateChanged;
}

Ms::Score* NotationNoteInput::score() const
{
    return m_getScore->score();
}

void NotationNoteInput::startEdit()
{
    m_undoStack->prepareChanges();
}

void NotationNoteInput::apply()
{
    m_undoStack->commitChanges();
}

void NotationNoteInput::updateInputState()
{
    TRACEFUNC;

    score()->inputState().update(score()->selection());

    notifyAboutStateChanged();
}

void NotationNoteInput::notifyAboutStateChanged()
{
    m_stateChanged.notify();
}

void NotationNoteInput::notifyNoteAddedChanged()
{
    m_noteAdded.notify();
}

std::set<SymbolId> NotationNoteInput::articulationIds() const
{
    const Ms::InputState& inputState = score()->inputState();
    return Ms::splitArticulations(inputState.articulationIds());
}

void NotationNoteInput::doubleNoteInputDuration()
{
    TRACEFUNC;

    Ms::EditData editData(m_scoreCallbacks);

    startEdit();
    score()->cmdPadNoteIncreaseTAB(editData);
    apply();

    notifyAboutStateChanged();
}

void NotationNoteInput::halveNoteInputDuration()
{
    TRACEFUNC;

    Ms::EditData editData(m_scoreCallbacks);

    startEdit();
    score()->cmdPadNoteDecreaseTAB(editData);
    apply();

    notifyAboutStateChanged();
}
