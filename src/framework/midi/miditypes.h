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

#ifndef MU_MIDI_MIDITYPES_H
#define MU_MIDI_MIDITYPES_H

#include <string>
#include <sstream>
#include <cstdint>
#include <vector>
#include <map>
#include <functional>
#include <set>
#include <cassert>
#include "async/channel.h"
#include "retval.h"
#include "midievent.h"

namespace mu::midi {
using track_t = int32_t;
using program_t = int32_t;
using bank_t = int32_t;
using tick_t = uint32_t;
using tempo_t = uint32_t;
using velocity_t = uint16_t;
using note_idx_t = uint8_t;
using TempoMap = std::map<tick_t, tempo_t>;
using Events = std::map<tick_t, std::vector<Event> >;

struct Program {
    Program(bank_t b, program_t p)
        : bank(b), program(p) {}

    bank_t bank = 0;
    program_t program = 0;

    bool operator==(const Program& other) const
    {
        return bank == other.bank
               && program == other.program;
    }
};
using Programs = std::vector<midi::Program>;

struct MidiMapping {
    int division = 480;
    TempoMap tempo;
    Programs programms;

    bool isValid() const
    {
        return !programms.empty() && !tempo.empty();
    }

    bool operator==(const MidiMapping& other) const
    {
        return division == other.division
               && tempo == other.tempo
               && programms == other.programms;
    }
};

struct MidiStream {
    tick_t lastTick = 0;

    ValCh<std::vector<Event> > controlEventsStream;
    async::Channel<Events, tick_t /*endTick*/> mainStream;
    async::Channel<Events, tick_t /*endTick*/> backgroundStream;
    async::Channel<tick_t /*from*/, tick_t /*from*/> eventsRequest;

    bool operator==(const MidiStream& other) const
    {
        return lastTick == other.lastTick
               && controlEventsStream.val == other.controlEventsStream.val;
    }
};

struct MidiData {
    MidiMapping mapping;
    MidiStream stream;

    bool isValid() const
    {
        return mapping.isValid() && stream.lastTick > 0;
    }

    bool operator==(const MidiData& other) const
    {
        return mapping == other.mapping
               && stream == other.stream;
    }
};

using MidiDeviceID = std::string;
struct MidiDevice {
    MidiDeviceID id;
    std::string name;

    bool operator==(const MidiDevice& other) const
    {
        return id == other.id;
    }
};

using MidiDeviceList = std::vector<MidiDevice>;
}

#endif // MU_MIDI_MIDITYPES_H
