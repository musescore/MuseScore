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

KeyState PianoKeyboardController::playbackKeyState(piano_key_t key) const 
{
    if (m_righthand_keys.find(key) != m_righthand_keys.cend()) {
        return KeyState::RightHand;
    }

    // if (m_lefthand_keys.find(key) != m_lefthand_keys.cend()) {
    //     return KeyState::LeftHand;
    // }

    return KeyState::None;
}

KeyState PianoKeyboardController::glissandoKeyState(piano_key_t key) const 
{
    if (m_glissando_curr_ticks == m_glissando_ticks) {
        if (key == m_glissando_note_key) {
            return KeyState::Glissando;
        }
    } else {
        if (m_glissando_curr_ticks > m_glissando_ticks && m_glissando_curr_ticks < m_glissando_ticks + m_glissando_duration_ticks) {
            int left_dis = m_glissando_curr_ticks - m_glissando_ticks;
            double ratio = left_dis / static_cast<double>(m_glissando_duration_ticks);
            if (m_glissando_note_key < m_glissando_endnote_min_key) {
                double _key = m_glissando_note_key + (m_glissando_endnote_min_key - m_glissando_note_key) * ratio;
                int __key = static_cast<int>(_key);
                if (key == (piano_key_t)__key) {
                    return KeyState::Glissando;
                }
            } else if (m_glissando_note_key > m_glissando_endnote_max_key) {
                double _key = m_glissando_note_key - (m_glissando_note_key - m_glissando_endnote_max_key) * ratio;
                int __key = static_cast<int>(_key);
                if (key == (piano_key_t)__key) {
                    return KeyState::Glissando;
                }
            }
        }
    }
    return KeyState::None;
}

bool compare_by_note(piano_key_t a, piano_key_t b) 
{
    return a < b;
}
bool compare_by_note_reverse(piano_key_t a, piano_key_t b) 
{
    return a > b;
}

KeyState PianoKeyboardController::arpeggioKeyState(piano_key_t key) const 
{
    if (m_arpeggio_duration_ticks == 0) {
        return KeyState::None;
    }
    if (m_arpeggio_curr_ticks > m_arpeggio_ticks && m_arpeggio_curr_ticks < m_arpeggio_ticks + m_arpeggio_duration_ticks) {
        int left_dis = m_arpeggio_curr_ticks - m_arpeggio_ticks;
        double ratio = left_dis / static_cast<double>(m_arpeggio_duration_ticks);
        size_t arpeggio_notes_count = m_arpeggio_notes_keys.size();

        std::vector<piano_key_t> sorted_keys;
        for (piano_key_t _key : m_arpeggio_notes_keys) {
            sorted_keys.push_back(_key);
        }
        if (m_arpeggio_isdown) {
            std::sort(sorted_keys.begin(), sorted_keys.end(), compare_by_note_reverse);
        } else {
            std::sort(sorted_keys.begin(), sorted_keys.end(), compare_by_note);
        }

        int index = 0;
        for (piano_key_t _key : sorted_keys) {
            int _ratio_count = static_cast<int>(arpeggio_notes_count * ratio);
            if (_ratio_count == 0) {
                if (_ratio_count == index) {
                    if (key == _key) {
                        return KeyState::Arpeggio;
                    }
                }
            } else {
                if (_ratio_count - 1 == index) {
                    if (key == _key) {
                        return KeyState::Arpeggio;
                    }
                }
            }
            ++index;
        }
    }
    return KeyState::None;
}

