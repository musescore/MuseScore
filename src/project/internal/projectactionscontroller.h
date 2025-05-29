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
#ifndef MU_PROJECT_PROJECTACTIONSCONTROLLER_H
#define MU_PROJECT_PROJECTACTIONSCONTROLLER_H

#include "iprojectfilescontroller.h"

#include <QObject>

#include "modularity/ioc.h"
#include "iinteractive.h"
#include "context/iglobalcontext.h"
#include "actions/actionable.h"
#include "actions/iactionsdispatcher.h"
#include "multiinstances/imultiinstancesprovider.h"
#include "multiinstances/iprojectprovider.h"
#include "cloud/musescorecom/imusescorecomservice.h"
#include "cloud/audiocom/iaudiocomservice.h"
#include "playback/iplaybackcontroller.h"
#include "print/iprintprovider.h"
#include "inotationreadersregister.h"
#include "iopensaveprojectscenario.h"
#include "imscmetareader.h"
#include "io/ifilesystem.h"
#include "internal/iexportprojectscenario.h"
#include "notation/inotationconfiguration.h"
#include "musesounds/imusesoundscheckupdatescenario.h"
#include "extensions/iextensionsprovider.h"
#include "tours/itoursservice.h"

#include "async/asyncable.h"

#include "iprojectconfiguration.h"
#include "iprojectcreator.h"
#include "irecentfilescontroller.h"
#include "iprojectautosaver.h"

