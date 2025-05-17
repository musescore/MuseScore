/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2022 MuseScore Limited
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
using namespace muse::midi;

PianoKeyboardController::PianoKeyboardController(const muse::modularity::ContextPtr& iocCtx)
    : muse::Injectable(iocCtx)
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

muse::async::Notification PianoKeyboardController::keyStatesChanged() const
{
    return m_keyStatesChanged;
}

KeyState PianoKeyboardController::playbackKeyState(piano_key_t key) const {
    if (m_righthand_keys.find(key) != m_righthand_keys.cend()) {
        return KeyState::RightHand;
    }

    // if (m_lefthand_keys.find(key) != m_lefthand_keys.cend()) {
    //     return KeyState::LeftHand;
    // }

    return KeyState::None;
}

bool PianoKeyboardController::playbackKeyStatesEmpty() const {
    if (m_righthand_keys.empty()) {
        return true;
    }
    return false;
}

muse::async::Notification PianoKeyboardController::playbackKeyStatesChanged() const
{
    return m_playbackKeyStatesChanged;
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
                // If no note is selected, the piano keyboard key stats should be cleared
                updateNotesKeys({});
                return;
            }

            std::vector<const Note*> notes;
            for (const mu::engraving::Note* note : selection->notes()) {
                notes.push_back(note);
            }

            m_isFromMidi = false;
            updateNotesKeys(notes);
        });

        notation->interaction()->playbackNotesChanged().onNotify(this, [this]() {
            auto notation = currentNotation();
            if (!notation) {
                return;
            }

            std::vector<const Note*> notes;
            for (const mu::engraving::Note* note : notation->interaction()->playbackNotes()) {
                notes.push_back(note);
            }
            m_isFromMidi = false;
            updatePlaybackNotesKeys(notes);
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

    const bool useWrittenPitch = notationConfiguration()->midiUseWrittenPitch().val;

    for (const mu::engraving::Note* note : receivedNotes) {
        newKeys.insert(static_cast<piano_key_t>(useWrittenPitch ? note->epitch() : note->ppitch()));
        for (const mu::engraving::Note* otherNote : note->chord()->notes()) {
            newOtherNotesInChord.insert(static_cast<piano_key_t>(useWrittenPitch ? otherNote->epitch() : otherNote->ppitch()));
        }
    }
}

void PianoKeyboardController::updatePlaybackNotesKeys(const std::vector<const Note*>& receivedNotes) {
    std::unordered_set<piano_key_t> newKeys;
    std::unordered_set<piano_key_t> newOtherNotesInChord;

    DEFER {
        if (newKeys != m_righthand_keys) {
            m_righthand_keys = newKeys;
        }

        m_playbackKeyStatesChanged.notify();
    };

    const bool useWrittenPitch = notationConfiguration()->midiUseWrittenPitch().val;

    for (const mu::engraving::Note* note : receivedNotes) {
        newKeys.insert(static_cast<piano_key_t>(useWrittenPitch ? note->epitch() : note->ppitch()));
    }
}

void PianoKeyboardController::sendNoteOn(piano_key_t key)
{
    auto notation = currentNotation();
    if (!notation) {
        return;
    }

    muse::midi::Event ev;
    ev.setMessageType(muse::midi::Event::MessageType::ChannelVoice10);
    ev.setOpcode(muse::midi::Event::Opcode::NoteOn);
    ev.setNote(key);
    ev.setVelocity7(80);

    notation->midiInput()->onMidiEventReceived(ev);
}

void PianoKeyboardController::sendNoteOff(piano_key_t key)
{
    auto notation = currentNotation();
    if (!notation) {
        return;
    }

    muse::midi::Event ev;
    ev.setMessageType(muse::midi::Event::MessageType::ChannelVoice10);
    ev.setOpcode(muse::midi::Event::Opcode::NoteOff);
    ev.setNote(key);

    notation->midiInput()->onMidiEventReceived(ev);
}

INotationPtr PianoKeyboardController::currentNotation() const
{
    return context()->currentNotation();
}
