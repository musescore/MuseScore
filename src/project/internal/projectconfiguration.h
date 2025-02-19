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
#ifndef MU_PROJECT_PROJECTCONFIGURATION_H
#define MU_PROJECT_PROJECTCONFIGURATION_H

#include "types/val.h"

#include "modularity/ioc.h"
#include "global/iglobalconfiguration.h"
#include "io/ifilesystem.h"
#include "accessibility/iaccessibilityconfiguration.h"
#include "notation/inotationconfiguration.h"
#include "cloud/icloudconfiguration.h"
#include "languages/ilanguagesservice.h"

#include "../iprojectconfiguration.h"

namespace mu::project {
class ProjectConfiguration : public IProjectConfiguration
{
    INJECT(muse::IGlobalConfiguration, globalConfiguration)
    INJECT(notation::INotationConfiguration, notationConfiguration)
    INJECT(muse::cloud::ICloudConfiguration, cloudConfiguration)
    INJECT(muse::accessibility::IAccessibilityConfiguration, accessibilityConfiguration)
    INJECT(muse::io::IFileSystem, fileSystem)
    INJECT(muse::languages::ILanguagesService, languagesService)

public:
    void init();

    muse::io::path_t recentFilesJsonPath() const override;
    muse::ByteArray compatRecentFilesData() const override;

    muse::io::path_t myFirstProjectPath() const override;

    muse::io::paths_t availableTemplateDirs() const override;
    muse::io::path_t templateCategoriesJsonPath(const muse::io::path_t& templatesDir) const override;

    muse::io::path_t userTemplatesPath() const override;
    void setUserTemplatesPath(const muse::io::path_t& path) override;
    muse::async::Channel<muse::io::path_t> userTemplatesPathChanged() const override;

    muse::io::path_t lastOpenedProjectsPath() const override;
    void setLastOpenedProjectsPath(const muse::io::path_t& path) override;

    muse::io::path_t lastSavedProjectsPath() const override;
    void setLastSavedProjectsPath(const muse::io::path_t& path) override;

    muse::io::path_t userProjectsPath() const override;
    void setUserProjectsPath(const muse::io::path_t& path) override;
    muse::async::Channel<muse::io::path_t> userProjectsPathChanged() const override;
    muse::io::path_t defaultUserProjectsPath() const override;

    bool shouldAskSaveLocationType() const override;
    void setShouldAskSaveLocationType(bool shouldAsk) override;

    bool isCloudProject(const muse::io::path_t& projectPath) const override;
    bool isLegacyCloudProject(const muse::io::path_t& projectPath) const override;
    muse::io::path_t cloudProjectPath(int scoreId) const override;
    int cloudScoreIdFromPath(const muse::io::path_t& projectPath) const override;

    muse::io::path_t cloudProjectSavingPath(int scoreId = 0) const override;

    muse::io::path_t defaultSavingFilePath(INotationProjectPtr project, const std::string& filenameAddition = "",
                                           const std::string& suffix = "") const override;

    SaveLocationType lastUsedSaveLocationType() const override;
    void setLastUsedSaveLocationType(SaveLocationType type) override;

    bool shouldWarnBeforePublish() const override;
    void setShouldWarnBeforePublish(bool shouldWarn) override;

    bool shouldWarnBeforeSavingPubliclyToCloud() const override;
    void setShouldWarnBeforeSavingPubliclyToCloud(bool shouldWarn) override;

    int homeScoresPageTabIndex() const override;
    void setHomeScoresPageTabIndex(int index) override;

    HomeScoresPageViewType homeScoresPageViewType() const override;
    void setHomeScoresPageViewType(HomeScoresPageViewType type) override;

    QColor templatePreviewBackgroundColor() const override;
    muse::async::Notification templatePreviewBackgroundChanged() const override;

    PreferredScoreCreationMode preferredScoreCreationMode() const override;
    void setPreferredScoreCreationMode(PreferredScoreCreationMode mode) override;

    bool inspectorExpandAccessibilitySection() const override;
    void setInspectorExpandAccessibilitySection(bool mode) override;

    MigrationOptions migrationOptions(MigrationType type) const override;
    void setMigrationOptions(MigrationType type, const MigrationOptions& opt, bool persistent = true) override;

    bool isAutoSaveEnabled() const override;
    void setAutoSaveEnabled(bool enabled) override;
    muse::async::Channel<bool> autoSaveEnabledChanged() const override;

    int autoSaveIntervalMinutes() const override;
    void setAutoSaveInterval(int minutes) override;
    muse::async::Channel<int> autoSaveIntervalChanged() const override;

    bool alsoShareAudioCom() const override;
    void setAlsoShareAudioCom(bool share) override;
    muse::async::Channel<bool> alsoShareAudioComChanged() const override;

    bool showAlsoShareAudioComDialog() const override;
    void setShowAlsoShareAudioComDialog(bool show) override;

    bool hasAskedAlsoShareAudioCom() const override;
    void setHasAskedAlsoShareAudioCom(bool has) override;

    muse::io::path_t newProjectTemporaryPath() const override;

    bool isAccessibleEnabled() const override;

    bool shouldDestinationFolderBeOpenedOnExport() const override;
    void setShouldDestinationFolderBeOpenedOnExport(bool shouldDestinationFolderBeOpenedOnExport) override;

    QUrl supportForumUrl() const override;

    bool openDetailedProjectUploadedDialog() const override;
    void setOpenDetailedProjectUploadedDialog(bool show) override;

    bool hasAskedAudioGenerationSettings() const override;
    void setHasAskedAudioGenerationSettings(bool has) override;

    GenerateAudioTimePeriodType generateAudioTimePeriodType() const override;
    void setGenerateAudioTimePeriodType(GenerateAudioTimePeriodType type) override;
    muse::async::Channel<int> generateAudioTimePeriodTypeChanged() const override;

    int numberOfSavesToGenerateAudio() const override;
    void setNumberOfSavesToGenerateAudio(int number) override;
    muse::async::Channel<int> numberOfSavesToGenerateAudioChanged() const override;

    muse::io::path_t temporaryMp3FilePathTemplate() const override;

    muse::io::path_t projectBackupPath(const muse::io::path_t& projectPath) const override;

    bool showCloudIsNotAvailableWarning() const override;
    void setShowCloudIsNotAvailableWarning(bool show) override;

    bool disableVersionChecking() const override;
    void setDisableVersionChecking(bool disable) override;

    bool createBackupBeforeSaving() const override;
    void setCreateBackupBeforeSaving(bool create) override;

private:
    muse::io::path_t appTemplatesPath() const;
    muse::io::path_t legacyCloudProjectsPath() const;
    muse::io::path_t cloudProjectsPath() const;

    muse::async::Channel<muse::io::path_t> m_userTemplatesPathChanged;
    muse::async::Channel<muse::io::path_t> m_userScoresPathChanged;

    int m_homeScoresPageTabIndex = 0;
    muse::async::Notification m_homeScoresPageTabIndexChanged;

    muse::async::Channel<bool> m_autoSaveEnabledChanged;
    muse::async::Channel<int> m_autoSaveIntervalChanged;

    muse::async::Channel<int> m_generateAudioTimePeriodTypeChanged;
    muse::async::Channel<int> m_numberOfSavesToGenerateAudioChanged;

    muse::async::Channel<bool> m_alsoShareAudioComChanged;

    mutable std::map<MigrationType, MigrationOptions> m_migrationOptions;
};
}

#endif // MU_PROJECT_PROJECTCONFIGURATION_H
