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
#ifndef MU_MIDI_JACKMIDIOUTPORT_H
#define MU_MIDI_JACKMIDIOUTPORT_H

#include <memory>
#include "framework/audio/midiqueue.h"
#include "framework/audio/audiomodule.h"
#include "midi/midiportstate.h"

namespace muse::midi {
class JackMidiOutPort : public MidiPortState
{
public:
    JackMidiOutPort() = default;
    ~JackMidiOutPort() = default;
    void init();
    void deinit();

    std::vector<MidiDevice> availableDevices() const override;
    Ret connect(const MidiDeviceID& deviceID) override;
    void disconnect() override;
    bool isConnected() const override;
    MidiDeviceID deviceID() const override;
    bool supportsMIDI20Output() const override;
    Ret sendEvent(const Event& e) override;

private:
    bool deviceExists(const MidiDeviceID& deviceId) const;
    std::shared_ptr<ThreadSafeQueue<const Event> > m_midiQueue;
    struct Jack {
        void* midiOut = nullptr;
        void* client = nullptr;
        int port = -1;
        int segmentSize;
    };
    std::unique_ptr<Jack> m_jack;
    MidiDeviceID m_deviceID;
};
}

#endif // MU_MIDI_JACKMIDIOUTPORT_H
