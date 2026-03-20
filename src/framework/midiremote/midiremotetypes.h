/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-CLA-applies
 *
 * MuseScore
 * Music Composition & Notation
 *
 * Copyright (C) 2021 MuseScore Limited and others
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

#include "midi/midievent.h"

#include "global/types/string.h"
#include "global/utils.h"
#include "global/translation.h"

namespace muse::midiremote {
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

    String name() const
    {
        if (this->type == RemoteEventType::Note) {
            //: A MIDI remote event, namely a note event
            return muse::mtrc("shortcuts", "Note %1")
                   .arg(String::fromStdString(muse::pitchToString(this->value)));
        } else if (this->type == RemoteEventType::Controller) {
            //: A MIDI remote event, namely a MIDI controller event
            return muse::mtrc("shortcuts", "CC %1").arg(String::number(this->value));
        }

        //: No MIDI remote event
        return muse::mtrc("shortcuts", "None");
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

using MidiMappingList = std::vector<MidiControlsMapping>;

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
}
