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
#ifndef MU_PROJECT_PROJECTCONFIGURATIONMOCK_H
#define MU_PROJECT_PROJECTCONFIGURATIONMOCK_H

#include <gmock/gmock.h>

#include "project/iprojectconfiguration.h"

namespace mu::project {
class ProjectConfigurationMock : public project::IProjectConfiguration
{
public:
    MOCK_METHOD(io::paths, recentProjectPaths, (), (const, override));
    MOCK_METHOD(void, setRecentProjectPaths, (const io::paths&), (override));
    MOCK_METHOD(async::Channel<io::paths>, recentProjectPathsChanged, (), (const, override));

    MOCK_METHOD(io::path, myFirstProjectPath, (), (const, override));

    MOCK_METHOD(io::paths, availableTemplatesPaths, (), (const, override));

    MOCK_METHOD(io::path, userTemplatesPath, (), (const, override));
    MOCK_METHOD(void, setUserTemplatesPath, (const io::path&), (override));
    MOCK_METHOD(async::Channel<io::path>, userTemplatesPathChanged, (), (const, override));

    MOCK_METHOD(io::path, userProjectsPath, (), (const, override));
    MOCK_METHOD(void, setUserProjectsPath, (const io::path&), (override));
    MOCK_METHOD(async::Channel<io::path>, userProjectsPathChanged, (), (const, override));

    MOCK_METHOD(io::path, defaultSavingFilePath, (const io::path&), (const, override));

    MOCK_METHOD(QColor, templatePreviewBackgroundColor, (), (const, override));
    MOCK_METHOD(async::Notification, templatePreviewBackgroundChanged, (), (const, override));

    MOCK_METHOD(PreferredScoreCreationMode, preferredScoreCreationMode, (), (const, override));
    MOCK_METHOD(void, setPreferredScoreCreationMode, (PreferredScoreCreationMode), (override));
};
}

#endif // MU_PROJECT_PROJECTCONFIGURATIONMOCK_H
