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
#ifndef MU_MIDI_IMIDIOUTPORT_H
#define MU_MIDI_IMIDIOUTPORT_H

#include <vector>
#include <string>

#include "modularity/imoduleexport.h"
#include "miditypes.h"
#include "ret.h"

namespace mu {
namespace midi {
class IMidiOutPort : MODULE_EXPORT_INTERFACE
{
    INTERFACE_ID(IMidiPort)

public:
    virtual ~IMidiOutPort() = default;

    virtual std::vector<MidiDevice> devices() const = 0;

    virtual Ret connect(const MidiDeviceID& deviceID) = 0;
    virtual void disconnect() = 0;
    virtual bool isConnected() const = 0;
    virtual MidiDeviceID deviceID() const = 0;

    virtual Ret sendEvent(const Event& e) = 0;
};
}
}

#endif // MU_MIDI_IMIDIPORT_H
