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
#ifndef MU_MIDI_COREMIDIINPORT_H
#define MU_MIDI_COREMIDIINPORT_H

#include <memory>

#include "imidiinport.h"

namespace mu::midi {
class CoreMidiInPort : public IMidiInPort
{
public:
    CoreMidiInPort() = default;
    ~CoreMidiInPort() override;

    void init();

    MidiDeviceList devices() const override;
    async::Notification devicesChanged() const override;

    Ret connect(const MidiDeviceID& deviceID) override;
    void disconnect() override;
    bool isConnected() const override;
    MidiDeviceID deviceID() const override;

    async::Channel<tick_t, Event> eventReceived() const override;

    //internal
    void doProcess(uint32_t message, tick_t timing);

private:
    Ret run();
    void stop();

    void initCore();

    struct Core;
    std::unique_ptr<Core> m_core;
    MidiDeviceID m_deviceID;
    async::Notification m_devicesChanged;

    bool m_running = false;
    async::Channel<tick_t, Event> m_eventReceived;
};
}

#endif // MU_MIDI_COREMIDIINPORT_H
