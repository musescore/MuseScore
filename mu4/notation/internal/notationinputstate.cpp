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
#include "notationinputstate.h"

#include "libmscore/score.h"
#include "libmscore/input.h"
#include "libmscore/staff.h"
#include "libmscore/note.h"
#include "libmscore/chord.h"
#include "libmscore/slur.h"

#include "scorecallbacks.h"

using namespace mu::notation;
using namespace mu::async;

NotationInputState::NotationInputState(const IGetScore* getScore, INotationUndoStackPtr undoStack,
                                       Notification selectionChangedNotification)
    : m_getScore(getScore), m_undoStack(undoStack)
{
    m_scoreCallbacks = new ScoreCallbacks();

    selectionChangedNotification.onNotify(this, [this]() {
        updateInputState();
    });
}

NotationInputState::~NotationInputState()
{
    delete m_scoreCallbacks;
}

bool NotationInputState::isNoteEnterMode() const
{
    return score()->inputState().noteEntryMode();
}

bool NotationInputState::isPadActive(Pad pad) const
{
    switch (pad) {
    case Pad::NOTE00: return isDurationActive(DurationType::V_LONG);
    case Pad::NOTE0: return isDurationActive(DurationType::V_BREVE);
    case Pad::NOTE1: return isDurationActive(DurationType::V_WHOLE);
    case Pad::NOTE2: return isDurationActive(DurationType::V_HALF);
    case Pad::NOTE4: return isDurationActive(DurationType::V_QUARTER);
    case Pad::NOTE8: return isDurationActive(DurationType::V_EIGHTH);
    case Pad::NOTE16: return isDurationActive(DurationType::V_16TH);
    case Pad::NOTE32: return isDurationActive(DurationType::V_32ND);
    case Pad::NOTE64: return isDurationActive(DurationType::V_64TH);
    case Pad::NOTE128: return isDurationActive(DurationType::V_128TH);
    case Pad::NOTE256: return isDurationActive(DurationType::V_256TH);
    case Pad::NOTE512: return isDurationActive(DurationType::V_512TH);
    case Pad::NOTE1024: return isDurationActive(DurationType::V_1024TH);
    case Pad::REST: /*todo*/ return false;
    case Pad::DOT: /*todo*/ return false;
    case Pad::DOTDOT: /*todo*/ return false;
    case Pad::DOT3: /*todo*/ return false;
    case Pad::DOT4: /*todo*/ return false;
    }

    return false;
}

Duration NotationInputState::duration() const
{
    return score()->inputState().duration();
}

void NotationInputState::startNoteEntry()
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

    score()->select(el, SelectType::SINGLE, 0); // todoooo

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

void NotationInputState::endNoteEntry()
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

void NotationInputState::padNote(const Pad& pad)
{
    Ms::EditData ed;
    ed.view = m_scoreCallbacks;

    m_undoStack->prepareChanges();
    score()->padToggle(pad, ed);
    m_undoStack->commitChanges();

    m_stateChanged.notify();
}

void NotationInputState::putNote(const QPointF& pos, bool replace, bool insert)
{
    m_undoStack->prepareChanges();
    score()->putNote(pos, replace, insert);
    m_undoStack->commitChanges();

    m_noteAdded.notify();
}

Notification NotationInputState::noteAdded() const
{
    return m_noteAdded;
}

Notification NotationInputState::stateChanged() const
{
    return m_stateChanged;
}

Ms::Score* NotationInputState::score() const
{
    return m_getScore->score();
}

bool NotationInputState::isDurationActive(DurationType durationType) const
{
    return duration() == durationType;
}

void NotationInputState::updateInputState()
{
    score()->inputState().update(score()->selection());

    m_stateChanged.notify();
}
