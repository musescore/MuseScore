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
#ifndef MUSE_MIDI_DUMMYMIDIPORT_H
#define MUSE_MIDI_DUMMYMIDIPORT_H

#include "../imidioutport.h"

namespace muse::midi {
class DummyMidiOutPort : public IMidiOutPort
{
public:

    void init();

    MidiDeviceList availableDevices() const override;
    async::Notification availableDevicesChanged() const override;

    Ret connect(const MidiDeviceID& deviceID) override;
    void disconnect() override;
    bool isConnected() const override;
    MidiDeviceID deviceID() const override;

    bool supportsMIDI20Output() const override;

    Ret sendEvent(const Event& e) override;

private:
    MidiDeviceID m_connectedDeviceID;
};
}

#endif // MUSE_MIDI_DUMMYMIDIPORT_H
