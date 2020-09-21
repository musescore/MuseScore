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
#include "coremidiinport.h"

#include <CoreAudio/HostTime.h>
#include <CoreServices/CoreServices.h>
#include <CoreMIDI/CoreMIDI.h>
#include <algorithm>
#include "log.h"
#include "midierrors.h"

using namespace mu;
using namespace mu::midi;

struct mu::midi::CoreMidiInPort::Core {
    MIDIClientRef client = 0;
    MIDIPortRef inputPort = 0;
    MIDIEndpointRef sourceId = 0;
    int deviceID = -1;
};

CoreMidiInPort::CoreMidiInPort()
{
    m_core = std::unique_ptr<Core>(new Core());
    initCore();
}

CoreMidiInPort::~CoreMidiInPort()
{
    if (isRunning()) {
        stop();
    }

    if (isConnected()) {
        disconnect();
    }

    if (m_core->inputPort) {
        MIDIPortDispose(m_core->inputPort);
    }

    if (m_core->client) {
        MIDIClientDispose(m_core->client);
    }
}

std::vector<MidiDevice> CoreMidiInPort::devices() const
{
    std::vector<MidiDevice> ret;

    CFRunLoopRunInMode(kCFRunLoopDefaultMode, 0, false);
    int sources = MIDIGetNumberOfSources();
    for (int sourceIndex = 0; sourceIndex <= sources; sourceIndex++) {
        MIDIEndpointRef sourceRef = MIDIGetSource(sourceIndex);
        if (sourceRef != 0) {
            CFStringRef stringRef = 0;
            char name[256];

            if (MIDIObjectGetStringProperty(sourceRef, kMIDIPropertyDisplayName, &stringRef) != noErr) {
                LOGE() << "Can't get property kMIDIPropertyDisplayName";
                continue;
            }
            CFStringGetCString(stringRef, name, sizeof(name), kCFStringEncodingUTF8);
            CFRelease(stringRef);

            MidiDevice dev;
            dev.id = std::to_string(sourceIndex);
            dev.name = name;

            ret.push_back(std::move(dev));
        }
    }

    return ret;
}

static void proccess(const MIDIPacketList* list, void* readProc, void* srcConn)
{
    UNUSED(srcConn);

    CoreMidiInPort* self = static_cast<CoreMidiInPort*>(readProc);
    const MIDIPacket* packet = &list->packet[0];

    for (UInt32 index = 0; index < list->numPackets; index++) {
        if (packet->length != 0 && packet->length <= 4) {
            uint32_t message(0);
            memcpy(&message, packet->data, std::min(sizeof(message), sizeof(char) * packet->length));
            self->doProcess(message, packet->timeStamp);
        }
        if (packet->length > 4) {
            LOGW() << "unsupported midi message size " << packet->length << " bytes";
        }

        packet = MIDIPacketNext(packet);
    }
}

void CoreMidiInPort::initCore()
{
    OSStatus result;

    QString name = "MuseScore";
    result = MIDIClientCreate(name.toCFString(), nullptr, nullptr, &m_core->client);
    IF_ASSERT_FAILED(result == noErr) {
        LOGE() << "failed create midi input client";
        return;
    }

    QString portName = "MuseScore MIDI input port";
    result = MIDIInputPortCreate(m_core->client, portName.toCFString(), proccess, this, &m_core->inputPort);
    IF_ASSERT_FAILED(result == noErr) {
        LOGE() << "failed create midi input port";
    }
}

void CoreMidiInPort::doProcess(uint32_t message, tick_t timing)
{
    auto e = Event::fromMIDI10Package(message).toMIDI20();
    if (e) {
        m_eventReceived.send({ timing, e });
    }
}

Ret CoreMidiInPort::connect(const MidiDeviceID& deviceID)
{
    if (isConnected()) {
        disconnect();
    }

    if (!m_core->client) {
        return make_ret(Err::MidiFailedConnect, "failed create client");
    }

    if (!m_core->inputPort) {
        return make_ret(Err::MidiFailedConnect, "failed create port");
    }

    m_core->deviceID = std::stoi(deviceID);
    m_core->sourceId = MIDIGetSource(m_core->deviceID);
    if (m_core->sourceId == 0) {
        return make_ret(Err::MidiFailedConnect, "failed get source");
    }

    m_deviceID = deviceID;
    return Ret(true);
}

void CoreMidiInPort::disconnect()
{
    if (!isConnected()) {
        return;
    }
    stop();
    m_core->sourceId = 0;
    m_deviceID.clear();
}

bool CoreMidiInPort::isConnected() const
{
    return m_core->sourceId && !m_deviceID.empty();
}

MidiDeviceID CoreMidiInPort::deviceID() const
{
    return m_deviceID;
}

Ret CoreMidiInPort::run()
{
    if (!isConnected()) {
        return make_ret(Err::MidiNotConnected);
    }

    OSStatus result = MIDIPortConnectSource(m_core->inputPort, m_core->sourceId, nullptr /*connRefCon*/);
    if (result == noErr) {
        m_running = true;
        return Ret(true);
    }
    m_running = false;
    return make_ret(Err::MidiFailedConnect);
}

void CoreMidiInPort::stop()
{
    if (!isConnected()) {
        LOGE() << "midi port is not connected";
        return;
    }

    OSStatus result = MIDIPortDisconnectSource(m_core->inputPort, m_core->sourceId);
    switch (result) {
    case kMIDINoConnection:
        LOGI() << "wasn't started";
        break;
    case noErr: break;
    default:
        LOGE() << "can't disconnect midi port " << result;
    }
    m_running = false;
}

bool CoreMidiInPort::isRunning() const
{
    return m_running;
}

async::Channel<std::pair<tick_t, Event> > CoreMidiInPort::eventReceived() const
{
    return m_eventReceived;
}
