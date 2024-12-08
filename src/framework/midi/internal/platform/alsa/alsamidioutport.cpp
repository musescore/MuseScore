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
#include "alsamidioutport.h"

#include <alsa/asoundlib.h>
#include <alsa/seq.h>
#include <alsa/seq_midi_event.h>

#include "midierrors.h"
#include "translation.h"
#include "defer.h"
#include "log.h"

struct muse::midi::AlsaMidiOutPort::Alsa {
    snd_seq_t* midiOut = nullptr;
    int client = -1;
    int port = -1;
};

using namespace muse;
using namespace muse::midi;

void AlsaMidiOutPort::init()
{
    LOGI("---- linux ALSA init ----");
    m_alsa = std::make_shared<Alsa>();

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

void AlsaMidiOutPort::deinit()
{
    LOGI("---- linux ALSA deinit ----");
    if (isConnected()) {
        disconnect();
    }
}

std::vector<MidiDevice> AlsaMidiOutPort::availableDevices() const
{
    int streams = SND_SEQ_OPEN_OUTPUT;
    const unsigned int cap = SND_SEQ_PORT_CAP_SUBS_WRITE | SND_SEQ_PORT_CAP_WRITE;
    const unsigned int type_hw = SND_SEQ_PORT_TYPE_PORT | SND_SEQ_PORT_TYPE_HARDWARE;
    const unsigned int type_sw = SND_SEQ_PORT_TYPE_PORT | SND_SEQ_PORT_TYPE_SOFTWARE;

    std::vector<MidiDevice> ret;

    snd_seq_client_info_t* cinfo;
    snd_seq_port_info_t* pinfo;
    int client;
    int err;
    snd_seq_t* handle;

    err = snd_seq_open(&handle, "hw", streams, 0);
    if (err < 0) {
        LOGE("snd_seq_open failed: err: %s", snd_strerror(err));
        return ret;
    }

    snd_seq_client_info_alloca(&cinfo);
    snd_seq_client_info_set_client(cinfo, -1);

    int index = 0;
    while (snd_seq_query_next_client(handle, cinfo) >= 0) {
        client = snd_seq_client_info_get_client(cinfo);
        if (client == SND_SEQ_CLIENT_SYSTEM) {
            continue;
        }

        snd_seq_port_info_alloca(&pinfo);
        snd_seq_port_info_set_client(pinfo, client);

        snd_seq_port_info_set_port(pinfo, -1);
        while (snd_seq_query_next_port(handle, pinfo) >= 0) {
            uint32_t types = snd_seq_port_info_get_type(pinfo);
            uint32_t caps = snd_seq_port_info_get_capability(pinfo);

            bool canConnect = ((caps & cap) == cap) && (((types & type_hw) == type_hw) || ((types & type_sw) == type_sw));

            if (canConnect) {
                MidiDevice dev;
                dev.name = "ALSA/" + std::string(snd_seq_client_info_get_name(cinfo));
                int client = snd_seq_port_info_get_client(pinfo);
                int port = snd_seq_port_info_get_port(pinfo);
                dev.id = makeUniqueDeviceId(index++, client, port);

                ret.push_back(std::move(dev));
            }
        }
    }

    snd_seq_close(handle);

    return ret;
}

muse::Ret AlsaMidiOutPort::connect(const MidiDeviceID& deviceID)
{
    Ret ret = muse::make_ok();

    std::vector<int> deviceParams = splitDeviceId(deviceID);
    IF_ASSERT_FAILED(deviceParams.size() == 3) {
        return make_ret(Err::MidiInvalidDeviceID, "invalid device id: " + deviceID);
    }

    if (isConnected()) {
        disconnect();
    }

    int err = snd_seq_open(&m_alsa->midiOut, "default", SND_SEQ_OPEN_OUTPUT, 0);
    if (err < 0) {
        return make_ret(Err::MidiFailedConnect, "failed open seq, err: " + std::string(snd_strerror(err)));
    }
    snd_seq_set_client_name(m_alsa->midiOut, "MuseScore");

    int port = snd_seq_create_simple_port(m_alsa->midiOut, "MuseScore Port-0", SND_SEQ_PORT_CAP_READ, SND_SEQ_PORT_TYPE_MIDI_GENERIC);
    if (port < 0) {
        return make_ret(Err::MidiFailedConnect, "failed create port");
    }

    m_alsa->client = deviceParams.at(1);
    m_alsa->port = deviceParams.at(2);
    err = snd_seq_connect_to(m_alsa->midiOut, port, m_alsa->client, m_alsa->port);
    if (err < 0) {
        return make_ret(Err::MidiFailedConnect,  "failed connect, err: " + std::string(snd_strerror(err)));
    }
    return Ret(true);
}

void AlsaMidiOutPort::disconnect()
{
    if (!isConnected()) {
        return;
    }

    snd_seq_disconnect_from(m_alsa->midiOut, 0, m_alsa->client, m_alsa->port);
    snd_seq_close(m_alsa->midiOut);

    LOGD() << "Disconnected from " << m_deviceID;

    m_alsa->client = -1;
    m_alsa->port = -1;
    m_alsa->midiOut = nullptr;
    m_deviceID.clear();
}

bool AlsaMidiOutPort::isConnected() const
{
    return m_alsa && m_alsa->midiOut && !m_deviceID.empty();
}

MidiDeviceID AlsaMidiOutPort::deviceID() const
{
    return m_deviceID;
}

bool AlsaMidiOutPort::supportsMIDI20Output() const
{
    return false;
}

Ret AlsaMidiOutPort::sendEvent(const Event& e)
{
    if (!isConnected()) {
        return make_ret(Err::MidiNotConnected);
    }

    if (e.isChannelVoice20()) {
        auto events = e.toMIDI10();
        for (auto& event : events) {
            Ret ret = sendEvent(event);
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

    snd_seq_event_output_direct(m_alsa->midiOut, &seqev);

    return Ret(true);
}

bool AlsaMidiOutPort::deviceExists(const MidiDeviceID& deviceId) const
{
    for (const MidiDevice& device : availableDevices()) {
        if (device.id == deviceId) {
            return true;
        }
    }

    return false;
}