namespace mu::project {
class ProjectActionsController : public IProjectFilesController, public muse::mi::IProjectProvider, public muse::Injectable,
    public muse::actions::Actionable, public muse::async::Asyncable
{
    muse::Inject<IProjectConfiguration> configuration = { this };
    muse::Inject<INotationReadersRegister> readers = { this };
    muse::Inject<IProjectCreator> projectCreator = { this };
    muse::Inject<IRecentFilesController> recentFilesController = { this };
    muse::Inject<IProjectAutoSaver> projectAutoSaver = { this };
    muse::Inject<IOpenSaveProjectScenario> openSaveProjectScenario = { this };
    muse::Inject<IExportProjectScenario> exportProjectScenario = { this };
    muse::Inject<IMscMetaReader> mscMetaReader = { this };
    muse::Inject<muse::actions::IActionsDispatcher> dispatcher = { this };
    muse::Inject<muse::IInteractive> interactive = { this };
    muse::Inject<context::IGlobalContext> globalContext = { this };
    muse::Inject<muse::mi::IMultiInstancesProvider> multiInstancesProvider = { this };
    muse::Inject<muse::cloud::IMuseScoreComService> museScoreComService = { this };
    muse::Inject<muse::cloud::IAudioComService> audioComService = { this };
    muse::Inject<notation::INotationConfiguration> notationConfiguration = { this };
    muse::Inject<playback::IPlaybackController> playbackController = { this };
    muse::Inject<print::IPrintProvider> printProvider = { this };
    muse::Inject<muse::io::IFileSystem> fileSystem = { this };
    muse::Inject<musesounds::IMuseSoundsCheckUpdateScenario> museSoundsCheckUpdateScenario = { this };
    muse::Inject<muse::extensions::IExtensionsProvider> extensionsProvider = { this };
    muse::Inject<muse::tours::IToursService> toursService = { this };

public:

    ProjectActionsController(const muse::modularity::ContextPtr& iocCtx)
        : muse::Injectable(iocCtx) {}

    void init();

    bool canReceiveAction(const muse::actions::ActionCode& code) const override;

    bool isUrlSupported(const QUrl& url) const override;
    bool isFileSupported(const muse::io::path_t& path) const override;
    muse::Ret openProject(const ProjectFile& file) override;
    bool closeOpenedProject(bool goToHome = true) override;
    bool saveProject(const muse::io::path_t& path = muse::io::path_t()) override;
    bool saveProjectLocally(
        const muse::io::path_t& path = muse::io::path_t(), SaveMode saveMode = SaveMode::Save, bool createBackup = true) override;

    // mi::IProjectProvider
    bool isProjectOpened(const muse::io::path_t& scorePath) const override;
    bool isAnyProjectOpened() const override;

    const ProjectBeingDownloaded& projectBeingDownloaded() const override;
    muse::async::Notification projectBeingDownloadedChanged() const override;

private:
    void setupConnections();

    project::INotationProjectPtr currentNotationProject() const;
    notation::IMasterNotationPtr currentMasterNotation() const;
    notation::INotationPtr currentNotation() const;
    notation::INotationInteractionPtr currentInteraction() const;
    notation::INotationSelectionPtr currentNotationSelection() const;

    void newProject();

    void openProject(const muse::actions::ActionData& args);
    muse::Ret openProject(const muse::io::path_t& path, const QString& displayNameOverride = QString());
    void downloadAndOpenCloudProject(int scoreId, const QString& hash = QString(), const QString& secret = QString(), bool isOwner = true);
    muse::Ret openMuseScoreUrl(const QUrl& url);
    muse::Ret openScoreFromMuseScoreCom(const QUrl& url);

    bool checkCanIgnoreError(const muse::Ret& ret, const muse::io::path_t& filepath);
    bool askIfUserAgreesToOpenProjectWithIncompatibleVersion(const std::string& errorText);
    void warnFileTooNew(const muse::io::path_t& filepath);
    bool askIfUserAgreesToOpenCorruptedProject(const muse::String& projectName, const std::string& errorText);
    void warnProjectCriticallyCorrupted(const muse::String& projectName, const std::string& errorText);
    void warnProjectCannotBeOpened(const muse::Ret& ret, const muse::io::path_t& filepath);

    muse::IInteractive::Button askAboutSavingScore(INotationProjectPtr project);

    muse::Ret canSaveProject() const;
    bool saveProject(SaveMode saveMode, SaveLocationType saveLocationType = SaveLocationType::Undefined, bool force = false);

    struct AudioFile {
        QString format;
        QIODevice* device = nullptr;

        AudioFile() {}

        bool isValid() const
        {
            return !format.isEmpty() && device != nullptr;
        }
    };

    void publish();
    void shareAudio(const AudioFile& existingAudio);
    void shareAudio() { shareAudio(AudioFile()); }

    bool saveProjectAt(const SaveLocation& saveLocation, SaveMode saveMode = SaveMode::Save, bool force = false);
    bool saveProjectToCloud(CloudProjectInfo info, SaveMode saveMode = SaveMode::Save);

    void alsoShareAudioCom(const AudioFile& audio);

    muse::Ret askAudioGenerationSettings() const;
    muse::RetVal<bool> needGenerateAudio(bool isPublic) const;
    AudioFile exportMp3(const notation::INotationPtr notation) const;

    void showUploadProgressDialog();
    void closeUploadProgressDialog();

    muse::Ret uploadProject(const CloudProjectInfo& info, const AudioFile& audio, bool openEditUrl, bool publishMode);
    void uploadAudio(const AudioFile& audio, const QUrl& sourceUrl, const QUrl& urlToOpen, bool isFirstSave, bool publishMode);

    void onProjectSuccessfullyUploaded(const QUrl& urlToOpen = QUrl(), bool isFirstSave = true);
    muse::Ret onProjectUploadFailed(const muse::Ret& ret, const CloudProjectInfo& info, const AudioFile& audio, bool openEditUrl,
                                    bool publishMode);

    void onAudioSuccessfullyUploaded(const QUrl& urlToOpen);
    void onAudioUploadFailed(const muse::Ret& ret);

    void warnCloudIsNotAvailable();

    bool askIfUserAgreesToSaveProjectWithErrors(const muse::Ret& ret, const SaveLocation& location);
    void warnScoreWithoutPartsCannotBeSaved();
    bool askIfUserAgreesToSaveCorruptedScore(const SaveLocation& location, const std::string& errorText, bool newlyCreated);
    void warnCorruptedScoreCannotBeSavedOnCloud(const std::string& errorText, bool canRevert);
    bool askIfUserAgreesToSaveCorruptedScoreLocally(const std::string& errorText, bool canRevert);
    bool askIfUserAgreesToSaveCorruptedScoreUponOpenning(const SaveLocation& location, const std::string& errorText);
    void showErrCorruptedScoreCannotBeSaved(const SaveLocation& location, const std::string& errorText);

    void warnScoreCouldnotBeSaved(const muse::Ret& ret);
    void warnScoreCouldnotBeSaved(const std::string& errorText);
    int warnScoreHasBecomeCorruptedAfterSave(const muse::Ret& ret);

    void revertCorruptedScoreToLastSaved();

    RecentFile makeRecentFile(INotationProjectPtr project);

    void moveProject(INotationProjectPtr project, const muse::io::path_t& newPath, bool replace);

    void importPdf();

    void clearRecentScores();

    void continueLastSession();

    void openProjectProperties();

    muse::io::path_t selectScoreOpeningFile();
    muse::io::path_t selectScoreSavingFile(const muse::io::path_t& defaultFilePath, const QString& saveTitle);

    muse::RetVal<INotationProjectPtr> loadProject(const muse::io::path_t& filePath);
    muse::Ret doOpenProject(const muse::io::path_t& filePath);
    muse::Ret doOpenCloudProject(const muse::io::path_t& filePath, const CloudProjectInfo& info, bool isOwner = true);
    muse::Ret doOpenCloudProjectOffline(const muse::io::path_t& filePath, const QString& displayNameOverride);

    muse::Ret doFinishOpenProject();
    muse::Ret openPageIfNeed(muse::Uri pageUri);

    void exportScore();
    void printScore();

    bool hasSelection() const;

    QUrl scoreManagerUrl() const;

    bool m_isProjectSaving = false;
    bool m_isProjectClosing = false;
    bool m_isProjectProcessing = false;
    bool m_isProjectPublishing = false;
    bool m_isProjectUploading = false;
    bool m_isAudioSharing = false;
    bool m_isProjectDownloading = false;

    muse::ProgressPtr m_uploadingProjectProgress = nullptr;
    muse::ProgressPtr m_uploadingAudioProgress = nullptr;

    int m_numberOfSavesToCloud = 0;

    ProjectBeingDownloaded m_projectBeingDownloaded;
    muse::async::Notification m_projectBeingDownloadedChanged;
};
}

#endif // MU_PROJECT_PROJECTACTIONSCONTROLLER_H
