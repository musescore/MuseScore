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

#include "log.h"

#include "global/xmlreader.h"
#include "global/xmlwriter.h"

using namespace mu::shortcuts;
using namespace mu::framework;
using namespace mu::midi;

constexpr std::string_view MIDIMAPPING_TAG("MidiMapping");
constexpr std::string_view EVENT_TAG("Event");
constexpr std::string_view MAPPING_ACTION_CODE_TAG("key");
constexpr std::string_view MAPPING_EVENT_TYPE_TAG("EventType");
constexpr std::string_view MAPPING_EVENT_VALUE_TAG("EventValue");

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
    if (isSettingMode()) {
        return make_ret(Ret::Code::Ok); // todo
    }

    RemoteEvent event = remoteEventFromMidiEvent(ev);

    for (const MidiMapping& midiMapping : m_midiMappings) {
        if (midiMapping.event == event) {
            dispatcher()->dispatch(midiMapping.action);
            return make_ret(Ret::Code::Ok);
        }
    }

    return Ret(Ret::Code::Undefined); //
}

void MidiRemote::readMidiMappings()
{
    io::path midiMappingsPath = configuration()->midiMappingsPath();
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

        MidiMapping midiMapping = readMidiMapping(reader);
        if (midiMapping.isValid()) {
            m_midiMappings.push_back(midiMapping);
        }
    }

    if (!reader.success()) {
        LOGE() << "failed parse xml, error: " << reader.error();
    }
}

MidiMapping MidiRemote::readMidiMapping(XmlReader& reader) const
{
    MidiMapping midiMapping;

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

    io::path midiMappingsPath = configuration()->midiMappingsPath();
    XmlWriter writer(midiMappingsPath);

    writer.writeStartDocument();
    writer.writeStartElement(MIDIMAPPING_TAG);

    for (const MidiMapping& midiMapping : midiMappings) {
        writeMidiMapping(writer, midiMapping);
    }

    writer.writeEndElement();
    writer.writeEndDocument();

    return writer.success();
}

void MidiRemote::writeMidiMapping(XmlWriter& writer, const MidiMapping& midiMapping) const
{
    writer.writeStartElement(EVENT_TAG);
    writer.writeTextElement(MAPPING_ACTION_CODE_TAG, midiMapping.action);
    writer.writeTextElement(MAPPING_EVENT_TYPE_TAG, std::to_string(midiMapping.event.type));
    writer.writeTextElement(MAPPING_EVENT_VALUE_TAG, std::to_string(midiMapping.event.value));
    writer.writeEndElement();
}
