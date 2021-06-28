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
#ifndef MU_WORKSPACE_WORKSPACEMANAGER_H
#define MU_WORKSPACE_WORKSPACEMANAGER_H

#include "iworkspacemanager.h"

#include "modularity/ioc.h"
#include "async/asyncable.h"
#include "../iworkspaceconfiguration.h"
#include "../system/ifilesystem.h"
#include "workspace.h"

namespace mu::workspace {
class WorkspaceManager : public IWorkspaceManager, public async::Asyncable
{
    INJECT(workspace, IWorkspaceConfiguration, configuration)
    INJECT(workspace, system::IFileSystem, fileSystem)

public:

    WorkspaceManager() = default;

    void init();
    void deinit();
    bool isInited() const;

    IWorkspacePtr defaultWorkspace() const override;

    IWorkspacePtr currentWorkspace() const override;
    async::Notification currentWorkspaceChanged() const override;

    IWorkspacePtrList workspaces() const override;
    Ret setWorkspaces(const IWorkspacePtrList& workspaces) override;
    async::Notification workspacesListChanged() const override;

    IWorkspacePtr newWorkspace(const std::string& workspaceName) const override;

private:
    void load();

    io::paths findWorkspaceFiles() const;

    WorkspacePtr doNewWorkspace(const std::string& workspaceName) const;

    void setupDefaultWorkspace();
    void setupCurrentWorkspace();
    void saveCurrentWorkspace();

    Ret removeMissingWorkspaces(const IWorkspacePtrList& newWorkspaceList);
    Ret removeWorkspace(const IWorkspacePtr& workspace);
    bool canRemoveWorkspace(const std::string& workspaceName) const;

    Ret addNonExistentWorkspaces(const IWorkspacePtrList& newWorkspaceList);
    Ret addWorkspace(IWorkspacePtr workspace);

    WorkspacePtr findByName(const std::string& name) const;
    WorkspacePtr findAndInit(const std::string& name) const;

    WorkspacePtr m_defaultWorkspace;
    WorkspacePtr m_currentWorkspace;
    async::Notification m_currentWorkspaceChanged;

    std::vector<WorkspacePtr> m_workspaces;
    async::Notification m_workspacesListChanged;
};
}

#endif // MU_WORKSPACE_WORKSPACEMANAGER_H
