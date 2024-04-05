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
#ifndef MUSE_WORKSPACE_WORKSPACECONFIGURATIONSTUB_H
#define MUSE_WORKSPACE_WORKSPACECONFIGURATIONSTUB_H

#include "workspace/iworkspaceconfiguration.h"

namespace muse::workspace {
class WorkspaceConfigurationStub : public IWorkspaceConfiguration
{
public:

    io::paths_t workspacePaths() const override;

    io::path_t userWorkspacesPath() const override;

    std::string currentWorkspaceName() const override;
    void setCurrentWorkspaceName(const std::string& workspaceName) override;
    async::Channel<std::string> currentWorkspaceNameChanged() const override;
};
}

#endif // MU_WORKSPACE_WORKSPACECONFIGURATIONSTUB_H
