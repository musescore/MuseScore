/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2023 MuseScore Limited
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

muse::io::path_t ProjectConfigurationStub::recentFilesJsonPath() const
{
    return muse::io::path_t();
}

ByteArray ProjectConfigurationStub::compatRecentFilesData() const
{
    return ByteArray();
}

muse::io::path_t ProjectConfigurationStub::myFirstProjectPath() const
{
    return muse::io::path_t();
}

io::paths_t ProjectConfigurationStub::availableTemplateDirs() const
{
    return io::paths_t();
}

muse::io::path_t ProjectConfigurationStub::templateCategoriesJsonPath(const muse::io::path_t&) const
{
    return muse::io::path_t();
}

muse::io::path_t ProjectConfigurationStub::userTemplatesPath() const
{
    return muse::io::path_t();
}

void ProjectConfigurationStub::setUserTemplatesPath(const muse::io::path_t&)
{
}

muse::async::Channel<muse::io::path_t> ProjectConfigurationStub::userTemplatesPathChanged() const
{
    static muse::async::Channel<muse::io::path_t> ch;
    return ch;
}

muse::io::path_t ProjectConfigurationStub::lastOpenedProjectsPath() const
{
    return muse::io::path_t();
}

void ProjectConfigurationStub::setLastOpenedProjectsPath(const muse::io::path_t&)
{
}

muse::io::path_t ProjectConfigurationStub::lastSavedProjectsPath() const
{
    return muse::io::path_t();
}

void ProjectConfigurationStub::setLastSavedProjectsPath(const muse::io::path_t&)
{
}

muse::io::path_t ProjectConfigurationStub::userProjectsPath() const
{
    return muse::io::path_t();
}

void ProjectConfigurationStub::setUserProjectsPath(const muse::io::path_t&)
{
}

muse::async::Channel<muse::io::path_t> ProjectConfigurationStub::userProjectsPathChanged() const
{
    static muse::async::Channel<muse::io::path_t> ch;
    return ch;
}

muse::io::path_t ProjectConfigurationStub::defaultUserProjectsPath() const
{
    return muse::io::path_t();
}

bool ProjectConfigurationStub::shouldAskSaveLocationType() const
{
    return false;
}

void ProjectConfigurationStub::setShouldAskSaveLocationType(bool)
{
}

bool ProjectConfigurationStub::isCloudProject(const muse::io::path_t&) const
{
    return false;
}

muse::io::path_t ProjectConfigurationStub::cloudProjectSavingFilePath(const muse::io::path_t&) const
{
    return muse::io::path_t();
}

muse::io::path_t ProjectConfigurationStub::defaultSavingFilePath(INotationProjectPtr, const std::string&, const std::string&) const
{
    return muse::io::path_t();
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

muse::async::Notification ProjectConfigurationStub::templatePreviewBackgroundChanged() const
{
    static muse::async::Notification n;
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

muse::async::Channel<bool> ProjectConfigurationStub::autoSaveEnabledChanged() const
{
    static muse::async::Channel<bool> ch;
    return ch;
}

int ProjectConfigurationStub::autoSaveIntervalMinutes() const
{
    return 1;
}

void ProjectConfigurationStub::setAutoSaveInterval(int)
{
}

muse::async::Channel<int> ProjectConfigurationStub::autoSaveIntervalChanged() const
{
    static muse::async::Channel<int> ch;
    return ch;
}

bool ProjectConfigurationStub::alsoShareAudioCom() const
{
    return false;
}

void ProjectConfigurationStub::setAlsoShareAudioCom(bool share)
{
}

muse::async::Channel<bool> ProjectConfigurationStub::alsoShareAudioComChanged() const
{
    static muse::async::Channel<bool> ch;
    return ch;
}

bool ProjectConfigurationStub::showAlsoShareAudioComDialog() const
{
    return false;
}

void ProjectConfigurationStub::setShowAlsoShareAudioComDialog(bool show)
{
}

bool ProjectConfigurationStub::hasAskedAlsoShareAudioCom() const
{
    return false;
}

void ProjectConfigurationStub::setHasAskedAlsoShareAudioCom(bool has)
{
}

muse::io::path_t ProjectConfigurationStub::newProjectTemporaryPath() const
{
    return muse::io::path_t();
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

muse::io::path_t ProjectConfigurationStub::temporaryMp3FilePathTemplate() const
{
    return muse::io::path_t();
}

muse::io::path_t ProjectConfigurationStub::projectBackupPath(const muse::io::path_t&) const
{
    return muse::io::path_t();
}

bool ProjectConfigurationStub::showCloudIsNotAvailableWarning() const
{
    return false;
}

void ProjectConfigurationStub::setShowCloudIsNotAvailableWarning(bool)
{
}

bool ProjectConfigurationStub::disableVersionChecking() const
{
    return false;
}

void ProjectConfigurationStub::setDisableVersionChecking(bool)
{
}
