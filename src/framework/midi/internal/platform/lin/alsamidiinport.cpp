/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-CLA-applies
 *
 * MuseScore
 * Music Composition & Notation
 *
 * Copyright (C) 2021 MuseScore BVBA and others
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

AlsaMidiInPort::~AlsaMidiInPort()
{
    if (isConnected()) {
        disconnect();
    }
}

void AlsaMidiInPort::init()
{
    m_alsa = std::unique_ptr<Alsa>(new Alsa());

    m_devicesListener.startWithCallback([this]() {
        return devices();
    });

    m_devicesListener.devicesChanged().onNotify(this, [this]() {
        bool connectedDeviceRemoved = true;
        for (const MidiDevice& device: devices()) {
            if (m_deviceID == device.id) {
                connectedDeviceRemoved = false;
            }
        }

        if (connectedDeviceRemoved) {
            disconnect();
        }

        m_devicesChanged.notify();
    });
}

MidiDeviceList AlsaMidiInPort::devices() const
{
    std::lock_guard lock(m_devicesMutex);

    int streams = SND_SEQ_OPEN_INPUT;
    unsigned int cap = SND_SEQ_PORT_CAP_SUBS_WRITE | SND_SEQ_PORT_CAP_WRITE;

    MidiDeviceList ret;

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
        if (client == SND_SEQ_CLIENT_SYSTEM) {
            continue;
        }

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

mu::async::Notification AlsaMidiInPort::devicesChanged() const
{
    return m_devicesChanged;
}

mu::Ret AlsaMidiInPort::connect(const MidiDeviceID& deviceID)
{
    if (!deviceExists(deviceID)) {
        return make_ret(Err::MidiFailedConnect, "not found device, id: " + deviceID);
    }

    std::vector<std::string> cp;
    strings::split(deviceID, cp, ":");
    IF_ASSERT_FAILED(cp.size() == 2) {
        return make_ret(Err::MidiInvalidDeviceID, "invalid device id: " + deviceID);
    }

    if (isConnected()) {
        disconnect();
    }

    int err = snd_seq_open(&m_alsa->midiIn, "default", SND_SEQ_OPEN_INPUT, 0 /*block mode*/);
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

    stop();
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
    snd_seq_event_t* ev = nullptr;
    uint32_t data = 0;
    uint32_t value = 0;
    Event e;

    while (m_running.load() && isConnected()) {
        snd_seq_event_input(m_alsa->midiIn, &ev);

        if (!ev) {
            continue;
        }

        switch (ev->type) {
        case SND_SEQ_EVENT_SYSEX:
        {
            NOT_SUPPORTED << "event type: SND_SEQ_EVENT_SYSEX";
            continue;
        }
        case SND_SEQ_EVENT_NOTEOFF:
            data = 0x80
                   | (ev->data.note.channel & 0x0F)
                   | ((ev->data.note.note & 0x7F) << 8)
                   | ((ev->data.note.velocity & 0x7F) << 16);
            break;
        case SND_SEQ_EVENT_NOTEON:
            data = 0x90
                   | (ev->data.note.channel & 0x0F)
                   | ((ev->data.note.note & 0x7F) << 8)
                   | ((ev->data.note.velocity & 0x7F) << 16);
            break;
        case SND_SEQ_EVENT_KEYPRESS:
            data = 0xA0
                   | (ev->data.note.channel & 0x0F)
                   | ((ev->data.note.note & 0x7F) << 8)
                   | ((ev->data.note.velocity & 0x7F) << 16);
            break;
        case SND_SEQ_EVENT_CONTROLLER:
            data = 0xB0
                   | (ev->data.control.channel & 0x0F)
                   | ((ev->data.control.param & 0x7F) << 8)
                   | ((ev->data.control.value & 0x7F) << 16);
            break;
        case SND_SEQ_EVENT_PGMCHANGE:
            data = 0xC0
                   | (ev->data.control.channel & 0x0F)
                   | ((ev->data.control.value & 0x7F) << 8);
            break;
        case SND_SEQ_EVENT_CHANPRESS:
            data = 0xD0
                   | (ev->data.control.channel & 0x0F)
                   | ((ev->data.control.value & 0x7F) << 8);
            break;
        case SND_SEQ_EVENT_PITCHBEND:
            value = ev->data.control.value + 8192;
            data = 0xE0
                   | (ev->data.note.channel & 0x0F)
                   | ((value & 0x7F) << 8)
                   | (((value >> 7) & 0x7F) << 16);
            break;
        default:
            NOT_SUPPORTED << "event type: " << ev->type;
            continue;
        }

        e = Event::fromMIDI10Package(data);

        e = e.toMIDI20();
        if (e) {
            m_eventReceived.send(static_cast<tick_t>(ev->time.tick), e);
        }

        snd_seq_free_event(ev);
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

mu::async::Channel<tick_t, Event> AlsaMidiInPort::eventReceived() const
{
    return m_eventReceived;
}

bool AlsaMidiInPort::deviceExists(const MidiDeviceID& deviceId) const
{
    for (const MidiDevice& device : devices()) {
        if (device.id == deviceId) {
            return true;
        }
    }

    return false;
}
