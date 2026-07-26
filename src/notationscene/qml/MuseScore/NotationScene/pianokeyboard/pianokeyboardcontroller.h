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

#pragma once

#include "async/asyncable.h"
#include "audio/common/audiotypes.h"

#include "modularity/ioc.h"
#include "context/iglobalcontext.h"
#include "notation/inotationconfiguration.h"
#include "playback/iplaybackcontroller.h"

#include "pianokeyboardtypes.h"

namespace mu::engraving {
class Note;
}

namespace mu::notation {
class PianoKeyboardController : public muse::Contextable, public muse::async::Asyncable
{
public:
    muse::GlobalInject<INotationConfiguration> notationConfiguration;

private:
    muse::ContextInject<context::IGlobalContext> context = { this };
    muse::ContextInject<playback::IPlaybackController> playbackController = { this };

public:
    PianoKeyboardController(const muse::modularity::ContextPtr& iocCtx);

    void init();
    void setPlaybackTrackingEnabled(bool enabled);

    std::optional<piano_key_t> pressedKey() const;
    void setPressedKey(std::optional<piano_key_t> key);
    void setHoveredKey(std::optional<piano_key_t> key);

    KeyState keyState(piano_key_t key) const;
    muse::async::Notification keyStatesChanged() const;

    bool isFromMidi() const;

private:
    INotationPtr currentNotation() const;

    void onNotationChanged();
    void onMasterNotationChanged();
    void updateNotesKeys(const std::vector<const engraving::Note*>& receivedNotes, bool isFromMidi);

    void onPlaybackStatusChanged(muse::audio::PlaybackStatus status);
    void onPlaybackPositionChanged(muse::audio::secs_t position);
    void refreshPlaybackKeys();
    void updatePlaybackKeys(muse::audio::secs_t position, bool force = false);
    bool replacePlaybackKeys(std::unordered_set<piano_key_t> keys);

    void sendNoteOn(piano_key_t key);
    void sendNoteOff(piano_key_t key);

    INotationPtr m_notation;

    std::optional<piano_key_t> m_pressedKey = std::nullopt;
    std::optional<piano_key_t> m_hoveredKey = std::nullopt;
    std::unordered_set<piano_key_t> m_keys;
    std::unordered_set<piano_key_t> m_otherNotesInChord;
    std::unordered_set<piano_key_t> m_playbackKeys;

    bool m_isFromMidi = false;
    bool m_isInitialized = false;
    bool m_isPlaybackTrackingEnabled = false;
    bool m_isWaitingForCountIn = false;

    muse::audio::PlaybackStatus m_playbackStatus = muse::audio::PlaybackStatus::Stopped;
    std::optional<muse::audio::secs_t> m_lastPlaybackPosition;
    std::optional<muse::audio::secs_t> m_countInStartPosition;

    muse::async::Notification m_keyStatesChanged;
};
}
