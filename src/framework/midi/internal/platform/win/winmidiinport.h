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
#ifndef MUSE_MIDI_WINMIDIINPORT_H
#define MUSE_MIDI_WINMIDIINPORT_H

#include <memory>

#include "async/asyncable.h"

#include "imidiinport.h"
#include "internal/midideviceslistener.h"

namespace muse::midi {
class WinMidiInPort : public IMidiInPort, public async::Asyncable
{
public:
    WinMidiInPort() = default;
    ~WinMidiInPort() = default;

    void init();
    void deinit();

    MidiDeviceList availableDevices() const override;
    async::Notification availableDevicesChanged() const override;

    Ret connect(const MidiDeviceID& deviceID) override;
    void disconnect() override;
    bool isConnected() const override;
    MidiDeviceID deviceID() const override;
    async::Notification deviceChanged() const override;

    async::Channel<tick_t, Event> eventReceived() const override;

    // internal;
    void doProcess(uint32_t message, tick_t timing);

private:
    Ret run();
    void stop();

    struct Win;
    std::shared_ptr<Win> m_win;
    MidiDeviceID m_deviceID;
    bool m_running = false;
    async::Notification m_deviceChanged;

    async::Notification m_availableDevicesChanged;
    MidiDevicesListener m_devicesListener;

    mutable std::mutex m_devicesMutex;

    async::Channel<tick_t, Event > m_eventReceived;
};
}

#endif // MUSE_MIDI_WINMIDIINPORT_H
