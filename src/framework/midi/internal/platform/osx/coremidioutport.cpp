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
#include "coremidioutport.h"

#include <QString>

#include <CoreAudio/HostTime.h>
#include <CoreServices/CoreServices.h>
#include <CoreMIDI/CoreMIDI.h>

#include "log.h"
#include "midierrors.h"

using namespace mu::midi;

struct mu::midi::CoreMidiOutPort::Core {
    MIDIClientRef client = 0;
    MIDIPortRef outputPort = 0;
    MIDIEndpointRef destinationId = 0;
    int deviceID = -1;
};

CoreMidiOutPort::~CoreMidiOutPort()
{
    if (isConnected()) {
        disconnect();
    }

    if (m_core->outputPort) {
        MIDIPortDispose(m_core->outputPort);
    }
    if (m_core->client) {
        MIDIClientDispose(m_core->client);
    }
}

void CoreMidiOutPort::init()
{
    m_core = std::make_shared<Core>();
    initCore();
}

void CoreMidiOutPort::initCore()
{
    OSStatus result;

    static auto onCoreMidiNotificationReceived = [](const MIDINotification* notification, void* refCon) {
        auto self = static_cast<CoreMidiOutPort*>(refCon);
        IF_ASSERT_FAILED(self) {
            return;
        }

        switch (notification->messageID) {
        case kMIDIMsgObjectAdded:
        case kMIDIMsgObjectRemoved:
            if (notification->messageSize == sizeof(MIDIObjectAddRemoveNotification)) {
                auto addRemoveNotification = (const MIDIObjectAddRemoveNotification*)notification;
                MIDIObjectType objectType = addRemoveNotification->childType;

                if (objectType == kMIDIObjectType_Destination) {
                    if (notification->messageID == kMIDIMsgObjectRemoved) {
                        MIDIObjectRef removedObject = addRemoveNotification->child;

                        if (self->isConnected() && removedObject == self->m_core->destinationId) {
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
        LOGE() << "failed create midi output client";
        return;
    }

    QString portName = "MuseScore output port";
    result = MIDIOutputPortCreate(m_core->client, portName.toCFString(), &m_core->outputPort);
    IF_ASSERT_FAILED(result == noErr) {
        LOGE() << "failed create midi output port";
    }
}

MidiDeviceList CoreMidiOutPort::devices() const
{
    MidiDeviceList ret;

    CFRunLoopRunInMode(kCFRunLoopDefaultMode, 0, false);
    int destinations = MIDIGetNumberOfDestinations();
    for (int destIndex = 0; destIndex <= destinations; destIndex++) {
        MIDIEndpointRef destRef = MIDIGetDestination(destIndex);
        if (destRef != 0) {
            CFStringRef stringRef = nullptr;
            char name[256];

            if (MIDIObjectGetStringProperty(destRef, kMIDIPropertyDisplayName, &stringRef) != noErr) {
                LOGE() << "Can't get property kMIDIPropertyDisplayName";
                continue;
            }
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

mu::async::Notification CoreMidiOutPort::devicesChanged() const
{
    return m_devicesChanged;
}

mu::Ret CoreMidiOutPort::connect(const MidiDeviceID& deviceID)
{
    if (isConnected()) {
        disconnect();
    }

    if (!m_core->client) {
        return make_ret(Err::MidiFailedConnect, "failed create client");
    }

    if (!m_core->outputPort) {
        return make_ret(Err::MidiFailedConnect, "failed create port");
    }

    m_core->deviceID = std::stoi(deviceID);
    m_core->destinationId = MIDIGetDestination(m_core->deviceID);
    if (!m_core->destinationId) {
        return make_ret(Err::MidiFailedConnect, "failed get destination");
    }

    m_deviceID = deviceID;
    return Ret(true);
}

void CoreMidiOutPort::disconnect()
{
    if (!isConnected()) {
        return;
    }

    if (m_core->destinationId != 0) {
        m_core->destinationId = 0;
    }

    m_core->destinationId = 0;
    m_deviceID.clear();
}

bool CoreMidiOutPort::isConnected() const
{
    return m_core->destinationId && !m_deviceID.empty();
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

    MIDIPacketList packetList;
    auto events = e.toMIDI10();
    for (auto& event : events) {
        uint32_t msg = event.to_MIDI10Package();

        if (!msg) {
            return make_ret(Err::MidiSendError, "message wasn't converted");
        }

        MIDIPacket* packet = MIDIPacketListInit(&packetList);

        MIDITimeStamp timeStamp = AudioGetCurrentHostTime();
        packet = MIDIPacketListAdd(&packetList, sizeof(packetList), packet, timeStamp, sizeof(msg), reinterpret_cast<Byte*>(&msg));
    }

    OSStatus result = MIDISend(m_core->outputPort, m_core->destinationId, &packetList);
    if (result != noErr) {
        LOGE() << "midi send error: " << result;
        return make_ret(Err::MidiSendError, "failed send message. Core error: " + std::to_string(result));
    }

    return Ret(true);
}
