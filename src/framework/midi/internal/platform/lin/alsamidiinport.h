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
#ifndef MU_MIDI_ALSAMIDIINPORT_H
#define MU_MIDI_ALSAMIDIINPORT_H

#include <memory>
#include <thread>

#include "../../abstractmidiinport.h"
#include "internal/midideviceslistener.h"

namespace mu::midi {
class AlsaMidiInPort : public AbstractMidiInPort
{
public:
    AlsaMidiInPort() = default;
    ~AlsaMidiInPort() override;

    void init() override;

    MidiDeviceList availableDevices() const override;
    async::Notification availableDevicesChanged() const override;

    Ret connect(const MidiDeviceID& deviceID) override;
    void disconnect() override;
    bool isConnected() const override;
    MidiDeviceID deviceID() const override;
    async::Notification deviceChanged() const override;

private:
    Ret run();
    void stop();

    static void process(AlsaMidiInPort* self);
    void doProcess();

    bool deviceExists(const MidiDeviceID& deviceId) const;

    struct Alsa;
    std::shared_ptr<Alsa> m_alsa;
    MidiDeviceID m_deviceID;
    std::shared_ptr<std::thread> m_thread;
    std::atomic<bool> m_running{ false };
    async::Notification m_deviceChanged;

    async::Notification m_availableDevicesChanged;
    MidiDevicesListener m_devicesListener;

    mutable std::mutex m_devicesMutex;
};
}

#endif // MU_MIDI_ALSAMIDIINPORT_H
