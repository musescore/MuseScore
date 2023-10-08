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
#include "framework/audio/audiomodule.h"
#include "jackmidioutport.h"

#include <jack/jack.h>
#include <jack/midiport.h>

#include "midierrors.h"
#include "translation.h"
#include "log.h"

using namespace muse::midi;

void JackMidiOutPort::init()
{
}

void JackMidiOutPort::deinit()
{
    if (isConnected()) {
        disconnect();
    }
}

std::vector<MidiDevice> JackMidiOutPort::availableDevices() const
{
    std::vector<MidiDevice> ret;
    MidiDevice dev;
    dev.name = "JACK";
    dev.id = makeUniqueDeviceId(0, 9999, 0);
    ret.push_back(std::move(dev));
    return ret;
}

muse::Ret JackMidiOutPort::connect(const MidiDeviceID& deviceID)
{
    if (!m_jack->client) {
        return make_ret(Err::MidiInvalidDeviceID, "jack-open fail, device: " + deviceID);
    }
    return muse::Ret(true);
}

void JackMidiOutPort::disconnect()
{
    if (!isConnected()) {
        return;
    }
    if (!m_jack->client) {
        return;
    }
    jack_client_t* client = static_cast<jack_client_t*>(m_jack->client);

    //FIX:
    //const char* sn = jack_port_name((jack_port_t*) src);
    //const char* dn = jack_port_name((jack_port_t*) dst);
    //jackdisconnect(m_jack->midiOut, 0, m_jack->client, m_jack->port);
    if (jack_deactivate(client)) {
        LOGE() << "failed to deactive jack";
    }
    m_jack->port = -1;
    m_jack->midiOut = nullptr;
    m_deviceID.clear();
}

bool JackMidiOutPort::isConnected() const
{
    return !(m_jack.get() && m_jack->midiOut && !m_deviceID.empty());
}

MidiDeviceID JackMidiOutPort::deviceID() const
{
    return m_deviceID;
}

bool JackMidiOutPort::supportsMIDI20Output() const
{
    return false;
}

bool JackMidiOutPort::deviceExists(const MidiDeviceID& deviceId) const
{
    for (const MidiDevice& device : availableDevices()) {
        if (device.id == deviceId) {
            return true;
        }
    }

    return false;
}

// Not used
muse::Ret JackMidiOutPort::sendEvent(const Event&)
{
    return muse::Ret(true);
}
