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
#include "async/promise.h"
#include "interactive/iinteractive.h"
#include "interactive/iplatforminteractive.h"
#include "context/iglobalcontext.h"
#include "cloud/musescorecom/imusescorecomservice.h"
#include "cloud/audiocom/iaudiocomservice.h"
#include "io/ifilesystem.h"
#include "notation/inotationconfiguration.h"
#include "progress.h"

#include "iexportprojectscenario.h"
#include "iprojectconfiguration.h"
#include "irecentfilescontroller.h"
#include "iopenprojectscenario.h"

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
    muse::ContextInject<IExportProjectScenario> exportProjectScenario = { this };
    muse::ContextInject<muse::IInteractive> interactive = { this };
    muse::ContextInject<context::IGlobalContext> globalContext = { this };
    muse::ContextInject<IOpenProjectScenario> openProjectScenario = { this };

    SaveProjectScenario(const muse::modularity::ContextPtr& iocCtx)
        : muse::Contextable(iocCtx) {}

    muse::async::Promise<muse::Ret> saveProject(SaveMode saveMode, SaveLocationType saveLocationType = SaveLocationType::Undefined,
                                                bool force = false) override;
    muse::async::Promise<muse::Ret> saveProject(const muse::io::path_t& path = muse::io::path_t()) override;
    muse::async::Promise<muse::Ret> saveProjectAt(const muse::rcommand::Params& params) override;

    muse::async::Promise<muse::Ret> publish() override;
    muse::async::Promise<muse::Ret> shareAudio() override;

    bool isBusy(BusyStatus status) const override;
    muse::async::Notification busyChanged() const override;

