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

    KeyState glissandoKeyState(piano_key_t key) const;
    KeyState arpeggioKeyState(piano_key_t key) const;
    KeyState trillKeyState(piano_key_t key) const;
    KeyState trillKeyState1(piano_key_t key) const;

    bool isFromMidi() const;

    bool isPlaying() const;
    bool islastMeasure() const;

    muse::async::Notification glissandoEndNotesChanged() const;
    muse::async::Notification glissandoTickChanged() const;

    muse::async::Notification arpeggioNotesChanged() const;
    muse::async::Notification arpeggioTickChanged() const;

    muse::async::Notification clefKeySigsKeysChanged() const;
    std::set<uint> clefKeySigsKeys() const;
    
    void updatePianoKeyboardKeys(piano_key_t _lowestKey, piano_key_t _numKeys);

private:
    INotationPtr currentNotation() const;

    void onNotationChanged();
    void updateNotesKeys(const std::vector<const Note*>& receivedNotes);
    void updatePlaybackNotesKeys(const std::vector<const Note*>& receivedNotes);

    void updateGlissandoNotesKeys(const std::vector<const Note*>& receivedNotes, const mu::engraving::Note* glissandoNote);
    void updateArpeggioNotesKeys(const std::vector<const Note*>& receivedNotes);

    void sendNoteOn(piano_key_t key);
    void sendNoteOff(piano_key_t key);

    std::optional<piano_key_t> m_pressedKey = std::nullopt;
    std::unordered_set<piano_key_t> m_keys;
    std::unordered_set<piano_key_t> m_otherNotesInChord;

    std::unordered_set<piano_key_t> m_righthand_keys;
    std::unordered_set<piano_key_t> m_lefthand_keys;

    piano_key_t m_glissando_note_key;
    std::unordered_set<piano_key_t> m_glissando_endnotes_keys;
    piano_key_t m_glissando_endnote_min_key;
    piano_key_t m_glissando_endnote_max_key;
    int m_glissando_ticks;
    int m_glissando_duration_ticks;
    int m_glissando_curr_ticks;
    muse::async::Notification m_glissandoEndNotesChanged;
    muse::async::Notification m_glissandoTickChanged;

    std::unordered_set<piano_key_t> m_arpeggio_notes_keys;
    int m_arpeggio_ticks;
    int m_arpeggio_duration_ticks;
    int m_arpeggio_curr_ticks;
    bool m_arpeggio_isdown = false;

    Note* receive_note = nullptr;
    piano_key_t m_trill_note_key;
    int m_trill_ticks;
    int m_trill_duration_ticks;
    int m_trill_tremolo_type;
    int m_trill_curr_ticks;
    Note* receive_note1 = nullptr;
    piano_key_t m_trill_note_key1;
    int m_trill_ticks1;
    int m_trill_duration_ticks1;
    int m_trill_tremolo_type1;
    int m_trill_curr_ticks1;

    std::set<uint> m_clefKeySigsKeys;
    muse::async::Notification m_clefKeySigsKeysChanged;

    bool m_islastMeasure;

    bool m_isFromMidi = false;

    muse::async::Notification m_keyStatesChanged;

    muse::async::Notification m_playbackKeyStatesChanged;

    piano_key_t lowestKey;
    piano_key_t numKeys;
};
}

#endif // MU_NOTATION_PIANOKEYBOARDCONTROLLER_H
