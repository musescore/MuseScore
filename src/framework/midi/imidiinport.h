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
#ifndef MU_MIDI_IMIDIINPORT_H
#define MU_MIDI_IMIDIINPORT_H

#include "modularity/imoduleexport.h"

#include "ret.h"
#include "async/channel.h"
#include "async/notification.h"
#include "miditypes.h"

namespace mu::midi {
class IMidiInPort : MODULE_EXPORT_INTERFACE
{
    INTERFACE_ID(IMidiInPort)

public:
    virtual ~IMidiInPort() = default;

    virtual MidiDeviceList devices() const = 0;
    virtual async::Notification devicesChanged() const = 0;

    virtual Ret connect(const MidiDeviceID& deviceID) = 0;
    virtual void disconnect() = 0;
    virtual bool isConnected() const = 0;
    virtual MidiDeviceID deviceID() const = 0;

    virtual Ret run() = 0;
    virtual void stop() = 0;
    virtual bool isRunning() const = 0;
    virtual async::Channel<tick_t, Event> eventReceived() const = 0;
};
}

#endif // MU_MIDI_IMIDIINPORT_H
