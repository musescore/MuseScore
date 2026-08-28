/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2022 MuseScore Limited and others
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

#include "iopenprojectscenario.h"

#include <QString>
#include <QUrl>

#include "modularity/ioc.h"
#include "async/asyncable.h"
#include "context/iglobalcontext.h"
#include "cloud/musescorecom/imusescorecomservice.h"
#include "interactive/iinteractive.h"
#include "interactive/iplatforminteractive.h"
#include "io/ifilesystem.h"
#include "multiwindows/imultiwindowsprovider.h"
#include "musesounds/imusesamplercheckupdatescenario.h"
#include "musesounds/imusesoundscheckupdatescenario.h"

#include "imscmetareader.h"
#include "iprojectautosaver.h"
#include "iprojectconfiguration.h"
#include "iprojectcreator.h"
#include "irecentfilescontroller.h"

namespace mu::project {
class OpenProjectScenario : public IOpenProjectScenario, public muse::Contextable, public muse::async::Asyncable
{
    friend class OpenProjectScenarioTests;

public:
    muse::GlobalInject<IProjectConfiguration> configuration;
    muse::GlobalInject<muse::io::IFileSystem> fileSystem;
    muse::GlobalInject<muse::mi::IMultiWindowsProvider> multiwindowsProvider;
    muse::GlobalInject<muse::cloud::IMuseScoreComService> museScoreComService;
    muse::GlobalInject<IProjectCreator> projectCreator;
    muse::GlobalInject<IMscMetaReader> mscMetaReader;
    muse::GlobalInject<muse::IPlatformInteractive> platformInteractive;
    muse::ContextInject<IProjectAutoSaver> projectAutoSaver = { this };
    muse::ContextInject<IRecentFilesController> recentFilesController = { this };
    muse::ContextInject<muse::IInteractive> interactive = { this };
    muse::ContextInject<context::IGlobalContext> globalContext = { this };
    muse::ContextInject<musesounds::IMuseSoundsCheckUpdateScenario> museSoundsCheckUpdateScenario = { this };
    muse::ContextInject<musesounds::IMuseSamplerCheckUpdateScenario> museSamplerCheckUpdateScenario = { this };

    OpenProjectScenario(const muse::modularity::ContextPtr& iocCtx)
        : muse::Contextable(iocCtx) {}

    using IOpenProjectScenario::openProject;

    muse::Ret openProject(const ProjectFile& file) override;
    muse::Ret openProject(const muse::io::path_t& path, const QString& displayNameOverride = QString()) override;
    muse::Ret openProject(const muse::rcommand::Params& params) override;

    void revertToLastSaved() override;
    muse::Ret finishOpening() override;

    bool isProjectOpened(const muse::io::path_t& scorePath) const override;

    const ProjectBeingDownloaded& projectBeingDownloaded() const override;
    muse::async::Notification projectBeingDownloadedChanged() const override;

    bool isBusy(IProjectCommandsController::BusyStatus status) const override;
    muse::async::Notification busyChanged() const override;

    void showCloudOpenError(const muse::Ret& ret) const override;

private:
    using BusyStatus = IProjectCommandsController::BusyStatus;

    void setBusy(BusyStatus status, bool isBusy);

    muse::RetVal<INotationProjectPtr> loadProject(const muse::io::path_t& filePath);
    muse::Ret loadWithFallback(const std::shared_ptr<INotationProject>& project, const muse::io::path_t& loadPath,
                               const std::string& format);

    muse::Ret doOpenProject(const muse::io::path_t& filePath);
    muse::Ret doOpenCloudProject(const muse::io::path_t& filePath, const CloudProjectInfo& info, bool isOwner = true);
    muse::Ret doOpenCloudProjectOffline(const muse::io::path_t& filePath, const QString& displayNameOverride);

    void downloadAndOpenCloudProject(int scoreId, const QString& hash = QString(), const QString& secret = QString(),
                                     bool isOwner = true);
    muse::Ret openMuseScoreUrl(const QUrl& url);
    muse::Ret openScoreFromMuseScoreCom(const QUrl& url);

    muse::Ret openPageIfNeed(muse::Uri pageUri);

    bool shouldRetryLoadAfterError(const muse::Ret& ret, const muse::io::path_t& filepath);
    bool askIfUserAgreesToOpenProjectWithIncompatibleVersion(const std::string& errorText);
    void warnFileTooNew(const muse::io::path_t& filepath);
    bool askIfUserAgreesToOpenCorruptedProject(const muse::String& projectName, const std::string& errorText);
    void warnProjectCriticallyCorrupted(const muse::String& projectName, const std::string& errorText);
    void warnProjectCannotBeOpened(const muse::Ret& ret, const muse::io::path_t& filepath);

    muse::async::Promise<muse::io::path_t> selectScoreOpeningFile() const;

    RecentFile makeRecentFile(INotationProjectPtr project);

    std::set<BusyStatus> m_busyStatuses;
    muse::async::Notification m_busyChanged;

    ProjectBeingDownloaded m_projectBeingDownloaded;
    muse::async::Notification m_projectBeingDownloadedChanged;
};
}
