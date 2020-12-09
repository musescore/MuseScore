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

#include "framework/global/xmlreader.h"

using namespace mu::workspace;
using namespace mu::framework;

static constexpr std::string_view TOOLBAR_TAG("Toolbar");
static constexpr std::string_view ACTION_TAG("action");
static constexpr std::string_view TOOLBAR_NAME_TAG("name");

AbstractDataPtrList WorkspaceToolbarStream::read(IODevice& sourceDevice) const
{
    XmlReader reader(&sourceDevice);
    AbstractDataPtrList toolbars;

    while (!reader.atEnd()) {
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

    toolbar->tag = TOOLBAR_TAG;
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

void WorkspaceToolbarStream::write(AbstractDataPtrList dataList, IODevice& destinationDevice) const
{
    Q_UNUSED(destinationDevice);
    Q_UNUSED(dataList);
}
