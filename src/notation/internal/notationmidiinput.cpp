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
#include "notationmidiinput.h"

#include <QGuiApplication>

#include "log.h"

#include "libmscore/note.h"
#include "libmscore/segment.h"

using namespace mu::notation;

static constexpr int PLAY_INTERVAL = 20;

NotationMidiInput::NotationMidiInput(IGetScore* getScore, INotationInteractionPtr notationInteraction, INotationUndoStackPtr undoStack)
    : m_getScore(getScore), m_notationInteraction(notationInteraction), m_undoStack(undoStack)
{
    QObject::connect(&m_playTimer, &QTimer::timeout, [this]() { doPlayNotes(); });
}

void NotationMidiInput::onMidiEventsReceived(const std::vector<midi::Event>& events)
{
    std::vector<midi::Event> midi10Events;
    for (const midi::Event& event : events) {
        std::vector<midi::Event> _events = { event };
        if (event.isChannelVoice20()) {
            _events = event.toMIDI10();
        }

        for (const midi::Event& event : _events) {
            if (event.opcode() == midi::Event::Opcode::NoteOn || event.opcode() == midi::Event::Opcode::NoteOff) {
                midi10Events.push_back(event);
            }
        }
    }

    for (const midi::Event& event : midi10Events) {
        Note* note = onAddNote(event);
        if (note) {
            m_playNotesQueue.push_back(note);
        }
    }

    if (m_playNotesQueue.empty()) {
        return;
    }

    if (!m_playTimer.isActive()) {
        m_playTimer.start(PLAY_INTERVAL);
    }
}

mu::engraving::Score* NotationMidiInput::score() const
{
    IF_ASSERT_FAILED(m_getScore) {
        return nullptr;
    }
    return m_getScore->score();
}

void NotationMidiInput::doPlayNotes()
{
    if (!m_playNotesQueue.empty()) {
        playbackController()->playElements(m_playNotesQueue);
    }

    m_playNotesQueue.clear();
    m_playTimer.stop();
}

Note* NotationMidiInput::onAddNote(const midi::Event& e)
{
    mu::engraving::Score* sc = score();
    if (!sc) {
        return nullptr;
    }

    mu::engraving::MidiInputEvent inputEv;
    inputEv.pitch = e.note();
    inputEv.velocity = e.velocity();

    sc->activeMidiPitches().remove_if([&inputEv](const mu::engraving::MidiInputEvent& val) {
        return inputEv.pitch == val.pitch;
    });

    if (e.opcode() == midi::Event::Opcode::NoteOff || e.velocity() == 0) {
        return nullptr;
    }

    const mu::engraving::InputState& is = sc->inputState();
    if (!is.noteEntryMode()) {
        return nullptr;
    }

    if (sc->activeMidiPitches().empty()) {
        inputEv.chord = false;
    } else {
        inputEv.chord = true;
    }

    // holding shift while inputting midi will add the new pitch to the prior existing chord
    if (QGuiApplication::keyboardModifiers() & Qt::ShiftModifier) {
        mu::engraving::EngravingItem* cr = is.lastSegment()->element(is.track());
        if (cr && cr->isChord()) {
            inputEv.chord = true;
        }
    }

    m_undoStack->prepareChanges();

    mu::engraving::Note* note = sc->addMidiPitch(inputEv.pitch, inputEv.chord);

    sc->activeMidiPitches().push_back(inputEv);
    m_undoStack->commitChanges();

    m_noteChanged.notify();

    m_notationInteraction->showItem(is.cr());

    return note;
}

mu::async::Notification NotationMidiInput::noteChanged() const
{
    return m_noteChanged;
}
