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
#ifndef MU_PROJECT_IPROJECTCONFIGURATION_H
#define MU_PROJECT_IPROJECTCONFIGURATION_H

#include <QStringList>
#include <QColor>

#include "modularity/imoduleinterface.h"
#include "io/path.h"
#include "async/channel.h"
#include "async/notification.h"
#include "inotationproject.h"
#include "projecttypes.h"

namespace mu::project {
class IProjectConfiguration : MODULE_EXPORT_INTERFACE
{
    INTERFACE_ID(IProjectConfiguration)

public:
    virtual ~IProjectConfiguration() = default;

    virtual io::paths_t recentProjectPaths() const = 0;
    virtual void setRecentProjectPaths(const io::paths_t& recentScorePaths) = 0;
    virtual async::Channel<io::paths_t> recentProjectPathsChanged() const = 0;

    virtual io::path_t myFirstProjectPath() const = 0;

    virtual io::paths_t availableTemplateDirs() const = 0;
    virtual io::path_t templateCategoriesJsonPath(const io::path_t& templatesDir) const = 0;

    virtual io::path_t userTemplatesPath() const = 0;
    virtual void setUserTemplatesPath(const io::path_t& path) = 0;
    virtual async::Channel<io::path_t> userTemplatesPathChanged() const = 0;

    virtual io::path_t defaultProjectsPath() const = 0;
    virtual void setDefaultProjectsPath(const io::path_t& path) = 0;

    virtual io::path_t lastOpenedProjectsPath() const = 0;
    virtual void setLastOpenedProjectsPath(const io::path_t& path) = 0;

    virtual io::path_t lastSavedProjectsPath() const = 0;
    virtual void setLastSavedProjectsPath(const io::path_t& path) = 0;

    virtual io::path_t userProjectsPath() const = 0;
    virtual void setUserProjectsPath(const io::path_t& path) = 0;
    virtual async::Channel<io::path_t> userProjectsPathChanged() const = 0;

    virtual bool shouldAskSaveLocationType() const = 0;
    virtual void setShouldAskSaveLocationType(bool shouldAsk) = 0;

    virtual bool isCloudProject(const io::path_t& projectPath) const = 0;

    virtual io::path_t cloudProjectSavingFilePath(const io::path_t& projectName) const = 0;
    virtual io::path_t defaultSavingFilePath(INotationProjectPtr project, const std::string& filenameAddition = "",
                                             const std::string& suffix = "") const = 0;

    virtual SaveLocationType lastUsedSaveLocationType() const = 0;
    virtual void setLastUsedSaveLocationType(SaveLocationType type) = 0;

    virtual bool shouldWarnBeforePublish() const = 0;
    virtual void setShouldWarnBeforePublish(bool shouldWarn) = 0;

    virtual bool shouldWarnBeforeSavingPubliclyToCloud() const = 0;
    virtual void setShouldWarnBeforeSavingPubliclyToCloud(bool shouldWarn) = 0;

    virtual QColor templatePreviewBackgroundColor() const = 0;
    virtual async::Notification templatePreviewBackgroundChanged() const = 0;

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
    virtual async::Channel<bool> autoSaveEnabledChanged() const = 0;

    virtual int autoSaveIntervalMinutes() const = 0;
    virtual void setAutoSaveInterval(int minutes) = 0;
    virtual async::Channel<int> autoSaveIntervalChanged() const = 0;

    virtual io::path_t newProjectTemporaryPath() const = 0;

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

    virtual io::path_t temporaryMp3FilePathTemplate() const = 0;

    virtual io::path_t projectBackupPath(const io::path_t& projectPath) const = 0;

    virtual bool showCloudIsNotAvailableWarning() const = 0;
    virtual void setShowCloudIsNotAvailableWarning(bool show) = 0;
};
}

#endif // MU_PROJECT_IPROJECTCONFIGURATION_H
