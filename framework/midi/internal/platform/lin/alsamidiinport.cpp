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
#include "alsamidiinport.h"

#include <alsa/asoundlib.h>
#include <alsa/seq.h>
#include <alsa/seq_midi_event.h>

#include "log.h"
#include "midierrors.h"
#include "stringutils.h"

struct mu::midi::AlsaMidiInPort::Alsa {
    snd_seq_t* midiIn = nullptr;
    int client = -1;
    int port = -1;
};

using namespace mu::midi;

AlsaMidiInPort::AlsaMidiInPort()
{
    m_alsa = std::unique_ptr<Alsa>(new Alsa());
}

AlsaMidiInPort::~AlsaMidiInPort()
{
    if (isRunning()) {
        stop();
    }

    if (isConnected()) {
        disconnect();
    }
}

std::vector<MidiDevice> AlsaMidiInPort::devices() const
{
    int streams = SND_SEQ_OPEN_INPUT;
    unsigned int cap = SND_SEQ_PORT_CAP_SUBS_WRITE | SND_SEQ_PORT_CAP_WRITE;

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

mu::Ret AlsaMidiInPort::connect(const MidiDeviceID& deviceID)
{
    std::vector<std::string> cp;
    strings::split(deviceID, cp, ":");
    IF_ASSERT_FAILED(cp.size() == 2) {
        return make_ret(Err::MidiInvalidDeviceID, "invalid device id: " + deviceID);
    }

    if (isConnected()) {
        disconnect();
    }

    int err = snd_seq_open(&m_alsa->midiIn, "default", SND_SEQ_OPEN_INPUT, 0);
    if (err < 0) {
        return make_ret(Err::MidiFailedConnect, "failed open seq, err: " + std::string(snd_strerror(err)));
    }

    snd_seq_set_client_name(m_alsa->midiIn, "MuseScore");
    int port = snd_seq_create_simple_port(m_alsa->midiIn, "MuseScore Input Port", SND_SEQ_PORT_CAP_WRITE, SND_SEQ_PORT_TYPE_MIDI_GENERIC);
    if (port < 0) {
        return make_ret(Err::MidiFailedConnect, "failed create port");
    }

    m_alsa->client = std::stoi(cp.at(0));
    m_alsa->port = std::stoi(cp.at(1));
    err = snd_seq_connect_from(m_alsa->midiIn, 0, m_alsa->client, m_alsa->port);
    if (err < 0) {
        return make_ret(Err::MidiFailedConnect,  "failed connect, err: " + std::string(snd_strerror(err)));
    }

    m_deviceID = deviceID;
    return Ret(true);
}

void AlsaMidiInPort::disconnect()
{
    if (!isConnected()) {
        return;
    }

    snd_seq_disconnect_to(m_alsa->midiIn, 0, m_alsa->client, m_alsa->port);
    snd_seq_close(m_alsa->midiIn);

    m_alsa->client = -1;
    m_alsa->port = -1;
    m_alsa->midiIn = nullptr;
    m_deviceID.clear();
}

bool AlsaMidiInPort::isConnected() const
{
    return !m_deviceID.empty();
}

MidiDeviceID AlsaMidiInPort::deviceID() const
{
    return m_deviceID;
}

mu::Ret AlsaMidiInPort::run()
{
    if (!isConnected()) {
        return make_ret(Err::MidiNotConnected);
    }

    if (m_thread) {
        LOGW() << "already started";
        return Ret(true);
    }

    m_running.store(true);
    m_thread = std::make_shared<std::thread>(process, this);
    return Ret(true);
}

void AlsaMidiInPort::process(AlsaMidiInPort* self)
{
    self->doProcess();
}

void AlsaMidiInPort::doProcess()
{
    snd_seq_event_t* seqv = nullptr;

    Event e;

    while (m_running.load() && isConnected()) {
        std::this_thread::sleep_for(std::chrono::milliseconds(2));

        snd_seq_event_input(m_alsa->midiIn, &seqv);
        if (!seqv) {
            continue;
        }

        switch (seqv->type) {
        case SND_SEQ_EVENT_SYSEX:
            NOT_IMPLEMENTED;
            continue;
        case SND_SEQ_EVENT_NOTEOFF:
            e.setType(EventType::ME_NOTEOFF);
            e.setChannel(seqv->data.note.channel);
            e.setNote(seqv->data.note.note);
            e.setVelocity(seqv->data.note.velocity);
            break;
        case SND_SEQ_EVENT_NOTEON:
            e.setType(EventType::ME_NOTEON);
            e.setChannel(seqv->data.note.channel);
            e.setNote(seqv->data.note.note);
            e.setVelocity(seqv->data.note.velocity);
            break;
        case SND_SEQ_EVENT_KEYPRESS:
            NOT_IMPLEMENTED;
            continue;
        case SND_SEQ_EVENT_CONTROLLER:
            e.setType(EventType::ME_CONTROLLER);
            e.setChannel(seqv->data.note.channel);
            e.setIndex(seqv->data.control.param);
            e.setData(seqv->data.control.value);
            break;
        case SND_SEQ_EVENT_PGMCHANGE:
            e.setType(EventType::ME_PROGRAM);
            e.setChannel(seqv->data.note.channel);
            e.setProgram(seqv->data.control.value);
            break;
        case SND_SEQ_EVENT_CHANPRESS:
            NOT_IMPLEMENTED;
            continue;
        case SND_SEQ_EVENT_PITCHBEND: {
            e.setType(EventType::ME_PITCHBEND);
            e.setChannel(seqv->data.note.channel);
            e.setData(seqv->data.control.value + 8192);
        }
        break;
        default:
            continue;
        }
        e = e.toMIDI20();
        if (e) {
            m_eventReceived.send({ static_cast<tick_t>(seqv->time.tick), e });
        }
    }
}

void AlsaMidiInPort::stop()
{
    if (!m_thread) {
        LOGW() << "already stoped";
        return;
    }

    m_running.store(false);
    m_thread->join();
    m_thread = nullptr;
}

bool AlsaMidiInPort::isRunning() const
{
    if (m_thread) {
        return true;
    }
    return false;
}

mu::async::Channel<std::pair<tick_t, Event> > AlsaMidiInPort::eventReceived() const
{
    return m_eventReceived;
}
