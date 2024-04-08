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

#include "translation.h"
#include "midierrors.h"
#include "defer.h"
#include "log.h"

using namespace muse;
using namespace muse::midi;

struct muse::midi::CoreMidiInPort::Core {
    MIDIClientRef client = 0;
    MIDIPortRef inputPort = 0;
    MIDIEndpointRef sourceId = 0;
    int deviceID = -1;
};

CoreMidiInPort::CoreMidiInPort()
    : m_core(std::make_unique<Core>())
{
}

CoreMidiInPort::~CoreMidiInPort()
{
}

void CoreMidiInPort::init()
{
    initCore();
}

void CoreMidiInPort::deinit()
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

MidiDeviceList CoreMidiInPort::availableDevices() const
{
    MidiDeviceList ret;

    ret.push_back({ NONE_DEVICE_ID, muse::trc("midi", "No device") });

    CFRunLoopRunInMode(kCFRunLoopDefaultMode, 0, false);
    ItemCount sources = MIDIGetNumberOfSources();
    for (ItemCount sourceIndex = 0; sourceIndex <= sources; sourceIndex++) {
        MIDIEndpointRef sourceRef = MIDIGetSource(sourceIndex);
        if (sourceRef != 0) {
            MidiDevice dev;

            SInt32 id = 0;
            if (MIDIObjectGetIntegerProperty(sourceRef, kMIDIPropertyUniqueID, &id) != noErr) {
                LOGE() << "Can't get property kMIDIPropertyUniqueID";
                continue;
            }

            CFStringRef stringRef = 0;
            char name[256];
            if (MIDIObjectGetStringProperty(sourceRef, kMIDIPropertyDisplayName, &stringRef) != noErr) {
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

async::Notification CoreMidiInPort::availableDevicesChanged() const
{
    return m_availableDevicesChanged;
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
        case kMIDIMsgObjectRemoved: {
            if (notification->messageSize != sizeof(MIDIObjectAddRemoveNotification)) {
                LOGW() << "Received corrupted MIDIObjectAddRemoveNotification";
                break;
            }

            auto addRemoveNotification = (const MIDIObjectAddRemoveNotification*)notification;

            if (addRemoveNotification->childType != kMIDIObjectType_Source) {
                break;
            }

            if (notification->messageID == kMIDIMsgObjectRemoved) {
                MIDIObjectRef removedObject = addRemoveNotification->child;

                if (self->isConnected() && removedObject == self->m_core->sourceId) {
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
                && propertyChangeNotification->objectType != kMIDIObjectType_Source) {
                break;
            }

            if (CFStringCompare(propertyChangeNotification->propertyName, kMIDIPropertyDisplayName, 0) == kCFCompareEqualTo
                || CFStringCompare(propertyChangeNotification->propertyName, kMIDIPropertyName, 0) == kCFCompareEqualTo) {
                self->availableDevicesChanged().notify();
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
        LOGE() << "failed create midi input client";
        return;
    }

    QString portName = "MuseScore MIDI input port";
    if (__builtin_available(macOS 11.0, *)) {
        MIDIReceiveBlock receiveBlock = ^ (const MIDIEventList* eventList, void* /*srcConnRefCon*/) {
            // For reference have a look at Table 4 on page 22f in
            // Universal MIDI Packet (UMP) Format
            // and MIDI 2.0 Protocol
            // With MIDI 1.0 Protocol in UMP Format
            //
            // MIDI Association Document: M2-104-UM
            // Document Version 1.1.2
            // Draft Date 2023-10-27
            // Published 2023-11-10
            const uint32_t message_type_to_size_in_byte[] =  { 1,  // 0x0
                                                               1,  // 0x1
                                                               1,  // 0x2
                                                               2,  // 0x3
                                                               2,  // 0x4
                                                               4,  // 0x5
                                                               1,  // 0x6
                                                               1,  // 0x7
                                                               2,  // 0x8
                                                               2,  // 0x9
                                                               2,  // 0xA
                                                               3,  // 0xB
                                                               3,  // 0xC
                                                               4,  // 0xD
                                                               4,  // 0xE
                                                               4 };// 0xF
            const MIDIEventPacket* packet = eventList->packet;
            for (UInt32 index = 0; index < eventList->numPackets; index++) {
                LOGD() << "midi packet size " << packet->wordCount << " bytes";
                // Handle packet
                uint32_t pos = 0;
                while (pos < packet->wordCount) {
                    uint32_t most_significant_4_bit = packet->words[pos] >> 28;
                    uint32_t message_size = message_type_to_size_in_byte[most_significant_4_bit];

                    LOGD() << "midi message size " << message_size << " bytes";
                    Event e = Event::fromRawData(&packet->words[pos], message_size);
                    if (e) {
                        m_eventReceived.send((tick_t)packet->timeStamp, e);
                    }
                    pos += message_size;
                }
                packet = MIDIEventPacketNext(packet);
            }
        };

        result
            = MIDIInputPortCreateWithProtocol(m_core->client, portName.toCFString(), kMIDIProtocol_2_0, &m_core->inputPort, receiveBlock);
    } else {
        MIDIReadBlock readBlock = ^ (const MIDIPacketList* packetList, void* /*srcConnRefCon*/)
        {
            const MIDIPacket* packet = packetList->packet;
            for (UInt32 index = 0; index < packetList->numPackets; index++) {
                if (packet->length != 0 && packet->length <= 4) {
                    uint32_t message(0);
                    memcpy(&message, packet->data, std::min(sizeof(message), sizeof(char) * packet->length));

                    auto e = Event::fromMIDI10Package(message).toMIDI20();
                    if (e) {
                        m_eventReceived.send((tick_t)packet->timeStamp, e);
                    }
                } else if (packet->length > 4) {
                    LOGW() << "unsupported midi message size " << packet->length << " bytes";
                }

                packet = MIDIPacketNext(packet);
            }
        };

        result = MIDIInputPortCreateWithBlock(m_core->client, portName.toCFString(), &m_core->inputPort, readBlock);
    }

    IF_ASSERT_FAILED(result == noErr) {
        LOGE() << "failed create midi input port";
    }
}

Ret CoreMidiInPort::connect(const MidiDeviceID& deviceID)
{
    DEFER {
        m_deviceChanged.notify();
    };

    if (isConnected()) {
        disconnect();
    }

    Ret ret = muse::make_ok();

    if (!deviceID.empty() && deviceID != NONE_DEVICE_ID) {
        if (!m_core->client) {
            return make_ret(Err::MidiFailedConnect, "failed create client");
        }

        if (!m_core->inputPort) {
            return make_ret(Err::MidiFailedConnect, "failed create port");
        }

        MIDIObjectRef obj;
        MIDIObjectType type;

        OSStatus err = MIDIObjectFindByUniqueID(std::stoi(deviceID), &obj, &type);
        if (err != noErr) {
            return make_ret(Err::MidiFailedConnect, "failed get source");
        }

        m_core->deviceID = std::stoi(deviceID);
        m_core->sourceId = (MIDIEndpointRef)obj;

        m_deviceID = deviceID;
        ret = run();
    } else {
        m_deviceID = deviceID;
    }

    if (ret) {
        LOGD() << "Connected to " << m_deviceID;
    }

    return ret;
}

void CoreMidiInPort::disconnect()
{
    if (!isConnected()) {
        return;
    }

    stop();

    LOGD() << "Disconnected from " << m_deviceID;

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

async::Notification CoreMidiInPort::deviceChanged() const
{
    return m_deviceChanged;
}

async::Channel<tick_t, Event> CoreMidiInPort::eventReceived() const
{
    return m_eventReceived;
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
