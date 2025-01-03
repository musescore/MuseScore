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
#ifndef MUSE_MIDI_COREMIDIOUTPORT_H
#define MUSE_MIDI_COREMIDIOUTPORT_H

#include <memory>

#include "midi/imidioutport.h"

#include "modularity/ioc.h"
#include "imidiconfiguration.h"

namespace muse::midi {
class CoreMidiOutPort : public IMidiOutPort
{
    INJECT(IMidiConfiguration, configuration)

public:
    CoreMidiOutPort();
    ~CoreMidiOutPort() override;

    void init();
    void deinit();

    MidiDeviceList availableDevices() const override;
    async::Notification availableDevicesChanged() const override;

    Ret connect(const MidiDeviceID& deviceID) override;
    void disconnect() override;
    bool isConnected() const override;
    MidiDeviceID deviceID() const override;
    async::Notification deviceChanged() const override;

    bool supportsMIDI20Output() const override;

    Ret sendEvent(const Event& e) override;

private:
    void initCore();

    struct Core;
    std::unique_ptr<Core> m_core;
    MidiDeviceID m_deviceID;
    async::Notification m_deviceChanged;
    async::Notification m_availableDevicesChanged;
};
}

#endif // MUSE_MIDI_COREMIDIOUTPORT_H
