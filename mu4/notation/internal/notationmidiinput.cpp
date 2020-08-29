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
#include "notationmidiinput.h"

#include <QGuiApplication>

#include "log.h"

#include "libmscore/score.h"
#include "libmscore/segment.h"

using namespace mu::notation;

NotationMidiInput::NotationMidiInput(IGetScore* getScore)
    : m_getScore(getScore)
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

    if (e.type() == midi::EventType::ME_NOTEON || e.type() == midi::EventType::ME_NOTEOFF) {
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

    if (e.type() == midi::EventType::ME_NOTEOFF || e.velocity() == 0) {
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

    sc->startCmd();
    sc->addMidiPitch(inputEv.pitch, inputEv.chord);
    sc->activeMidiPitches()->push_back(inputEv);

    sc->endCmd();
    m_noteChanged.notify();
}

mu::async::Notification NotationMidiInput::noteChanged() const
{
    return m_noteChanged;
}