KeyState PianoKeyboardController::trillKeyState(piano_key_t key) const 
{
    if (m_trill_duration_ticks == 0) {
        return KeyState::None;
    }

    if (m_trill_curr_ticks >= m_trill_ticks && m_trill_curr_ticks <= m_trill_ticks + m_trill_duration_ticks) {
        int left_dis = m_trill_curr_ticks - m_trill_ticks;
        double ratio = left_dis / static_cast<double>(m_trill_duration_ticks);

        if (m_trill_tremolo_type > 0) {
            if (receive_note) {
                DurationType noteDurationtype = receive_note->chord()->durationType().type();
                int frequency = m_trill_tremolo_type / 10;
                if (noteDurationtype == mu::engraving::DurationType::V_WHOLE) {
                    frequency *= 4;
                } else if (noteDurationtype == mu::engraving::DurationType::V_HALF) {
                    frequency *= 2;
                } 
                int _ratio_count = static_cast<int>(frequency * ratio);
                int _int_note_key = static_cast<int>(m_trill_note_key);
                if (_ratio_count % 2 == 1) {
                    _int_note_key += 1;
                } 
                if (key == (piano_key_t)_int_note_key) {
                    return KeyState::Trill;
                }
            }
        } else {
            if (receive_note && receive_note->chord()->durationType().type() <= mu::engraving::DurationType::V_QUARTER) {
                int _ratio_count = static_cast<int>(48 * ratio);
                int _int_note_key = static_cast<int>(m_trill_note_key);
                if (_ratio_count % 2 == 0) {
                    _int_note_key -= 1;
                } else {
                    _int_note_key -= 2;
                }
                if (key == (piano_key_t)_int_note_key) {
                    return KeyState::Trill;
                }
            } else {
                int _int_note_key = static_cast<int>(m_trill_note_key);
                if (ratio < 0.333 || ratio > 0.666) {
                    _int_note_key -= 2;
                } else {
                    _int_note_key -= 1;
                }
                if (key == (piano_key_t)_int_note_key) {
                    return KeyState::Trill;
                }
            }
        }
    }
    return KeyState::None;
}
KeyState PianoKeyboardController::trillKeyState1(piano_key_t key) const 
{
    if (m_trill_duration_ticks1 == 0) {
        return KeyState::None;
    }

    if (m_trill_curr_ticks1 >= m_trill_ticks1 && m_trill_curr_ticks1 <= m_trill_ticks1 + m_trill_duration_ticks1) {
        int left_dis = m_trill_curr_ticks1 - m_trill_ticks1;
        double ratio = left_dis / static_cast<double>(m_trill_duration_ticks1);

        if (m_trill_tremolo_type1 > 0) {
            if (receive_note1) {
                DurationType noteDurationtype = receive_note1->chord()->durationType().type();
                int frequency = m_trill_tremolo_type1 / 10;
                if (noteDurationtype == mu::engraving::DurationType::V_WHOLE) {
                    frequency *= 4;
                } else if (noteDurationtype == mu::engraving::DurationType::V_HALF) {
                    frequency *= 2;
                } 
                int _ratio_count = static_cast<int>(frequency * ratio);
                int _int_note_key = static_cast<int>(m_trill_note_key1);
                if (_ratio_count % 2 == 1) {
                    _int_note_key += 1;
                } 
                if (key == (piano_key_t)_int_note_key) {
                    return KeyState::Trill;
                }
            }
        } else {
            if (receive_note1 && receive_note1->chord()->durationType().type() <= mu::engraving::DurationType::V_QUARTER) {
                int _ratio_count = static_cast<int>(48 * ratio);
                int _int_note_key = static_cast<int>(m_trill_note_key1);
                if (_ratio_count % 2 == 0) {
                    _int_note_key -= 1;
                } else {
                    _int_note_key -= 2;
                }
                if (key == (piano_key_t)_int_note_key) {
                    return KeyState::Trill;
                }
            } else {
                int _int_note_key = static_cast<int>(m_trill_note_key1);
                if (ratio < 0.333 || ratio > 0.666) {
                    _int_note_key -= 2;
                } else {
                    _int_note_key -= 1;
                }
                if (key == (piano_key_t)_int_note_key) {
                    return KeyState::Trill;
                }
            }
        }
    }
    return KeyState::None;
}

bool PianoKeyboardController::playbackKeyStatesEmpty() const 
{
    if (m_righthand_keys.empty()) {
        return true;
    }
    return false;
}

muse::async::Notification PianoKeyboardController::playbackKeyStatesChanged() const
{
    return m_playbackKeyStatesChanged;
}

muse::async::Notification PianoKeyboardController::glissandoEndNotesChanged() const 
{
    return m_glissandoEndNotesChanged;
}
muse::async::Notification PianoKeyboardController::glissandoTickChanged() const 
{
    return m_glissandoTickChanged;
}

muse::async::Notification PianoKeyboardController::clefKeySigsKeysChanged() const 
{
    return m_clefKeySigsKeysChanged;
}

std::set<uint> PianoKeyboardController::clefKeySigsKeys() const 
{
    return m_clefKeySigsKeys;
}

void PianoKeyboardController::clearClefKeySigsKeys() 
{
    m_clefKeySigsKeys.clear();
}

bool PianoKeyboardController::isFromMidi() const
{
    return m_isFromMidi;
}

