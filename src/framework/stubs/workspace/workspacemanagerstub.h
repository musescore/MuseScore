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
#ifndef MUSE_WORKSPACE_WORKSPACEMANAGERSTUB_H
#define MUSE_WORKSPACE_WORKSPACEMANAGERSTUB_H

#include "workspace/iworkspacemanager.h"

namespace muse::workspace {
class WorkspaceManagerStub : public IWorkspaceManager
{
public:
    IWorkspacePtr defaultWorkspace() const override;

    IWorkspacePtr currentWorkspace() const override;
    void prepareCurrentWorkspaceForChange() override;
    async::Notification currentWorkspaceAboutToBeChanged() const override;
    async::Notification currentWorkspaceChanged() const override;

    IWorkspacePtrList workspaces() const override;
    Ret setWorkspaces(const IWorkspacePtrList& workspaces) override;
    async::Notification workspacesListChanged() const override;

    IWorkspacePtr cloneWorkspace(const IWorkspacePtr& workspace, const std::string& newWorkspaceName) const override;
};
}

#endif // MU_WORKSPACE_WORKSPACEMANAGERSTUB_H
