//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2020 MuseScore BVBA and others
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
//=============================================================================

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
#include "midievent.h"

namespace mu::midi {
using track_t = unsigned int;
using program_t = unsigned int;
using bank_t = unsigned int;
using tick_t = int;
using msec_t = uint64_t;
using tempo_t = unsigned int;
using TempoMap = std::map<tick_t, tempo_t>;

using SynthName = std::string;
using SynthMap = std::map<midi::channel_t, SynthName>;

using EventType = Ms::EventType;
using CntrType = Ms::CntrType;
using Events = std::multimap<tick_t, Event>;

struct Chunk {
    tick_t beginTick = 0;
    tick_t endTick = 0;
    Events events;
};
using Chunks = std::map<tick_t /*begin*/, Chunk>;

struct Program {
    channel_t channel = 0;
    program_t program = 0;
    bank_t bank = 0;
};
using Programs = std::vector<midi::Program>;

struct Track {
    track_t num = 0;
    std::vector<channel_t> channels;
};

struct MidiData {
    int division = 480;
    TempoMap tempoMap;
    SynthMap synthMap;
    std::vector<Event> initEvents;  //! NOTE Set channels programs and others
    std::vector<Track> tracks;
    Chunks chunks;

    bool isValid() const { return !tracks.empty(); }

    std::set<channel_t> channels() const
    {
        std::set<channel_t> cs;
        for (const Event& e : initEvents) {
            cs.insert(e.channel());
        }
        return cs;
    }

    std::vector<Event> initEventsForChannels(const std::set<channel_t>& chs) const
    {
        std::vector<Event> evts;
        for (const Event& e : initEvents) {
            if (chs.find(e.channel()) != chs.end()) {
                evts.push_back(e);
            }
        }
        return evts;
    }

    tick_t lastChunksTick() const
    {
        if (chunks.empty()) {
            return 0;
        }
        return chunks.rbegin()->second.endTick;
    }

    std::string dump(bool withEvents = false)
    {
        std::stringstream ss;
        ss << "division: " << division << "\n";
        ss << "tempo changes: " << tempoMap.size() << "\n";
        for (const auto& it : tempoMap) {
            ss << "  tick: " << it.first << ", tempo: " << it.second << "\n";
        }
        ss << "\n";
        ss << "tracks count: " << tracks.size() << "\n";
        ss << "channels count: " << channels().size() << "\n";

        if (withEvents) {
            //! TODO
        }

        ss.flush();
        return ss.str();
    }
};

struct MidiStream {
    MidiData initData;

    bool isStreamingAllowed = false;
    tick_t lastTick = 0;
    async::Channel<Chunk> stream;
    async::Channel<tick_t> request;

    bool isValid() const { return initData.isValid(); }
};

using MidiDeviceID = std::string;
struct MidiDevice {
    MidiDeviceID id;
    std::string name;
};

enum class MidiActionType
{
    Rewind,
    Play,
    Loop,
    Stop,
    NoteInputMode,
    Note1,
    Note2,
    Note4,
    Note8,
    Note16,
    Note32,
    Note64,
    Rest,
    Dot,
    DotDot,
    Tie,
    Undo,
};

inline std::vector<MidiActionType> allMidiActionTypes()
{
    return {
        MidiActionType::Rewind,
        MidiActionType::Loop,
        MidiActionType::Play,
        MidiActionType::Stop,
        MidiActionType::NoteInputMode,
        MidiActionType::Note1,
        MidiActionType::Note2,
        MidiActionType::Note4,
        MidiActionType::Note8,
        MidiActionType::Note16,
        MidiActionType::Note32,
        MidiActionType::Note64,
        MidiActionType::Rest,
        MidiActionType::Dot,
        MidiActionType::DotDot,
        MidiActionType::Tie,
        MidiActionType::Undo
    };
}
}

#endif // MU_MIDI_MIDITYPES_H
