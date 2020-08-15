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

using namespace mu::midi;

uint32_t MidiParser::message(const Event& e)
{
    union {
        unsigned char data_as_bytes[4];
        uint32_t data_as_uint32;
    } u;

    switch (e.type) {
    case ME_NOTEOFF:
        u.data_as_bytes[0] = 0x80 | e.channel;
        u.data_as_bytes[1] = e.a;
        u.data_as_bytes[2] = 0;
        u.data_as_bytes[3] = 0;
        break;

    case ME_NOTEON:
        u.data_as_bytes[0] = 0x90 | e.channel;
        u.data_as_bytes[1] = e.a;
        u.data_as_bytes[2] = e.b;
        u.data_as_bytes[3] = 0;
        break;

    case ME_CONTROLLER:
        u.data_as_bytes[0] = 0xB0 | e.channel;
        u.data_as_bytes[1] = e.a;
        u.data_as_bytes[2] = e.b;
        u.data_as_bytes[3] = 0;
        break;

    case ME_PROGRAMCHANGE:
        u.data_as_bytes[0] = 0xC0 | e.channel;
        u.data_as_bytes[1] = e.a;
        u.data_as_bytes[2] = 0;
        u.data_as_bytes[3] = 0;
        break;

    case ME_PITCHBEND:
        u.data_as_bytes[0] = 0xE0 | e.channel;
        u.data_as_bytes[2] = e.a;
        u.data_as_bytes[1] = e.b;
        u.data_as_bytes[3] = 0;
        break;

    default:
        return 0;
        break;
    }
    return u.data_as_uint32;
}
