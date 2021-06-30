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
#include "midiremote.h"

#include "global/xmlreader.h"
#include "global/xmlwriter.h"

#include "multiinstances/resourcelockguard.h"

#include "log.h"

using namespace mu::shortcuts;
using namespace mu::framework;
using namespace mu::midi;

constexpr std::string_view MIDIMAPPING_TAG("MidiMapping");
constexpr std::string_view EVENT_TAG("Event");
constexpr std::string_view MAPPING_ACTION_CODE_TAG("key");
constexpr std::string_view MAPPING_EVENT_TYPE_TAG("EventType");
constexpr std::string_view MAPPING_EVENT_VALUE_TAG("EventValue");

static const std::string REALTIME_ADVANCE_ACTION_NAME("realtime-advance");

void MidiRemote::load()
{
    readMidiMappings();
}

const MidiMappingList& MidiRemote::midiMappings() const
{
    return m_midiMappings;
}

mu::Ret MidiRemote::setMidiMappings(const MidiMappingList& midiMappings)
{
    if (m_midiMappings == midiMappings) {
        return true;
    }

    bool ok = writeMidiMappings(midiMappings);

    if (ok) {
        m_midiMappings = midiMappings;
    }

    return ok;
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

mu::Ret MidiRemote::process(const Event& ev)
{
    if (needIgnoreEvent(ev)) {
        return make_ret(Ret::Code::Ok);
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
    mi::ResourceLockGuard resource_guard(multiInstancesProvider(), "MIDIMAPPING");

    io::path midiMappingsPath = configuration()->midiMappingUserAppDataPath();
    XmlReader reader(midiMappingsPath);

    reader.readNextStartElement();
    if (reader.tagName() != MIDIMAPPING_TAG) {
        return;
    }

    while (reader.readNextStartElement()) {
        if (reader.tagName() != EVENT_TAG) {
            reader.skipCurrentElement();
            continue;
        }

        MidiControlsMapping midiMapping = readMidiMapping(reader);
        if (midiMapping.isValid()) {
            m_midiMappings.push_back(midiMapping);
        }
    }

    if (!reader.success()) {
        LOGE() << "failed parse xml, error: " << reader.error();
    }
}

MidiControlsMapping MidiRemote::readMidiMapping(XmlReader& reader) const
{
    MidiControlsMapping midiMapping;

    while (reader.readNextStartElement()) {
        std::string tag(reader.tagName());

        if (tag == MAPPING_ACTION_CODE_TAG) {
            midiMapping.action = reader.readString();
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

    mi::ResourceLockGuard resource_guard(multiInstancesProvider(), "MIDIMAPPING");

    io::path midiMappingsPath = configuration()->midiMappingUserAppDataPath();
    XmlWriter writer(midiMappingsPath);

    writer.writeStartDocument();
    writer.writeStartElement(MIDIMAPPING_TAG);

    for (const MidiControlsMapping& midiMapping : midiMappings) {
        writeMidiMapping(writer, midiMapping);
    }

    writer.writeEndElement();
    writer.writeEndDocument();

    return writer.success();
}

void MidiRemote::writeMidiMapping(XmlWriter& writer, const MidiControlsMapping& midiMapping) const
{
    writer.writeStartElement(EVENT_TAG);
    writer.writeTextElement(MAPPING_ACTION_CODE_TAG, midiMapping.action);
    writer.writeTextElement(MAPPING_EVENT_TYPE_TAG, std::to_string(midiMapping.event.type));
    writer.writeTextElement(MAPPING_EVENT_VALUE_TAG, std::to_string(midiMapping.event.value));
    writer.writeEndElement();
}

bool MidiRemote::needIgnoreEvent(const Event& event) const
{
    if (isSettingMode()) {
        return true;
    }

    static const QList<Event::Opcode> releaseOps {
        Event::Opcode::NoteOff
    };

    bool release = releaseOps.contains(event.opcode());
    if (release) {
        bool advanceToNextNoteOnKeyRelease = configuration()->advanceToNextNoteOnKeyRelease();
        RemoteEvent remoteEvent = remoteEventFromMidiEvent(event);
        bool isRealtimeAdvance = this->remoteEvent(REALTIME_ADVANCE_ACTION_NAME) == remoteEvent;
        if (!advanceToNextNoteOnKeyRelease || !isRealtimeAdvance) {
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
