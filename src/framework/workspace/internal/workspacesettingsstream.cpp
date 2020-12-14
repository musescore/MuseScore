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
#include "workspacesettingsstream.h"

#include "global/xmlreader.h"
#include "global/xmlwriter.h"

#include "workspacetypes.h"

#include "log.h"

using namespace mu::workspace;
using namespace mu::framework;

static constexpr std::string_view SETTINGS_TAG("Preferences");
static constexpr std::string_view SETTING_ELEMENT_TAG("Preference");
static constexpr std::string_view NAME_ATTRIBUTE("name");

AbstractDataPtrList WorkspaceSettingsStream::read(IODevice& sourceDevice) const
{
    XmlReader reader(&sourceDevice);

    while (reader.canRead()) {
        reader.readNextStartElement();

        if (reader.tagName() == SETTINGS_TAG) {
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
        if (reader.tagName() != SETTING_ELEMENT_TAG) {
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

    writer.writeStartElement(SETTINGS_TAG);

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
    return WorkspaceTag::Settings;
}
