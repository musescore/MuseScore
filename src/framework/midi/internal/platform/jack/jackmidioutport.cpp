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
#include "jackmidioutport.h"

#include <jack/jack.h>
#include <jack/midiport.h>

#include "midierrors.h"
#include "translation.h"
#include "defer.h"
#include "log.h"

struct mu::midi::JackMidiOutPort::Jack {
    jack_port_t* midiOut = nullptr;
    int client = -1;
    int port = -1;
};

using namespace mu::midi;

void JackMidiOutPort::init()
{
    m_jack = std::make_shared<Jack>();

    m_devicesListener.startWithCallback([this]() {
        return availableDevices();
    });

    m_devicesListener.devicesChanged().onNotify(this, [this]() {
        bool connectedDeviceRemoved = true;
        for (const MidiDevice& device: availableDevices()) {
            if (m_deviceID == device.id) {
                connectedDeviceRemoved = false;
            }
        }

        if (connectedDeviceRemoved) {
            disconnect();
        }

        m_availableDevicesChanged.notify();
    });
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
    ret.push_back({ NONE_DEVICE_ID, trc("midi", "No device") });

    MidiDevice dev;
    dev.name = "jack"; //snd_seq_client_info_get_name(cinfo);
    int client = 0; //FIXsnd_seq_port_info_get_client(pinfo);
    int port = 0; //FIXsnd_seq_port_info_get_port(pinfo);
    dev.id = makeUniqueDeviceId(0, client, port);
    ret.push_back(std::move(dev));
    return ret;
}

mu::async::Notification JackMidiOutPort::availableDevicesChanged() const
{
    return m_availableDevicesChanged;
}

mu::Ret JackMidiOutPort::connect(const MidiDeviceID& deviceID)
{
    //----> common with jackaudio <----
    jack_client_t* client;
    client = jack_client_open(_jackName, options, &status);
    //----><----
    if (!deviceExists(deviceID)) {
        return make_ret(Err::MidiFailedConnect, "not found device, id: " + deviceID);
    }

    DEFER {
        m_deviceChanged.notify();
    };

    Ret ret = make_ok();

    if (!deviceID.empty() && deviceID != NONE_DEVICE_ID) {
        std::vector<int> deviceParams = splitDeviceId(deviceID);
        IF_ASSERT_FAILED(deviceParams.size() == 3) {
            return make_ret(Err::MidiInvalidDeviceID, "invalid device id: " + deviceID);
        }

        if (isConnected()) {
            disconnect();
        }

        jack_port_t* port = jack_port_register(client, "MuseScore-midi", JACK_DEFAULT_MIDI_TYPE, JackPortIsOutput, 0);
        if (port == 0) {
            return make_ret(Err::MidiFailedConnect, "failed open jack-midi");
        } else {
            m_jack->midiOut = port;
        }

        m_jack->client = deviceParams.at(1);
        m_jack->port = deviceParams.at(2);
    }

    m_deviceID = deviceID;

    if (ret) {
        LOGD() << "Connected to " << m_deviceID;
    }

    return Ret(true);
}

void JackMidiOutPort::disconnect()
{
    if (!isConnected()) {
        return;
    }

    snd_seq_disconnect_from(m_jack->midiOut, 0, m_jack->client, m_jack->port);
    snd_seq_close(m_jack->midiOut);

    LOGD() << "Disconnected from " << m_deviceID;

    m_jack->client = -1;
    m_jack->port = -1;
    m_jack->midiOut = nullptr;
    m_deviceID.clear();
}

bool JackMidiOutPort::isConnected() const
{
    return m_jack && m_jack->midiOut && !m_deviceID.empty();
}

MidiDeviceID JackMidiOutPort::deviceID() const
{
    return m_deviceID;
}

mu::async::Notification JackMidiOutPort::deviceChanged() const
{
    return m_deviceChanged;
}

bool JackMidiOutPort::supportsMIDI20Output() const
{
    return false;
}

mu::Ret JackMidiOutPort::sendEvent(const Event& e)
{
    // LOGI() << e.to_string();

    if (!isConnected()) {
        return make_ret(Err::MidiNotConnected);
    }

    if (e.isChannelVoice20()) {
        auto events = e.toMIDI10();
        for (auto& event : events) {
            mu::Ret ret = sendEvent(event);
            if (!ret) {
                return ret;
            }
        }
        return Ret(true);
    }

    snd_seq_event_t seqev;
    memset(&seqev, 0, sizeof(seqev));
    snd_seq_ev_set_direct(&seqev);
    snd_seq_ev_set_source(&seqev, 0);
    snd_seq_ev_set_dest(&seqev, SND_SEQ_ADDRESS_SUBSCRIBERS, 0);

    switch (e.opcode()) {
    case Event::Opcode::NoteOn:
        snd_seq_ev_set_noteon(&seqev, e.channel(), e.note(), e.velocity());
        break;
    case Event::Opcode::NoteOff:
        snd_seq_ev_set_noteoff(&seqev, e.channel(), e.note(), e.velocity());
        break;
    case Event::Opcode::ProgramChange:
        snd_seq_ev_set_pgmchange(&seqev, e.channel(), e.program());
        break;
    case Event::Opcode::ControlChange:
        snd_seq_ev_set_controller(&seqev, e.channel(), e.index(), e.data());
        break;
    case Event::Opcode::PitchBend:
        snd_seq_ev_set_pitchbend(&seqev, e.channel(), e.data() - 8192);
        break;
    default:
        NOT_SUPPORTED << "event: " << e.to_string();
        return make_ret(Err::MidiNotSupported);
    }

    snd_seq_event_output_direct(m_jack->midiOut, &seqev);

    return Ret(true);
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
