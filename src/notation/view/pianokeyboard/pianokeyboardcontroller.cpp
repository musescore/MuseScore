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

#include "defer.h"
#include "log.h"

using namespace mu::notation;
using namespace mu::midi;

void PianoKeyboardController::init()
{
    onNotationChanged();

    context()->currentNotationChanged().onNotify(this, [this]() {
        onNotationChanged();
    });
}

KeyState PianoKeyboardController::keyState(piano_key_t key) const
{
    if (m_pressedKey == key) {
        return KeyState::Played;
    }

    if (m_keys.find(key) != m_keys.cend()) {
        return KeyState::Selected;
    }

    if (m_otherNotesInChord.find(key) != m_otherNotesInChord.cend()) {
        return KeyState::OtherInSelectedChord;
    }

    return KeyState::None;
}

mu::async::Notification PianoKeyboardController::keyStatesChanged() const
{
    return m_keyStatesChanged;
}

bool PianoKeyboardController::isFromMidi() const
{
    return m_isFromMidi;
}

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

void PianoKeyboardController::onNotationChanged()
{
    if (auto notation = currentNotation()) {
        notation->interaction()->selectionChanged().onNotify(this, [this]() {
            auto notation = currentNotation();
            if (!notation) {
                return;
            }

            auto selection = notation->interaction()->selection();
            if (selection->isNone()) {
                return;
            }

            std::vector<const Note*> notes;
            for (const mu::engraving::Note* note : selection->notes()) {
                notes.push_back(note);
            }

            m_isFromMidi = false;
            updateNotesKeys(notes);
        });

        notation->midiInput()->notesReceived().onReceive(this, [this](const std::vector<const Note*>& notes) {
            m_isFromMidi = true;
            updateNotesKeys(notes);
        });
    }
}

void PianoKeyboardController::updateNotesKeys(const std::vector<const Note*>& receivedNotes)
{
    std::unordered_set<piano_key_t> newKeys;
    std::unordered_set<piano_key_t> newOtherNotesInChord;

    DEFER {
        if (newKeys != m_keys
            || newOtherNotesInChord != m_otherNotesInChord) {
            m_keys = newKeys;
            m_otherNotesInChord = newOtherNotesInChord;
        }

        m_keyStatesChanged.notify();
    };

    for (const mu::engraving::Note* note : receivedNotes) {
        newKeys.insert(static_cast<piano_key_t>(note->epitch()));
        for (const mu::engraving::Note* otherNote : note->chord()->notes()) {
            newOtherNotesInChord.insert(static_cast<piano_key_t>(otherNote->epitch()));
        }
    }
}

void PianoKeyboardController::sendNoteOn(piano_key_t key)
{
    auto notation = currentNotation();
    if (!notation) {
        return;
    }

    mu::midi::Event ev;
    ev.setMessageType(mu::midi::Event::MessageType::ChannelVoice10);
    ev.setOpcode(mu::midi::Event::Opcode::NoteOn);
    ev.setNote(key);
    ev.setVelocity(80);

    notation->midiInput()->onMidiEventReceived(ev);
}

void PianoKeyboardController::sendNoteOff(piano_key_t key)
{
    auto notation = currentNotation();
    if (!notation) {
        return;
    }

    mu::midi::Event ev;
    ev.setMessageType(mu::midi::Event::MessageType::ChannelVoice10);
    ev.setOpcode(mu::midi::Event::Opcode::NoteOff);
    ev.setNote(key);

    notation->midiInput()->onMidiEventReceived(ev);
}

INotationPtr PianoKeyboardController::currentNotation() const
{
    return context()->currentNotation();
}
