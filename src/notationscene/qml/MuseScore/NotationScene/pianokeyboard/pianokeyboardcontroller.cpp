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

#include "midi/midievent.h"

#include "notation/imasternotation.h"
#include "notation/inotation.h"
#include "notation/inotationinteraction.h" // IWYU pragma: keep
#include "notation/inotationmidiinput.h"
#include "notation/inotationnoteinput.h" // IWYU pragma: keep
#include "notation/inotationselection.h" // IWYU pragma: keep
#include "notation/inotationsolomutestate.h"

using namespace mu::notation;
using namespace muse::midi;

PianoKeyboardController::PianoKeyboardController(const muse::modularity::ContextPtr& iocCtx)
    : muse::Contextable(iocCtx)
{
}

void PianoKeyboardController::init()
{
    if (m_isInitialized) {
        return;
    }

    m_isInitialized = true;
    onNotationChanged();

    auto globalContext = context();
    if (!globalContext) {
        return;
    }

    globalContext->currentNotationChanged().onNotify(this, [this]() {
        onNotationChanged();
    });

    globalContext->currentMasterNotationChanged().onNotify(this, [this]() {
        onMasterNotationChanged();
    });

    auto playbackState = globalContext->playbackState();
    if (!playbackState) {
        return;
    }

    playbackState->playbackStatusChanged().onReceive(this, [this](muse::audio::PlaybackStatus status) {
        onPlaybackStatusChanged(status);
    });

    playbackState->playbackPositionChanged().onReceive(this, [this](muse::audio::secs_t position) {
        onPlaybackPositionChanged(position);
    });

    auto controller = playbackController();
    if (controller) {
        controller->playbackInitedChanged().onReceive(this, [this](bool) {
            refreshPlaybackKeys();
        });

        controller->trackAdded().onReceive(this, [this](muse::audio::TrackId) {
            refreshPlaybackKeys();
        });

        controller->trackRemoved().onReceive(this, [this](muse::audio::TrackId) {
            refreshPlaybackKeys();
        });
    }

    auto configuration = notationConfiguration();
    if (configuration) {
        configuration->isPlayChordSymbolsChanged().onNotify(this, [this]() {
            refreshPlaybackKeys();
        });
    }

    onPlaybackStatusChanged(playbackState->playbackStatus());
}

void PianoKeyboardController::setPlaybackTrackingEnabled(bool enabled)
{
    if (m_isPlaybackTrackingEnabled == enabled) {
        return;
    }

    m_isPlaybackTrackingEnabled = enabled;

    if (!enabled) {
        replacePlaybackKeys({});
        m_lastPlaybackPosition.reset();
        return;
    }

    if (!m_isInitialized || m_playbackStatus != muse::audio::PlaybackStatus::Running
        || m_isWaitingForCountIn) {
        return;
    }

    auto globalContext = context();
    auto playbackState = globalContext ? globalContext->playbackState() : nullptr;
    if (playbackState) {
        updatePlaybackKeys(playbackState->playbackPosition(), true);
    }
}

