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

//#define DEBUG_COREMIDIOUTPORT
#ifdef DEBUG_COREMIDIOUTPORT
#define LOG_MIDI_D LOGD
#define LOG_MIDI_W LOGW
#else
#define LOG_MIDI_D LOGN
#define LOG_MIDI_W LOGN
#endif

struct muse::midi::CoreMidiOutPort::Core {
    MIDIClientRef client = 0;
    MIDIPortRef outputPort = 0;
    MIDIEndpointRef destinationId = 0;
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

Ret CoreMidiOutPort::sendEvent(const Event& e)
{
    if (!isConnected()) {
        return make_ret(Err::MidiNotConnected);
    }

    OSStatus result;
    MIDITimeStamp timeStamp = AudioGetCurrentHostTime();

    // Note: there could be three cases: MIDI2+MIDIEventList, MIDI1+MIDIEventList, MIDI1+MIDIPacketList.
    //       But we only maintain 2 code paths and may drop configuration()->useMIDI20Output() and MIDIPacketList later.

    if (supportsMIDI20Output() && configuration()->useMIDI20Output()) {
        if (__builtin_available(macOS 11.0, *)) {
            MIDIProtocolID protocolId = kMIDIProtocol_2_0;

            ByteCount wordCount = e.midi20WordCount();
            if (wordCount == 0) {
                LOG_MIDI_W() << "Failed to send message for event: " << e.to_string();
                return make_ret(Err::MidiSendError, "failed send message. unknown word count");
            }
            LOG_MIDI_D() << "Sending MIDIEventList event: " << e.to_string();

            MIDIEventList eventList;
            MIDIEventPacket* packet = MIDIEventListInit(&eventList, protocolId);

            if (e.messageType() == Event::MessageType::ChannelVoice20 && e.opcode() == muse::midi::Event::Opcode::NoteOn
                && e.velocity() < 128) {
                LOG_MIDI_W() << "Detected low MIDI 2.0 ChannelVoiceMessage velocity.";
            }

            MIDIEventListAdd(&eventList, sizeof(eventList), packet, timeStamp, wordCount, e.rawData());

            result = MIDISendEventList(m_core->outputPort, m_core->destinationId, &eventList);
        }
    } else {
        const std::vector<Event> events = e.toMIDI10();

        if (events.empty()) {
            return make_ret(Err::MidiSendError, "midi 1.0 messages list was empty");
        }

        MIDIPacketList packetList;
        // packetList has one packet of 256 bytes by default, which is more than enough for one midi 2.0 event.
        MIDIPacket* packet = MIDIPacketListInit(&packetList);
        ByteCount packetListSize = sizeof(packetList);
        Byte bytesPackage[4];

        for (const Event& event : events) {
            int length = event.to_MIDI10BytesPackage(bytesPackage);
            assert(length <= 3);
            if (length == 0) {
                LOG_MIDI_W() << "Failed Sending MIDIPacketList event: " << event.to_string();
            } else {
                LOG_MIDI_D() << "Sending MIDIPacketList event: " << event.to_string();
                packet = MIDIPacketListAdd(&packetList, packetListSize, packet, timeStamp, length, bytesPackage);
            }
        }

        result = MIDISend(m_core->outputPort, m_core->destinationId, &packetList);
    }

    if (result != noErr) {
        LOGE() << "midi send error: " << result;
        return make_ret(Err::MidiSendError, "failed send message. Core error: " + std::to_string(result));
    }

    return Ret(true);
}
