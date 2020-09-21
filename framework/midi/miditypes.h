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

namespace mu {
namespace midi {
static const unsigned int AUDIO_CHANNELS = 2;

using track_t = unsigned int;
using program_t = unsigned int;
using bank_t = unsigned int;
using tick_t = int;
using msec_t = uint64_t;
using tempo_t = unsigned int;
using TempoMap = std::map<tick_t, tempo_t>;

using SynthName = std::string;
using SynthMap = std::map<channel_t, SynthName>;

enum class SoundFontFormat {
    Undefined = 0,
    SF2,
    SF3,
    SFZ,
};
using SoundFontFormats = std::set<SoundFontFormat>;

struct SynthesizerState {
    enum ValID {
        UndefinedID = -1,
        SoundFontID = 0,
    };

    struct Val {
        ValID id = UndefinedID;
        std::string val;
        Val() = default;
        Val(ValID id, const std::string& val)
            : id(id), val(val) {}

        bool operator ==(const Val& other) const { return other.id == id && other.val == val; }
        bool operator !=(const Val& other) const { return !operator ==(other); }
    };

    struct Group {
        std::string name;
        std::vector<Val> vals;

        bool isValid() const { return !name.empty(); }

        bool operator ==(const Group& other) const { return other.name == name && other.vals == vals; }
        bool operator !=(const Group& other) const { return !operator ==(other); }
    };

    std::map<std::string, Group> groups;

    bool isNull() const { return groups.empty(); }
};

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
}
}

#endif // MU_MIDI_MIDITYPES_H
