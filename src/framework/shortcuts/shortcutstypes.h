/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-CLA-applies
 *
 * MuseScore
 * Music Composition & Notation
 *
 * Copyright (C) 2021 MuseScore BVBA and others
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
#ifndef MU_SHORTCUTS_SHORTCUTSTYPES_H
#define MU_SHORTCUTS_SHORTCUTSTYPES_H

#include <string>
#include <list>
#include <QKeySequence>

#include "stringutils.h"
#include "midi/midievent.h"

namespace mu::shortcuts {
struct Shortcut
{
    std::string action;
    std::vector<std::string> sequences;
    std::string context;
    QKeySequence::StandardKey standardKey = QKeySequence::UnknownKey;

    Shortcut() = default;
    Shortcut(const std::string& a)
        : action(a) {}

    bool isValid() const
    {
        return !action.empty() && (!sequences.empty() || standardKey != QKeySequence::UnknownKey);
    }

    bool operator ==(const Shortcut& sc) const
    {
        return action == sc.action && sequences == sc.sequences && standardKey == sc.standardKey;
    }

    std::string sequencesAsString() const { return sequencesToString(sequences); }

    static std::string sequencesToString(const std::vector<std::string>& seqs)
    {
        return strings::join(seqs, "; ");
    }

    static std::vector<std::string> sequencesFromString(const std::string& str)
    {
        std::vector<std::string> seqs;
        strings::split(str, seqs, "; ");
        return seqs;
    }
};

using ShortcutList = std::list<Shortcut>;

enum RemoteEventType {
    Undefined = 0,
    Note,
    Controller
};

struct RemoteEvent {
    RemoteEventType type = RemoteEventType::Undefined;
    int value = -1;

    RemoteEvent() = default;
    RemoteEvent(RemoteEventType type, int value)
        : type(type), value(value) {}

    bool isValid() const
    {
        return type != RemoteEventType::Undefined && value != -1;
    }

    bool operator ==(const RemoteEvent& other) const
    {
        return type == other.type && value == other.value;
    }

    bool operator !=(const RemoteEvent& other) const
    {
        return !operator==(other);
    }
};

struct MidiControlsMapping {
    std::string action;
    RemoteEvent event;

    MidiControlsMapping() = default;
    MidiControlsMapping(const std::string& action)
        : action(action) {}

    bool isValid() const
    {
        return !action.empty() && event.isValid();
    }

    bool operator ==(const MidiControlsMapping& other) const
    {
        return action == other.action && other.event == event;
    }
};

using MidiMappingList = std::list<MidiControlsMapping>;

inline RemoteEvent remoteEventFromMidiEvent(const midi::Event& midiEvent)
{
    RemoteEvent event;
    bool isNote = midiEvent.isOpcodeIn({ midi::Event::Opcode::NoteOff, midi::Event::Opcode::NoteOn });
    bool isController = midiEvent.isOpcodeIn({ midi::Event::Opcode::ControlChange });
    if (isNote) {
        event.type = RemoteEventType::Note;
        event.value = midiEvent.note();
    } else if (isController) {
        event.type = RemoteEventType::Controller;
        event.value = midiEvent.index();
    }

    return event;
}

inline bool needIgnoreKey(int key)
{
    if (key == 0) {
        return true;
    }

    static const std::set<Qt::Key> ignoredKeys {
        Qt::Key_Shift,
        Qt::Key_Control,
        Qt::Key_Meta,
        Qt::Key_Alt,
        Qt::Key_AltGr,
        Qt::Key_CapsLock,
        Qt::Key_NumLock,
        Qt::Key_ScrollLock,
        Qt::Key_unknown
    };

    return ignoredKeys.find(static_cast<Qt::Key>(key)) != ignoredKeys.end();
}

inline std::pair<int, Qt::KeyboardModifiers> correctKeyInput(int key, Qt::KeyboardModifiers modifiers)
{
    // replace Backtab with Shift+Tab
    if (key == Qt::Key_Backtab && modifiers == Qt::ShiftModifier) {
        key = Qt::Key_Tab;
    }

    modifiers &= ~Qt::KeypadModifier;

    return { key, modifiers };
}

inline QString sequencesToNativeText(const std::vector<std::string>& sequences)
{
    QList<QKeySequence> keySequenceList;

    for (const std::string& sequence : sequences) {
        keySequenceList << QKeySequence(QString::fromStdString(sequence));
    }

    return QKeySequence::listToString(keySequenceList, QKeySequence::NativeText);
}
}

#endif // MU_SHORTCUTS_SHORTCUTSTYPES_H
