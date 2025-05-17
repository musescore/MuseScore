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
#include "inotationconfiguration.h"

#include "pianokeyboardtypes.h"

namespace mu::notation {
class PianoKeyboardController : public muse::Injectable, public muse::async::Asyncable
{
    muse::Inject<INotationConfiguration> notationConfiguration = { this };
    muse::Inject<context::IGlobalContext> context = { this };

public:
    PianoKeyboardController(const muse::modularity::ContextPtr& iocCtx);

    std::optional<piano_key_t> pressedKey() const;
    void setPressedKey(std::optional<piano_key_t> key);

    KeyState keyState(piano_key_t key) const;
    muse::async::Notification keyStatesChanged() const;

    KeyState playbackKeyState(piano_key_t key) const;
    bool playbackKeyStatesEmpty() const;
    muse::async::Notification playbackKeyStatesChanged() const;

    bool isFromMidi() const;

private:
    INotationPtr currentNotation() const;

    void onNotationChanged();
    void updateNotesKeys(const std::vector<const Note*>& receivedNotes);
    void updatePlaybackNotesKeys(const std::vector<const Note*>& receivedNotes);

    void sendNoteOn(piano_key_t key);
    void sendNoteOff(piano_key_t key);

    std::optional<piano_key_t> m_pressedKey = std::nullopt;
    std::unordered_set<piano_key_t> m_keys;
    std::unordered_set<piano_key_t> m_otherNotesInChord;

    std::unordered_set<piano_key_t> m_righthand_keys;
    std::unordered_set<piano_key_t> m_lefthand_keys;

    bool m_isFromMidi = false;

    muse::async::Notification m_keyStatesChanged;

    muse::async::Notification m_playbackKeyStatesChanged;
};
}

#endif // MU_NOTATION_PIANOKEYBOARDCONTROLLER_H
