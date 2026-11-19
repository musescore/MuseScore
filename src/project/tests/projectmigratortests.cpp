/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
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
#include <gmock/gmock.h>

#include "project/internal/projectmigrator.h"

using namespace muse;
using namespace mu::project;

namespace mu::project {
class ProjectMigratorTests : public ::testing::Test
{
protected:
    void SetUp() override
    {
        m_migrator = std::make_shared<ProjectMigrator>(modularity::globalCtx());
    }

    std::shared_ptr<ProjectMigrator> m_migrator;
};
}

TEST_F(ProjectMigratorTests, MigrationType_File36_IsVer36)
{
    EXPECT_EQ(m_migrator->migrationType(302), MigrationType::Ver_3_6);
}

TEST_F(ProjectMigratorTests, MigrationType_FilePre36_IsPre36)
{
    EXPECT_EQ(m_migrator->migrationType(301), MigrationType::Pre_3_6);
}

TEST_F(ProjectMigratorTests, MigrationType_CurrentFile_IsUnknown)
{
    EXPECT_EQ(m_migrator->migrationType(400), MigrationType::Unknown);
}
