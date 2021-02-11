//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2021 MuseScore BVBA and others
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
#ifndef MU_WORKSPACE_WORKSPACEACTIONCONTROLLER_H
#define MU_WORKSPACE_WORKSPACEACTIONCONTROLLER_H

#include "modularity/ioc.h"
#include "actions/iactionsdispatcher.h"
#include "actions/actionable.h"
#include "iworkspaceconfiguration.h"

namespace mu::workspace {
class WorkspaceActionController : public actions::Actionable
{
    INJECT(appshell, actions::IActionsDispatcher, dispatcher)
    INJECT(appshell, IWorkspaceConfiguration, configuration)

public:
    void init();

private:
    void selectWorkspace(const actions::ActionData& args);
};
}

#endif // MU_WORKSPACE_WORKSPACEACTIONCONTROLLER_H
