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

#include <set>

#include "io/fileinfo.h"
#include "workspace/tests/workspace_test_helpers.h"

using namespace muse;
using namespace muse::workspace;

TEST(Workspace_ConfigTests, ConfigIsValid)
{
    const auto& testCfg = WorkspaceTestConfig::instance();

    //! [THEN] default_workspace_name is a non-empty string
    EXPECT_FALSE(testCfg.defaultWorkspaceName().empty());

    //! [THEN] builtin_workspace_files is a non-empty list
    EXPECT_FALSE(testCfg.builtinFiles().empty());

    //! [THEN] Each builtin file exists on disk
    bool defaultFoundInBuiltins = false;
    for (const auto& filename : testCfg.builtinFiles()) {
        io::path_t fullPath = io::path_t(testCfg.builtinWorkspacesDir() + "/" + filename);
        EXPECT_TRUE(io::FileInfo::exists(fullPath)) << "missing: " << fullPath.toStdString();

        //! [THEN] Check if the default workspace matches a builtin file (without .mws)
        std::string baseName = io::completeBasename(fullPath).toStdString();
        if (baseName == testCfg.defaultWorkspaceName()) {
            defaultFoundInBuiltins = true;
        }
    }

    EXPECT_TRUE(defaultFoundInBuiltins)
        << "default workspace \"" << testCfg.defaultWorkspaceName() << "\" not found in builtin files";
}

//! If this test fails, a new field was added to workspaces.cfg.
//! Please add validation for it in ConfigIsValid above, then update EXPECTED_KEYS.
TEST(Workspace_ConfigTests, NoUntestedConfigKeys)
{
    const Config& cfg = WorkspaceTestConfig::instance().config();

    const std::set<std::string> EXPECTED_KEYS = {
        "default_workspace_name",
        "builtin_workspace_files",
    };

    std::set<std::string> actualKeys;
    for (const auto& [key, val] : cfg.data()) {
        actualKeys.insert(key);
    }

    EXPECT_EQ(actualKeys, EXPECTED_KEYS)
        << "workspaces.cfg has changed — add validation in ConfigIsValid and update EXPECTED_KEYS";
}
