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

#include <vector>

#include "../iworkspacemanager.h"
#include "modularity/ioc.h"
#include "async/asyncable.h"
#include "../iworkspaceconfiguration.h"
#include "workspace.h"
#include "extensions/iextensionscontroller.h"

namespace mu {
namespace workspace {
class WorkspaceManager : public IWorkspaceManager, async::Asyncable
{
    INJECT(workspace, IWorkspaceConfiguration, configuration)
    INJECT(workspace, extensions::IExtensionsController, extensionsController)

public:

    RetValCh<std::shared_ptr<IWorkspace> > currentWorkspace() const override;

    void init();

private:
    void load();

    std::vector<io::path> findWorkspaceFiles() const;
    void setupCurrentWorkspace();

    std::shared_ptr<Workspace> findByName(const std::string& name) const;
    std::shared_ptr<Workspace> findAndInit(const std::string& name) const;

    std::shared_ptr<Workspace> m_currentWorkspace;
    std::vector<std::shared_ptr<Workspace> > m_workspaces;
    async::Channel<std::shared_ptr<IWorkspace> > m_currentWorkspaceChanged;
};
}
}
#endif // MU_WORKSPACE_WORKSPACEMANAGER_H
