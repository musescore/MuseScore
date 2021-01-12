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
#ifndef MU_MIDI_IMIDIPORTDATASENDER_H
#define MU_MIDI_IMIDIPORTDATASENDER_H

#include "modularity/imoduleexport.h"
#include "miditypes.h"

namespace mu {
namespace midi {
class IMidiPortDataSender : MODULE_EXPORT_INTERFACE
{
    INTERFACE_ID(IMidiPortDataSender)

public:
    virtual ~IMidiPortDataSender() = default;

    virtual void setMidiStream(std::shared_ptr<MidiStream> stream) = 0;

    virtual bool sendEvents(tick_t from, tick_t toTick) = 0;
    virtual bool sendSingleEvent(const midi::Event& event) = 0;
};
}
}

#endif // MU_MIDI_IMIDIPORTDATASENDER_H
