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
#ifndef MUSE_WORKSPACE_IWORKSPACECONFIGURATION_H
#define MUSE_WORKSPACE_IWORKSPACECONFIGURATION_H

#include <vector>

#include "io/path.h"
#include "modularity/imoduleinterface.h"
#include "async/channel.h"

namespace muse::workspace {
class IWorkspaceConfiguration : MODULE_EXPORT_INTERFACE
{
    INTERFACE_ID(IWorkspaceConfiguration)

public:
    virtual ~IWorkspaceConfiguration() = default;

    virtual io::paths_t workspacePaths() const = 0;

    virtual io::paths_t builtinWorkspacesFilePaths() const = 0;
    virtual io::path_t userWorkspacesPath() const = 0;

    virtual std::string defaultWorkspaceName() const = 0;

    virtual std::string currentWorkspaceName() const = 0;
    virtual void setCurrentWorkspaceName(const std::string& workspaceName) = 0;
    virtual async::Channel<std::string> currentWorkspaceNameChanged() const = 0;
};
}

#endif // MUSE_WORKSPACE_IWORKSPACECONFIGURATION_H
