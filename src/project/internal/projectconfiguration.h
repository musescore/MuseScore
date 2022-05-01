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
#ifndef MU_PROJECT_PROJECTCONFIGURATION_H
#define MU_PROJECT_PROJECTCONFIGURATION_H

#include "global/val.h"

#include "modularity/ioc.h"
#include "global/iglobalconfiguration.h"
#include "system/ifilesystem.h"
#include "accessibility/iaccessibilityconfiguration.h"
#include "notation/inotationconfiguration.h"

#include "../iprojectconfiguration.h"

namespace mu::project {
class ProjectConfiguration : public IProjectConfiguration
{
    INJECT(project, framework::IGlobalConfiguration, globalConfiguration)
    INJECT(project, notation::INotationConfiguration, notationConfiguration)
    INJECT(project, accessibility::IAccessibilityConfiguration, accessibilityConfiguration)
    INJECT(project, system::IFileSystem, fileSystem)

public:
    static const QString DEFAULT_FILE_SUFFIX;
    static const QString DEFAULT_EXPORT_SUFFIX;

    void init();

    io::paths recentProjectPaths() const override;
    void setRecentProjectPaths(const io::paths& recentScorePaths) override;
    async::Channel<io::paths> recentProjectPathsChanged() const override;

    io::path myFirstProjectPath() const override;

    io::paths availableTemplateDirs() const override;
    io::path templateCategoriesJsonPath(const io::path& templatesDir) const override;

    io::path userTemplatesPath() const override;
    void setUserTemplatesPath(const io::path& path) override;
    async::Channel<io::path> userTemplatesPathChanged() const override;

    io::path userProjectsPath() const override;
    void setUserProjectsPath(const io::path& path) override;
    async::Channel<io::path> userProjectsPathChanged() const override;

    io::path cloudProjectsPath() const override;
    bool isCloudProject(const io::path& path) const override;

    bool shouldAskSaveLocationType() const override;
    void setShouldAskSaveLocationType(bool shouldAsk) override;

    io::path defaultSavingFilePath(INotationProjectPtr project, const QString& filenameAddition = QString(),
                                   const QString& suffix = QString()) const override;

    SaveLocationType lastUsedSaveLocationType() const override;
    void setLastUsedSaveLocationType(SaveLocationType type) override;

    bool shouldWarnBeforePublishing() const override;
    void setShouldWarnBeforePublishing(bool shouldWarn) override;

    QColor templatePreviewBackgroundColor() const override;
    async::Notification templatePreviewBackgroundChanged() const override;

    PreferredScoreCreationMode preferredScoreCreationMode() const override;
    void setPreferredScoreCreationMode(PreferredScoreCreationMode mode) override;

    MigrationOptions migrationOptions(MigrationType type) const override;
    void setMigrationOptions(MigrationType type, const MigrationOptions& opt, bool persistent = true) override;

    bool isAutoSaveEnabled() const override;
    void setAutoSaveEnabled(bool enabled) override;
    async::Channel<bool> autoSaveEnabledChanged() const override;

    int autoSaveIntervalMinutes() const override;
    void setAutoSaveInterval(int minutes) override;
    async::Channel<int> autoSaveIntervalChanged() const override;

    io::path newProjectTemporaryPath() const override;

    bool isAccessibleEnabled() const override;

    bool shouldDestinationFolderBeOpenedOnExport() const override;
    void setShouldDestinationFolderBeOpenedOnExport(bool shouldDestinationFolderBeOpenedOnExport) override;

private:
    io::paths parseRecentProjectsPaths(const mu::Val& value) const;

    io::path appTemplatesPath() const;

    async::Channel<io::paths> m_recentProjectPathsChanged;
    async::Channel<io::path> m_userTemplatesPathChanged;
    async::Channel<io::path> m_userScoresPathChanged;

    async::Channel<bool> m_autoSaveEnabledChanged;
    async::Channel<int> m_autoSaveIntervalChanged;

    mutable std::map<MigrationType, MigrationOptions> m_migrationOptions;
};
}

#endif // MU_PROJECT_PROJECTCONFIGURATION_H
