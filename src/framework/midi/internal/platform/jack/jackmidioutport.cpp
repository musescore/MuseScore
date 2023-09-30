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
//#include "defer.h"
#include "log.h"

struct mu::midi::JackMidiOutPort::Jack {
    jack_port_t* midiOut = nullptr;
    jack_client_t* client = nullptr;
    int port = -1;
};

using namespace mu::midi;

void JackMidiOutPort::init()
{
    LOGI("---- linux JACK init ----");
    m_jack = std::make_shared<Jack>();
    /*

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
    */
}

void JackMidiOutPort::deinit()
{
    if (isConnected()) {
        disconnect();
    }
}

std::vector<MidiDevice> JackMidiOutPort::availableDevices() const
{
    //LOGI("---- linux JACK availableDevices ----");
    std::vector<MidiDevice> ret;
    MidiDevice dev;
    dev.name = "JACK";
    dev.id = makeUniqueDeviceId(0,9999,0);
    ret.push_back(std::move(dev));
    return ret;
}

mu::Ret JackMidiOutPort::connect(const MidiDeviceID& deviceID)
{
    LOGI("---- linux JACK connect ----");
    //----> common with jackaudio <----
    jack_client_t* client;
    jack_options_t options = (jack_options_t)0;
    jack_status_t status;
    client = jack_client_open("MuseScore", options, &status);
    if (!client) {
        return make_ret(Err::MidiInvalidDeviceID, "jack-open fail, device: " + deviceID);
    }
    return Ret(true);
}

void JackMidiOutPort::disconnect()
{
    LOGI("---- linux JACK disconnect ----");
    if (!isConnected()) {
        return;
    }

    //FIX:
    //const char* sn = jack_port_name((jack_port_t*) src);
    //const char* dn = jack_port_name((jack_port_t*) dst);
    //jackdisconnect(m_jack->midiOut, 0, m_jack->client, m_jack->port);
    if (jack_deactivate(m_jack->client)) {
        LOGE() << "failed to deactive jack";
    }

    m_jack->client = nullptr;
    m_jack->port = -1;
    m_jack->midiOut = nullptr;
    m_deviceID.clear();
}

bool JackMidiOutPort::isConnected() const
{
    LOGI("---- linux JACK isConnect ----");
    return m_jack && m_jack->midiOut && !m_deviceID.empty();
}

MidiDeviceID JackMidiOutPort::deviceID() const
{
    LOGI("---- linux JACK deviceID ----");
    return m_deviceID;
}

bool JackMidiOutPort::supportsMIDI20Output() const
{
    LOGI("---- linux JACK supportsMIDI20Output ----");
    return false;
}

mu::Ret JackMidiOutPort::sendEvent(const Event& e)
{
    LOGI() << "---- linux JACK sendEvent ----" << e.to_string();
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

//    snd_seq_event_t seqev;
//    memset(&seqev, 0, sizeof(seqev));
//    snd_seq_ev_set_direct(&seqev);
//    snd_seq_ev_set_source(&seqev, 0);
//    snd_seq_ev_set_dest(&seqev, SND_SEQ_ADDRESS_SUBSCRIBERS, 0);

    switch (e.opcode()) {
    case Event::Opcode::NoteOn:
//        snd_seq_ev_set_noteon(&seqev, e.channel(), e.note(), e.velocity());
        break;
    case Event::Opcode::NoteOff:
//        snd_seq_ev_set_noteoff(&seqev, e.channel(), e.note(), e.velocity());
        break;
    case Event::Opcode::ProgramChange:
//        snd_seq_ev_set_pgmchange(&seqev, e.channel(), e.program());
        break;
    case Event::Opcode::ControlChange:
//        snd_seq_ev_set_controller(&seqev, e.channel(), e.index(), e.data());
        break;
    case Event::Opcode::PitchBend:
//        snd_seq_ev_set_pitchbend(&seqev, e.channel(), e.data() - 8192);
        break;
    default:
        NOT_SUPPORTED << "event: " << e.to_string();
        return make_ret(Err::MidiNotSupported);
    }

//    snd_seq_event_output_direct(m_jack->midiOut, &seqev);

    return Ret(true);
}

bool JackMidiOutPort::deviceExists(const MidiDeviceID& deviceId) const
{
    LOGI("---- linux JACK deviceExists ----");
    for (const MidiDevice& device : availableDevices()) {
        if (device.id == deviceId) {
            return true;
        }
    }

    return false;
}
