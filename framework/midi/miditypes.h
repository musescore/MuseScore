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

#include "async/channel.h"

namespace mu {
namespace midi {
static const unsigned int AUDIO_CHANNELS = 2;

enum EventType {
    ME_INVALID = 0,
    ME_NOTEOFF,
    ME_NOTEON,
    ME_CONTROLLER,
    ME_PITCHBEND,
    ME_META,
    META_TEMPO,
    ME_PROGRAMCHANGE,
    ME_ALLNOTESOFF,

    ME_TICK1,
    ME_TICK2,

    MIDI_EOT
};

enum CntrType {
    CTRL_INVALID = 0,
    CTRL_PROGRAM
};

using track_t = unsigned int;
using channel_t = unsigned int;
using program_t = unsigned int;
using bank_t = unsigned int;
using tick_t = int;
using tempo_t = unsigned int;

struct Event {
    channel_t channel = 0;
    EventType type = ME_INVALID;
    int a = 0;
    int b = 0;

    Event() = default;
    Event(channel_t ch, EventType type, int a, int b)
        : channel(ch), type(type), a(a), b(b) {}

    bool operator ==(const Event& other) const
    {
        return channel == other.channel && type == other.type
               && a == other.a && b == other.b;
    }

    bool operator !=(const Event& other) const
    {
        return !operator==(other);
    }

    static std::string type_to_string(EventType t)
    {
        switch (t) {
        case EventType::ME_INVALID:     return "INVALID";
        case EventType::ME_NOTEOFF:     return "NOTEOFF";
        case EventType::ME_NOTEON:      return "NOTEON";
        case EventType::ME_CONTROLLER:  return "CONTROLLER";
        case EventType::ME_PITCHBEND:   return "PITCHBEND";
        case EventType::ME_META:        return "META";
        case EventType::META_TEMPO:     return "TEMPO";
        case EventType::ME_PROGRAMCHANGE: return "PROGRAMCHANGE";
        case EventType::ME_ALLNOTESOFF: return "ALLNOTESOFF";
        case EventType::ME_TICK1:       return "TICK1";
        case EventType::ME_TICK2:       return "TICK2";
        case EventType::MIDI_EOT:       return "EOT";
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
    std::vector<Program> programs;
};

struct MidiData {
    int division = 480;
    std::map<tick_t, tempo_t> tempomap;
    std::map<channel_t, std::string> synthmap;
    std::vector<Track> tracks;
    Events events;

    bool isValid() const { return !tracks.empty(); }

    Programs programs() const
    {
        Programs progs;
        for (const Track& t  : tracks) {
            for (const Program& p : t.programs) {
                progs.push_back(p);
            }
        }

        return progs;
    }

    std::vector<channel_t> channels() const
    {
        std::vector<channel_t> cs;
        for (const Track& t : tracks) {
            for (const Program& p : t.programs) {
                cs.push_back(p.channel);
            }
        }
        return cs;
    }

    std::string dump(bool withEvents = false)
    {
        std::stringstream ss;
        ss << "division: " << division << "\n";
        ss << "tempo changes: " << tempomap.size() << "\n";
        for (const auto& it : tempomap) {
            ss << "  tick: " << it.first << ", tempo: " << it.second << "\n";
        }
        ss << "\n";
        ss << "tracks count: " << tracks.size() << "\n";
        ss << "channels count: " << channels().size() << "\n";
        for (size_t ti = 0; ti < tracks.size(); ++ti) {
            ss << "track: " << ti << ", channels: " << tracks.at(ti).programs.size() << "\n";
            for (const Program& p : tracks.at(ti).programs) {
                ss << "  channel: " << p.channel
                   << ", bank: " << p.bank
                   << ", program: " << p.program
                   << "\n";
            }
        }

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
}
}

#endif // MU_MIDI_MIDITYPES_H
