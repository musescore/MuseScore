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
#ifndef MU_WORKSPACE_WORKSPACEMANAGER_H
#define MU_WORKSPACE_WORKSPACEMANAGER_H

#include "../iworkspacemanager.h"
#include "modularity/ioc.h"
#include "async/asyncable.h"
#include "../iworkspaceconfiguration.h"
#include "../system/ifilesystem.h"
#include "workspace.h"
#include "extensions/iextensionscontroller.h"

namespace mu::workspace {
class WorkspaceManager : public IWorkspaceManager, async::Asyncable
{
    INJECT(workspace, IWorkspaceConfiguration, configuration)
    INJECT(workspace, extensions::IExtensionsController, extensionsController)
    INJECT(workspaces, framework::IFileSystem, fileSystem)

public:
    void init();
    void deinit();

    RetValCh<IWorkspacePtr> currentWorkspace() const override;

    RetVal<IWorkspacePtrList> workspaces() const override;
    Ret setWorkspaces(const IWorkspacePtrList& workspaces) override;

private:
    void load();
    void saveCurrentWorkspace();

    Ret removeMissingWorkspaces(const IWorkspacePtrList& newWorkspaceList);
    Ret removeWorkspace(const IWorkspacePtr& workspace);
    bool canRemoveWorkspace(const std::string& workspaceName) const;

    Ret createInexistentWorkspaces(const IWorkspacePtrList& newWorkspaceList);
    Ret createWorkspace(IWorkspacePtr workspace);

    io::paths findWorkspaceFiles() const;
    void setupCurrentWorkspace();

    WorkspacePtr findByName(const std::string& name) const;
    WorkspacePtr findAndInit(const std::string& name) const;

    WorkspacePtr m_currentWorkspace;
    std::vector<WorkspacePtr> m_workspaces;

    async::Channel<IWorkspacePtr> m_currentWorkspaceChanged;
};
}

#endif // MU_WORKSPACE_WORKSPACEMANAGER_H
