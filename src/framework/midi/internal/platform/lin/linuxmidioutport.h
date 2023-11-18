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
#ifndef MU_MIDI_LINUXMIDIOUTPORT_H
#define MU_MIDI_LINUXMIDIOUTPORT_H

#include <memory>

#include "modularity/ioc.h"
#include "async/asyncable.h"
#include "framework/audio/audiomodule.h"
#include "midi/imidioutport.h"
#include "internal/midideviceslistener.h"

#if defined(JACK_AUDIO)
#include "internal/platform/jack/jackmidioutport.h"
#endif

#include "internal/platform/alsa/alsamidioutport.h"

namespace muse::midi {
class LinuxMidiOutPort : public IMidiOutPort, public async::Asyncable
{
    Inject<muse::audio::IAudioDriver> audioDriver;
public:
    LinuxMidiOutPort() = default;
    ~LinuxMidiOutPort() = default;

    void init();
    void deinit();

    std::vector<MidiDevice> availableDevices() const override;
    async::Notification availableDevicesChanged() const override;

    Ret connect(const MidiDeviceID& deviceID) override;
    void disconnect() override;
    bool isConnected() const override;
    MidiDeviceID deviceID() const override;
    async::Notification deviceChanged() const override;

    bool supportsMIDI20Output() const override;

    Ret sendEvent(const Event& e) override;

private:
    bool deviceExists(const MidiDeviceID& deviceId) const;

    MidiDeviceID m_deviceID;
    async::Notification m_deviceChanged;

    async::Notification m_availableDevicesChanged;
    MidiDevicesListener m_devicesListener;

    mutable std::mutex m_devicesMutex;

    MidiPortState* m_midiOutPortCurrent;
#if defined(JACK_AUDIO)
    std::unique_ptr<JackMidiOutPort> m_midiOutPortJack;
#endif
    std::unique_ptr<AlsaMidiOutPort> m_midiOutPortAlsa;
};
}

#endif // MU_MIDI_ALSAMIDIOUTPORT_H
