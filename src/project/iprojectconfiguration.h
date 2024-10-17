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
#ifndef MU_PROJECT_IPROJECTCONFIGURATION_H
#define MU_PROJECT_IPROJECTCONFIGURATION_H

#include <QStringList>
#include <QColor>

#include "modularity/imoduleinterface.h"
#include "async/channel.h"
#include "async/notification.h"
#include "io/path.h"
#include "types/bytearray.h"

#include "inotationproject.h"
#include "types/projecttypes.h"

namespace mu::project {
class IProjectConfiguration : MODULE_EXPORT_INTERFACE
{
    INTERFACE_ID(IProjectConfiguration)

public:
    virtual ~IProjectConfiguration() = default;

    virtual muse::io::path_t recentFilesJsonPath() const = 0;
    virtual muse::ByteArray compatRecentFilesData() const = 0;

    virtual muse::io::path_t myFirstProjectPath() const = 0;

    virtual muse::io::paths_t availableTemplateDirs() const = 0;
    virtual muse::io::path_t templateCategoriesJsonPath(const muse::io::path_t& templatesDir) const = 0;

    virtual muse::io::path_t userTemplatesPath() const = 0;
    virtual void setUserTemplatesPath(const muse::io::path_t& path) = 0;
    virtual muse::async::Channel<muse::io::path_t> userTemplatesPathChanged() const = 0;

    virtual muse::io::path_t lastOpenedProjectsPath() const = 0;
    virtual void setLastOpenedProjectsPath(const muse::io::path_t& path) = 0;

    virtual muse::io::path_t lastSavedProjectsPath() const = 0;
    virtual void setLastSavedProjectsPath(const muse::io::path_t& path) = 0;

    virtual muse::io::path_t userProjectsPath() const = 0;
    virtual void setUserProjectsPath(const muse::io::path_t& path) = 0;
    virtual muse::async::Channel<muse::io::path_t> userProjectsPathChanged() const = 0;
    virtual muse::io::path_t defaultUserProjectsPath() const = 0;

    virtual bool shouldAskSaveLocationType() const = 0;
    virtual void setShouldAskSaveLocationType(bool shouldAsk) = 0;

    virtual bool isCloudProject(const muse::io::path_t& projectPath) const = 0;
    virtual bool isLegacyCloudProject(const muse::io::path_t& projectPath) const = 0;
    virtual muse::io::path_t cloudProjectPath(int scoreId) const = 0;
    virtual int cloudScoreIdFromPath(const muse::io::path_t& projectPath) const = 0;

    virtual muse::io::path_t cloudProjectSavingPath(int scoreId = 0) const = 0;

    virtual muse::io::path_t defaultSavingFilePath(INotationProjectPtr project, const std::string& filenameAddition = "",
                                                   const std::string& suffix = "") const = 0;

    virtual SaveLocationType lastUsedSaveLocationType() const = 0;
    virtual void setLastUsedSaveLocationType(SaveLocationType type) = 0;

    virtual bool shouldWarnBeforePublish() const = 0;
    virtual void setShouldWarnBeforePublish(bool shouldWarn) = 0;

    virtual bool shouldWarnBeforeSavingPubliclyToCloud() const = 0;
    virtual void setShouldWarnBeforeSavingPubliclyToCloud(bool shouldWarn) = 0;

    virtual int homeScoresPageTabIndex() const = 0;
    virtual void setHomeScoresPageTabIndex(int index) = 0;

    enum class HomeScoresPageViewType {
        Grid,
        List
    };

    virtual HomeScoresPageViewType homeScoresPageViewType() const = 0;
    virtual void setHomeScoresPageViewType(HomeScoresPageViewType type) = 0;

    virtual QColor templatePreviewBackgroundColor() const = 0;
    virtual muse::async::Notification templatePreviewBackgroundChanged() const = 0;

    enum class PreferredScoreCreationMode {
        FromInstruments,
        FromTemplate
    };

    virtual PreferredScoreCreationMode preferredScoreCreationMode() const = 0;
    virtual void setPreferredScoreCreationMode(PreferredScoreCreationMode mode) = 0;

    virtual MigrationOptions migrationOptions(MigrationType type) const = 0;
    virtual void setMigrationOptions(MigrationType type, const MigrationOptions& opt, bool persistent = true) = 0;

    virtual bool isAutoSaveEnabled() const = 0;
    virtual void setAutoSaveEnabled(bool enabled) = 0;
    virtual muse::async::Channel<bool> autoSaveEnabledChanged() const = 0;

    virtual int autoSaveIntervalMinutes() const = 0;
    virtual void setAutoSaveInterval(int minutes) = 0;
    virtual muse::async::Channel<int> autoSaveIntervalChanged() const = 0;

    virtual bool alsoShareAudioCom() const = 0;
    virtual void setAlsoShareAudioCom(bool share) = 0;
    virtual muse::async::Channel<bool> alsoShareAudioComChanged() const = 0;

    virtual bool showAlsoShareAudioComDialog() const = 0;
    virtual void setShowAlsoShareAudioComDialog(bool show) = 0;

    virtual bool hasAskedAlsoShareAudioCom() const = 0;
    virtual void setHasAskedAlsoShareAudioCom(bool has) = 0;

    virtual muse::io::path_t newProjectTemporaryPath() const = 0;

    virtual bool isAccessibleEnabled() const = 0;

    virtual bool shouldDestinationFolderBeOpenedOnExport() const = 0;
    virtual void setShouldDestinationFolderBeOpenedOnExport(bool shouldDestinationFolderBeOpenedOnExport) = 0;

    virtual QUrl supportForumUrl() const = 0;

    virtual bool openDetailedProjectUploadedDialog() const = 0;
    virtual void setOpenDetailedProjectUploadedDialog(bool show) = 0;

    virtual bool hasAskedAudioGenerationSettings() const = 0;
    virtual void setHasAskedAudioGenerationSettings(bool has) = 0;

    virtual GenerateAudioTimePeriodType generateAudioTimePeriodType() const = 0;
    virtual void setGenerateAudioTimePeriodType(GenerateAudioTimePeriodType type) = 0;

    virtual int numberOfSavesToGenerateAudio() const = 0;
    virtual void setNumberOfSavesToGenerateAudio(int number) = 0;

    virtual muse::io::path_t temporaryMp3FilePathTemplate() const = 0;

    virtual muse::io::path_t projectBackupPath(const muse::io::path_t& projectPath) const = 0;

    virtual bool showCloudIsNotAvailableWarning() const = 0;
    virtual void setShowCloudIsNotAvailableWarning(bool show) = 0;

    virtual bool disableVersionChecking() const = 0;
    virtual void setDisableVersionChecking(bool disable) = 0;

    virtual bool createBackupBeforeSaving() const = 0;
    virtual void setCreateBackupBeforeSaving(bool create) = 0;
};
}

#endif // MU_PROJECT_IPROJECTCONFIGURATION_H
