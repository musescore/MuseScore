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

using namespace mu;
using namespace mu::workspace;
using namespace mu::framework;

static const Settings::Key CURRENT_WORKSPACE("workspace", "workspace");

void WorkspaceConfiguration::init()
{
    settings()->setDefaultValue(CURRENT_WORKSPACE, Val(std::string(DEFAULT_WORKSPACE_NAME)));
    settings()->valueChanged(CURRENT_WORKSPACE).onReceive(this, [this](const Val& name) {
        m_currentWorkspaceNameChanged.send(name.toString());
    });
}

io::paths WorkspaceConfiguration::workspacePaths() const
{
    io::paths paths;
    paths.push_back(userWorkspacesPath());

    std::vector<io::path> extensionsPath = this->extensionsPaths();
    paths.insert(paths.end(), extensionsPath.begin(), extensionsPath.end());

    return paths;
}

io::path WorkspaceConfiguration::templateWorkspacePath() const
{
    return ":/workspace/template";
}

io::path WorkspaceConfiguration::userWorkspacesPath() const
{
    return globalConfiguration()->userAppDataPath() + "/workspaces";
}

ValCh<std::string> WorkspaceConfiguration::currentWorkspaceName() const
{
    ValCh<std::string> result;
    result.ch = m_currentWorkspaceNameChanged;
    result.val = settings()->value(CURRENT_WORKSPACE).toString();

    return result;
}

void WorkspaceConfiguration::setCurrentWorkspaceName(const std::string& workspaceName)
{
    settings()->setValue(CURRENT_WORKSPACE, Val(workspaceName));
}

io::paths WorkspaceConfiguration::extensionsPaths() const
{
    return extensionsConfiguration()->workspacesPaths();
}
