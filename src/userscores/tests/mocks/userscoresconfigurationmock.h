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
#ifndef MU_USERSCORES_USERSCORESCONFIGURATIONMOCK_H
#define MU_USERSCORES_USERSCORESCONFIGURATIONMOCK_H

#include <gmock/gmock.h>

#include "userscores/iuserscoresconfiguration.h"

namespace mu::userscores {
class UserScoresConfigurationMock : public IUserScoresConfiguration
{
public:
    MOCK_METHOD(ValCh<io::paths>, recentScorePaths, (), (const, override));
    MOCK_METHOD(void, setRecentScorePaths, (const io::paths&), (override));

    MOCK_METHOD(io::path, myFirstScorePath, (), (const, override));

    MOCK_METHOD(io::paths, availableTemplatesPaths, (), (const, override));

    MOCK_METHOD(ValCh<io::path>, templatesPath, (), (const, override));
    MOCK_METHOD(void, setTemplatesPath, (const io::path&), (override));

    MOCK_METHOD(ValCh<io::path>, scoresPath, (), (const, override));
    MOCK_METHOD(void, setScoresPath, (const io::path&), (override));

    MOCK_METHOD(io::path, defaultSavingFilePath, (const io::path&), (const, override));
    MOCK_METHOD(io::path, defaultExportPath, (const std::string&), (const, override));
    MOCK_METHOD(io::path, completeExportPath, (io::path, notation::INotationPtr, bool, bool, int), (const, override));

    MOCK_METHOD(QColor, templatePreviewBackgroundColor, (), (const, override));
    MOCK_METHOD(async::Notification, templatePreviewBackgroundChanged, (), (const, override));

    MOCK_METHOD(PreferredScoreCreationMode, preferredScoreCreationMode, (), (const, override));
    MOCK_METHOD(void, setPreferredScoreCreationMode, (PreferredScoreCreationMode), (override));
};
}

#endif // MU_USERSCORES_USERSCORESCONFIGURATIONMOCK_H
