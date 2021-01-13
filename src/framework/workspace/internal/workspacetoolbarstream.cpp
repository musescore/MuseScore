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
#include "workspacetoolbarstream.h"

#include "global/xmlreader.h"
#include "global/xmlwriter.h"

#include "workspacetypes.h"

#include "log.h"

using namespace mu::workspace;
using namespace mu::framework;

static constexpr std::string_view TOOLBAR_TAG("Toolbar");
static constexpr std::string_view ACTION_TAG("action");
static constexpr std::string_view TOOLBAR_NAME_TAG("name");

AbstractDataPtrList WorkspaceToolbarStream::read(IODevice& sourceDevice) const
{
    XmlReader reader(&sourceDevice);
    AbstractDataPtrList toolbars;

    while (reader.canRead()) {
        reader.readNextStartElement();

        if (reader.tagName() != TOOLBAR_TAG) {
            continue;
        }

        auto toolbar = readToolbar(reader);
        toolbars.push_back(toolbar);
    }

    return toolbars;
}

AbstractDataPtr WorkspaceToolbarStream::readToolbar(XmlReader& reader) const
{
    ToolbarDataPtr toolbar = std::make_shared<ToolbarData>();

    toolbar->tag = tag();
    toolbar->name = reader.attribute(TOOLBAR_NAME_TAG);

    while (reader.readNextStartElement()) {
        if (reader.tagName() == ACTION_TAG) {
            toolbar->actions.push_back(reader.readString());
        } else {
            reader.skipCurrentElement();
        }
    }

    return toolbar;
}

void WorkspaceToolbarStream::write(const AbstractDataPtrList& toolbars, IODevice& destinationDevice) const
{
    XmlWriter writer(&destinationDevice);

    for (const AbstractDataPtr& toolbar : toolbars) {
        writeToolbar(writer, toolbar);
    }
}

void WorkspaceToolbarStream::writeToolbar(XmlWriter& writer, const AbstractDataPtr& data) const
{
    ToolbarDataPtr toolbar = std::dynamic_pointer_cast<ToolbarData>(data);
    IF_ASSERT_FAILED(toolbar) {
        return;
    }

    writer.writeStartElement(TOOLBAR_TAG);
    writer.writeAttribute(TOOLBAR_NAME_TAG, toolbar->name);

    for (const std::string& actionCode : toolbar->actions) {
        writer.writeTextElement(ACTION_TAG, actionCode);
    }

    writer.writeEndElement();
}

WorkspaceTag WorkspaceToolbarStream::tag() const
{
    return WorkspaceTag::Toolbar;
}
