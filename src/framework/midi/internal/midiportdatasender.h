/**
 * // SPDX-License-Identifier: GPL-3.0-only
 * // MuseScore-CLA-applies
 * //=============================================================================
 * //  MuseScore
 * //  Music Composition & Notation
 * //
 * //  Copyright (C) 2021 MuseScore BVBA and others
 * //
 * //  This program is free software: you can redistribute it and/or modify
 * //  it under the terms of the GNU General Public License version 3 as
 * //  published by the Free Software Foundation.
 * //
 * //  This program is distributed in the hope that it will be useful,
 * //  but WITHOUT ANY WARRANTY; without even the implied warranty of
 * //  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * //  GNU General Public License for more details.
 * //
 * //  You should have received a copy of the GNU General Public License
 * //  along with this program.  If not, see <https://www.gnu.org/licenses/>.
 * //============================================================================= **/
#ifndef MU_MIDI_MIDIPORTDATASENDER_H
#define MU_MIDI_MIDIPORTDATASENDER_H

#include "../imidiportdatasender.h"

#include "modularity/ioc.h"
#include "../imidioutport.h"
#include "async/asyncable.h"

namespace mu::midi {
class MidiPortDataSender : public IMidiPortDataSender, public async::Asyncable
{
    INJECT(midi, IMidiOutPort, midiOutPort)

public:

    void setMidiStream(std::shared_ptr<MidiStream> stream) override;

    bool sendEvents(tick_t from, tick_t toTick) override;
    bool sendSingleEvent(const midi::Event& event) override;

private:

    void onChunkReceived(const Chunk& chunk);

    std::shared_ptr<MidiStream> m_stream;
    bool m_isStreamConnected = false;
    MidiData m_midiData;
};
}

#endif // MU_MIDI_MIDIPORTDATASENDER_H
