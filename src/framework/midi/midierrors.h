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
#ifndef MU_MIDI_MIDIERRORS_H
#define MU_MIDI_MIDIERRORS_H

#include "ret.h"

namespace mu {
namespace midi {
enum class Err {
    Undefined       = int(Ret::Code::Undefined),
    NoError         = int(Ret::Code::Ok),
    UnknownError    = int(Ret::Code::MidiFirst),

    // synth
    SynthNotInited = 601,
    SoundFontNotLoaded = 602,
    SoundFontFailedLoad = 603,
    SoundFontFailedUnload = 604,

    // midiport
    MidiInvalidDeviceID = 620,
    MidiFailedConnect = 621,
    MidiNotConnected = 622,
    MidiNotSupported = 623,
    MidiSendError = 624
};

inline Ret make_ret(Err e)
{
    return Ret(static_cast<int>(e));
}

inline Ret make_ret(Err e, const std::string& text)
{
    return Ret(static_cast<int>(e), text);
}
}
}

#endif // MU_MIDI_MIDIERRORS_H
