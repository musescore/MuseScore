//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2020 MuseScore BVBA and others
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
//=============================================================================
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

    MOCK_METHOD(QColor, templatePreviewBackgroundColor, (), (const, override));
    MOCK_METHOD(async::Notification, templatePreviewBackgroundChanged, (), (const, override));

    MOCK_METHOD(PreferredScoreCreationMode, preferredScoreCreationMode, (), (const, override));
    MOCK_METHOD(void, setPreferredScoreCreationMode, (PreferredScoreCreationMode), (override));
};
}

#endif // MU_USERSCORES_USERSCORESCONFIGURATIONMOCK_H
