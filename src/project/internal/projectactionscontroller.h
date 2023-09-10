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
#include "cloud/musescorecom/imusescorecomservice.h"
#include "cloud/audiocom/iaudiocomservice.h"
#include "playback/iplaybackcontroller.h"
#include "print/iprintprovider.h"
#include "inotationreadersregister.h"
#include "isaveprojectscenario.h"
#include "io/ifilesystem.h"
#include "internal/iexportprojectscenario.h"
#include "notation/inotationconfiguration.h"

#include "async/asyncable.h"

#include "iprojectconfiguration.h"
#include "iprojectcreator.h"
#include "irecentfilescontroller.h"
#include "iprojectautosaver.h"

namespace mu::project {
class ProjectActionsController : public IProjectFilesController, public QObject, public actions::Actionable, public async::Asyncable
{
    INJECT(IProjectConfiguration, configuration)
    INJECT(INotationReadersRegister, readers)
    INJECT(IProjectCreator, projectCreator)
    INJECT(IRecentFilesController, recentFilesController)
    INJECT(IProjectAutoSaver, projectAutoSaver)
    INJECT(ISaveProjectScenario, saveProjectScenario)
    INJECT(IExportProjectScenario, exportProjectScenario)
    INJECT(actions::IActionsDispatcher, dispatcher)
    INJECT(framework::IInteractive, interactive)
    INJECT(context::IGlobalContext, globalContext)
    INJECT(mi::IMultiInstancesProvider, multiInstancesProvider)
    INJECT(cloud::IMuseScoreComService, museScoreComService)
    INJECT(cloud::IAudioComService, audioComService)
    INJECT(notation::INotationConfiguration, notationConfiguration)
    INJECT(playback::IPlaybackController, playbackController)
    INJECT(print::IPrintProvider, printProvider)
    INJECT(io::IFileSystem, fileSystem)

public:
    void init();

    bool canReceiveAction(const actions::ActionCode& code) const override;

    bool isFileSupported(const io::path_t& path) const override;
    Ret openProject(const io::path_t& path) override;
    Ret openProject(const ProjectFile& file) override;
    bool closeOpenedProject(bool quitApp = false) override;
    bool isProjectOpened(const io::path_t& scorePath) const override;
    bool isAnyProjectOpened() const override;
    bool saveProject(const io::path_t& path = io::path_t()) override;

    const ProjectBeingDownloaded& projectBeingDownloaded() const override;
    async::Notification projectBeingDownloadedChanged() const override;

private:
    void setupConnections();

    project::INotationProjectPtr currentNotationProject() const;
    notation::IMasterNotationPtr currentMasterNotation() const;
    notation::INotationPtr currentNotation() const;
    notation::INotationInteractionPtr currentInteraction() const;
    notation::INotationSelectionPtr currentNotationSelection() const;

    void newProject();

    void openProject(const actions::ActionData& args);
    void doOpenProject(const io::path_t& path, int scoreId);
    void downloadAndOpenCloudProject(int scoreId);

    void showScoreDownloadError(const Ret& ret);

    bool checkCanIgnoreError(const Ret& ret, const io::path_t& filepath);
    bool askIfUserAgreesToOpenProjectWithIncompatibleVersion(const std::string& errorText);
    void warnFileTooNew(const io::path_t& filepath);
    bool askIfUserAgreesToOpenCorruptedProject(const String& projectName, const std::string& errorText);
    void warnProjectCriticallyCorrupted(const String& projectName, const std::string& errorText);
    void warnProjectCannotBeOpened(const Ret& ret, const io::path_t& filepath);

    framework::IInteractive::Button askAboutSavingScore(INotationProjectPtr project);

    Ret canSaveProject() const;
    bool saveProject(SaveMode saveMode, SaveLocationType saveLocationType = SaveLocationType::Undefined, bool force = false);

    void publish();
    void shareAudio();

    bool saveProjectAt(const SaveLocation& saveLocation, SaveMode saveMode = SaveMode::Save, bool force = false);
    bool saveProjectLocally(const io::path_t& path = io::path_t(), SaveMode saveMode = SaveMode::Save);
    bool saveProjectToCloud(CloudProjectInfo info, SaveMode saveMode = SaveMode::Save);

    struct AudioFile {
        QString format;
        QIODevice* device = nullptr;

        AudioFile() {}

        bool isValid() const
        {
            return !format.isEmpty() && device != nullptr;
        }
    };

    Ret askAudioGenerationSettings() const;
    RetVal<bool> needGenerateAudio(bool isPublic) const;
    AudioFile exportMp3(const notation::INotationPtr notation) const;

    void showUploadProgressDialog();
    void closeUploadProgressDialog();

    Ret uploadProject(const CloudProjectInfo& info, const AudioFile& audio, bool openEditUrl, bool publishMode);
    void uploadAudio(const AudioFile& audio, const QUrl& sourceUrl, const QUrl& urlToOpen, bool isFirstSave);

    void onProjectSuccessfullyUploaded(const QUrl& urlToOpen = QUrl(), bool isFirstSave = true);
    Ret onProjectUploadFailed(const Ret& ret, const CloudProjectInfo& info, const AudioFile& audio, bool openEditUrl, bool publishMode);

    void onAudioSuccessfullyUploaded(const QUrl& urlToOpen);
    void onAudioUploadFailed(const Ret& ret);

    void warnCloudIsNotAvailable();

    bool askIfUserAgreesToSaveProjectWithErrors(const Ret& ret, const SaveLocation& location);
    void warnScoreWithoutPartsCannotBeSaved();
    bool askIfUserAgreesToSaveCorruptedScore(const SaveLocation& location, const std::string& errorText, bool newlyCreated);
    void warnCorruptedScoreCannotBeSavedOnCloud(const std::string& errorText, bool canRevert);
    bool askIfUserAgreesToSaveCorruptedScoreLocally(const std::string& errorText, bool canRevert);
    bool askIfUserAgreesToSaveCorruptedScoreUponOpenning(const SaveLocation& location, const std::string& errorText);
    void showErrCorruptedScoreCannotBeSaved(const SaveLocation& location, const std::string& errorText);

    void warnScoreCouldnotBeSaved(const Ret& ret);
    void warnScoreCouldnotBeSaved(const std::string& errorText);

    void revertCorruptedScoreToLastSaved();

    ProjectFile makeRecentFile(INotationProjectPtr project);

    void moveProject(INotationProjectPtr project, const io::path_t& newPath, bool replace);

    void importPdf();

    void clearRecentScores();

    void continueLastSession();

    void openProjectProperties();

    io::path_t selectScoreOpeningFile();
    io::path_t selectScoreSavingFile(const io::path_t& defaultFilePath, const QString& saveTitle);

    RetVal<INotationProjectPtr> loadProject(const io::path_t& filePath);
    Ret doOpenProject(const io::path_t& filePath);
    Ret doOpenCloudProject(const io::path_t& filePath, const CloudProjectInfo& info);

    Ret openPageIfNeed(Uri pageUri);

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

    framework::ProgressPtr m_uploadingProjectProgress = nullptr;
    framework::ProgressPtr m_uploadingAudioProgress = nullptr;

    int m_numberOfSavesToCloud = 0;

    ProjectBeingDownloaded m_projectBeingDownloaded;
    async::Notification m_projectBeingDownloadedChanged;
};
}

#endif // MU_PROJECT_PROJECTACTIONSCONTROLLER_H
