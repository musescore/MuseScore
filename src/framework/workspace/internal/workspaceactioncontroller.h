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
#ifndef MUSE_WORKSPACE_WORKSPACEACTIONCONTROLLER_H
#define MUSE_WORKSPACE_WORKSPACEACTIONCONTROLLER_H

#include "actions/actionable.h"

#include "modularity/ioc.h"
#include "iinteractive.h"
#include "actions/iactionsdispatcher.h"
#include "iworkspaceconfiguration.h"
#include "iworkspacemanager.h"

namespace muse::workspace {
class WorkspaceActionController : public Injectable, public actions::Actionable
{
    Inject<actions::IActionsDispatcher> dispatcher = { this };
    Inject<IInteractive> interactive = { this };
    Inject<IWorkspaceConfiguration> configuration = { this };
    Inject<IWorkspaceManager> manager = { this };

public:
    WorkspaceActionController(const modularity::ContextPtr& iocCtx)
        : Injectable(iocCtx) {}

    void init();

private:
    void selectWorkspace(const muse::actions::ActionData& args);
    void openConfigureWorkspacesDialog();
    void createNewWorkspace();
};
}

#endif // MUSE_WORKSPACE_WORKSPACEACTIONCONTROLLER_H
