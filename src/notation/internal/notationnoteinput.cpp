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
#include "libmscore/articulation.h"
#include "libmscore/system.h"
#include "libmscore/stafftype.h"

#include "scorecallbacks.h"

using namespace mu::notation;
using namespace mu::async;

NotationNoteInput::NotationNoteInput(const IGetScore* getScore, INotationInteraction* interaction, INotationUndoStackPtr undoStack)
    : m_getScore(getScore), m_interaction(interaction), m_undoStack(undoStack)
{
    m_scoreCallbacks = new ScoreCallbacks();

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
    Ms::InputState& inputState = score()->inputState();

    NoteInputState noteInputState;
    noteInputState.method = inputState.noteEntryMethod();
    noteInputState.duration = inputState.duration();
    noteInputState.accidentalType = inputState.accidentalType();
    noteInputState.articulationIds = articulationIds();
    noteInputState.withSlur = inputState.slur() != nullptr;
    noteInputState.currentVoiceIndex = inputState.voice();
    noteInputState.isRest = inputState.rest();

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

    m_interaction->select({ el }, SelectType::SINGLE, 0);

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

    notifyAboutStateChanged();
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

    notifyAboutStateChanged();
}

void NotationNoteInput::toggleNoteInputMethod(NoteInputMethod method)
{
    Ms::InputState& inputState = score()->inputState();
    inputState.setNoteEntryMethod(method);

    notifyAboutStateChanged();
}

void NotationNoteInput::addNote(NoteName noteName, NoteAddingMode addingMode)
{
    Ms::EditData editData;
    editData.view = m_scoreCallbacks;

    startEdit();
    int inote = static_cast<int>(noteName);
    bool addToUpOnCurrentChord = addingMode == NoteAddingMode::CurrentChord;
    bool insertNewChord = addingMode == NoteAddingMode::InsertChord;
    score()->cmdAddPitch(editData, inote, addToUpOnCurrentChord, insertNewChord);
    apply();

    notifyAboutStateChanged();
}

void NotationNoteInput::padNote(const Pad& pad)
{
    Ms::EditData ed;
    ed.view = m_scoreCallbacks;

    startEdit();
    score()->padToggle(pad, ed);
    apply();

    notifyAboutStateChanged();
}

void NotationNoteInput::putNote(const QPointF& pos, bool replace, bool insert)
{
    startEdit();
    score()->putNote(pos, replace, insert);
    apply();

    notifyNoteAddedChanged();
}

void NotationNoteInput::setAccidental(AccidentalType accidentalType)
{
    Ms::EditData editData;
    editData.view = m_scoreCallbacks;

    score()->toggleAccidental(accidentalType, editData);

    notifyAboutStateChanged();
}

void NotationNoteInput::setArticulation(SymbolId articulationSymbolId)
{
    Ms::InputState& inputState = score()->inputState();

    std::set<SymbolId> articulations = Ms::updateArticulations(
        inputState.articulationIds(), articulationSymbolId, Ms::ArticulationsUpdateMode::Remove);
    inputState.setArticulationIds(articulations);

    notifyAboutStateChanged();
}

void NotationNoteInput::setCurrentVoiceIndex(int voiceIndex)
{
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

void NotationNoteInput::addTuplet(const TupletOptions& options)
{
    Ms::InputState& inputState = score()->inputState();

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

QRectF NotationNoteInput::cursorRect() const
{
    if (!isNoteInputMode()) {
        return QRectF();
    }

    Ms::InputState& inputState = score()->inputState();
    Ms::Segment* segment = inputState.segment();
    if (!segment) {
        return QRectF();
    }

    Ms::System* system = segment->measure()->system();
    if (!system) {
        return QRectF();
    }

    int track = inputState.track() == -1 ? 0 : inputState.track();
    int staffIdx = track / VOICES;

    Staff* staff = score()->staff(staffIdx);
    if (!staff) {
        return QRectF();
    }

    constexpr int sideMargin = 4;
    constexpr int skylineMargin = 20;

    QRectF segmentContentRect = segment->contentRect();
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

    QRectF result = QRectF(x, y, w, h);

    if (configuration()->canvasOrientation().val == framework::Orientation::Horizontal) {
        result.translate(system->page()->pos());
    }

    return result;
}

void NotationNoteInput::addSlur(Ms::Slur* slur)
{
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
    Ms::InputState& inputState = score()->inputState();
    Ms::Slur* slur = inputState.slur();
    if (!slur) {
        return;
    }

    startEdit();
    score()->removeElement(slur);
    apply();

    addSlur(nullptr);
}

void NotationNoteInput::addTie()
{
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
    Ms::InputState& inputState = score()->inputState();
    return Ms::splitArticulations(inputState.articulationIds());
}
