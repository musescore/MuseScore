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

#ifndef MU_AUDIO_MIDITYPES_H
#define MU_AUDIO_MIDITYPES_H

#include <string>
#include <sstream>
#include <cstdint>
#include <vector>
#include <map>
#include <functional>

#include "async/channel.h"

namespace mu {
namespace audio {
namespace midi {
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

    MIDI_EOT
};

enum CntrType {
    CTRL_INVALID = 0,
    CTRL_PROGRAM
};

struct Event {
    uint32_t tick = 0;
    EventType type = ME_INVALID;
    int a = 0;
    int b = 0;

    Event() = default;
    Event(uint32_t tick, EventType type, int a, int b)
        : tick(tick), type(type), a(a), b(b) {}

    bool operator ==(const Event& other) const
    {
        return tick == other.tick && type == other.type
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
        str += "tick: " + std::to_string(tick);
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

struct Channel {
    uint16_t num = 0;
    uint16_t bank = 0;
    uint16_t program = 0;
    std::vector<Event> events;
};

struct Track {
    uint16_t num = 0;
    std::vector<Channel> channels;
};

struct Program {
    uint16_t ch = 0;
    uint16_t prog = 0;
    uint16_t bank = 0;
};

using Programs = std::vector<midi::Program>;

struct MidiData {
    uint16_t division = 480;
    std::map<uint32_t /*tick*/, uint32_t /*tempo*/> tempomap;
    std::vector<Track> tracks;

    bool isValid() const { return !tracks.empty(); }

    Programs programs() const
    {
        Programs progs;
        for (const Track& t  : tracks) {
            for (const Channel& ch  : t.channels) {
                Program p;
                p.ch = ch.num;
                p.bank = ch.bank;
                p.prog = ch.program;
                progs.push_back(std::move(p));
            }
        }

        return progs;
    }

    uint16_t channelsCount() const
    {
        uint16_t c = 0;
        for (const Track& t : tracks) {
            c += t.channels.size();
        }
        return c;
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
        ss << "channels count: " << channelsCount() << "\n";
        for (size_t ti = 0; ti < tracks.size(); ++ti) {
            ss << "track: " << ti << ", channels: " << tracks.at(ti).channels.size() << "\n";
            for (const Channel& ch : tracks.at(ti).channels) {
                ss << "  ch num: " << ch.num
                   << ", bank: " << ch.bank
                   << ", prog: " << ch.program
                   << ", events: " << ch.events.size()
                   << "\n";

                if (withEvents) {
                    for (const Event& e : ch.events) {
                        ss << e.to_string() << "\n";
                    }
                }
            }
        }

        ss.flush();
        return ss.str();
    }
};

struct MidiStream {
    MidiData initData;

    async::Channel<MidiData> stream;
    async::Channel<uint32_t> request;

    bool isValid() const { return initData.isValid(); }
};
}
}
}

#endif // MU_AUDIO_MIDITYPES_H
