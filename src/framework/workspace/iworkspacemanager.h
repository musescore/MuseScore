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
#ifndef MUSE_WORKSPACE_IWORKSPACEMANAGER_H
#define MUSE_WORKSPACE_IWORKSPACEMANAGER_H

#include "modularity/imoduleinterface.h"

#include "iworkspace.h"
#include "types/retval.h"
#include "async/notification.h"

namespace muse::workspace {
class IWorkspaceManager : MODULE_EXPORT_INTERFACE
{
    INTERFACE_ID(IWorkspaceManager)

public:
    virtual ~IWorkspaceManager() = default;

    //! NOTE Default from user app data, writable, full managed
    virtual IWorkspacePtr defaultWorkspace() const = 0;

    //! NOTE Current selected by a user, writable, optionally managed
    virtual IWorkspacePtr currentWorkspace() const = 0;
    virtual async::Notification currentWorkspaceAboutToBeChanged() const = 0;
    virtual async::Notification currentWorkspaceChanged() const = 0;

    virtual IWorkspacePtrList workspaces() const = 0;
    virtual Ret setWorkspaces(const IWorkspacePtrList& workspaces) = 0;
    virtual async::Notification workspacesListChanged() const = 0;

    virtual IWorkspacePtr newWorkspace(const std::string& workspaceName) const = 0;
};
}

#endif // MUSE_WORKSPACE_IWORKSPACEMANAGER_H
