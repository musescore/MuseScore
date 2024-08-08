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
#ifndef MUSE_WORKSPACE_WORKSPACEMANAGER_H
#define MUSE_WORKSPACE_WORKSPACEMANAGER_H

#include "iworkspacemanager.h"

#include "modularity/ioc.h"
#include "async/asyncable.h"
#include "../iworkspaceconfiguration.h"
#include "io/ifilesystem.h"
#include "workspace.h"

namespace muse::workspace {
class WorkspaceManager : public IWorkspaceManager, public Injectable, public async::Asyncable
{
    Inject<IWorkspaceConfiguration> configuration = { this };
    Inject<io::IFileSystem> fileSystem = { this };

public:

    WorkspaceManager(const modularity::ContextPtr& iocCtx)
        : Injectable(iocCtx) {}

    void init();
    void deinit();
    bool isInited() const;

    IWorkspacePtr defaultWorkspace() const override;

    IWorkspacePtr currentWorkspace() const override;
    async::Notification currentWorkspaceAboutToBeChanged() const override;
    async::Notification currentWorkspaceChanged() const override;

    IWorkspacePtrList workspaces() const override;
    Ret setWorkspaces(const IWorkspacePtrList& workspaces) override;
    async::Notification workspacesListChanged() const override;

    IWorkspacePtr newWorkspace(const std::string& workspaceName) const override;

private:
    void load();

    io::paths_t findWorkspaceFiles() const;

    WorkspacePtr doNewWorkspace(const std::string& workspaceName) const;

    void appendNewWorkspace(WorkspacePtr workspace);
    void setupConnectionsToNewWorkspace(const IWorkspacePtr workspace);

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
    async::Notification m_currentWorkspaceAboutToBeChanged;
    async::Notification m_currentWorkspaceChanged;

    std::vector<WorkspacePtr> m_workspaces;
    async::Notification m_workspacesListChanged;
};
}

#endif // MUSE_WORKSPACE_WORKSPACEMANAGER_H
