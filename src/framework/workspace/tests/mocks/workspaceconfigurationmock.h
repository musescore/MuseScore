/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-CLA-applies
 *
 * MuseScore
 * Music Composition & Notation
 *
 * Copyright (C) 2026 MuseScore Limited and others
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
#pragma once

#include <gmock/gmock.h>

#include "workspace/iworkspaceconfiguration.h"

namespace muse::workspace {
class WorkspaceConfigurationMock : public IWorkspaceConfiguration
{
public:
    MOCK_METHOD(io::paths_t, workspacePaths, (), (const, override));
    MOCK_METHOD(io::paths_t, builtinWorkspacesFilePaths, (), (const, override));
    MOCK_METHOD(io::path_t, userWorkspacesPath, (), (const, override));
    MOCK_METHOD(std::string, defaultWorkspaceName, (), (const, override));
    MOCK_METHOD(std::string, currentWorkspaceName, (), (const, override));
    MOCK_METHOD(void, setCurrentWorkspaceName, (const std::string&), (override));
    MOCK_METHOD(async::Channel<std::string>, currentWorkspaceNameChanged, (), (const, override));
};
}
