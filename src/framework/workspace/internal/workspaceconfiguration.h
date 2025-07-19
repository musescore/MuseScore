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
#ifndef MUSE_WORKSPACE_WORKSPACECONFIGURATION_H
#define MUSE_WORKSPACE_WORKSPACECONFIGURATION_H

#include "global/types/config.h"
#include "async/asyncable.h"

#include "modularity/ioc.h"
#include "iglobalconfiguration.h"
#include "io/ifilesystem.h"

#include "../iworkspaceconfiguration.h"

namespace muse::workspace {
class WorkspaceConfiguration : public IWorkspaceConfiguration, public Injectable, public async::Asyncable
{
    Inject<IGlobalConfiguration> globalConfiguration = { this };
    Inject<io::IFileSystem> fileSystem = { this };

public:
    WorkspaceConfiguration(const modularity::ContextPtr& iocCtx)
        : Injectable(iocCtx) {}

    void init();

    io::paths_t workspacePaths() const override;

    io::paths_t builtinWorkspacesFilePaths() const override;
    io::path_t userWorkspacesPath() const override;

    std::string defaultWorkspaceName() const override;

    std::string currentWorkspaceName() const override;
    void setCurrentWorkspaceName(const std::string& workspaceName) override;
    async::Channel<std::string> currentWorkspaceNameChanged() const override;

private:
    Config m_config;

    async::Channel<std::string> m_currentWorkspaceNameChanged;
};
}

#endif // MUSE_WORKSPACE_WORKSPACECONFIGURATION_H
