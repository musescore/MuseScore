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

    AbstractDataPtrList read(io::Device& sourceDevice) const override;
    void write(const AbstractDataPtrList& settingsList, io::Device& destinationDevice) const override;

    WorkspaceTag tag() const override;

private:
    SettingsDataPtr readSettings(framework::XmlReader& reader) const;
    void writeSettings(framework::XmlWriter& writer, const AbstractDataPtr& data) const;

    std::string_view tagName() const;

    WorkspaceTag m_tag = WorkspaceTag::Unknown;
};
}

#endif // MU_WORKSPACE_WORKSPACESETTINGSSTREAM_H
