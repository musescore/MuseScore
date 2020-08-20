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
#include "coremidioutport.h"

#include <QString>

#include <CoreAudio/HostTime.h>
#include <CoreServices/CoreServices.h>
#include <CoreMIDI/CoreMIDI.h>

#include "log.h"
#include "../../midiparser.h"
#include "midierrors.h"

using namespace mu::midi;

struct mu::midi::CoreMidiOutPort::Core {
    MIDIClientRef client;
    MIDIPortRef inputPort;
    MIDIEndpointRef sourceId;
    int deviceID = -1;
};

CoreMidiOutPort::CoreMidiOutPort()
{
    m_core = new Core();
}

CoreMidiOutPort::~CoreMidiOutPort()
{
    if (isConnected()) {
        disconnect();
    }
    delete m_core;
}

std::vector<MidiDevice> CoreMidiOutPort::devices() const
{
    std::vector<MidiDevice> ret;

    CFRunLoopRunInMode(kCFRunLoopDefaultMode, 0, false);
    int destinations = MIDIGetNumberOfDestinations();
    for (int destIndex = 0; destIndex <= destinations; destIndex++) {
        MIDIEndpointRef destRef = MIDIGetDestination(destIndex);
        if (destRef != 0) {
            CFStringRef stringRef = nullptr;
            char name[256];

            MIDIObjectGetStringProperty(destRef, kMIDIPropertyDisplayName, &stringRef);
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

mu::Ret CoreMidiOutPort::connect(const std::string& deviceID)
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

    QString portName = "MuseScore Port " + QString::fromStdString(deviceID);
    result = MIDIOutputPortCreate(m_core->client, portName.toCFString(), &m_core->outputPort);
    if (result != noErr) {
        MIDIClientDispose(m_core->client);
        return make_ret(Err::MidiFailedConnect, "failed create port");
    }

    m_core->deviceID = std::atoi(deviceID);
    m_core->destinationId = MIDIGetDestination(m_core->deviceID);
    if (m_core->destinationId == 0) {
        MIDIPortDispose(m_core->outputPort);
        MIDIClientDispose(m_core->client);
        return make_ret(Err::MidiFailedConnect, "failed get destination");
    }

    m_deviceID = deviceID;
    return make_ret(Err::NoError);
}

void CoreMidiOutPort::disconnect()
{
    if (!isConnected()) {
        return;
    }

    if (m_core->destinationId != 0) {
        MIDIEndpointDispose(m_core->destinationId);
        m_core->destinationId = 0;
    }

    if (m_core->outputPort != 0) {
        MIDIPortDispose(m_core->outputPort);
        m_core->outputPort = 0;
    }

    if (m_core->client != 0) {
        MIDIClientDispose(m_core->client);
        m_core->client = 0;
    }

    m_deviceID.clear();
}

bool CoreMidiOutPort::isConnected() const
{
    return !m_deviceID.empty();
}

std::string CoreMidiOutPort::deviceID() const
{
    return m_deviceID;
}

mu::Ret CoreMidiOutPort::sendEvent(const Event& e)
{
    if (!isConnected()) {
        return make_ret(Err::MidiNotConnected);
    }

    uint32_t msg = MidiParser::message(e);

    MIDIPacketList packetList;
    MIDIPacket* packet = MIDIPacketListInit(&packetList);

    MIDITimeStamp timeStamp = AudioGetCurrentHostTime();
    packet = MIDIPacketListAdd(&packetList, sizeof(packetList), packet, timeStamp, sizeof(msg), (Byte*)msg);

    MIDISend(m_core->outputPort, m_core->destinationId, &packetList);

    return make_ret(Err::NoError);
}
