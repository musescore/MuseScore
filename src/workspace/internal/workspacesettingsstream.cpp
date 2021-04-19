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
#include "workspacesettingsstream.h"

#include "global/xmlreader.h"
#include "global/xmlwriter.h"

#include "workspacetypes.h"

#include "log.h"

using namespace mu::workspace;
using namespace mu::framework;
using namespace mu::system;

static constexpr std::string_view SETTINGS_TAG("Preferences");
static constexpr std::string_view UI_ARRANGMENT_TAG("UiArrangment");
static constexpr std::string_view SETTING_ELEMENT_TAG("Preference");
static constexpr std::string_view NAME_ATTRIBUTE("name");

WorkspaceSettingsStream::WorkspaceSettingsStream(WorkspaceTag tag)
    : m_tag(tag)
{
}

AbstractDataPtrList WorkspaceSettingsStream::read(IODevice& sourceDevice) const
{
    XmlReader reader(&sourceDevice);

    while (reader.canRead()) {
        reader.readNextStartElement();

        if (reader.tagName() == tagName()) {
            return { readSettings(reader) };
        }
    }

    return {};
}

SettingsDataPtr WorkspaceSettingsStream::readSettings(XmlReader& reader) const
{
    SettingsDataPtr settings = std::make_shared<SettingsData>();
    settings->tag = tag();

    while (reader.readNextStartElement()) {
        if (reader.tagName() == SETTING_ELEMENT_TAG) {
            std::string key = reader.attribute(NAME_ATTRIBUTE);
            Val val(reader.readString());
            settings->values.insert({ key, val });
        } else {
            reader.skipCurrentElement();
        }
    }

    return settings;
}

void WorkspaceSettingsStream::write(const AbstractDataPtrList& settingsList, IODevice& destinationDevice) const
{
    XmlWriter writer(&destinationDevice);

    for (const AbstractDataPtr& settings : settingsList) {
        writeSettings(writer, settings);
    }
}

void WorkspaceSettingsStream::writeSettings(XmlWriter& writer, const AbstractDataPtr& data) const
{
    SettingsDataPtr settings = std::dynamic_pointer_cast<SettingsData>(data);
    IF_ASSERT_FAILED(settings) {
        return;
    }

    writer.writeStartElement(tagName());

    for (auto it = settings->values.begin(); it != settings->values.end(); ++it) {
        if (it->second.isNull()) {
            continue;
        }

        writer.writeStartElement(SETTING_ELEMENT_TAG);
        writer.writeAttribute(NAME_ATTRIBUTE, it->first);
        writer.writeCharacters(it->second.toString());
        writer.writeEndElement();
    }

    writer.writeEndElement();
}

WorkspaceTag WorkspaceSettingsStream::tag() const
{
    return m_tag;
}

std::string_view WorkspaceSettingsStream::tagName() const
{
    return tag() == WorkspaceTag::Settings ? SETTINGS_TAG : UI_ARRANGMENT_TAG;
}