bool PianoKeyboardController::isPlaying() const 
{
    if (auto notation = currentNotation()) {
        return notation->interaction()->isPlaying();
    }
    return false;
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

        notation->interaction()->glissandoEndNotesChanged().onNotify(this, [this]() {
            auto notation = currentNotation();
            if (!notation) {
                return;
            }
            m_glissando_ticks = notation->interaction()->glissandoNoteTicks();
            m_glissando_duration_ticks = notation->interaction()->glissandoNoteDurationticks();
            std::vector<const Note*> notes;
            for (const mu::engraving::Note* note : notation->interaction()->glissandoEndNotes()) {
                notes.push_back(note);
            }
            const mu::engraving::Note* glissandoNote = notation->interaction()->glissandoNote();
            if (glissandoNote != nullptr) {
                updateGlissandoNotesKeys(notes, glissandoNote);
            }
        });

        notation->interaction()->glissandoTickChanged().onNotify(this, [this]() {
            auto notation = currentNotation();
            if (!notation) {
                return;
            }
            m_glissando_curr_ticks = notation->interaction()->glissandoCurrticks();

            // m_glissandoTickChanged.notify();
        });

        notation->interaction()->arpeggioNotesChanged().onNotify(this, [this]() {
            auto notation = currentNotation();
            if (!notation) {
                return;
            }
            m_arpeggio_ticks = notation->interaction()->arpeggioNoteTicks();
            m_arpeggio_duration_ticks = notation->interaction()->arpeggioNoteDurationticks();
            m_arpeggio_isdown = notation->interaction()->arpeggioIsDown();
            std::vector<const Note*> notes;
            for (const mu::engraving::Note* note : notation->interaction()->arpeggioNotes()) {
                notes.push_back(note);
            }
            updateArpeggioNotesKeys(notes);
        });

        notation->interaction()->arpeggioTickChanged().onNotify(this, [this]() {
            auto notation = currentNotation();
            if (!notation) {
                return;
            }
            m_arpeggio_curr_ticks = notation->interaction()->arpeggioCurrticks();
        });

        notation->interaction()->trillNoteChanged().onNotify(this, [this]() {
            auto notation = currentNotation();
            if (!notation) {
                return;
            }
            m_trill_ticks = notation->interaction()->trillNoteTicks();
            m_trill_duration_ticks = notation->interaction()->trillNoteDurationticks();
            
            Note *receivedNote = notation->interaction()->trillNote();

            receive_note = receivedNote;
            if (receivedNote) {
                const bool useWrittenPitch = notationConfiguration()->midiUseWrittenPitch().val;
                m_trill_note_key = static_cast<piano_key_t>(useWrittenPitch ? receivedNote->epitch() : receivedNote->ppitch());
            } else {
                m_trill_note_key = static_cast<piano_key_t>(10000);
            }
        });
        notation->interaction()->trillNoteChanged1().onNotify(this, [this]() {
            auto notation = currentNotation();
            if (!notation) {
                return;
            }
            m_trill_ticks1 = notation->interaction()->trillNoteTicks1();
            m_trill_duration_ticks1 = notation->interaction()->trillNoteDurationticks1();
            
            Note *receivedNote = notation->interaction()->trillNote1();

            receive_note1 = receivedNote;
            if (receivedNote) {
                const bool useWrittenPitch = notationConfiguration()->midiUseWrittenPitch().val;
                m_trill_note_key1 = static_cast<piano_key_t>(useWrittenPitch ? receivedNote->epitch() : receivedNote->ppitch());
            } else {
                m_trill_note_key1 = static_cast<piano_key_t>(10000);
            }
        });

        notation->interaction()->trillTickChanged().onNotify(this, [this]() {
            auto notation = currentNotation();
            if (!notation) {
                return;
            }
            m_trill_curr_ticks = notation->interaction()->trillCurrticks();
            m_trill_tremolo_type = notation->interaction()->trillNoteTremolotype();
        });
        notation->interaction()->trillTickChanged1().onNotify(this, [this]() {
            auto notation = currentNotation();
            if (!notation) {
                return;
            }
            m_trill_curr_ticks1 = notation->interaction()->trillCurrticks1();
            m_trill_tremolo_type1 = notation->interaction()->trillNoteTremolotype1();
        });

        notation->interaction()->clefKeySigsKeysChanged().onNotify(this, [this]() {
            auto notation = currentNotation();
            if (!notation) {
                return;
            }
            for (auto key : notation->interaction()->clefKeySigsKeys()) {
                m_clefKeySigsKeys.insert(key);
            }
            notation->interaction()->clearClefKeySigsKeys();

            m_clefKeySigsKeysChanged.notify();
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

void PianoKeyboardController::updatePlaybackNotesKeys(const std::vector<const Note*>& receivedNotes) 
{
    std::unordered_set<piano_key_t> newKeys;

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

void PianoKeyboardController::updateGlissandoNotesKeys(const std::vector<const Note*>& receivedNotes, const mu::engraving::Note* glissandoNote) 
{
    std::unordered_set<piano_key_t> newKeys;

    DEFER {
        if (newKeys != m_glissando_endnotes_keys) {
            m_glissando_endnotes_keys = newKeys;
        }
        for (const auto& key : m_glissando_endnotes_keys) {
            m_glissando_endnote_min_key = key;
            m_glissando_endnote_max_key = key;
            break;
        }
        for (const auto& key : m_glissando_endnotes_keys) {
            if (key < m_glissando_endnote_min_key) {
                m_glissando_endnote_min_key = key;
            } else if (key > m_glissando_endnote_max_key) {
                m_glissando_endnote_max_key = key;
            }
        }
        // m_glissandoEndNotesChanged.notify();
    };

    const bool useWrittenPitch = notationConfiguration()->midiUseWrittenPitch().val;
    m_glissando_note_key = static_cast<piano_key_t>(useWrittenPitch ? glissandoNote->epitch() : glissandoNote->ppitch());
    for (const mu::engraving::Note* note : receivedNotes) {
        newKeys.insert(static_cast<piano_key_t>(useWrittenPitch ? note->epitch() : note->ppitch()));
    }
}

void PianoKeyboardController::updateArpeggioNotesKeys(const std::vector<const Note*>& receivedNotes) 
{
    std::unordered_set<piano_key_t> newKeys;

    DEFER {
        if (newKeys != m_arpeggio_notes_keys) {
            m_arpeggio_notes_keys = newKeys;
        }
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
