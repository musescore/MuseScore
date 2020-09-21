//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2020 MuseScore BVBA and others
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
//=============================================================================
#include "alsamidioutport.h"

#include <alsa/asoundlib.h>
#include <alsa/seq.h>
#include <alsa/seq_midi_event.h>

#include "log.h"
#include "stringutils.h"
#include "midierrors.h"

struct mu::midi::AlsaMidiOutPort::Alsa {
    snd_seq_t* midiOut = nullptr;
    int client = -1;
    int port = -1;
};

using namespace mu::midi;

AlsaMidiOutPort::AlsaMidiOutPort()
{
    m_alsa = std::unique_ptr<Alsa>(new Alsa());
}

AlsaMidiOutPort::~AlsaMidiOutPort()
{
    if (isConnected()) {
        disconnect();
    }
}

std::vector<MidiDevice> AlsaMidiOutPort::devices() const
{
    int streams = SND_SEQ_OPEN_OUTPUT;
    unsigned int cap = SND_SEQ_PORT_CAP_SUBS_READ | SND_SEQ_PORT_CAP_READ;

    std::vector<MidiDevice> ret;

    snd_seq_client_info_t* cinfo;
    snd_seq_port_info_t* pinfo;
    int client;
    int err;
    snd_seq_t* handle;

    err = snd_seq_open(&handle, "hw", streams, 0);
    if (err < 0) {
        /* Use snd_strerror(errno) to get the error here. */
        return ret;
    }

    snd_seq_client_info_alloca(&cinfo);
    snd_seq_client_info_set_client(cinfo, -1);

    while (snd_seq_query_next_client(handle, cinfo) >= 0) {
        client = snd_seq_client_info_get_client(cinfo);
        snd_seq_port_info_alloca(&pinfo);
        snd_seq_port_info_set_client(pinfo, client);

        snd_seq_port_info_set_port(pinfo, -1);
        while (snd_seq_query_next_port(handle, pinfo) >= 0) {
            if ((snd_seq_port_info_get_capability(pinfo) & cap) == cap) {
                MidiDevice dev;
                dev.id = std::to_string(snd_seq_port_info_get_client(pinfo));
                dev.id += ":" + std::to_string(snd_seq_port_info_get_port(pinfo));
                dev.name = snd_seq_client_info_get_name(cinfo);

                ret.push_back(std::move(dev));
            }
        }
    }

    snd_seq_close(handle);

    return ret;
}

mu::Ret AlsaMidiOutPort::connect(const MidiDeviceID& deviceID)
{
    std::vector<std::string> cp;
    strings::split(deviceID, cp, ":");
    IF_ASSERT_FAILED(cp.size() == 2) {
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

    m_alsa->client = std::stoi(cp.at(0));
    m_alsa->port = std::stoi(cp.at(1));
    err = snd_seq_connect_to(m_alsa->midiOut, port, m_alsa->client, m_alsa->port);
    if (err < 0) {
        return make_ret(Err::MidiFailedConnect,  "failed connect, err: " + std::string(snd_strerror(err)));
    }

    m_deviceID = deviceID;

    return Ret(true);
}

void AlsaMidiOutPort::disconnect()
{
    if (!isConnected()) {
        return;
    }

    snd_seq_disconnect_from(m_alsa->midiOut, 0, m_alsa->client, m_alsa->port);
    snd_seq_close(m_alsa->midiOut);

    m_alsa->client = -1;
    m_alsa->port = -1;
    m_alsa->midiOut = nullptr;
    m_deviceID.clear();
}

bool AlsaMidiOutPort::isConnected() const
{
    return !m_deviceID.empty();
}

MidiDeviceID AlsaMidiOutPort::deviceID() const
{
    return m_deviceID;
}

mu::Ret AlsaMidiOutPort::sendEvent(const Event& e)
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

    switch (e.type()) {
    case EventType::ME_NOTEON:
        snd_seq_ev_set_noteon(&seqev, e.channel(), e.note(), e.velocity());
        break;
    case EventType::ME_NOTEOFF:
        snd_seq_ev_set_noteoff(&seqev, e.channel(), e.note(), e.velocity());
        break;
    case EventType::ME_PROGRAM:
        snd_seq_ev_set_pgmchange(&seqev, e.channel(), e.program());
        break;
    case EventType::ME_CONTROLLER:
        snd_seq_ev_set_controller(&seqev, e.channel(), e.index(), e.data());
        break;
    case EventType::ME_PITCHBEND:
        snd_seq_ev_set_pitchbend(&seqev, e.channel(), e.data() - 8192);
        break;
    default:
        NOT_SUPPORTED << "event: " << e.to_string();
        return make_ret(Err::MidiNotSupported);
    }

    snd_seq_event_output_direct(m_alsa->midiOut, &seqev);

    return Ret(true);
}
