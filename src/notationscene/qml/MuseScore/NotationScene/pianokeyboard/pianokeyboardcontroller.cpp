/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2022 MuseScore Limited and others
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
    : muse::Contextable(iocCtx)
{
    onNotationChanged();

    context()->currentNotationChanged().onNotify(this, [this]() {
        m_playingKeys.clear();
        m_playingInstruments.clear();
        onNotationChanged();
    });

    playbackController()->currentPlaybackPositionChanged().onReceive(this,
        [this](const muse::audio::secs_t pos, const muse::midi::tick_t) {
            auto notation = currentNotation();
            if (!notation) {
                return;
            }

            auto playback = notation->masterNotation()->playback();
            if (!playback) {
                return;
            }

            muse::mpe::timestamp_t micros = static_cast<muse::mpe::timestamp_t>(pos * 1'000'000.0);
            auto activeNotes = playback->activeNotesAtTimestamp(micros);

            std::vector<piano_key_t> activeKeys;
            std::map<piano_key_t, std::set<uint64_t> > activeInstruments;
            activeKeys.reserve(activeNotes.size());
            for (const auto& note : activeNotes) {
                piano_key_t k = static_cast<piano_key_t>(note.pitch);
                activeKeys.push_back(k);
                activeInstruments[k].insert(note.trackId);
            }

            updatePlayingKeys(activeKeys);
            setPlayingInstruments(activeInstruments);
        });

    playbackController()->isPlayingChanged().onNotify(this, [this]() {
        if (playbackController()->isPlaying()) {
            m_keys.clear();
            m_otherNotesInChord.clear();
            m_keyStatesChanged.notify();
        } else {
            updatePlayingKeys({});
        }
    });
}

KeyState PianoKeyboardController::keyState(piano_key_t key) const
{
    if (m_pressedKey == key) {
        return KeyState::Played;
    }

    if (m_playingKeys.find(key) != m_playingKeys.cend()) {
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

void PianoKeyboardController::setHoveredKey(std::optional<piano_key_t> key)
{
    if (m_hoveredKey == key) {
        return;
    }

    auto notation = currentNotation();
    if (notation && notation->interaction()->noteInput()->isNoteInputMode()) {
        if (key.has_value()) {
            notation->interaction()->showShadowNoteForMidiPitch(key.value());
        } else {
            notation->interaction()->hideShadowNote();
        }
    }

    m_hoveredKey = key;
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
        }, Asyncable::Mode::SetReplace /* FIXME */);

        notation->midiInput()->notesReceived().onReceive(this, [this](const std::vector<const Note*>& notes) {
            m_isFromMidi = true;
            m_playingKeys.clear();
            updateNotesKeys(notes);
        }, Asyncable::Mode::SetReplace /* FIXME */);
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

void PianoKeyboardController::updatePlayingKeys(const std::vector<piano_key_t>& keys)
{
    std::unordered_set<piano_key_t> newPlayingKeys(keys.begin(), keys.end());
    if (newPlayingKeys != m_playingKeys) {
        m_playingKeys = newPlayingKeys;
        m_keyStatesChanged.notify();
    }
}

void PianoKeyboardController::setPlayingInstruments(const std::map<piano_key_t, std::set<uint64_t> >& instruments)
{
    if (m_playingInstruments != instruments) {
        m_playingInstruments = instruments;
        m_keyStatesChanged.notify();
    }
}

const std::set<uint64_t>& PianoKeyboardController::playingInstruments(piano_key_t key) const
{
    static const std::set<uint64_t> empty;
    auto it = m_playingInstruments.find(key);
    if (it != m_playingInstruments.end()) {
        return it->second;
    }
    return empty;
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
