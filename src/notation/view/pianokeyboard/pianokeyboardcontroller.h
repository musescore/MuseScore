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
#ifndef MU_NOTATION_PIANOKEYBOARDCONTROLLER_H
#define MU_NOTATION_PIANOKEYBOARDCONTROLLER_H

#include "async/asyncable.h"

#include "modularity/ioc.h"
#include "context/iglobalcontext.h"
#include "audio/iplayback.h"
#include "playback/iplaybackcontroller.h"

#include "pianokeyboardtypes.h"

namespace mu::notation {
class PianoKeyboardController : public muse::async::Asyncable
{
    INJECT(context::IGlobalContext, context)
    INJECT(muse::audio::IPlayback, playback)
    INJECT(playback::IPlaybackController, playbackController)

public:
    PianoKeyboardController() = default;

    void init();

    std::optional<piano_key_t> pressedKey() const;
    void setPressedKey(std::optional<piano_key_t> key);

    KeyState keyState(piano_key_t key) const;
    muse::async::Notification keyStatesChanged() const;

    bool isFromMidi() const;

private:
    INotationPtr currentNotation() const;
    IMasterNotationPtr currentMasterNotation() const;
    INotationPlaybackPtr masterNotationPlayback() const;

    void onNotationChanged();
    void updateNotesKeys(const std::vector<const Note*>& receivedNotes);

    void onNoteMidiEventReceived(muse::audio::TrackId trackId, const muse::midi::Event& event);

    void sendNoteOn(piano_key_t key);
    void sendNoteOff(piano_key_t key);

    void addPlayedKey(muse::audio::TrackId trackId, std::optional<piano_key_t> key);
    void removePlayedKey(muse::audio::TrackId trackId, std::optional<piano_key_t> key);
    void removePlayedTrackKeys(muse::audio::TrackId trackId);

    muse::audio::TrackId instrumentTrackIdToTrackId(InstrumentTrackId trackId) const;
    InstrumentTrackId trackIdToInstrumentTrackId(muse::audio::TrackId trackId) const;

    std::optional<piano_key_t> m_pressedKey = std::nullopt;
    std::unordered_map<piano_key_t, std::unordered_set<muse::audio::TrackId> > m_playedKeys;
    std::unordered_set<piano_key_t> m_keys;
    std::unordered_set<piano_key_t> m_otherNotesInChord;

    bool m_isFromMidi = false;

    muse::async::Notification m_keyStatesChanged;
};
}

#endif // MU_NOTATION_PIANOKEYBOARDCONTROLLER_H
