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
#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include <QTemporaryDir>

#include "modularity/ioc.h"
#include "global/configreader.h"
#include "workspace/internal/workspacemanager.h"
#include "workspace/iworkspaceconfiguration.h"
#include "workspace/tests/mocks/workspaceconfigurationmock.h"
#include "multiwindows/tests/mocks/multiwindowsprovidermock.h"

using ::testing::Return;

using namespace muse;
using namespace muse::workspace;

static const std::string BUILTIN_WORKSPACE_DIR = BUILTIN_WORKSPACES_DIR;

const Config& workspaceTestConfig();

namespace muse::workspace {
class Workspace_WorkspaceManagerTests : public ::testing::Test
{
public:
    void SetUp() override
    {
        const Config& cfg = workspaceTestConfig();
        m_defaultWorkspaceName = cfg.value("default_workspace_name").toString();

        ValList builtinFiles = cfg.value("builtin_workspace_files").toList();
        for (const Val& v : builtinFiles) {
            m_builtinFiles.push_back(v.toString());
        }

        m_userWorkspacesDir = std::make_unique<QTemporaryDir>();
        ASSERT_TRUE(m_userWorkspacesDir->isValid());

        m_multiWindowsProvider = std::make_shared<::testing::NiceMock<mi::MultiWindowsProviderMock> >();
        modularity::globalIoc()->registerExport<mi::IMultiWindowsProvider>("utests", m_multiWindowsProvider);

        m_workspaceConfig = std::dynamic_pointer_cast<muse::workspace::WorkspaceConfigurationMock>(
            modularity::globalIoc()->resolve<IWorkspaceConfiguration>("utests"));
        ASSERT_NE(m_workspaceConfig, nullptr);

        m_userWorkspacesPath = m_userWorkspacesDir->path().toStdString();
    }

    void TearDown() override
    {
        if (m_manager) {
            m_manager->deinit();
            m_manager.reset();
        }
        modularity::globalIoc()->unregister<mi::IMultiWindowsProvider>("utests");
    }

    void setupBuiltinPaths(const std::vector<std::string>& filenames)
    {
        io::paths_t paths;
        for (const auto& name : filenames) {
            paths.push_back(io::path_t(BUILTIN_WORKSPACE_DIR + "/" + name));
        }

        ON_CALL(*m_workspaceConfig, builtinWorkspacesFilePaths())
        .WillByDefault(Return(paths));

        ON_CALL(*m_workspaceConfig, userWorkspacesPath())
        .WillByDefault(Return(io::path_t(m_userWorkspacesPath)));
    }

    void initManager()
    {
        m_manager = std::make_unique<WorkspaceManager>(modularity::globalCtx());
        m_manager->init();
    }

protected:
    std::string m_defaultWorkspaceName;
    std::vector<std::string> m_builtinFiles;

    std::unique_ptr<QTemporaryDir> m_userWorkspacesDir;
    std::string m_userWorkspacesPath;

    std::shared_ptr<::testing::NiceMock<mi::MultiWindowsProviderMock> > m_multiWindowsProvider;
    std::shared_ptr<muse::workspace::WorkspaceConfigurationMock> m_workspaceConfig;

    std::unique_ptr<WorkspaceManager> m_manager;
};

TEST_F(Workspace_WorkspaceManagerTests, BuiltinWorkspaceLoadsOnInit)
{
    //! [GIVEN] All builtin workspace files exist
    setupBuiltinPaths(m_builtinFiles);

    //! [GIVEN] Current workspace is the default
    ON_CALL(*m_workspaceConfig, currentWorkspaceName())
    .WillByDefault(Return(m_defaultWorkspaceName));

    //! [WHEN] WorkspaceManager is initialized
    initManager();

    //! [THEN] Default workspace is loaded
    EXPECT_NE(m_manager->defaultWorkspace(), nullptr);
    EXPECT_EQ(m_manager->defaultWorkspace()->name(), m_defaultWorkspaceName);

    //! [THEN] Current workspace matches default
    EXPECT_NE(m_manager->currentWorkspace(), nullptr);
    EXPECT_EQ(m_manager->currentWorkspace()->name(), m_defaultWorkspaceName);

    //! [THEN] All builtin workspaces are available
    EXPECT_EQ(m_manager->workspaces().size(), m_builtinFiles.size());
}

TEST_F(Workspace_WorkspaceManagerTests, FallbackToDefaultOnMissingCurrentWorkspace)
{
    //! [GIVEN] All builtin workspace files exist
    setupBuiltinPaths(m_builtinFiles);

    //! [GIVEN] Current workspace name references a workspace that no longer exists
    ON_CALL(*m_workspaceConfig, currentWorkspaceName())
    .WillByDefault(Return("MyDeletedWorkspace"));

    //! [GIVEN] Manager will correct the setting to default
    EXPECT_CALL(*m_workspaceConfig, setCurrentWorkspaceName(m_defaultWorkspaceName))
    .Times(1);

    //! [WHEN] WorkspaceManager is initialized
    initManager();

    //! [THEN] Falls back to default workspace
    EXPECT_NE(m_manager->currentWorkspace(), nullptr);
    EXPECT_EQ(m_manager->currentWorkspace()->name(), m_defaultWorkspaceName);
    EXPECT_NE(m_manager->defaultWorkspace(), nullptr);
    EXPECT_EQ(m_manager->defaultWorkspace()->name(), m_defaultWorkspaceName);
}

TEST_F(Workspace_WorkspaceManagerTests, EmptyUserDirUsesBuiltinWorkspaces)
{
    //! [GIVEN] All builtin workspace files exist, user dir is empty
    setupBuiltinPaths(m_builtinFiles);

    ON_CALL(*m_workspaceConfig, currentWorkspaceName())
    .WillByDefault(Return(m_defaultWorkspaceName));

    //! [WHEN] WorkspaceManager is initialized
    initManager();

    //! [THEN] Workspaces are loaded from builtins only
    EXPECT_EQ(m_manager->workspaces().size(), m_builtinFiles.size());

    //! [THEN] Current workspace is loaded and usable
    auto current = m_manager->currentWorkspace();
    ASSERT_NE(current, nullptr);
    EXPECT_EQ(current->name(), m_defaultWorkspaceName);

    //! [THEN] Reading workspace data does not crash
    auto data = current->rawData("ui_settings");
    EXPECT_TRUE(data.ret);
}
}
