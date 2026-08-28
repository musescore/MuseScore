/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2021 MuseScore Limited and others
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

#include "iprojectfilescontroller.h"

#include <QObject>
#include <QString>

#include "iprojectcommandscontroller.h"

#include "modularity/ioc.h"
#include "interactive/iinteractive.h"
#include "interactive/iplatforminteractive.h"
#include "context/iglobalcontext.h"
#include "actions/actionable.h"
#include "actions/iactionsdispatcher.h"
#include "rcommand/commandtypes.h"
#include "rcommand/icommanddispatcher.h"
#include "rcommand/commandable.h"
#include "rcommand/icommanddispatcher.h"
#include "multiwindows/imultiwindowsprovider.h"
#include "multiwindows/iprojectprovider.h"
#include "cloud/musescorecom/imusescorecomservice.h"
#include "cloud/audiocom/iaudiocomservice.h"
#include "print/iprintprovider.h"
#include "iexportprojectscenario.h"
#include "inotationreadersregister.h"
#include "iopensaveprojectscenario.h"
#include "isaveprojectscenario.h"
#include "imscmetareader.h"
#include "io/ifilesystem.h"
#include "notation/inotationconfiguration.h"
#include "musesounds/imusesoundscheckupdatescenario.h"
#include "musesounds/imusesamplercheckupdatescenario.h"
#include "extensions/iextensionsprovider.h"

#include "async/asyncable.h"

#include "iprojectconfiguration.h"
#include "iprojectcreator.h"
#include "irecentfilescontroller.h"
#include "iprojectautosaver.h"

