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

#ifndef MUSE_AUDIO_JACKAUDIODRIVER_H
#define MUSE_AUDIO_JACKAUDIODRIVER_H

#include <jack/jack.h>

#include "framework/midi/miditypes.h"
#include "iaudiodriver.h"

namespace muse::audio {
class JackDriverState : public AudioDriverState
{
public:
    JackDriverState();
    ~JackDriverState();

    std::string name() const override;
    bool open(const IAudioDriver::Spec& spec, IAudioDriver::Spec* activeSpec) override;
    void close() override;
    bool isOpened() const override;
    bool pushMidiEvent(muse::midi::Event& e) override;
    void registerMidiInputQueue(async::Channel<muse::midi::tick_t, muse::midi::Event>) override;

    std::string deviceName() const;
    void deviceName(const std::string newDeviceName);
    std::vector<muse::midi::MidiDevice> availableMidiDevices(muse::midi::MidiPortDirection direction) const;

    void* m_jackDeviceHandle = nullptr;
    float* m_buffer = nullptr;
    std::vector<jack_port_t*> m_outputPorts;
    std::vector<jack_port_t*> m_midiInputPorts;
    std::vector<jack_port_t*> m_midiOutputPorts;
    ThreadSafeQueue<muse::midi::Event> m_midiQueue;
    async::Channel<muse::midi::tick_t, muse::midi::Event> m_eventReceived;

private:
    std::string m_deviceName;
};
}

#endif // MUSE_AUDIO_JACKAUDIODRIVER_H
