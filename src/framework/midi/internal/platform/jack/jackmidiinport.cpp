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
#include "jackmidiinport.h"

#include <jack/jack.h>
#include <jack/midiport.h>

#include "midierrors.h"
#include "stringutils.h"
#include "translation.h"
#include "defer.h"
#include "log.h"

using namespace muse::midi;

void JackMidiInPort::init()
{
    LOGI("---- linux JACK-midi input init ----");
    m_jack = std::make_unique<Jack>();
}

void JackMidiInPort::deinit()
{
    if (isConnected()) {
        disconnect();
    }
}

std::vector<MidiDevice> JackMidiInPort::availableDevices() const
{
    std::vector<MidiDevice> ports;

    std::vector<MidiDevice> ret;

    const char** prts = jack_get_ports(m_jack->client, 0, 0, 0);
    int devIndex = 0;
    for (const char** p = prts; p && *p; ++p) {
        jack_port_t* port = jack_port_by_name(m_jack->client, *p);
        int flags = jack_port_flags(port);
        if (!(flags & JackPortIsInput))
              continue;
        char buffer[128];
        strncpy(buffer, *p, sizeof(buffer) - 1);
        buffer[sizeof(buffer) - 1] = 0;
        if (strncmp(buffer, "MuseScore", 9) == 0)
              continue;
        MidiDevice dev;
        dev.name = buffer;
        dev.id = makeUniqueDeviceId(devIndex++,0,0);
        ports.push_back(std::move(dev));
    }
    return ports;
}

muse::Ret JackMidiInPort::connect(const MidiDeviceID& deviceID)
{
    return muse::Ret(true);
}

void JackMidiInPort::disconnect()
{
}

bool JackMidiInPort::isConnected() const
{
    LOGI("---- JACK input isConnect ----");
    return m_jack && m_jack->midiIn && !m_deviceID.empty();
}

MidiDeviceID JackMidiInPort::deviceID() const
{
    return m_deviceID;
}

bool JackMidiInPort::deviceExists(const MidiDeviceID& deviceId) const
{
    LOGI("---- JACK-midi input deviceExists ----");
    for (const MidiDevice& device : availableDevices()) {
        if (device.id == deviceId) {
            return true;
        }
    }

    return false;
}
