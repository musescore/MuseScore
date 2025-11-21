/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-CLA-applies
 *
 * MuseScore
 * Music Composition & Notation
 *
 * Copyright (C) 2021 MuseScore Limited and others
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

#include "midiremote.h"

#include "global/io/file.h"
#include "global/serialization/xmlstreamreader.h"
#include "global/serialization/xmlstreamwriter.h"

#include "multiinstances/resourcelockguard.h"

#include "log.h"

using namespace muse;
using namespace muse::shortcuts;
using namespace muse::midi;

static constexpr std::string_view MIDIMAPPING_TAG("MidiMapping");
static constexpr std::string_view EVENT_TAG("Event");
static constexpr std::string_view MAPPING_ACTION_CODE_TAG("key");
static constexpr std::string_view MAPPING_EVENT_TYPE_TAG("EventType");
static constexpr std::string_view MAPPING_EVENT_VALUE_TAG("EventValue");

static const std::string REALTIME_ADVANCE_ACTION_NAME("realtime-advance");

static const std::string MIDI_MAPPING_RESOURCE_NAME("MIDI_MAPPING");

void MidiRemote::init()
{
    multiInstancesProvider()->resourceChanged().onReceive(this, [this](const std::string& resourceName) {
        if (resourceName == MIDI_MAPPING_RESOURCE_NAME) {
            readMidiMappings();
        }
    });

    readMidiMappings();
}

const MidiMappingList& MidiRemote::midiMappings() const
{
    return m_midiMappings;
}

Ret MidiRemote::setMidiMappings(const MidiMappingList& midiMappings)
{
    if (m_midiMappings == midiMappings) {
        return true;
    }

    bool ok = writeMidiMappings(midiMappings);

    if (ok) {
        m_midiMappings = midiMappings;
        m_midiMappingsChanged.notify();
    }

    return ok;
}

void MidiRemote::resetMidiMappings()
{
    muse::mi::WriteResourceLockGuard resource_guard(multiInstancesProvider(), MIDI_MAPPING_RESOURCE_NAME);
    fileSystem()->remove(configuration()->midiMappingUserAppDataPath());

    m_midiMappings = {};
    m_midiMappingsChanged.notify();
}

async::Notification MidiRemote::midiMappingsChanged() const
{
    return m_midiMappingsChanged;
}

void MidiRemote::setIsSettingMode(bool arg)
{
    m_isSettingMode = arg;
}

bool MidiRemote::isSettingMode() const
{
    return m_isSettingMode;
}

void MidiRemote::setCurrentActionEvent(const Event& ev)
{
    UNUSED(ev);
    NOT_IMPLEMENTED;
}

Ret MidiRemote::process(const Event& ev)
{
    if (needIgnoreEvent(ev)) {
        return Ret(Ret::Code::Undefined);
    }

    RemoteEvent event = remoteEventFromMidiEvent(ev);

    for (const MidiControlsMapping& midiMapping : m_midiMappings) {
        if (midiMapping.event == event) {
            dispatcher()->dispatch(midiMapping.action);
            return make_ret(Ret::Code::Ok);
        }
    }

    return Ret(Ret::Code::Undefined);
}

void MidiRemote::readMidiMappings()
{
    muse::mi::ReadResourceLockGuard resource_guard(multiInstancesProvider(), MIDI_MAPPING_RESOURCE_NAME);

    const io::path_t midiMappingsPath = configuration()->midiMappingUserAppDataPath();
    io::File mappingsFile(midiMappingsPath);
    if (!mappingsFile.open(io::IODevice::ReadOnly)) {
        LOGD() << "failed to open midi mappings file: " << mappingsFile.error();
        return;
    }

    XmlStreamReader reader(&mappingsFile);

    reader.readNextStartElement();
    if (reader.name() != MIDIMAPPING_TAG) {
        return;
    }

    while (reader.readNextStartElement()) {
        if (reader.name() != EVENT_TAG) {
            reader.skipCurrentElement();
            continue;
        }

        MidiControlsMapping midiMapping = readMidiMapping(reader);
        if (midiMapping.isValid()) {
            m_midiMappings.push_back(midiMapping);
        }
    }

    if (reader.isError()) {
        LOGE() << "failed parse xml, error: " << reader.error();
    }
}

MidiControlsMapping MidiRemote::readMidiMapping(XmlStreamReader& reader) const
{
    MidiControlsMapping midiMapping;

    while (reader.readNextStartElement()) {
        const std::string tag(reader.name());

        if (tag == MAPPING_ACTION_CODE_TAG) {
            midiMapping.action = reader.readAsciiText();
        } else if (tag == MAPPING_EVENT_TYPE_TAG) {
            midiMapping.event.type = static_cast<RemoteEventType>(reader.readInt());
        } else if (tag == MAPPING_EVENT_VALUE_TAG) {
            midiMapping.event.value = reader.readInt();
        } else {
            reader.skipCurrentElement();
        }
    }

    return midiMapping;
}

bool MidiRemote::writeMidiMappings(const MidiMappingList& midiMappings) const
{
    TRACEFUNC;

    muse::mi::WriteResourceLockGuard resource_guard(multiInstancesProvider(), MIDI_MAPPING_RESOURCE_NAME);

    const io::path_t midiMappingsPath = configuration()->midiMappingUserAppDataPath();
    io::File mappingsFile(midiMappingsPath);
    if (!mappingsFile.open(io::IODevice::WriteOnly)) {
        return false;
    }

    XmlStreamWriter writer(&mappingsFile);

    writer.startDocument();
    writer.startElement(MIDIMAPPING_TAG);

    for (const MidiControlsMapping& midiMapping : midiMappings) {
        writeMidiMapping(writer, midiMapping);
    }

    writer.endElement();
    writer.flush();

    return !mappingsFile.hasError();
}

void MidiRemote::writeMidiMapping(XmlStreamWriter& writer, const MidiControlsMapping& midiMapping) const
{
    writer.startElement(EVENT_TAG);
    writer.element(MAPPING_ACTION_CODE_TAG, midiMapping.action);
    writer.element(MAPPING_EVENT_TYPE_TAG, midiMapping.event.type);
    writer.element(MAPPING_EVENT_VALUE_TAG, midiMapping.event.value);
    writer.endElement();
}

bool MidiRemote::needIgnoreEvent(const Event& event) const
{
    if (isSettingMode()) {
        return true;
    }

    if (event.opcode() != Event::Opcode::NoteOn && event.opcode() != Event::Opcode::NoteOff
        && event.opcode() != Event::Opcode::ControlChange) {
        return true;
    }

    static const QList<Event::Opcode> releaseOps {
        Event::Opcode::NoteOff
    };

    bool release = releaseOps.contains(event.opcode())
                   || (event.opcode() == Event::Opcode::ControlChange && event.data() == 0);

    if (release) {
        bool advanceToNextNoteOnKeyRelease = configuration()->advanceToNextNoteOnKeyRelease();
        if (!advanceToNextNoteOnKeyRelease) {
            return true;
        }

        RemoteEvent remoteEvent = remoteEventFromMidiEvent(event);
        RemoteEvent realtimeEvent = this->remoteEvent(REALTIME_ADVANCE_ACTION_NAME);
        if (!realtimeEvent.isValid() || remoteEvent != realtimeEvent) {
            return true;
        }
    }

    return false;
}

RemoteEvent MidiRemote::remoteEvent(const std::string& action) const
{
    for (const MidiControlsMapping& midiMapping : m_midiMappings) {
        if (midiMapping.action == action) {
            return midiMapping.event;
        }
    }

    return RemoteEvent();
}
