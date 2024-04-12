/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2022 MuseScore Limited
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

#ifndef MU_ENGRAVING_COMPAT_MIDICOREEVENT_H
#define MU_ENGRAVING_COMPAT_MIDICOREEVENT_H

#include <cstdint>

#include "global/allocator.h"

namespace mu::engraving {
struct PitchWheelSpecs {
    //! @note amplitude of the pitch wheel in semitones
    uint16_t mAmplitude{ 12 };

    //! @note absolute limit of the  wheel pitch value (thus,  pitch shift can be set using values from {-mLimit, mLimit} interval
    uint16_t mLimit    { 8192 };

    //! @note step of pith wheel in ticks
    uint16_t mStep     { 10 };
};

//---------------------------------------------------------
//   Event types
//---------------------------------------------------------

enum EventType {
    ME_INVALID    = 0,
    ME_NOTEOFF    = 0x80,
    ME_NOTEON     = 0x90,
    ME_POLYAFTER  = 0xa0,
    ME_CONTROLLER = 0xb0,   //! Control change message
    ME_PROGRAM    = 0xc0,   //! Program change message
    ME_AFTERTOUCH = 0xd0,   //! Channel pressure message
    ME_PITCHBEND  = 0xe0,
    ME_SYSEX      = 0xf0,
    ME_META       = 0xff,
    ME_SONGPOS    = 0xf2,
    ME_ENDSYSEX   = 0xf7,
    ME_CLOCK      = 0xf8,
    ME_START      = 0xfa,
    ME_CONTINUE   = 0xfb,
    ME_STOP       = 0xfc,
    ME_SENSE      = 0xfe,     // active sense (used by yamaha)

    ME_NOTE       = 0x1,
    ME_CHORD      = 0x2,
    ME_TICK1      = 0x3,    // metronome tick akzent
    ME_TICK2      = 0x4,    // metronome tick
    ME_EOT        = 0x5
};

//---------------------------------------------------------
//   Midi Meta Events
//---------------------------------------------------------

enum {
    META_SEQUENCE_NUMBER = 0,
    META_TEXT            = 1,
    META_COPYRIGHT       = 2,
    META_TRACK_NAME      = 3,
    META_INSTRUMENT_NAME = 4,
    META_LYRIC           = 5,
    META_MARKER          = 6,
    META_CUE_POINT       = 7,
    META_PROGRAM_NAME    = 8,   // MIDI Meta Events 8 and 9 were defined as above by the MMA in 1998
    META_DEVICE_NAME     = 9,   // It is therefore necessary to redefine MuseScore's private meta events
    META_TRACK_COMMENT   = 0xf,   // Using the block starting 0x10 seems sensible as that is currently clear
    META_TITLE           = 0x10,       // mscore extension
    META_SUBTITLE        = 0x11,       // mscore extension
    META_COMPOSER        = 0x12,     // mscore extension
    META_TRANSLATOR      = 0x13,     // mscore extension
    META_POET            = 0x14,     // mscore extension
    META_PORT_CHANGE     = 0x21,
    META_CHANNEL_PREFIX  = 0x22,
    META_EOT             = 0x2f,    // end of track
    META_TEMPO           = 0x51,
    META_TIME_SIGNATURE  = 0x58,
    META_KEY_SIGNATURE   = 0x59,
    META_SPECIFIC        = 0x7F     // sequencer specific
};

//---------------------------------------------------------
//   Midi Controller
//---------------------------------------------------------

enum CntrType {
    CTRL_HBANK               = 0x00,
    CTRL_LBANK               = 0x20,

    CTRL_HDATA               = 0x06,
    CTRL_LDATA               = 0x26,

    CTRL_HNRPN               = 0x63,
    CTRL_LNRPN               = 0x62,

    CTRL_HRPN                = 0x65,
    CTRL_LRPN                = 0x64,

    CTRL_MODULATION          = 0x01,
    CTRL_BREATH              = 0x02,
    CTRL_FOOT                = 0x04,
    CTRL_PORTAMENTO_TIME_MSB = 0x05,
    CTRL_VOLUME              = 0x07,
    CTRL_PANPOT              = 0x0a,
    CTRL_EXPRESSION          = 0x0b,
    CTRL_PORTAMENTO_TIME_LSB = 0x25,
    CTRL_SUSTAIN             = 0x40,
    CTRL_PORTAMENTO          = 0x41,
    CTRL_SOSTENUTO           = 0x42,
    CTRL_SOFT_PEDAL          = 0x43,
    CTRL_HARMONIC_CONTENT    = 0x47,
    CTRL_RELEASE_TIME        = 0x48,
    CTRL_ATTACK_TIME         = 0x49,

    CTRL_BRIGHTNESS          = 0x4a,
    CTRL_PORTAMENTO_CONTROL  = 0x54,
    CTRL_REVERB_SEND         = 0x5b,
    CTRL_CHORUS_SEND         = 0x5d,
    CTRL_VARIATION_SEND      = 0x5e,

