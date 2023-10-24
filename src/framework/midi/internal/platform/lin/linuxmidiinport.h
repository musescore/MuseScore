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
#ifndef MU_MIDI_LINUXMIDIINPORT_H
#define MU_MIDI_LINUXMIDIINPORT_H

#include <memory>
#include <thread>

#include "async/asyncable.h"

#include "framework/audio/audiomodule.h"
#include "imidiinport.h"
#include "internal/midideviceslistener.h"

#if defined(JACK_AUDIO)
#include "internal/platform/jack/jackmidiinport.h"
#endif

#include "internal/platform/alsa/alsamidiinport.h"

namespace muse::midi {
class LinuxMidiInPort : public IMidiInPort, public async::Asyncable
{
    Inject<muse::audio::IAudioDriver> audioDriver;
public:
    LinuxMidiInPort() = default;
    ~LinuxMidiInPort() = default;

    void init(std::shared_ptr<muse::audio::AudioModule> am);
    void deinit();

    std::vector<MidiDevice> availableDevices() const override;
    async::Notification availableDevicesChanged() const override;

    Ret connect(const MidiDeviceID& deviceID) override;
    void disconnect() override;
    bool isConnected() const override;
    MidiDeviceID deviceID() const override;
    async::Notification deviceChanged() const override;

    async::Channel<tick_t, Event> eventReceived() const override;

private:
    Ret run();
    void stop();

    static void process(LinuxMidiInPort* self);
    void doProcess();

    bool deviceExists(const MidiDeviceID& deviceId) const;

    MidiDeviceID m_deviceID;
    std::shared_ptr<std::thread> m_thread;
    std::atomic<bool> m_running{ false };
    async::Notification m_deviceChanged;

    async::Notification m_availableDevicesChanged;
    MidiDevicesListener m_devicesListener;

    mutable std::mutex m_devicesMutex;

    async::Channel<tick_t, Event > m_eventReceived;

#if defined(JACK_AUDIO)
    std::unique_ptr<JackMidiInPort> m_midiInPortJack;
#endif
    std::unique_ptr<AlsaMidiInPort> m_midiInPortAlsa;
};
}

#endif // MU_MIDI_LINUXMIDIINPORT_H
