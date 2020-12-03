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
#ifndef MU_MIDI_IMIDIINPORT_H
#define MU_MIDI_IMIDIINPORT_H

#include <vector>

#include "modularity/ioc.h"

#include "miditypes.h"
#include "ret.h"
#include "async/channel.h"

namespace mu {
namespace midi {
class IMidiInPort : MODULE_EXPORT_INTERFACE
{
    INTERFACE_ID(IMidiInPort)
public:
    virtual ~IMidiInPort() = default;

    virtual std::vector<MidiDevice> devices() const = 0;

    virtual Ret connect(const MidiDeviceID& deviceID) = 0;
    virtual void disconnect() = 0;
    virtual bool isConnected() const = 0;
    virtual MidiDeviceID deviceID() const = 0;

    virtual Ret run() = 0;
    virtual void stop() = 0;
    virtual bool isRunning() const = 0;
    virtual async::Channel<std::pair<tick_t, Event> > eventReceived() const = 0;
};
}
}

#endif // MU_MIDI_IMIDIINPORT_H
