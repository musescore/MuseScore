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

#include "audio/iplayer.h"
#include "audio/iaudiooutput.h"

#include "defer.h"
#include "log.h"

using namespace mu::notation;
using namespace muse::midi;

void PianoKeyboardController::init()
{
    onNotationChanged();

    context()->currentNotationChanged().onNotify(this, [this]() {
        onNotationChanged();
    });

    playback()->midiEvent().onReceive(this, [this](muse::audio::TrackId trackId, const muse::midi::Event& event) {
        onNoteMidiEventReceived(trackId, event);
    });

    playback()->playingNotesRevoked().onReceive(this, [this](muse::audio::TrackId trackId) {
        removePlayedTrackKeys(trackId);
    });

    playback()->audioOutput()->outputParamsChanged().onReceive(this, [this](muse::audio::TrackSequenceId, muse::audio::TrackId trackId,
                                                                            muse::audio::AudioOutputParams params) {
        if (params.muted) {
            removePlayedTrackKeys(trackId);
        }
    });

    auto notationPlayback = masterNotationPlayback();
    if (notationPlayback) {
        notationPlayback->trackRemoved().onReceive(this, [this](InstrumentTrackId instrumentTrackId) {
            removePlayedTrackKeys(instrumentTrackIdToTrackId(instrumentTrackId));
        });
    }
}

KeyState PianoKeyboardController::keyState(piano_key_t key) const
{
    auto it = m_playedKeys.find(key);
    if (m_pressedKey == key || (it != m_playedKeys.cend() && !it->second.empty())) {
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
    m_playedKeys.clear();
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

void PianoKeyboardController::onNoteMidiEventReceived(muse::audio::TrackId trackId, const Event& event)
{
    INotationPlaybackPtr notationPlayback = masterNotationPlayback();
    if (!notationPlayback) {
        return;
    }

    InstrumentTrackId instrumentTrackId = trackIdToInstrumentTrackId(trackId);
    if (!instrumentTrackId.isValid()) {
        return;
    }

    muse::midi::Event::Opcode opcode = event.opcode();
    uint8_t note = event.note();

    auto handleEvent = [this, notationPlayback, trackId, instrumentTrackId, opcode, note](muse::audio::AudioOutputParams params) {
        if (params.muted) {
            return;
        }

        if (notationPlayback->isChordSymbolsTrack(instrumentTrackId) || instrumentTrackId == notationPlayback->metronomeTrackId()) {
            return;
        }

        switch (opcode) {
        case Event::Opcode::NoteOn:
            addPlayedKey(trackId, note);
            break;
        case Event::Opcode::NoteOff:
            removePlayedKey(trackId, note);
            break;
        default:
            break;
        }
    };

    std::shared_ptr<muse::audio::IAudioOutput> audioOutput = playback()->audioOutput();
    if (audioOutput) {
        audioOutput->outputParams(playbackController()->currentTrackSequenceId(), trackId).onResolve(this, handleEvent);
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
    ev.setVelocity(80);

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

void PianoKeyboardController::addPlayedKey(muse::audio::TrackId trackId, std::optional<piano_key_t> key)
{
    if (m_playedKeys.find(key.value()) == m_playedKeys.cend()) {
        m_playedKeys.insert({ key.value(), { trackId } });
        m_keyStatesChanged.notify();
        return;
    }

    m_playedKeys[key.value()].insert(trackId);
    m_keyStatesChanged.notify();
}

void PianoKeyboardController::removePlayedKey(muse::audio::TrackId trackId, std::optional<piano_key_t> key)
{
    auto it = m_playedKeys.find(key.value());
    if (it != m_playedKeys.cend()) {
        auto& keyTracks = m_playedKeys[key.value()];
        auto keyIt = keyTracks.find(trackId);
        if (keyIt != keyTracks.cend()) {
            keyTracks.erase(keyIt);
        }

        if (keyTracks.empty()) {
            m_playedKeys.erase(it);
        }
    }

    m_keyStatesChanged.notify();
}

void PianoKeyboardController::removePlayedTrackKeys(muse::audio::TrackId trackId)
{
    bool changed = false;
    for (auto&[key, tracks] : m_playedKeys) {
        if (tracks.erase(trackId) == 1) {
            changed = true;
        }
    }

    if (changed) {
        m_keyStatesChanged.notify();
    }
}

muse::audio::TrackId PianoKeyboardController::instrumentTrackIdToTrackId(InstrumentTrackId trackId) const
{
    return playbackController()->instrumentTrackIdMap().at(trackId);
}

InstrumentTrackId PianoKeyboardController::trackIdToInstrumentTrackId(muse::audio::TrackId trackId) const
{
    auto& trackIdMap = playbackController()->instrumentTrackIdMap();
    for (const auto& pair : trackIdMap) {
        if (pair.second == trackId) {
            return pair.first;
        }
    }

    return InstrumentTrackId();
}

INotationPtr PianoKeyboardController::currentNotation() const
{
    return context()->currentNotation();
}

IMasterNotationPtr PianoKeyboardController::currentMasterNotation() const
{
    return context()->currentMasterNotation();
}

INotationPlaybackPtr PianoKeyboardController::masterNotationPlayback() const
{
    IMasterNotationPtr masterNotation = currentMasterNotation();

    if (masterNotation) {
        return masterNotation->playback();
    }

    return nullptr;
}
