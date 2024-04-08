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

#include "translation.h"
#include "midierrors.h"
#include "defer.h"
#include "log.h"

using namespace muse;
using namespace muse::midi;

struct muse::midi::CoreMidiOutPort::Core {
    MIDIClientRef client = 0;
    MIDIPortRef outputPort = 0;
    MIDIEndpointRef destinationId = 0;
    MIDIProtocolID destinationProtocolId = kMIDIProtocol_1_0;
    int deviceID = -1;
};

CoreMidiOutPort::CoreMidiOutPort()
    : m_core(std::make_unique<Core>())
{
}

CoreMidiOutPort::~CoreMidiOutPort()
{
}

void CoreMidiOutPort::init()
{
    initCore();
}

void CoreMidiOutPort::deinit()
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

void CoreMidiOutPort::getDestinationProtocolId()
{
    if (__builtin_available(macOS 11.0, *)) {
        SInt32 protocol = 0;
        OSStatus err = MIDIObjectGetIntegerProperty(m_core->destinationId, kMIDIPropertyProtocolID, &protocol);
        m_core->destinationProtocolId = err == noErr ? (MIDIProtocolID)protocol : kMIDIProtocol_1_0;
    }
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
        case kMIDIMsgObjectRemoved: {
            if (notification->messageSize != sizeof(MIDIObjectAddRemoveNotification)) {
                LOGW() << "Received corrupted MIDIObjectAddRemoveNotification";
                break;
            }

            auto addRemoveNotification = (const MIDIObjectAddRemoveNotification*)notification;

            if (addRemoveNotification->childType != kMIDIObjectType_Destination) {
                break;
            }

            if (notification->messageID == kMIDIMsgObjectRemoved) {
                MIDIObjectRef removedObject = addRemoveNotification->child;

                if (self->isConnected() && removedObject == self->m_core->destinationId) {
                    self->disconnect();
                }
            }

            self->availableDevicesChanged().notify();
        } break;

        case kMIDIMsgPropertyChanged: {
            if (notification->messageSize != sizeof(MIDIObjectPropertyChangeNotification)) {
                LOGW() << "Received corrupted MIDIObjectPropertyChangeNotification";
                break;
            }

            auto propertyChangeNotification = (const MIDIObjectPropertyChangeNotification*)notification;

            if (propertyChangeNotification->objectType != kMIDIObjectType_Device
                && propertyChangeNotification->objectType != kMIDIObjectType_Destination) {
                break;
            }

            if (CFStringCompare(propertyChangeNotification->propertyName, kMIDIPropertyDisplayName, 0) == kCFCompareEqualTo
                || CFStringCompare(propertyChangeNotification->propertyName, kMIDIPropertyName, 0) == kCFCompareEqualTo) {
                self->availableDevicesChanged().notify();
            } else if (__builtin_available(macOS 11.0, *)) {
                if (CFStringCompare(propertyChangeNotification->propertyName, kMIDIPropertyProtocolID, 0) == kCFCompareEqualTo
                    && self->isConnected() && propertyChangeNotification->object == self->m_core->destinationId) {
                    self->getDestinationProtocolId();
                }
            }
        } break;

        // General message that should be ignored because we handle specific ones
        case kMIDIMsgSetupChanged:

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

MidiDeviceList CoreMidiOutPort::availableDevices() const
{
    MidiDeviceList ret;

    ret.push_back({ NONE_DEVICE_ID, muse::trc("midi", "No device") });

    CFRunLoopRunInMode(kCFRunLoopDefaultMode, 0, false);
    ItemCount destinations = MIDIGetNumberOfDestinations();
    for (ItemCount destIndex = 0; destIndex <= destinations; destIndex++) {
        MIDIEndpointRef destRef = MIDIGetDestination(destIndex);
        if (destRef != 0) {
            MidiDevice dev;

            SInt32 id = 0;
            if (MIDIObjectGetIntegerProperty(destRef, kMIDIPropertyUniqueID, &id) != noErr) {
                LOGE() << "Can't get property kMIDIPropertyUniqueID";
                continue;
            }

            CFStringRef stringRef = nullptr;
            char name[256];

            if (MIDIObjectGetStringProperty(destRef, kMIDIPropertyDisplayName, &stringRef) != noErr) {
                LOGE() << "Can't get property kMIDIPropertyDisplayName";
                continue;
            }
            CFStringGetCString(stringRef, name, sizeof(name), kCFStringEncodingUTF8);
            CFRelease(stringRef);

            dev.id = std::to_string(id);
            dev.name = name;

            ret.push_back(std::move(dev));
        }
    }

    return ret;
}

async::Notification CoreMidiOutPort::availableDevicesChanged() const
{
    return m_availableDevicesChanged;
}

Ret CoreMidiOutPort::connect(const MidiDeviceID& deviceID)
{
    DEFER {
        m_deviceChanged.notify();
    };

    if (isConnected()) {
        disconnect();
    }

    if (!deviceID.empty() && deviceID != NONE_DEVICE_ID) {
        if (!m_core->client) {
            return make_ret(Err::MidiFailedConnect, "failed create client");
        }

        if (!m_core->outputPort) {
            return make_ret(Err::MidiFailedConnect, "failed create port");
        }

        MIDIObjectRef obj;
        MIDIObjectType type;

        OSStatus err = MIDIObjectFindByUniqueID(std::stoi(deviceID), &obj, &type);
        if (err != noErr) {
            return make_ret(Err::MidiFailedConnect, "failed get destination");
        }

        m_core->deviceID = std::stoi(deviceID);
        m_core->destinationId = (MIDIEndpointRef)obj;

        getDestinationProtocolId();
    }

    m_deviceID = deviceID;

    LOGD() << "Connected to " << m_deviceID;

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

    LOGD() << "Disconnected from " << m_deviceID;

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

async::Notification CoreMidiOutPort::deviceChanged() const
{
    return m_deviceChanged;
}

bool CoreMidiOutPort::supportsMIDI20Output() const
{
    if (__builtin_available(macOS 11.0, *)) {
        return true;
    }

    return false;
}

static ByteCount packetListSize(const std::vector<Event>& events)
{
    if (events.empty()) {
        return 0;
    }

    // TODO: should be dynamic per type of event
    constexpr size_t eventSize = sizeof(Event().to_MIDI10Package());

    return offsetof(MIDIPacketList, packet) + events.size() * (offsetof(MIDIPacket, data) + eventSize);
}

Ret CoreMidiOutPort::sendEvent(const Event& e)
{
    if (!isConnected()) {
        return make_ret(Err::MidiNotConnected);
    }

    OSStatus result;
    MIDITimeStamp timeStamp = AudioGetCurrentHostTime();

    if (__builtin_available(macOS 11.0, *)) {
        MIDIProtocolID protocolId = configuration()->useMIDI20Output() ? m_core->destinationProtocolId : kMIDIProtocol_1_0;

        MIDIEventList eventList;
        MIDIEventPacket* packet = MIDIEventListInit(&eventList, protocolId);
        // TODO: Replace '4' with something specific for the type of element?
        MIDIEventListAdd(&eventList, sizeof(eventList), packet, timeStamp, 4, e.rawData());

        result = MIDISendEventList(m_core->outputPort, m_core->destinationId, &eventList);
    } else {
        const std::vector<Event> events = e.toMIDI10();

        ByteCount packetListSize = ::packetListSize(events);
        if (packetListSize == 0) {
            return make_ret(Err::MidiSendError, "midi 1.0 messages list was empty");
        }

        MIDIPacketList packetList;
        MIDIPacket* packet = MIDIPacketListInit(&packetList);

        for (const Event& event : events) {
            uint32_t msg = event.to_MIDI10Package();

            if (!msg) {
                return make_ret(Err::MidiSendError, "message wasn't converted");
            }

            packet = MIDIPacketListAdd(&packetList, packetListSize, packet, timeStamp, sizeof(msg), reinterpret_cast<Byte*>(&msg));
        }

        result = MIDISend(m_core->outputPort, m_core->destinationId, &packetList);
    }

    if (result != noErr) {
        LOGE() << "midi send error: " << result;
        return make_ret(Err::MidiSendError, "failed send message. Core error: " + std::to_string(result));
    }

    return Ret(true);
}
