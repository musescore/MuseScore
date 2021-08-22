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

CoreMidiInPort::~CoreMidiInPort()
{
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

void CoreMidiInPort::init()
{
    m_core = std::make_shared<Core>();
    initCore();
}

MidiDeviceList CoreMidiInPort::devices() const
{
    MidiDeviceList ret;

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

async::Notification CoreMidiInPort::devicesChanged() const
{
    return m_devicesChanged;
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
        } else if (packet->length > 4) {
            LOGW() << "unsupported midi message size " << packet->length << " bytes";
        }

        packet = MIDIPacketNext(packet);
    }
}

void CoreMidiInPort::initCore()
{
    OSStatus result;

    static auto onCoreMidiNotificationReceived = [](const MIDINotification* notification, void* refCon) {
        auto self = static_cast<CoreMidiInPort*>(refCon);
        IF_ASSERT_FAILED(self) {
            return;
        }

        switch (notification->messageID) {
        case kMIDIMsgObjectAdded:
        case kMIDIMsgObjectRemoved:
            if (notification->messageSize == sizeof(MIDIObjectAddRemoveNotification)) {
                auto addRemoveNotification = (const MIDIObjectAddRemoveNotification*)notification;
                MIDIObjectType objectType = addRemoveNotification->childType;

                if (objectType == kMIDIObjectType_Source) {
                    if (notification->messageID == kMIDIMsgObjectRemoved) {
                        MIDIObjectRef removedObject = addRemoveNotification->child;

                        if (self->isConnected() && removedObject == self->m_core->sourceId) {
                            self->disconnect();
                        }
                    }

                    self->devicesChanged().notify();
                }
            } else {
                LOGW() << "Received corrupted MIDIObjectAddRemoveNotification";
            }
            break;

        // General message that should be ignored because we handle specific ones
        case kMIDIMsgSetupChanged:

        // Questionable whether we should send a notification for this ones
        // Possibly we should send a notification when the changed property is the device name
        case kMIDIMsgPropertyChanged:
        case kMIDIMsgThruConnectionsChanged:
        case kMIDIMsgSerialPortOwnerChanged:

        case kMIDIMsgIOError:
            break;
        }
    };

    QString name = "MuseScore";
    result = MIDIClientCreate(name.toCFString(), onCoreMidiNotificationReceived, this, &m_core->client);
    IF_ASSERT_FAILED(result == noErr) {
        LOGE() << "failed create midi input client";
        return;
    }

    QString portName = "MuseScore MIDI input port";
    // TODO: MIDIInputPortCreate is deprecated according to the documentation.
    // Need to use MIDIInputPortCreateWithProtocol instead.
    result = MIDIInputPortCreate(m_core->client, portName.toCFString(), proccess, this, &m_core->inputPort);
    IF_ASSERT_FAILED(result == noErr) {
        LOGE() << "failed create midi input port";
    }
}

void CoreMidiInPort::doProcess(uint32_t message, tick_t timing)
{
    auto e = Event::fromMIDI10Package(message).toMIDI20();
    if (e) {
        m_eventReceived.send(timing, e);
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
    return run();
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

async::Channel<tick_t, Event> CoreMidiInPort::eventReceived() const
{
    return m_eventReceived;
}
