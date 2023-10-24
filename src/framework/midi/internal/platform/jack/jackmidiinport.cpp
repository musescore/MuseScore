/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-CLA-applies
 *
 * MuseScore
 * Music Composition & Notation
 *
 * Copyright (C) 2023 MuseScore BVBA and others
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
//#include "linuxmidiinport.h"
#include "jackmidiinport.h"

#include <jack/jack.h>
#include <jack/midiport.h>

#include "midierrors.h"
#include "stringutils.h"
#include "translation.h"
#include "defer.h"
#include "log.h"

struct mu::midi::JackMidiInPort::Jack {
    jack_port_t* midiIn = nullptr;
    jack_client_t* client = nullptr;
    int port = -1;
    int segmentSize;
};

using namespace mu::midi;

void JackMidiInPort::init()
{
    m_jack = std::make_unique<Jack>();
}

void JackMidiInPort::deinit()
{
    if (isConnected()) {
        disconnect();
    }
}

mu::Ret JackMidiInPort::connect(const MidiDeviceID&)
{
    return Ret(true);
}

void JackMidiInPort::disconnect()
{
}

bool JackMidiInPort::isConnected() const
{
    return m_jack && m_jack->midiIn && !m_deviceID.empty();
}

MidiDeviceID JackMidiInPort::deviceID() const
{
    return m_deviceID;
}

bool JackMidiInPort::deviceExists(const MidiDeviceID& deviceId) const
{
    for (const MidiDevice& device : availableDevices()) {
        if (device.id == deviceId) {
            return true;
        }
    }

    return false;
}

bool JackMidiInPort::supportsMIDI20Output() const
{
    return false;
}

// dummy
mu::Ret JackMidiInPort::sendEvent(const Event&)
{
    return mu::make_ok();
}

std::vector<MidiDevice> JackMidiInPort::availableDevices() const
{
    std::vector<MidiDevice> x;
    return x; // dummy
}