    CTRL_ALL_SOUNDS_OFF      = 0x78,   // 120
    CTRL_RESET_ALL_CTRL      = 0x79,   // 121
    CTRL_LOCAL_OFF           = 0x7a,   // 122
    CTRL_ALL_NOTES_OFF       = 0x7b,   // 123

    // special midi events are mapped to internal
    // controller
    //
    CTRL_PROGRAM   = 0x81,
    /*             = 0x82,*/
    CTRL_PRESS     = 0x83,
    CTRL_POLYAFTER = 0x84
};

//---------------------------------------------------------
//   MidiCoreEvent
//---------------------------------------------------------

class MidiCoreEvent
{
    OBJECT_ALLOCATOR(engraving, MidiCoreEvent)
protected:
    uint8_t _type    = 0;
    uint8_t _channel = 0;
    uint8_t _a       = 0;
    uint8_t _b       = 0;

public:
    MidiCoreEvent() {}
    MidiCoreEvent(uint8_t t, uint8_t c, uint8_t a, uint8_t b)
        : _type(t), _channel(c), _a(a), _b(b) {}

    void set(uint8_t t, uint8_t c, uint8_t a, uint8_t b)
    {
        _type    = t;
        _channel = c;
        _a       = a;
        _b       = b;
    }

    uint8_t type() const { return _type; }
    void  setType(uint8_t t) { _type = t; }
    uint8_t channel() const { return _channel; }
    void  setChannel(uint8_t c) { _channel = c; }

    uint8_t dataA() const { return _a; }
    uint8_t pitch() const { return _a; }
    uint8_t controller() const { return _a; }

    void setDataA(int v) { _a = static_cast<uint8_t>(v); }
    void setPitch(int v) { _a = static_cast<uint8_t>(v); }
    void setController(int v) { _a = static_cast<uint8_t>(v); }

    uint8_t dataB() const { return _b; }
    uint8_t velo() const { return _b; }
    uint8_t value() const { return _b; }

    void setDataB(int v) { _b = static_cast<uint8_t>(v); }
    void setVelo(int v) { _b = static_cast<uint8_t>(v); }
    void setValue(int v) { _b = static_cast<uint8_t>(v); }

    void setData(int a, int b) { _a = static_cast<uint8_t>(a); _b = static_cast<uint8_t>(b); }
    void setData(int t, int a, int b) { _type = static_cast<uint8_t>(t); _a = static_cast<uint8_t>(a); _b = static_cast<uint8_t>(b); }

    bool isChannelEvent() const
    {
        switch (_type) {
        case ME_NOTEOFF:
        case ME_NOTEON:
        case ME_POLYAFTER:
        case ME_CONTROLLER:
        case ME_PROGRAM:
        case ME_AFTERTOUCH:
        case ME_PITCHBEND:
        case ME_NOTE:
        case ME_CHORD:
            return true;
        default:
            break;
        }

        return false;
    }

    bool operator==(const MidiCoreEvent& e) const
    {
        return e._type == _type && e._channel == _channel && e._a == _a && e._b == _b;
    }

    uint32_t toPackage() const
    {
        union {
            unsigned char data_as_bytes[4];
            uint32_t data_as_uint32 = 0;
        } u;

        switch (type()) {
        case EventType::ME_NOTEOFF:
            u.data_as_bytes[0] = 0x80 | channel();
            u.data_as_bytes[1] = static_cast<unsigned char>(pitch());
            u.data_as_bytes[2] = 0;
            u.data_as_bytes[3] = 0;
            break;

        case EventType::ME_NOTEON:
            u.data_as_bytes[0] = 0x90 | channel();
            u.data_as_bytes[1] = static_cast<unsigned char>(pitch());
            u.data_as_bytes[2] = static_cast<unsigned char>(velo());
            u.data_as_bytes[3] = 0;
            break;

        case EventType::ME_CONTROLLER:
            u.data_as_bytes[0] = 0xB0 | channel();
            u.data_as_bytes[1] = static_cast<unsigned char>(controller());
            u.data_as_bytes[2] = static_cast<unsigned char>(value());
            u.data_as_bytes[3] = 0;
            break;

        case EventType::ME_PROGRAM:
            u.data_as_bytes[0] = 0xC0 | channel();
            u.data_as_bytes[1] = static_cast<unsigned char>(value());
            u.data_as_bytes[2] = 0;
            u.data_as_bytes[3] = 0;
            break;

        case EventType::ME_PITCHBEND:
            u.data_as_bytes[0] = 0xE0 | channel();
            u.data_as_bytes[1] = pitch() & 0x7F;
            u.data_as_bytes[2] = static_cast<unsigned char>(pitch() >> 7);
            u.data_as_bytes[3] = 0;
            break;

        default:
            break;
        }
        return u.data_as_uint32;
    }
};
}
#endif
