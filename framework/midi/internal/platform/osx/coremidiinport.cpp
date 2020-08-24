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

#include "log.h"
#include "midierrors.h"
#include "../../midiparser.h"

using namespace mu;
using namespace mu::midi;

struct mu::midi::CoreMidiInPort::Core {
    MIDIClientRef client;
    MIDIPortRef outputPort;
    MIDIEndpointRef destinationId;
    int deviceID = -1;
};

CoreMidiInPort::CoreMidiInPort()
{
    m_core = std::move(std::unique_ptr<Core>());
}

CoreMidiInPort::~CoreMidiInPort()
{
    if (isRunning()) {
        stop();
    }

    if (isConnected()) {
        disconnect();
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

            MIDIObjectGetStringProperty(sourceRef, kMIDIPropertyDisplayName, &stringRef);
            CFStringGetCString(stringRef, name, sizeof(name), kCFStringEncodingUTF8);
            CFRelease(stringRef);

            MidiDevice dev;
            dev.id = std::to_string(destIndex);
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
    MIDIPacket* packet = const_cast<MIDIPacket*>(list->packet);

    for (UInt32 index = 0; index < list->numPackets; index++) {
        UInt16 byteCount = packet->length;

        // Check that the MIDIPacket has data, and is a normal midi
        // message. (We don't support Sysex, status, etc for CoreMIDI at
        // the moment.)
        if (byteCount != 0
            && packet->data[0] < 0xF0
            && (packet->data[0] & 0x80) != 0x00) {
            self->doProcess(*packet->data, packet->timeStamp);
        }

        packet = MIDIPacketNext(packet);
    }
}

void CoreMidiInPort::doProcess(uint32_t message, tick_t timing)
{
    Event e = MidiParser::toEvent(message);
    m_eventReceived.send({ timing, e });
}

Ret CoreMidiInPort::connect(const MidiDeviceID& deviceID)
{
    if (isConnected()) {
        disconnect();
    }

    OSStatus result;

    QString name = "MuseScore";
    result = MIDIClientCreate(name.toCFString(), nullptr, nullptr, &m_core->client);
    if (result != noErr) {
        return make_ret(Err::MidiFailedConnect, "failed create client");
    }

    QString portName = "MuseScore Input Port " + QString::fromStdString(deviceID);
    result = MIDIInputPortCreate(m_core->client, portName.toCFString(), QMidiInReadProc, this, &m_core->inputPort);
    if (result != noErr) {
        MIDIClientDispose(m_core->client);
        return make_ret(Err::MidiFailedConnect, "failed create port");
    }

    m_core->deviceID = std::atoi(deviceID);
    m_core->sourceId = MIDIGetSource(m_core->deviceID);
    if (m_core->sourceId == 0) {
        MIDIPortDispose(m_core->inputPort);
        MIDIClientDispose(m_core->client);
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

    MIDIPortDisconnectSource(m_core->inputPort, m_core->sourceId);

    if (m_core->inputPort != 0) {
        MIDIPortDispose(m_core->inputPort);
        m_core->inputPort = 0;
    }

    if (m_core->client != 0) {
        MIDIClientDispose(m_core->client);
        m_core->client = 0;
    }

    m_deviceID.clear();
}

bool CoreMidiInPort::isConnected() const
{
    return !m_deviceID.empty();
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

    MIDIPortConnectSource(m_core->inputPort, m_core->sourceId, nullptr /*connRefCon*/);
    m_running = true;
    return Ret(true);
}

void CoreMidiInPort::stop()
{
    if (!isConnected()) {
        return make_ret(Err::MidiNotConnected);
    }

    MIDIPortDisconnectSource(m_core->inputPort, m_core->sourceId);
}

bool CoreMidiInPort::isRunning() const
{
    return m_running;
}

async::Channel<std::pair<tick_t, Event> > CoreMidiInPort::eventReceived() const
{
    return m_eventReceived;
}
