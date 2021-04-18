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
#ifndef MU_WORKSPACE_WORKSPACESETTINGSSTREAM_H
#define MU_WORKSPACE_WORKSPACESETTINGSSTREAM_H

#include "../iworkspacedatastream.h"

namespace mu::framework {
class XmlReader;
class XmlWriter;
}

namespace mu::workspace {
class WorkspaceSettingsStream : public IWorkspaceDataStream
{
public:
    WorkspaceSettingsStream(WorkspaceTag tag);

    AbstractDataPtrList read(system::IODevice& sourceDevice) const override;
    void write(const AbstractDataPtrList& settingsList, system::IODevice& destinationDevice) const override;

    WorkspaceTag tag() const override;

private:
    SettingsDataPtr readSettings(framework::XmlReader& reader) const;
    void writeSettings(framework::XmlWriter& writer, const AbstractDataPtr& data) const;

    std::string_view tagName() const;

    WorkspaceTag m_tag = WorkspaceTag::Unknown;
};
}

#endif // MU_WORKSPACE_WORKSPACESETTINGSSTREAM_H