private:
    static constexpr int RET_CODE_CHANGE_SAVE_LOCATION_TYPE = 1234;
    static constexpr int RET_CODE_CONFLICT_RESPONSE_SAVE_AS = 1235;
    static constexpr int RET_CODE_CONFLICT_RESPONSE_PUBLISH_AS_NEW_SCORE = 1236;
    static constexpr int RET_CODE_CONFLICT_RESPONSE_REPLACE = 1237;

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
    muse::async::Promise<muse::Ret> runIfNotBusy(BusyStatus status, const std::function<muse::async::Promise<muse::Ret>()>& flow);

    muse::async::Promise<muse::RetVal<muse::Val> > openDialog(const muse::UriQuery& query) const;

    muse::async::Promise<bool> refuseSaveAfter(muse::async::Promise<muse::Ret> shown);

    muse::async::Promise<muse::RetVal<SaveLocation> > askSaveLocation(INotationProjectPtr project, SaveMode mode,
                                                                      SaveLocationType preselectedType = SaveLocationType::Undefined) const;
    muse::async::Promise<muse::RetVal<SaveLocation> > askSaveLocationOfType(INotationProjectPtr project, SaveMode mode,
                                                                            SaveLocationType type) const;
    muse::async::Promise<muse::RetVal<muse::io::path_t> > askLocalPath(INotationProjectPtr project, SaveMode mode) const;
    muse::async::Promise<muse::Ret> saveProjectLocallyInstead(const INotationProjectPtr& project, SaveMode saveMode);
    muse::async::Promise<muse::RetVal<SaveLocationType> > saveLocationType() const;
    muse::async::Promise<muse::RetVal<SaveLocationType> > askSaveLocationType() const;
    muse::async::Promise<muse::RetVal<CloudProjectInfo> > askCloudLocation(INotationProjectPtr project, SaveMode mode) const;
    muse::async::Promise<muse::RetVal<CloudProjectInfo> > askPublishLocation(INotationProjectPtr project) const;
    muse::async::Promise<muse::RetVal<CloudAudioInfo> > askShareAudioLocation(INotationProjectPtr project) const;
    muse::async::Promise<muse::RetVal<CloudAudioInfo> > doAskShareAudioLocation(INotationProjectPtr project) const;
    muse::async::Promise<muse::RetVal<CloudProjectInfo> > doAskCloudLocation(INotationProjectPtr project, SaveMode mode,
                                                                             bool isPublishShare) const;
    muse::async::Promise<muse::RetVal<CloudProjectInfo> > doAskCloudLocationAuthorized(INotationProjectPtr project, SaveMode mode,
                                                                                       bool isPublishShare) const;
    muse::async::Promise<muse::RetVal<CloudProjectInfo> > askCloudProjectInfo(INotationProjectPtr project, SaveMode mode,
                                                                              bool isPublishShare) const;
    muse::async::Promise<muse::RetVal<CloudProjectInfo> > askCloudProjectInfo(INotationProjectPtr project, SaveMode mode,
                                                                              bool isPublishShare, const QString& defaultName,
                                                                              muse::cloud::Visibility defaultVisibility,
                                                                              const QUrl& existingScoreUrl) const;
    muse::async::Promise<bool> warnBeforePublishing(bool isPublishShare, muse::cloud::Visibility visibility) const;
    muse::async::Promise<bool> warnBeforeSavingToExistingPubliclyVisibleCloudProject() const;
    muse::async::Promise<muse::Ret> warnCloudNotAvailableForUploading(bool isPublishShare) const;
    muse::async::Promise<muse::Ret> warnCloudNotAvailableForSharingAudio() const;
    muse::async::Promise<muse::RetVal<muse::Val> > ensureAuthorization(const QString& cloudCode, bool publishingScore,
                                                                       const std::string& text) const;
    muse::async::Promise<muse::Ret> showCloudSaveError(const muse::Ret& ret, const CloudProjectInfo& info, bool isPublishShare,
                                                       bool alreadyAttempted) const;
    muse::async::Promise<muse::Ret> showAudioCloudShareError(const muse::Ret& ret) const;

    muse::Ret canSaveProject() const;
    muse::async::Promise<muse::Ret> saveProjectAt(const SaveLocation& saveLocation, SaveMode saveMode = SaveMode::Save, bool force = false);
    muse::async::Promise<muse::Ret> doSaveProjectAt(const SaveLocation& saveLocation, SaveMode saveMode);
    muse::async::Promise<muse::Ret> saveProjectToCloud(CloudProjectInfo info, SaveMode saveMode = SaveMode::Save);
    muse::async::Promise<muse::Ret> doSaveProjectToCloud(const CloudProjectInfo& info, SaveMode saveMode);
    muse::async::Promise<muse::Ret> saveAndUploadProject(const INotationProjectPtr& project, CloudProjectInfo info, SaveMode saveMode);
    muse::async::Promise<muse::Ret> doSaveAndUploadProject(const INotationProjectPtr& project, const CloudProjectInfo& info,
                                                           SaveMode saveMode, bool isPublic);
    muse::async::Promise<muse::Ret> saveProjectLocally(const muse::io::path_t& path, SaveMode saveMode = SaveMode::Save,
                                                       bool createBackup = true);
    muse::async::Promise<muse::Ret> saveCloudProjectLocally(const INotationProjectPtr& project, const CloudProjectInfo& info,
                                                            SaveMode saveMode);

    muse::async::Promise<muse::Ret> shareAudio(const AudioFile& existingAudio);
    muse::async::Promise<muse::Ret> uploadAudioToAudioCom(const AudioFile& audio, const INotationProjectPtr& project,
                                                          const CloudAudioInfo& info);
    muse::async::Promise<muse::Ret> alsoShareAudioCom(const AudioFile& audio);

    muse::async::Promise<muse::Ret> askAudioGenerationSettings() const;
    muse::async::Promise<muse::RetVal<bool> > needGenerateAudio(bool isPublic) const;
    bool needGenerateAudioAccordingToSettings() const;
    AudioFile exportMp3(const notation::INotationPtr notation) const;

    void showUploadProgressDialog();
    void closeUploadProgressDialog();

    muse::async::Promise<muse::Ret> uploadProject(const CloudProjectInfo& info, const AudioFile& audio, bool openEditUrl, bool publishMode);
    muse::async::Promise<muse::Ret> uploadAudioToMuseScoreCom(const AudioFile& audio, const QUrl& sourceUrl, const QUrl& urlToOpen,
                                                              bool isFirstSave, bool publishMode);

    muse::async::Promise<muse::Ret> onUploadFinished(const QUrl& urlToOpen, bool isFirstSave, const AudioFile& audio, bool publishMode);
    muse::async::Promise<muse::Ret> onProjectSuccessfullyUploaded(const QUrl& urlToOpen = QUrl(), bool isFirstSave = true);
    muse::async::Promise<muse::Ret> onProjectUploadFailed(const muse::Ret& ret, const CloudProjectInfo& info, const AudioFile& audio,
                                                          bool openEditUrl, bool publishMode);

    void onAudioSuccessfullyUploaded(const QUrl& urlToOpen);
    muse::async::Promise<muse::Ret> onAudioUploadFailed(const muse::Ret& ret);

    muse::async::Promise<muse::Ret> warnCloudIsNotAvailable();

    muse::async::Promise<bool> askIfUserAgreesToSaveProjectWithErrors(const muse::Ret& ret, const SaveLocation& location);
    muse::async::Promise<bool> askIfUserAgreesToSaveCorruptedScore(const SaveLocation& location, const std::string& errorText,
                                                                   bool newlyCreated);
    muse::async::Promise<muse::Ret> warnCorruptedScoreCannotBeSavedOnCloud(const std::string& errorText, bool canRevert);
    muse::async::Promise<bool> askIfUserAgreesToSaveCorruptedScoreLocally(const std::string& errorText, bool canRevert);
    muse::async::Promise<bool> askIfUserAgreesToSaveCorruptedScoreUponOpenning(const SaveLocation& location, const std::string& errorText);
    muse::async::Promise<muse::Ret> showErrCorruptedScoreCannotBeSaved(const SaveLocation& location, const std::string& errorText);

    muse::async::Promise<muse::Ret> warnScoreCouldnotBeSaved(const muse::Ret& ret);
    muse::async::Promise<muse::Ret> warnScoreCouldnotBeSaved(const std::string& errorText);
    muse::async::Promise<int> warnScoreHasBecomeCorruptedAfterSave(const muse::Ret& ret);

    muse::async::Promise<muse::Ret> askToRevertCorruptedScoreToLastSaved();

    RecentFile makeRecentFile(INotationProjectPtr project);
    void moveProject(INotationProjectPtr project, const muse::io::path_t& newPath, bool replace);

    QUrl scoreManagerUrl() const;

    std::set<BusyStatus> m_busyStatuses;
    muse::async::Notification m_busyChanged;

    muse::ProgressPtr m_uploadingProjectProgress = nullptr;
    muse::ProgressPtr m_uploadingAudioProgress = nullptr;

    int m_numberOfSavesToCloud = 0;
};
}
