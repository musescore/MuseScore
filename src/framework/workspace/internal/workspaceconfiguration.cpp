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
#include "workspaceconfiguration.h"

#include "settings.h"

#include "workspacetypes.h"

using namespace muse;
using namespace muse::workspace;

static const muse::Settings::Key CURRENT_WORKSPACE("workspace", "workspace");

void WorkspaceConfiguration::init()
{
    settings()->setDefaultValue(CURRENT_WORKSPACE, Val(std::string(DEFAULT_WORKSPACE_NAME)));
    settings()->valueChanged(CURRENT_WORKSPACE).onReceive(this, [this](const Val& name) {
        m_currentWorkspaceNameChanged.send(name.toString());
    });
}

io::paths_t WorkspaceConfiguration::workspacePaths() const
{
    io::paths_t paths;
    paths.push_back(userWorkspacesPath());

    return paths;
}

io::path_t WorkspaceConfiguration::userWorkspacesPath() const
{
    return globalConfiguration()->userAppDataPath() + "/workspaces";
}

std::string WorkspaceConfiguration::currentWorkspaceName() const
{
    return settings()->value(CURRENT_WORKSPACE).toString();
}

void WorkspaceConfiguration::setCurrentWorkspaceName(const std::string& workspaceName)
{
    //! NOTE Workspace selection does not need to be synchronized between instances
    settings()->setLocalValue(CURRENT_WORKSPACE, Val(workspaceName));
}

async::Channel<std::string> WorkspaceConfiguration::currentWorkspaceNameChanged() const
{
    return m_currentWorkspaceNameChanged;
}
