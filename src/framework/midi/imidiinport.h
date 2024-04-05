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
#ifndef MUSE_MIDI_IMIDIINPORT_H
#define MUSE_MIDI_IMIDIINPORT_H

#include "modularity/imoduleinterface.h"

#include "types/ret.h"
#include "async/channel.h"
#include "async/notification.h"
#include "miditypes.h"

namespace muse::midi {
class IMidiInPort : MODULE_EXPORT_INTERFACE
{
    INTERFACE_ID(IMidiInPort)

public:
    virtual ~IMidiInPort() = default;

    virtual MidiDeviceList availableDevices() const = 0;
    virtual async::Notification availableDevicesChanged() const = 0;

    virtual Ret connect(const MidiDeviceID& deviceID) = 0;
    virtual void disconnect() = 0;
    virtual bool isConnected() const = 0;
    virtual MidiDeviceID deviceID() const = 0;
    virtual async::Notification deviceChanged() const = 0;

    virtual async::Channel<tick_t, Event> eventReceived() const = 0;
};
}

#endif // MUSE_MIDI_IMIDIINPORT_H
