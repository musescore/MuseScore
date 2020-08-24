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

#include "async/channel.h"

#include "audio/midi/event.h"

namespace mu {
namespace midi {
static const unsigned int AUDIO_CHANNELS = 2;

using track_t = unsigned int;
using channel_t = unsigned int;
using program_t = unsigned int;
using bank_t = unsigned int;
using tick_t = int;
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

struct Event {
    channel_t channel = 0;
    EventType type = EventType::ME_INVALID;
    int a = 0;
    int b = 0;

    Event() = default;
    Event(channel_t ch, EventType type, int a, int b)
        : channel(ch), type(type), a(a), b(b) {}

    bool operator ==(const Event& other) const { return channel == other.channel && type == other.type && a == other.a && b == other.b; }
    bool operator !=(const Event& other) const { return !operator==(other); }

    static std::string type_to_string(EventType t)
    {
        switch (t) {
        case EventType::ME_INVALID:     return "ME_INVALID";
        case EventType::ME_NOTEOFF:     return "ME_NOTEOFF";
        case EventType::ME_NOTEON:      return "ME_NOTEON";
        case EventType::ME_POLYAFTER:   return "ME_POLYAFTER";
        case EventType::ME_CONTROLLER:  return "ME_CONTROLLER";
        case EventType::ME_PROGRAM:     return "ME_PROGRAM";
        case EventType::ME_AFTERTOUCH:  return "ME_AFTERTOUCH";
        case EventType::ME_PITCHBEND:   return "ME_PITCHBEND";
        case EventType::ME_SYSEX:       return "ME_SYSEX";
        case EventType::ME_META:        return "ME_META";
        case EventType::ME_SONGPOS:     return "ME_SONGPOS";
        case EventType::ME_ENDSYSEX:    return "ME_ENDSYSEX";
        case EventType::ME_CLOCK:       return "ME_CLOCK";
        case EventType::ME_START:       return "ME_START";
        case EventType::ME_CONTINUE:    return "ME_CONTINUE";
        case EventType::ME_STOP:        return "ME_STOP";
        case EventType::ME_SENSE:       return "ME_SENSE";

        case EventType::ME_NOTE:        return "ME_NOTE";
        case EventType::ME_CHORD:       return "ME_CHORD";
        case EventType::ME_TICK1:       return "ME_TICK1";
        case EventType::ME_TICK2:       return "ME_TICK2";
        case EventType::ME_EOT:         return "ME_EOT";
        }
        return std::string();
    }

    static std::string cc_to_string(int cc)
    {
        switch (cc) {
        case 2: return "BREATH_MSB";
        default: return std::to_string(cc);
        }
    }

    std::string to_string() const
    {
        std::string str;
        str += "channel: " + std::to_string(channel);
        str += ", type: " + type_to_string(type);
        switch (type) {
        case EventType::ME_NOTEON: {
            str += ", key: " + std::to_string(a);
            str += ", vel: " + std::to_string(b);
        } break;
        case EventType::ME_NOTEOFF: {
            str += ", key: " + std::to_string(a);
        } break;
        case EventType::ME_CONTROLLER: {
            str += ", cc: " + cc_to_string(a);
            str += ", val: " + std::to_string(b);
        } break;
        case EventType::ME_PITCHBEND: {
            int pitch = b << 7 | a;
            str += ", pitch: " + std::to_string(pitch);
        } break;
        default:
            str += ", a: " + std::to_string(a);
            str += ", b: " + std::to_string(b);
        }

        return str;
    }
};

using Events = std::multimap<tick_t, Event>;

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
    Events events;

    bool isValid() const { return !tracks.empty(); }

    std::set<channel_t> channels() const
    {
        std::set<channel_t> cs;
        for (const Event& e : initEvents) {
            cs.insert(e.channel);
        }
        return cs;
    }

    std::vector<Event> initEventsForChannels(const std::set<channel_t>& chs) const
    {
        std::vector<Event> evts;
        for (const Event& e : initEvents) {
            if (chs.find(e.channel) != chs.end()) {
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
    async::Channel<MidiData> stream;
    async::Channel<uint32_t> request;

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
