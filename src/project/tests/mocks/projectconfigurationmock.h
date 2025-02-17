/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2021 MuseScore Limited
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
    MOCK_METHOD(muse::io::path_t, recentFilesJsonPath, (), (const, override));
    MOCK_METHOD(muse::ByteArray, compatRecentFilesData, (), (const, override));

    MOCK_METHOD(muse::io::path_t, myFirstProjectPath, (), (const, override));

    MOCK_METHOD(muse::io::paths_t, availableTemplateDirs, (), (const, override));
    MOCK_METHOD(muse::io::path_t, templateCategoriesJsonPath, (const muse::io::path_t&), (const, override));

    MOCK_METHOD(muse::io::path_t, userTemplatesPath, (), (const, override));
    MOCK_METHOD(void, setUserTemplatesPath, (const muse::io::path_t&), (override));
    MOCK_METHOD(muse::async::Channel<muse::io::path_t>, userTemplatesPathChanged, (), (const, override));

    MOCK_METHOD(muse::io::path_t, lastOpenedProjectsPath, (), (const, override));
    MOCK_METHOD(void, setLastOpenedProjectsPath, (const muse::io::path_t&), (override));

    MOCK_METHOD(muse::io::path_t, lastSavedProjectsPath, (), (const, override));
    MOCK_METHOD(void, setLastSavedProjectsPath, (const muse::io::path_t&), (override));

    MOCK_METHOD(muse::io::path_t, userProjectsPath, (), (const, override));
    MOCK_METHOD(void, setUserProjectsPath, (const muse::io::path_t&), (override));
    MOCK_METHOD(muse::async::Channel<muse::io::path_t>, userProjectsPathChanged, (), (const, override));
    MOCK_METHOD(muse::io::path_t, defaultUserProjectsPath, (), (const, override));

    MOCK_METHOD(bool, shouldAskSaveLocationType, (), (const, override));
    MOCK_METHOD(void, setShouldAskSaveLocationType, (bool), (override));

    MOCK_METHOD(bool, isCloudProject, (const muse::io::path_t& projectPath), (const, override));
    MOCK_METHOD(bool, isLegacyCloudProject, (const muse::io::path_t& projectPath), (const, override));
    MOCK_METHOD(muse::io::path_t, cloudProjectPath, (int scoreId), (const, override));
    MOCK_METHOD(int, cloudScoreIdFromPath, (const muse::io::path_t& projectPath), (const, override));

    MOCK_METHOD(muse::io::path_t, cloudProjectSavingPath, (int scoreId), (const, override));

    MOCK_METHOD(muse::io::path_t, defaultSavingFilePath, (INotationProjectPtr, const std::string&, const std::string&), (const, override));

    MOCK_METHOD(SaveLocationType, lastUsedSaveLocationType, (), (const, override));
    MOCK_METHOD(void, setLastUsedSaveLocationType, (SaveLocationType), (override));

    MOCK_METHOD(bool, shouldWarnBeforePublish, (), (const, override));
    MOCK_METHOD(void, setShouldWarnBeforePublish, (bool), (override));

    MOCK_METHOD(bool, shouldWarnBeforeSavingPubliclyToCloud, (), (const, override));
    MOCK_METHOD(void, setShouldWarnBeforeSavingPubliclyToCloud, (bool), (override));

    MOCK_METHOD(HomeScoresPageViewType, homeScoresPageViewType, (), (const, override));
    MOCK_METHOD(void, setHomeScoresPageViewType, (HomeScoresPageViewType), (override));

    MOCK_METHOD(int, homeScoresPageTabIndex, (), (const, override));
    MOCK_METHOD(void, setHomeScoresPageTabIndex, (int), (override));

    MOCK_METHOD(QColor, templatePreviewBackgroundColor, (), (const, override));
    MOCK_METHOD(muse::async::Notification, templatePreviewBackgroundChanged, (), (const, override));

    MOCK_METHOD(PreferredScoreCreationMode, preferredScoreCreationMode, (), (const, override));
    MOCK_METHOD(void, setPreferredScoreCreationMode, (PreferredScoreCreationMode), (override));

    MOCK_METHOD(bool, inspectorExpandAccessibilitySection, (), (const, override));
    MOCK_METHOD(void, setInspectorExpandAccessibilitySection, (bool), (override));

    MOCK_METHOD(MigrationOptions, migrationOptions, (MigrationType), (const, override));
    MOCK_METHOD(void, setMigrationOptions, (MigrationType, const MigrationOptions&, bool), (override));

    MOCK_METHOD(bool, isAutoSaveEnabled, (), (const, override));
    MOCK_METHOD(void, setAutoSaveEnabled, (bool), (override));
    MOCK_METHOD(muse::async::Channel<bool>, autoSaveEnabledChanged, (), (const, override));

    MOCK_METHOD(int, autoSaveIntervalMinutes, (), (const, override));
    MOCK_METHOD(void, setAutoSaveInterval, (int), (override));
    MOCK_METHOD(muse::async::Channel<int>, autoSaveIntervalChanged, (), (const, override));

    MOCK_METHOD(bool, alsoShareAudioCom, (), (const, override));
    MOCK_METHOD(void, setAlsoShareAudioCom, (bool), (override));
    MOCK_METHOD(muse::async::Channel<bool>, alsoShareAudioComChanged, (), (const, override));

    MOCK_METHOD(bool, showAlsoShareAudioComDialog, (), (const, override));
    MOCK_METHOD(void, setShowAlsoShareAudioComDialog, (bool), (override));

    MOCK_METHOD(bool, hasAskedAlsoShareAudioCom, (), (const, override));
    MOCK_METHOD(void, setHasAskedAlsoShareAudioCom, (bool), (override));

    MOCK_METHOD(muse::io::path_t, newProjectTemporaryPath, (), (const, override));

    MOCK_METHOD(bool, isAccessibleEnabled, (), (const, override));

    MOCK_METHOD(bool, shouldDestinationFolderBeOpenedOnExport, (), (const, override));
    MOCK_METHOD(void, setShouldDestinationFolderBeOpenedOnExport, (bool), (override));

    MOCK_METHOD(QUrl, supportForumUrl, (), (const, override));

    MOCK_METHOD(bool, openDetailedProjectUploadedDialog, (), (const, override));
    MOCK_METHOD(void, setOpenDetailedProjectUploadedDialog, (bool), (override));

    MOCK_METHOD(bool, hasAskedAudioGenerationSettings, (), (const, override));
    MOCK_METHOD(void, setHasAskedAudioGenerationSettings, (bool), (override));

    MOCK_METHOD(GenerateAudioTimePeriodType, generateAudioTimePeriodType, (), (const, override));
    MOCK_METHOD(void, setGenerateAudioTimePeriodType, (GenerateAudioTimePeriodType), (override));
    MOCK_METHOD(muse::async::Channel<int>, generateAudioTimePeriodTypeChanged, (), (const, override));

    MOCK_METHOD(int, numberOfSavesToGenerateAudio, (), (const, override));
    MOCK_METHOD(void, setNumberOfSavesToGenerateAudio, (int), (override));
    MOCK_METHOD(muse::async::Channel<int>, numberOfSavesToGenerateAudioChanged, (), (const, override));

    MOCK_METHOD(muse::io::path_t, temporaryMp3FilePathTemplate, (), (const, override));

    MOCK_METHOD(muse::io::path_t, projectBackupPath, (const muse::io::path_t&), (const, override));

    MOCK_METHOD(bool, showCloudIsNotAvailableWarning, (), (const, override));
    MOCK_METHOD(void, setShowCloudIsNotAvailableWarning, (bool), (override));

    MOCK_METHOD(bool, disableVersionChecking, (), (const, override));
    MOCK_METHOD(void, setDisableVersionChecking, (bool), (override));

    MOCK_METHOD(bool, createBackupBeforeSaving, (), (const, override));
    MOCK_METHOD(void, setCreateBackupBeforeSaving, (bool), (override));
};
}

#endif // MU_PROJECT_PROJECTCONFIGURATIONMOCK_H
