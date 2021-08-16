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

#include "libmscore/masterscore.h"
#include "libmscore/segment.h"

using namespace mu::notation;

NotationMidiInput::NotationMidiInput(IGetScore* getScore, INotationUndoStackPtr undoStack)
    : m_getScore(getScore), m_undoStack(undoStack)
{
}

Ms::Score* NotationMidiInput::score() const
{
    IF_ASSERT_FAILED(m_getScore) {
        return nullptr;
    }
    return m_getScore->score();
}

void NotationMidiInput::onMidiEventReceived(const midi::Event& e)
{
    LOGI() << e.to_string();

    if (e.isChannelVoice20()) {
        auto events = e.toMIDI10();
        for (auto& event : events) {
            onMidiEventReceived(event);
        }
    }

    if (e.opcode() == midi::Event::Opcode::NoteOn || e.opcode() == midi::Event::Opcode::NoteOff) {
        onNoteReceived(e);
    }
}

void NotationMidiInput::onNoteReceived(const midi::Event& e)
{
    Ms::Score* sc = score();
    if (!sc) {
        return;
    }

    Ms::MidiInputEvent inputEv;
    inputEv.pitch = e.note();
    inputEv.velocity = e.velocity();

    sc->activeMidiPitches()->remove_if([&inputEv](const Ms::MidiInputEvent& val) {
        return inputEv.pitch == val.pitch;
    });

    if (e.opcode() == midi::Event::Opcode::NoteOff || e.velocity() == 0) {
        return;
    }

    const Ms::InputState& is = sc->inputState();
    if (!is.noteEntryMode()) {
        return;
    }

    if (sc->activeMidiPitches()->empty()) {
        inputEv.chord = false;
    } else {
        inputEv.chord = true;
    }

    // holding shift while inputting midi will add the new pitch to the prior existing chord
    if (QGuiApplication::keyboardModifiers() & Qt::ShiftModifier) {
        Ms::Element* cr = is.lastSegment()->element(is.track());
        if (cr && cr->isChord()) {
            inputEv.chord = true;
        }
    }

    m_undoStack->prepareChanges();
    sc->addMidiPitch(inputEv.pitch, inputEv.chord);
    sc->activeMidiPitches()->push_back(inputEv);
    m_undoStack->commitChanges();

    m_noteChanged.notify();
}

mu::async::Notification NotationMidiInput::noteChanged() const
{
    return m_noteChanged;
}
