/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-CLA-applies
 *
 * MuseScore
 * Music Composition & Notation
 *
 * Copyright (C) MuseScore BVBA and others
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
#ifndef MU_MIDI_MIDIPORTSTATE_H
#define MU_MIDI_MIDIPORTSTATE_H

#include "types/ret.h"
#include "miditypes.h"

namespace muse::midi {
class MidiPortState
{

public:
    virtual ~MidiPortState() = default;

    virtual void init() = 0;
    virtual void deinit() = 0;

    virtual std::vector<MidiDevice> availableDevices() const = 0;

    virtual Ret connect(const MidiDeviceID& deviceID) = 0;
    virtual void disconnect() = 0;
    virtual bool isConnected() const = 0;
    virtual MidiDeviceID deviceID() const = 0;

    //
    // Whether the output port supports it, rather than whether the receiver supports it
    // (If the receiver does not support MIDI 2.0, then it's the output port's resposibility to convert to MIDI 1.0)
    virtual bool supportsMIDI20Output() const = 0;

    virtual Ret sendEvent(const Event& e) = 0;
};
}


#endif // MU_MIDI_MIDIPORTSTATE_H
