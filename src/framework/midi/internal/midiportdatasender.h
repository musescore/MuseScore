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
#ifndef MU_MIDI_MIDIPORTDATASENDER_H
#define MU_MIDI_MIDIPORTDATASENDER_H

#include "../imidiportdatasender.h"

#include "modularity/ioc.h"
#include "../imidioutport.h"
#include "async/asyncable.h"

namespace mu {
namespace midi {
class MidiPortDataSender : public IMidiPortDataSender, public async::Asyncable
{
    INJECT(midi, IMidiOutPort, midiOutPort)

public:

    void setMidiStream(std::shared_ptr<MidiStream> stream) override;

    bool sendEvents(tick_t from, tick_t toTick) override;

private:

    void onChunkReceived(const Chunk& chunk);

    std::shared_ptr<MidiStream> m_stream;
    bool m_isStreamConnected = false;
    MidiData m_midiData;
};
}
}

#endif // MU_MIDI_MIDIPORTDATASENDER_H
