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
#include "workspaceuiarrangmentstream.h"

#include "global/xmlreader.h"
#include "global/xmlwriter.h"

#include "workspacetypes.h"

using namespace mu::workspace;
using namespace mu::framework;
using namespace mu::system;

static const std::string UI_ARRANGMENT_TAG("UiArrangment");
static const std::string UI_SETTINGS_TAG("Settings");
static const std::string UI_SETTINGS_NAME_TAG("name");

AbstractDataPtrList WorkspaceUiArrangmentStream::read(IODevice& sourceDevice) const
{
    XmlReader reader(&sourceDevice);

    while (reader.canRead()) {
        reader.readNextStartElement();

        if (reader.tagName() == UI_ARRANGMENT_TAG) {
            return { readSettings(reader) };
        }
    }

    return {};
}

UiArrangmentDataPtr WorkspaceUiArrangmentStream::readSettings(XmlReader& reader) const
{
    UiArrangmentDataPtr uiArrangment = std::make_shared<UiArrangmentData>();
    uiArrangment->tag = tag();

    while (reader.readNextStartElement()) {
        if (reader.tagName() == UI_SETTINGS_TAG) {
            uiArrangment->values.insert({ reader.attribute(UI_SETTINGS_NAME_TAG), Val(reader.readString()) });
        } else {
            reader.skipCurrentElement();
        }
    }

    return uiArrangment;
}

void WorkspaceUiArrangmentStream::write(const AbstractDataPtrList& uiArrangmentList, IODevice& destinationDevice) const
{
    XmlWriter writer(&destinationDevice);

    for (const AbstractDataPtr& uiArrangment : uiArrangmentList) {
        writeSettings(writer, uiArrangment);
    }
}

void WorkspaceUiArrangmentStream::writeSettings(XmlWriter& writer, const AbstractDataPtr& data) const
{
    UiArrangmentDataPtr uiArrangment = std::dynamic_pointer_cast<UiArrangmentData>(data);
    if (!uiArrangment) {
        return;
    }

    writer.writeStartElement(UI_ARRANGMENT_TAG);

    for (auto it = uiArrangment->values.begin(); it != uiArrangment->values.end(); ++it) {
        if (it->second.isNull()) {
            continue;
        }

        writer.writeStartElement(UI_SETTINGS_TAG);
        writer.writeAttribute(UI_SETTINGS_NAME_TAG, it->first);
        writer.writeCharacters(it->second.toString());
        writer.writeEndElement();
    }

    writer.writeEndElement();
}

bool WorkspaceUiArrangmentStream::isTagStartWith(const std::string& tag, const std::string& string) const
{
    return QString::fromStdString(tag).startsWith(QString::fromStdString(string));
}

WorkspaceTag WorkspaceUiArrangmentStream::tag() const
{
    return WorkspaceTag::UiArrangement;
}
