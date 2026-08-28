/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2026 MuseScore Limited and others
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

#pragma once

#include "isaveprojectscenario.h"

#include <QIODevice>
#include <QString>
#include <QUrl>

#include "modularity/ioc.h"
#include "async/asyncable.h"
#include "interactive/iinteractive.h"
#include "interactive/iplatforminteractive.h"
#include "context/iglobalcontext.h"
#include "cloud/musescorecom/imusescorecomservice.h"
#include "cloud/audiocom/iaudiocomservice.h"
#include "io/ifilesystem.h"
#include "notation/inotationconfiguration.h"
#include "progress.h"

#include "iexportprojectscenario.h"
#include "iopensaveprojectscenario.h"
#include "iprojectconfiguration.h"
#include "irecentfilescontroller.h"

namespace mu::project {
class SaveProjectScenario : public ISaveProjectScenario, public muse::Contextable, public muse::async::Asyncable
{
    friend class SaveProjectScenarioTests;

public:
    muse::GlobalInject<IProjectConfiguration> configuration;
    muse::GlobalInject<notation::INotationConfiguration> notationConfiguration;
    muse::GlobalInject<muse::io::IFileSystem> fileSystem;
    muse::GlobalInject<muse::cloud::IMuseScoreComService> museScoreComService;
    muse::GlobalInject<muse::cloud::IAudioComService> audioComService;
    muse::GlobalInject<muse::IPlatformInteractive> platformInteractive;
    muse::ContextInject<IRecentFilesController> recentFilesController = { this };
    muse::ContextInject<IOpenSaveProjectScenario> openSaveProjectScenario = { this };
    muse::ContextInject<IExportProjectScenario> exportProjectScenario = { this };
    muse::ContextInject<muse::IInteractive> interactive = { this };
    muse::ContextInject<context::IGlobalContext> globalContext = { this };

    SaveProjectScenario(const muse::modularity::ContextPtr& iocCtx)
        : muse::Contextable(iocCtx) {}

    muse::Ret saveProject(SaveMode saveMode, SaveLocationType saveLocationType = SaveLocationType::Undefined,
                          bool force = false) override;
    bool saveProject(const muse::io::path_t& path = muse::io::path_t()) override;
    bool saveProjectLocally(const muse::io::path_t& path, SaveMode saveMode = SaveMode::Save, bool createBackup = true) override;
    muse::Ret saveProjectAt(const muse::rcommand::Params& params) override;

    muse::Ret publish() override;
    muse::Ret shareAudio() override;

    bool isBusy(IProjectCommandsController::BusyStatus status) const override;
    muse::async::Notification busyChanged() const override;

    muse::async::Notification revertToLastSavedRequested() const override;

private:
    using BusyStatus = IProjectCommandsController::BusyStatus;

    struct AudioFile {
        QString format;
        std::shared_ptr<QIODevice> device = nullptr;

        AudioFile() {}

        bool isValid() const
        {
            return !format.isEmpty() && device != nullptr;
        }
    };

    INotationProjectPtr currentNotationProject() const;
    notation::INotationInteractionPtr currentInteraction() const;

    void setBusy(BusyStatus status, bool isBusy);

    muse::Ret canSaveProject() const;
    muse::Ret saveProjectAt(const SaveLocation& saveLocation, SaveMode saveMode = SaveMode::Save, bool force = false);
    bool saveProjectToCloud(CloudProjectInfo info, SaveMode saveMode = SaveMode::Save);

    muse::Ret shareAudio(const AudioFile& existingAudio);
    void uploadAudioToAudioCom(const AudioFile& audio, const INotationProjectPtr& project, const CloudAudioInfo& info);
    void alsoShareAudioCom(const AudioFile& audio);

    muse::Ret askAudioGenerationSettings() const;
    muse::RetVal<bool> needGenerateAudio(bool isPublic) const;
    AudioFile exportMp3(const notation::INotationPtr notation) const;

    void showUploadProgressDialog();
    void closeUploadProgressDialog();

    muse::Ret uploadProject(const CloudProjectInfo& info, const AudioFile& audio, bool openEditUrl, bool publishMode);
    void uploadAudioToMuseScoreCom(const AudioFile& audio, const QUrl& sourceUrl, const QUrl& urlToOpen, bool isFirstSave,
                                   bool publishMode);

    void onProjectSuccessfullyUploaded(const QUrl& urlToOpen = QUrl(), bool isFirstSave = true);
    muse::Ret onProjectUploadFailed(const muse::Ret& ret, const CloudProjectInfo& info, const AudioFile& audio, bool openEditUrl,
                                    bool publishMode);

    void onAudioSuccessfullyUploaded(const QUrl& urlToOpen);
    void onAudioUploadFailed(const muse::Ret& ret);

    void warnCloudIsNotAvailable();

    bool askIfUserAgreesToSaveProjectWithErrors(const muse::Ret& ret, const SaveLocation& location);
    bool askIfUserAgreesToSaveCorruptedScore(const SaveLocation& location, const std::string& errorText, bool newlyCreated);
    void warnCorruptedScoreCannotBeSavedOnCloud(const std::string& errorText, bool canRevert);
    bool askIfUserAgreesToSaveCorruptedScoreLocally(const std::string& errorText, bool canRevert);
    bool askIfUserAgreesToSaveCorruptedScoreUponOpenning(const SaveLocation& location, const std::string& errorText);
    void showErrCorruptedScoreCannotBeSaved(const SaveLocation& location, const std::string& errorText);

    void warnScoreCouldnotBeSaved(const muse::Ret& ret);
    void warnScoreCouldnotBeSaved(const std::string& errorText);
    int warnScoreHasBecomeCorruptedAfterSave(const muse::Ret& ret);

    void askToRevertCorruptedScoreToLastSaved();

    RecentFile makeRecentFile(INotationProjectPtr project);
    void moveProject(INotationProjectPtr project, const muse::io::path_t& newPath, bool replace);

    QUrl scoreManagerUrl() const;

    std::set<BusyStatus> m_busyStatuses;
    muse::async::Notification m_busyChanged;

    muse::ProgressPtr m_uploadingProjectProgress = nullptr;
    muse::ProgressPtr m_uploadingAudioProgress = nullptr;

    int m_numberOfSavesToCloud = 0;

    muse::async::Notification m_revertToLastSavedRequested;
};
}
