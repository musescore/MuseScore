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
#pragma once

#include <string>
#include <cstdint>
#include <vector>
#include <map>

#include "async/channel.h"
#include "types/retval.h"
#include "midievent.h"

namespace muse::midi {
using track_t = int32_t;
using program_t = int32_t;
using bank_t = int32_t;
using tick_t = uint32_t;
using tempo_t = uint32_t;
using velocity_t = uint16_t;
using note_idx_t = uint8_t;
using TempoMap = std::map<tick_t, tempo_t>;
using Events = std::map<tick_t, std::vector<Event> >;

static constexpr int EXPRESSION_CONTROLLER = 11;
static constexpr int SUSTAIN_PEDAL_CONTROLLER = 64;
static constexpr int SOSTENUTO_PEDAL_CONTROLLER = 66;

struct Program {
    Program(bank_t b = 0, program_t p = 0)
        : bank(b), program(p) {}

    bank_t bank = 0;
    program_t program = 0;

    bool operator==(const Program& other) const
    {
        return bank == other.bank
               && program == other.program;
    }

    bool operator<(const Program& other) const
    {
        if (bank < other.bank) {
            return true;
        }

        if (bank == other.bank) {
            return program < other.program;
        }

        return false;
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

static constexpr char NONE_DEVICE_ID[] = "-1";

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

inline MidiDeviceID makeUniqueDeviceId(int index, int arg1, int arg2)
{
    return std::to_string(index) + ":" + std::to_string(arg1) + ":" + std::to_string(arg2);
}

inline std::vector<int> splitDeviceId(const MidiDeviceID& deviceId)
{
    std::vector<int> result;

    std::size_t current, previous = 0;
    std::string delim = ":";
    current = deviceId.find(delim);
    std::size_t delimLen = delim.length();

    while (current != std::string::npos) {
        result.push_back(std::stoi(deviceId.substr(previous, current - previous)));
        previous = current + delimLen;
        current = deviceId.find(delim, previous);
    }
    result.push_back(std::stoi(deviceId.substr(previous, current - previous)));

    return result;
}
}