namespace mu::project {
class ProjectActionsController : public IProjectCommandsController, public IProjectFilesController, public muse::mi::IProjectProvider,
    public muse::Contextable, public muse::actions::Actionable, public muse::async::Asyncable, public muse::rcommand::Commandable
{
    friend class ProjectActionsControllerTests;

public:
    muse::GlobalInject<IProjectConfiguration> configuration;
    muse::GlobalInject<muse::mi::IMultiWindowsProvider> multiwindowsProvider;
    muse::GlobalInject<notation::INotationConfiguration> notationConfiguration;
    muse::GlobalInject<muse::io::IFileSystem> fileSystem;
    muse::GlobalInject<IMscMetaReader> mscMetaReader;
    muse::GlobalInject<IProjectCreator> projectCreator;
    muse::GlobalInject<muse::cloud::IMuseScoreComService> museScoreComService;
    muse::GlobalInject<muse::cloud::IAudioComService> audioComService;
    muse::GlobalInject<INotationReadersRegister> readers;
    muse::GlobalInject<muse::IPlatformInteractive> platformInteractive;
    muse::ContextInject<musesounds::IMuseSoundsCheckUpdateScenario> museSoundsCheckUpdateScenario = { this };
    muse::ContextInject<musesounds::IMuseSamplerCheckUpdateScenario> museSamplerCheckUpdateScenario = { this };
    muse::ContextInject<IRecentFilesController> recentFilesController = { this };
    muse::ContextInject<IProjectAutoSaver> projectAutoSaver = { this };
    muse::ContextInject<IOpenSaveProjectScenario> openSaveProjectScenario = { this };
    muse::ContextInject<ISaveProjectScenario> saveProjectScenario = { this };
    muse::ContextInject<IExportProjectScenario> exportProjectScenario = { this };
    muse::ContextInject<muse::actions::IActionsDispatcher> dispatcher = { this };
    muse::ContextInject<muse::rcommand::ICommandDispatcher> commandDispatcher = { this };
    muse::ContextInject<muse::IInteractive> interactive = { this };
    muse::ContextInject<context::IGlobalContext> globalContext = { this };
    muse::ContextInject<print::IPrintProvider> printProvider = { this };

    ProjectActionsController(const muse::modularity::ContextPtr& iocCtx)
        : muse::Contextable(iocCtx) {}

    void init();

    // IProjectCommandsController
    bool hasProject() const override;
    muse::async::Notification hasProjectChanged() const override;
    bool needSave() const override;
    muse::async::Notification needSaveChanged() const override;
    bool isBusy(BusyStatus status) const override;
    muse::async::Notification busyChanged() const override;
    bool hasSelection() const override;
    muse::async::Notification hasSelectionChanged() const override;
    // --------

    bool canReceiveAction(const muse::actions::ActionCode& code) const override;

    bool isUrlSupported(const QUrl& url) const override;
    bool isFileSupported(const muse::io::path_t& path) const override;
    muse::Ret openProject(const ProjectFile& file) override;
    bool closeOpenedProject(bool goToHome = true) override;
    bool saveProject(const muse::io::path_t& path = muse::io::path_t()) override;
    bool saveProjectLocally(const muse::io::path_t& path = muse::io::path_t(), SaveMode saveMode = SaveMode::Save,
                            bool createBackup = true) override;

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

    muse::Ret newProject();

    muse::Ret openProject(const muse::rcommand::Params& params);
    muse::Ret openProject(const muse::io::path_t& path, const QString& displayNameOverride = QString());
    void downloadAndOpenCloudProject(int scoreId, const QString& hash = QString(), const QString& secret = QString(), bool isOwner = true);
    muse::Ret openMuseScoreUrl(const QUrl& url);
    muse::Ret openScoreFromMuseScoreCom(const QUrl& url);

    muse::Ret closeProject();

    bool shouldRetryLoadAfterError(const muse::Ret& ret, const muse::io::path_t& filepath);
    bool askIfUserAgreesToOpenProjectWithIncompatibleVersion(const std::string& errorText);
    void warnFileTooNew(const muse::io::path_t& filepath);
    bool askIfUserAgreesToOpenCorruptedProject(const muse::String& projectName, const std::string& errorText);
    void warnProjectCriticallyCorrupted(const muse::String& projectName, const std::string& errorText);
    void warnProjectCannotBeOpened(const muse::Ret& ret, const muse::io::path_t& filepath);

    muse::IInteractive::Button askAboutSavingScore(INotationProjectPtr project);

    muse::Ret saveProject(SaveMode saveMode, SaveLocationType saveLocationType = SaveLocationType::Undefined, bool force = false);
    muse::Ret publish();
    muse::Ret sharedAudio();
    muse::Ret saveProjectAt(const muse::rcommand::Params& params);

    void revertCorruptedScoreToLastSaved();

    RecentFile makeRecentFile(INotationProjectPtr project);

    muse::Ret importPdf();
    muse::Ret importAudioToScore();

    muse::Ret clearRecentScores();

    muse::Ret continueLastSession();

    muse::Ret openProjectProperties();

    muse::async::Promise<muse::io::path_t> selectScoreOpeningFile() const;

    muse::RetVal<INotationProjectPtr> loadProject(const muse::io::path_t& filePath);
    muse::Ret loadWithFallback(const std::shared_ptr<INotationProject>& project, const muse::io::path_t& loadPath,
                               const std::string& format);
    muse::Ret doOpenProject(const muse::io::path_t& filePath);
    muse::Ret doOpenCloudProject(const muse::io::path_t& filePath, const CloudProjectInfo& info, bool isOwner = true);
    muse::Ret doOpenCloudProjectOffline(const muse::io::path_t& filePath, const QString& displayNameOverride);

    muse::Ret doFinishOpenProject();
    muse::Ret openPageIfNeed(muse::Uri pageUri);

    muse::Ret exportScore();
    muse::Ret printScore();

    void setBusy(BusyStatus status, bool isBusy);

    std::set<BusyStatus> m_busyStatuses;
    muse::async::Notification m_busyChanged;

    muse::async::Notification m_needSaveChanged;
    muse::async::Notification m_hasSelectionChanged;

    ProjectBeingDownloaded m_projectBeingDownloaded;
    muse::async::Notification m_projectBeingDownloadedChanged;
};
}
