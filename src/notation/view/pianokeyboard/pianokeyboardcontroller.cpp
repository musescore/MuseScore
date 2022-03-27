/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-CLA-applies
 *
 * MuseScore
 * Music Composition & Notation
 *
 * Copyright (C) 2022 MuseScore BVBA and others
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
#include "pianokeyboardcontroller.h"

#include "log.h"

using namespace mu::notation;
using namespace mu::midi;

std::optional<piano_key_t> PianoKeyboardController::pressedKey() const
{
    return m_pressedKey;
}

void PianoKeyboardController::setPressedKey(std::optional<piano_key_t> key)
{
    if (m_pressedKey == key) {
        return;
    }

    if (m_pressedKey.has_value()) {
        sendNoteOff(m_pressedKey.value());
    }

    if (key.has_value()) {
        sendNoteOn(key.value());
    }

    m_pressedKey = key;
    m_keyStatesChanged.notify();
}

mu::async::Notification PianoKeyboardController::keyStatesChanged() const
{
    return m_keyStatesChanged;
}

void PianoKeyboardController::sendNoteOn(piano_key_t key)
{
    auto midiInput = notationMidiInput();
    if (!midiInput) {
        return;
    }

    Event ev;
    ev.setMessageType(Event::MessageType::ChannelVoice10);
    ev.setOpcode(Event::Opcode::NoteOn);
    ev.setNote(key);
    ev.setVelocity(80);

    midiInput->onMidiEventReceived(ev);
}

void PianoKeyboardController::sendNoteOff(piano_key_t key)
{
    auto midiInput = notationMidiInput();
    if (!midiInput) {
        return;
    }

    Event ev;
    ev.setMessageType(Event::MessageType::ChannelVoice10);
    ev.setOpcode(Event::Opcode::NoteOff);
    ev.setNote(key);

    midiInput->onMidiEventReceived(ev);
}

INotationPtr PianoKeyboardController::notation() const
{
    return context()->currentNotation();
}

INotationMidiInputPtr PianoKeyboardController::notationMidiInput() const
{
    return notation() ? notation()->midiInput() : nullptr;
}
