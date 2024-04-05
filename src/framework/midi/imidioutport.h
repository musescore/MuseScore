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
#ifndef MUSE_MIDI_IMIDIOUTPORT_H
#define MUSE_MIDI_IMIDIOUTPORT_H

#include "modularity/imoduleinterface.h"

#include "types/ret.h"
#include "async/notification.h"
#include "miditypes.h"

namespace muse::midi {
class IMidiOutPort : MODULE_EXPORT_INTERFACE
{
    INTERFACE_ID(IMidiOutPort)

public:
    virtual ~IMidiOutPort() = default;

    virtual MidiDeviceList availableDevices() const = 0;
    virtual async::Notification availableDevicesChanged() const = 0;

    virtual Ret connect(const MidiDeviceID& deviceID) = 0;
    virtual void disconnect() = 0;
    virtual bool isConnected() const = 0;
    virtual MidiDeviceID deviceID() const = 0;
    virtual async::Notification deviceChanged() const = 0;

    // Whether the output port supports it, rather than whether the receiver supports it
    // (If the receiver does not support MIDI 2.0, then it's the output port's resposibility to convert to MIDI 1.0)
    virtual bool supportsMIDI20Output() const = 0;

    virtual Ret sendEvent(const Event& e) = 0;
};
}

#endif // MUSE_MIDI_IMIDIPORT_H
