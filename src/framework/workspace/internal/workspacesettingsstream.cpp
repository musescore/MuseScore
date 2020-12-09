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

std::shared_ptr<AbstractData> WorkspaceSettingsStream::read(XmlReader& xml) const
{
    std::shared_ptr<SettingsData> data = std::make_shared<SettingsData>();
    data->tag = "Preferences";

    while (xml.readNextStartElement()) {
        if ("Preference" == xml.tagName()) {
            std::string key = xml.attribute("name");
            Val val(xml.readString());
            data->vals.insert({ key, val });
        } else {
            xml.skipCurrentElement();
        }
    }

    return data;
}

void WorkspaceSettingsStream::write(Ms::XmlWriter& xml, std::shared_ptr<AbstractData> data) const
{
    Q_UNUSED(xml);
    Q_UNUSED(data);
}
