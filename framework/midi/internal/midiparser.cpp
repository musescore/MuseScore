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
#include "midiparser.h"

#include "log.h"

using namespace mu::midi;

uint32_t MidiParser::toMessage(const Event& e)
{
    union {
        unsigned char data_as_bytes[4];
        uint32_t data_as_uint32 = 0;
    } u;

    switch (e.type) {
    case EventType::ME_NOTEOFF:
        u.data_as_bytes[0] = 0x80 | e.channel;
        u.data_as_bytes[1] = e.a;
        u.data_as_bytes[2] = 0;
        u.data_as_bytes[3] = 0;
        break;

    case EventType::ME_NOTEON:
        u.data_as_bytes[0] = 0x90 | e.channel;
        u.data_as_bytes[1] = e.a;
        u.data_as_bytes[2] = e.b;
        u.data_as_bytes[3] = 0;
        break;

    case EventType::ME_CONTROLLER:
        u.data_as_bytes[0] = 0xB0 | e.channel;
        u.data_as_bytes[1] = e.a;
        u.data_as_bytes[2] = e.b;
        u.data_as_bytes[3] = 0;
        break;

    case EventType::ME_PROGRAM:
        u.data_as_bytes[0] = 0xC0 | e.channel;
        u.data_as_bytes[1] = e.a;
        u.data_as_bytes[2] = 0;
        u.data_as_bytes[3] = 0;
        break;

    case EventType::ME_PITCHBEND:
        u.data_as_bytes[0] = 0xE0 | e.channel;
        u.data_as_bytes[2] = e.a;
        u.data_as_bytes[1] = e.b;
        u.data_as_bytes[3] = 0;
        break;

    default:
        NOT_IMPLEMENTED << e.to_string();
        break;
    }
    return u.data_as_uint32;
}

Event MidiParser::toEvent(uint32_t msg)
{
    union {
        unsigned char data_as_bytes[4];
        uint32_t data_as_uint32 = 0;
    } u;

    u.data_as_uint32 = msg;

    Event e;

    switch (u.data_as_bytes[0] & 0xF0) {
    case 0x80: {
        e.type = EventType::ME_NOTEOFF;
        e.channel = u.data_as_bytes[0] & 0x0F;
        e.a = u.data_as_bytes[1];
        break;
    }
    case 0x90: {
        e.type = EventType::ME_NOTEON;
        e.channel = u.data_as_bytes[0] & 0x0F;
        e.a = u.data_as_bytes[1];
        e.b = u.data_as_bytes[2];
        break;
    }
    case 0xB0: {
        e.type = EventType::ME_CONTROLLER;
        e.channel = u.data_as_bytes[0] & 0x0F;
        e.a = u.data_as_bytes[1];
        e.b = u.data_as_bytes[2];
        break;
    }
    case 0xC0: {
        e.type = EventType::ME_PROGRAM;
        e.channel = u.data_as_bytes[0] & 0x0F;
        e.a = u.data_as_bytes[1];
        break;
    }
    case 0xE0: {
        e.type = EventType::ME_PITCHBEND;
        e.channel = u.data_as_bytes[0] & 0x0F;
        e.a = u.data_as_bytes[1];
        e.b = u.data_as_bytes[2];
        break;
    }
    default:
        NOT_IMPLEMENTED << msg;
    }

    return e;
}
