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
#include "workspaceconfigurationstub.h"

using namespace muse::workspace;
using namespace muse;

io::paths_t WorkspaceConfigurationStub::workspacePaths() const
{
    return io::paths_t();
}

io::paths_t WorkspaceConfigurationStub::builtinWorkspacesFilePaths() const
{
    return io::paths_t();
}

io::path_t WorkspaceConfigurationStub::userWorkspacesPath() const
{
    return io::path_t();
}

std::string WorkspaceConfigurationStub::defaultWorkspaceName() const
{
    return std::string();
}

std::string WorkspaceConfigurationStub::currentWorkspaceName() const
{
    return std::string();
}

void WorkspaceConfigurationStub::setCurrentWorkspaceName(const std::string&)
{
}

async::Channel<std::string> WorkspaceConfigurationStub::currentWorkspaceNameChanged() const
{
    return async::Channel<std::string>();
}