KeyState PianoKeyboardController::keyState(piano_key_t key) const
{
    if (m_pressedKey == key || m_playbackKeys.find(key) != m_playbackKeys.cend()) {
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
    INotationPtr notation = currentNotation();
    if (m_notation == notation) {
        return;
    }

    if (m_notation) {
        auto soloMuteState = m_notation->soloMuteState();
        if (soloMuteState) {
            soloMuteState->trackSoloMuteStateChanged().disconnect(this);
        }

        auto interaction = m_notation->interaction();
        if (interaction) {
            interaction->selectionChanged().disconnect(this);
        }

        auto midiInput = m_notation->midiInput();
        if (midiInput) {
            midiInput->notesReceived().disconnect(this);
        }
    }

    const bool notationStateChanged = !m_keys.empty() || !m_otherNotesInChord.empty() || m_isFromMidi;
    m_keys.clear();
    m_otherNotesInChord.clear();
    m_isFromMidi = false;

    const bool playbackStateChanged = replacePlaybackKeys({});
    m_lastPlaybackPosition.reset();
    m_audibleInstrumentTrackIds.reset();
    m_notation = notation;

    if (notation) {
        auto interaction = notation->interaction();
        if (interaction) {
            interaction->selectionChanged().onNotify(this, [this]() {
                auto notation = currentNotation();
                if (!notation) {
                    return;
                }

                auto interaction = notation->interaction();
                auto selection = interaction ? interaction->selection() : nullptr;
                if (!selection || selection->isNone()) {
                    updateNotesKeys({}, false);
                    refreshPlaybackKeys();
                    return;
                }

                std::vector<const Note*> notes;
                for (const mu::engraving::Note* note : selection->notes()) {
                    notes.push_back(note);
                }

                updateNotesKeys(notes, false);
                refreshPlaybackKeys();
            }, Asyncable::Mode::SetReplace);
        }

        auto soloMuteState = notation->soloMuteState();
        if (soloMuteState) {
            soloMuteState->trackSoloMuteStateChanged().onReceive(
                this, [this](const engraving::InstrumentTrackId&, const INotationSoloMuteState::SoloMuteState&) {
                refreshPlaybackKeys();
            }, Asyncable::Mode::SetReplace);
        }

        auto midiInput = notation->midiInput();
        if (midiInput) {
            midiInput->notesReceived().onReceive(this, [this](const std::vector<const Note*>& notes) {
                updateNotesKeys(notes, true);
            }, Asyncable::Mode::SetReplace);
        }
    }

    if (notationStateChanged || (playbackStateChanged && m_isPlaybackTrackingEnabled)) {
        m_keyStatesChanged.notify();
    }

    if (m_isPlaybackTrackingEnabled && m_playbackStatus == muse::audio::PlaybackStatus::Running
        && !m_isWaitingForCountIn) {
        refreshPlaybackKeys();
    }
}

void PianoKeyboardController::onMasterNotationChanged()
{
    m_lastPlaybackPosition.reset();
    m_audibleInstrumentTrackIds.reset();

    if (m_isPlaybackTrackingEnabled && m_playbackStatus == muse::audio::PlaybackStatus::Running
        && !m_isWaitingForCountIn) {
        refreshPlaybackKeys();
        return;
    }

    if (replacePlaybackKeys({}) && m_isPlaybackTrackingEnabled) {
        m_keyStatesChanged.notify();
    }
}

void PianoKeyboardController::updateNotesKeys(const std::vector<const Note*>& receivedNotes, bool isFromMidi)
{
    std::unordered_set<piano_key_t> newKeys;
    std::unordered_set<piano_key_t> newOtherNotesInChord;

    auto configuration = notationConfiguration();
    const bool useWrittenPitch = configuration && configuration->midiUseWrittenPitch().val;

    for (const mu::engraving::Note* note : receivedNotes) {
        newKeys.insert(static_cast<piano_key_t>(useWrittenPitch ? note->epitch() : note->ppitch()));
        for (const mu::engraving::Note* otherNote : note->chord()->notes()) {
            newOtherNotesInChord.insert(static_cast<piano_key_t>(useWrittenPitch ? otherNote->epitch() : otherNote->ppitch()));
        }
    }

    if (newKeys == m_keys
        && newOtherNotesInChord == m_otherNotesInChord
        && isFromMidi == m_isFromMidi) {
        return;
    }

    m_keys = std::move(newKeys);
    m_otherNotesInChord = std::move(newOtherNotesInChord);
    m_isFromMidi = isFromMidi;
    m_keyStatesChanged.notify();
}

void PianoKeyboardController::onPlaybackStatusChanged(muse::audio::PlaybackStatus status)
{
    if (m_playbackStatus == status) {
        return;
    }

    m_playbackStatus = status;
    m_lastPlaybackPosition.reset();
    m_audibleInstrumentTrackIds.reset();

    if (status != muse::audio::PlaybackStatus::Running) {
        m_isWaitingForCountIn = false;
        m_countInStartPosition.reset();
        if (replacePlaybackKeys({}) && m_isPlaybackTrackingEnabled) {
            m_keyStatesChanged.notify();
        }
        return;
    }

    auto configuration = notationConfiguration();
    m_isWaitingForCountIn = configuration && configuration->isCountInEnabled();
    m_countInStartPosition.reset();

    auto globalContext = context();
    auto playbackState = globalContext ? globalContext->playbackState() : nullptr;
    if (m_isWaitingForCountIn && playbackState) {
        m_countInStartPosition = playbackState->playbackPosition();
    }

    if (replacePlaybackKeys({}) && m_isPlaybackTrackingEnabled) {
        m_keyStatesChanged.notify();
    }

    if (m_isWaitingForCountIn || !m_isPlaybackTrackingEnabled) {
        return;
    }

    if (playbackState) {
        updatePlaybackKeys(playbackState->playbackPosition(), true);
    }
}

void PianoKeyboardController::onPlaybackPositionChanged(muse::audio::secs_t position)
{
    if (m_playbackStatus != muse::audio::PlaybackStatus::Running) {
        return;
    }

    if (m_isWaitingForCountIn) {
        if (m_countInStartPosition == position) {
            return;
        }

        m_isWaitingForCountIn = false;
        m_countInStartPosition.reset();
        m_lastPlaybackPosition.reset();
    }

    updatePlaybackKeys(position);
}

void PianoKeyboardController::refreshPlaybackKeys()
{
    m_audibleInstrumentTrackIds.reset();

    if (!m_isPlaybackTrackingEnabled || m_playbackStatus != muse::audio::PlaybackStatus::Running
        || m_isWaitingForCountIn) {
        return;
    }

    auto globalContext = context();
    auto playbackState = globalContext ? globalContext->playbackState() : nullptr;
    if (playbackState) {
        updatePlaybackKeys(playbackState->playbackPosition(), true);
    }
}

void PianoKeyboardController::updatePlaybackKeys(muse::audio::secs_t position, bool force)
{
    if (!m_isPlaybackTrackingEnabled || m_playbackStatus != muse::audio::PlaybackStatus::Running
        || m_isWaitingForCountIn || (!force && m_lastPlaybackPosition == position)) {
        return;
    }

    m_lastPlaybackPosition = position;

    auto globalContext = context();
    auto masterNotation = globalContext ? globalContext->currentMasterNotation() : nullptr;
    auto notationPlayback = masterNotation ? masterNotation->playback() : nullptr;
    auto controller = playbackController();

    std::unordered_set<piano_key_t> playbackKeys;
    if (notationPlayback && controller) {
        if (!m_audibleInstrumentTrackIds.has_value()) {
            m_audibleInstrumentTrackIds = controller->audibleInstrumentTrackIds();
        }

        const std::vector<muse::midi::note_idx_t> pitches = notationPlayback->activePlaybackPitches(
            position, *m_audibleInstrumentTrackIds);
        playbackKeys.insert(pitches.cbegin(), pitches.cend());
    }

    if (replacePlaybackKeys(std::move(playbackKeys))) {
        m_keyStatesChanged.notify();
    }
}

bool PianoKeyboardController::replacePlaybackKeys(std::unordered_set<piano_key_t> keys)
{
    if (m_playbackKeys == keys) {
        return false;
    }

    m_playbackKeys = std::move(keys);
    return true;
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
    auto globalContext = context();
    return globalContext ? globalContext->currentNotation() : nullptr;
}
