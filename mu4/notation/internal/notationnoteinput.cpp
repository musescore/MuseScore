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
#include "notationnoteinput.h"

#include "libmscore/score.h"
#include "libmscore/input.h"
#include "libmscore/staff.h"
#include "libmscore/note.h"
#include "libmscore/chord.h"
#include "libmscore/slur.h"

#include "scorecallbacks.h"

using namespace mu::notation;
using namespace mu::async;

NotationNoteInput::NotationNoteInput(const IGetScore* getScore, INotationInteraction* interaction, INotationUndoStackPtr undoStack)
    : m_getScore(getScore), m_interaction(interaction), m_undoStack(undoStack)
{
    m_scoreCallbacks = new ScoreCallbacks();

    m_interaction->selectionChanged().onNotify(this, [this]() {
        updateInputState();
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
    NoteInputState noteInputState;
    noteInputState.method = score()->inputState().noteEntryMethod();
    noteInputState.duration = score()->inputState().duration();
    noteInputState.accidentalType = score()->inputState().accidentalType();
    noteInputState.withSlur = score()->inputState().slur() != nullptr;

    return noteInputState;
}

void NotationNoteInput::startNoteInput()
{
    //! NOTE Coped from `void ScoreView::startNoteEntry()`
    Ms::InputState& is = score()->inputState();
    is.setSegment(0);

    //! TODO Find out what does and why.
    Element* el = score()->selection().element();
    if (!el) {
        el = score()->selection().firstChordRest();
    }

    if (el == nullptr
        || (el->type() != ElementType::CHORD && el->type() != ElementType::REST && el->type() != ElementType::NOTE)) {
        // if no note/rest is selected, start with voice 0
        int track = is.track() == -1 ? 0 : (is.track() / VOICES) * VOICES;
        // try to find an appropriate measure to start in
        Fraction tick = el ? el->tick() : Fraction(0,1);
        el = score()->searchNote(tick, track);
        if (!el) {
            el = score()->searchNote(Fraction(0,1), track);
        }
    }

    if (!el) {
        return;
    }

    if (el->type() == ElementType::CHORD) {
        Ms::Chord* c = static_cast<Ms::Chord*>(el);
        Ms::Note* note = c->selectedNote();
        if (note == 0) {
            note = c->upNote();
        }
        el = note;
    }
    //! ---

    Duration d(is.duration());
    if (!d.isValid() || d.isZero() || d.type() == Duration::DurationType::V_MEASURE) {
        is.setDuration(Duration(Duration::DurationType::V_QUARTER));
    }
    is.setAccidentalType(Ms::AccidentalType::NONE);

    m_interaction->select(el, SelectType::SINGLE, 0);

    is.setRest(false);
    is.setNoteEntryMode(true);

    //! TODO Find out why.
    score()->setUpdateAll();
    score()->update();
    //! ---

    Staff* staff = score()->staff(is.track() / VOICES);
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

    m_stateChanged.notify();
}

void NotationNoteInput::endNoteInput()
{
    Ms::InputState& is = score()->inputState();
    is.setNoteEntryMode(false);
    if (is.slur()) {
        const std::vector<Ms::SpannerSegment*>& el = is.slur()->spannerSegments();
        if (!el.empty()) {
            el.front()->setSelected(false);
        }
        is.setSlur(0);
    }

    m_stateChanged.notify();
}

void NotationNoteInput::toggleNoteInputMethod(NoteInputMethod method)
{
    Ms::InputState& inputState = score()->inputState();
    inputState.setNoteEntryMethod(method);

    m_stateChanged.notify();
}

void NotationNoteInput::addNote(NoteName noteName, NoteAddingMode addingMode)
{
    if (!isNoteInputMode()) {
        startNoteInput();
    }

    Ms::EditData editData;
    editData.view = m_scoreCallbacks;

    m_undoStack->prepareChanges();
    int inote = static_cast<int>(noteName);
    bool addToUpOnCurrentChord = addingMode == NoteAddingMode::CurrentChord;
    bool insertNewChord = addingMode == NoteAddingMode::InsertChord;
    score()->cmdAddPitch(editData, inote, addToUpOnCurrentChord, insertNewChord);
    m_undoStack->commitChanges();

    m_stateChanged.notify();
}

void NotationNoteInput::padNote(const Pad& pad)
{
    Ms::EditData ed;
    ed.view = m_scoreCallbacks;

    m_undoStack->prepareChanges();
    score()->padToggle(pad, ed);
    m_undoStack->commitChanges();

    m_stateChanged.notify();
}

void NotationNoteInput::putNote(const QPointF& pos, bool replace, bool insert)
{
    m_undoStack->prepareChanges();
    score()->putNote(pos, replace, insert);
    m_undoStack->commitChanges();

    m_noteAdded.notify();
}

void NotationNoteInput::toogleAccidental(AccidentalType accidentalType)
{
    Ms::EditData editData;
    editData.view = m_scoreCallbacks;

    score()->toggleAccidental(accidentalType, editData);

    m_stateChanged.notify();
}

void NotationNoteInput::addSlur(Ms::Slur* slur)
{
    Ms::InputState& inputState = score()->inputState();
    inputState.setSlur(slur);

    std::vector<Ms::SpannerSegment*> slurSpannerSegments = slur->spannerSegments();
    if (!slurSpannerSegments.empty()) {
        slurSpannerSegments.back()->setSelected(true);
    }
}

void NotationNoteInput::resetSlur()
{
    Ms::InputState& inputState = score()->inputState();
    if (!inputState.slur()) {
        return;
    }

    const std::vector<Ms::SpannerSegment*>& el = inputState.slur()->spannerSegments();
    if (!el.empty()) {
        el.front()->setSelected(false);
    }

    inputState.setSlur(nullptr);
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

void NotationNoteInput::updateInputState()
{
    score()->inputState().update(score()->selection());

    m_stateChanged.notify();
}
