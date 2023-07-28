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
#ifndef MU_WORKSPACE_WORKSPACEACTIONCONTROLLER_H
#define MU_WORKSPACE_WORKSPACEACTIONCONTROLLER_H

#include "modularity/ioc.h"
#include "actions/iactionsdispatcher.h"
#include "actions/actionable.h"
#include "iworkspaceconfiguration.h"
#include "iinteractive.h"

namespace mu::workspace {
class WorkspaceActionController : public actions::Actionable
{
    INJECT(actions::IActionsDispatcher, dispatcher)
    INJECT(IWorkspaceConfiguration, configuration)
    INJECT(framework::IInteractive, interactive)

public:
    void init();

private:
    void selectWorkspace(const actions::ActionData& args);
    void openConfigureWorkspacesDialog();

    void setCurrentWorkspaceName(const std::string& workspaceName);
};
}

#endif // MU_WORKSPACE_WORKSPACEACTIONCONTROLLER_H
