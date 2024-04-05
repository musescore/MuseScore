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
#ifndef MUSE_MIDI_MIDIERRORS_H
#define MUSE_MIDI_MIDIERRORS_H

#include "types/ret.h"

namespace muse::midi {
enum class Err {
    Undefined       = int(Ret::Code::Undefined),
    NoError         = int(Ret::Code::Ok),
    UnknownError    = int(Ret::Code::MidiFirst),

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

#endif // MUSE_MIDI_MIDIERRORS_H
