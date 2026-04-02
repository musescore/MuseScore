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

#include "testing/environment.h"

#include "global/configreader.h"
#include "workspace/tests/mocks/workspaceconfigurationmock.h"

using namespace ::testing;

static muse::Config s_workspaceConfig;

static muse::testing::SuiteEnvironment workspace_se
    = muse::testing::SuiteEnvironment()
      .setPreInit([](){
    s_workspaceConfig = muse::ConfigReader::read(muse::io::path_t(WORKSPACE_CONFIG_FILE));

    auto workspaceConfigMock = std::make_shared<::testing::NiceMock<muse::workspace::WorkspaceConfigurationMock> >();

    ON_CALL(*workspaceConfigMock, defaultWorkspaceName())
    .WillByDefault(Return(s_workspaceConfig.value("default_workspace_name").toString()));

    muse::modularity::globalIoc()->registerExport<muse::workspace::IWorkspaceConfiguration>("utests", workspaceConfigMock);
}).setDeInit([](){
    muse::modularity::globalIoc()->unregister<muse::workspace::IWorkspaceConfiguration>("utests");
});

const muse::Config& workspaceTestConfig()
{
    return s_workspaceConfig;
}
