/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-CLA-applies
 *
 * MuseScore
 * Music Composition & Notation
 *
 * Copyright (C) 2023 MuseScore BVBA and others
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
#include "projectconfigurationstub.h"

using namespace mu;
using namespace mu::project;

io::paths_t ProjectConfigurationStub::recentProjectPaths() const
{
    return io::paths_t();
}

void ProjectConfigurationStub::setRecentProjectPaths(const io::paths_t&)
{
}

async::Channel<io::paths_t> ProjectConfigurationStub::recentProjectPathsChanged() const
{
    static async::Channel<io::paths_t> ch;
    return ch;
}

io::path_t ProjectConfigurationStub::myFirstProjectPath() const
{
    return io::path_t();
}

io::paths_t ProjectConfigurationStub::availableTemplateDirs() const
{
    return io::paths_t();
}

io::path_t ProjectConfigurationStub::templateCategoriesJsonPath(const io::path_t&) const
{
    return io::path_t();
}

io::path_t ProjectConfigurationStub::userTemplatesPath() const
{
    return io::path_t();
}

void ProjectConfigurationStub::setUserTemplatesPath(const io::path_t&)
{
}

async::Channel<io::path_t> ProjectConfigurationStub::userTemplatesPathChanged() const
{
    static async::Channel<io::path_t> ch;
    return ch;
}

io::path_t ProjectConfigurationStub::lastOpenedProjectsPath() const
{
    return io::path_t();
}

void ProjectConfigurationStub::setLastOpenedProjectsPath(const io::path_t&)
{
}

io::path_t ProjectConfigurationStub::lastSavedProjectsPath() const
{
    return io::path_t();
}

void ProjectConfigurationStub::setLastSavedProjectsPath(const io::path_t&)
{
}

io::path_t ProjectConfigurationStub::userProjectsPath() const
{
    return io::path_t();
}

void ProjectConfigurationStub::setUserProjectsPath(const io::path_t&)
{
}

async::Channel<io::path_t> ProjectConfigurationStub::userProjectsPathChanged() const
{
    static async::Channel<io::path_t> ch;
    return ch;
}

io::path_t ProjectConfigurationStub::defaultUserProjectsPath() const
{
    return io::path_t();
}

bool ProjectConfigurationStub::shouldAskSaveLocationType() const
{
    return false;
}

void ProjectConfigurationStub::setShouldAskSaveLocationType(bool)
{
}

bool ProjectConfigurationStub::isCloudProject(const io::path_t&) const
{
    return false;
}

io::path_t ProjectConfigurationStub::cloudProjectSavingFilePath(const io::path_t&) const
{
    return io::path_t();
}

io::path_t ProjectConfigurationStub::defaultSavingFilePath(INotationProjectPtr, const std::string&, const std::string&) const
{
    return io::path_t();
}

SaveLocationType ProjectConfigurationStub::lastUsedSaveLocationType() const
{
    return SaveLocationType::Undefined;
}

void ProjectConfigurationStub::setLastUsedSaveLocationType(SaveLocationType)
{
}

bool ProjectConfigurationStub::shouldWarnBeforePublish() const
{
    return false;
}

void ProjectConfigurationStub::setShouldWarnBeforePublish(bool)
{
}

bool ProjectConfigurationStub::shouldWarnBeforeSavingPubliclyToCloud() const
{
    return false;
}

void ProjectConfigurationStub::setShouldWarnBeforeSavingPubliclyToCloud(bool)
{
}

QColor ProjectConfigurationStub::templatePreviewBackgroundColor() const
{
    return QColor();
}

async::Notification ProjectConfigurationStub::templatePreviewBackgroundChanged() const
{
    static async::Notification n;
    return n;
}

ProjectConfigurationStub::PreferredScoreCreationMode ProjectConfigurationStub::preferredScoreCreationMode() const
{
    return PreferredScoreCreationMode::FromInstruments;
}

void ProjectConfigurationStub::setPreferredScoreCreationMode(PreferredScoreCreationMode)
{
}

MigrationOptions ProjectConfigurationStub::migrationOptions(MigrationType) const
{
    return MigrationOptions();
}

void ProjectConfigurationStub::setMigrationOptions(MigrationType, const MigrationOptions&, bool)
{
}

bool ProjectConfigurationStub::isAutoSaveEnabled() const
{
    return false;
}

void ProjectConfigurationStub::setAutoSaveEnabled(bool)
{
}

async::Channel<bool> ProjectConfigurationStub::autoSaveEnabledChanged() const
{
    static async::Channel<bool> ch;
    return ch;
}

int ProjectConfigurationStub::autoSaveIntervalMinutes() const
{
    return 1;
}

void ProjectConfigurationStub::setAutoSaveInterval(int)
{
}

async::Channel<int> ProjectConfigurationStub::autoSaveIntervalChanged() const
{
    static async::Channel<int> ch;
    return ch;
}

io::path_t ProjectConfigurationStub::newProjectTemporaryPath() const
{
    return io::path_t();
}

bool ProjectConfigurationStub::isAccessibleEnabled() const
{
    return false;
}

bool ProjectConfigurationStub::shouldDestinationFolderBeOpenedOnExport() const
{
    return false;
}

void ProjectConfigurationStub::setShouldDestinationFolderBeOpenedOnExport(bool)
{
}

QUrl ProjectConfigurationStub::scoreManagerUrl() const
{
    return QUrl();
}

QUrl ProjectConfigurationStub::supportForumUrl() const
{
    return QUrl();
}

bool ProjectConfigurationStub::openDetailedProjectUploadedDialog() const
{
    return false;
}

void ProjectConfigurationStub::setOpenDetailedProjectUploadedDialog(bool)
{
}

bool ProjectConfigurationStub::hasAskedAudioGenerationSettings() const
{
    return false;
}

void ProjectConfigurationStub::setHasAskedAudioGenerationSettings(bool)
{
}

GenerateAudioTimePeriodType ProjectConfigurationStub::generateAudioTimePeriodType() const
{
    return GenerateAudioTimePeriodType::Never;
}

void ProjectConfigurationStub::setGenerateAudioTimePeriodType(GenerateAudioTimePeriodType)
{
}

int ProjectConfigurationStub::numberOfSavesToGenerateAudio() const
{
    return 1;
}

void ProjectConfigurationStub::setNumberOfSavesToGenerateAudio(int)
{
}

io::path_t ProjectConfigurationStub::temporaryMp3FilePathTemplate() const
{
    return io::path_t();
}

io::path_t ProjectConfigurationStub::projectBackupPath(const io::path_t&) const
{
    return io::path_t();
}

bool ProjectConfigurationStub::showCloudIsNotAvailableWarning() const
{
    return false;
}

void ProjectConfigurationStub::setShowCloudIsNotAvailableWarning(bool)
{
}
