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

#include "../workspacetypes.h"

#include "framework/global/xmlreader.h"

using namespace mu::workspace;
using namespace mu::framework;

static constexpr std::string_view SETTINGS_TAG("Preferences");
static constexpr std::string_view SETTING_TAG("Preference");
static constexpr std::string_view SETTING_NAME_TAG("name");

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
    settings->tag = SETTINGS_TAG;

    while (reader.readNextStartElement()) {
        if (reader.tagName() != SETTING_TAG) {
            std::string key = reader.attribute(SETTING_NAME_TAG);
            Val val(reader.readString());
            settings->vals.insert({ key, val });
        } else {
            reader.skipCurrentElement();
        }
    }

    return settings;
}

void WorkspaceSettingsStream::write(AbstractDataPtrList dataList, IODevice& destinationDevice) const
{
    Q_UNUSED(destinationDevice);
    Q_UNUSED(dataList);
}
