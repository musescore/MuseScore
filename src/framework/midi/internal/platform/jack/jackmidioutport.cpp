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
#include "log.h"

struct mu::midi::JackMidiOutPort::Jack {
    jack_port_t* midiOut = nullptr;
    jack_client_t* client = nullptr;
    int port = -1;
    int segmentSize;
};

using namespace mu::midi;

void JackMidiOutPort::init()
{
    LOGI("---- linux JACK init ----");
    m_jack = std::make_shared<Jack>();
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
    m_jack->client = client;
    m_jack->segmentSize = jack_get_buffer_size(client);
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

// FIX: is opcode an compatible MIDI enumeration?

mu::Ret sendEvent_noteonoff (void *pb, int framePos, const Event& e) {
    unsigned char* p = jack_midi_event_reserve(pb, framePos, 3);
    if (p == 0) {
        qDebug("JackMidi: buffer overflow, event lost");
        return mu::Ret(false);
    }
    if (e.opcode() == Event::Opcode::NoteOn) {
        p[0] = /* e.opcode() */ 0x90 | e.channel();
    } else {
        p[0] = /* e.opcode() */ 0x80 | e.channel();
    }
    p[1] = e.note();
    p[2] = e.velocity();
    return mu::Ret(true);
}

mu::Ret sendEvent_control (void *pb, int framePos, const Event& e) {
    unsigned char* p = jack_midi_event_reserve(pb, framePos, 3);
    if (p == 0) {
        qDebug("JackMidi: buffer overflow, event lost");
        return mu::Ret(false);
    }
    p[0] = /* e.opcode() */ 0xb0 | e.channel();
    p[1] = e.index();
    p[2] = e.data();
    return mu::Ret(true);
}

mu::Ret sendEvent_program (void *pb, int framePos, const Event& e) {
    unsigned char* p = jack_midi_event_reserve(pb, framePos, 2);
    if (p == 0) {
        qDebug("JackMidi: buffer overflow, event lost");
        return mu::Ret(false);
    }
    p[0] = /* e.opcode() */ 0xc0 | e.channel();
    p[1] = e.program();
    return mu::Ret(true);
}

mu::Ret sendEvent_pitchbend (void *pb, int framePos, const Event& e) {
    unsigned char* p = jack_midi_event_reserve(pb, framePos, 3);
    if (p == 0) {
        qDebug("JackMidi: buffer overflow, event lost");
        return mu::Ret(false);
    }
    p[0] = /* e.opcode() */ 0xe0 | e.channel();
    p[1] = e.data(); // dataA
    p[2] = e.velocity(); // dataB
    return mu::Ret(true);
}

mu::Ret JackMidiOutPort::sendEvent(const Event& e)
{
    int framePos = 0;
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

    //  int portIdx = seq->score()->midiPort(e.channel()); ??
    jack_port_t* port = m_jack->midiOut; // [portIdx];
    void* pb = jack_port_get_buffer(port, m_jack->segmentSize);

    switch (e.opcode()) {
    // FIX: Event::Opcode::POLYAFTER ?
    case Event::Opcode::NoteOn:
        return sendEvent_noteonoff(pb, framePos, e);
    case Event::Opcode::NoteOff:
        return sendEvent_noteonoff(pb, framePos, e);
    case Event::Opcode::ControlChange:
        return sendEvent_control(pb, framePos, e);
    case Event::Opcode::ProgramChange:
        return sendEvent_control(pb, framePos, e);
    case Event::Opcode::PitchBend:
        return sendEvent_pitchbend(pb, framePos, e);
    default:
        NOT_SUPPORTED << "event: " << e.to_string();
        return make_ret(Err::MidiNotSupported);
    }

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
