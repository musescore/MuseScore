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

#include "projectactionscontroller.h"

#include <QBuffer>
#include <QFileInfo>
#include <QTemporaryFile>
#include <QUrl>
#include <QUrlQuery>
#include <QTimer>
#include <qobject.h>

#include "async/async.h"
#include "defer.h"
#include "types/ret.h"
#include "rcommand/commandtypes.h"

#include "notation/imasternotation.h"
#include "notation/inotationinteraction.h"
#include "notation/inotationselection.h"

#include "inotationproject.h"
#include "types/projecturis.h"

#include "../projectcommands.h"
#include "appshell/appshellcommands.h"

#include "log.h"

using namespace mu;
using namespace mu::project;
using namespace mu::notation;
using namespace muse;

void ProjectActionsController::init()
{
    auto d = commandDispatcher();

    d->onRequest(this, PROJECT_NEW_COMMAND, [this]() { return newProject(); });
    d->onRequest(this, PROJECT_OPEN_COMMAND, [this](const rcommand::Params& params) { return openProject(params); });
    d->onRequest(this, PROJECT_CLOSE_COMMAND, [this]() { return closeProject(); });

    d->onRequest(this, PROJECT_SAVE_COMMAND, [this]() { return runAsync(saveProject(SaveMode::Save)); });
    d->onRequest(this, PROJECT_SAVE_AS_COMMAND, [this]() { return runAsync(saveProject(SaveMode::SaveAs)); });
    d->onRequest(this, PROJECT_SAVE_A_COPY_COMMAND, [this]() { return runAsync(saveProject(SaveMode::SaveCopy)); });
    d->onRequest(this, PROJECT_SAVE_SELECTION_COMMAND, [this]() {
        return runAsync(saveProject(SaveMode::SaveSelection, SaveLocationType::Local));
    });
    d->onRequest(this, PROJECT_SAVE_TO_CLOUD_COMMAND, [this]() { return runAsync(saveProject(SaveMode::Save, SaveLocationType::Cloud)); });
    d->onRequest(this, PROJECT_SAVE_AT_COMMAND, [this](const rcommand::Params& params) { return runAsync(saveProjectAt(params)); });

    d->onRequest(this, PROJECT_PUBLISH_COMMAND, [this]() { return runAsync(publish()); });
    d->onRequest(this, PROJECT_SHARE_AUDIO_COMMAND, [this]() { return runAsync(sharedAudio()); });

    d->onRequest(this, PROJECT_EXPORT_COMMAND, [this]() { return exportScore(); });
    d->onRequest(this, PROJECT_CONVERT_TO_SCORE_COMMAND, [this]() { return convertFileToScore(); });

    d->onRequest(this, PROJECT_PRINT_COMMAND, [this]() { return printScore(); });
    d->onRequest(this, PROJECT_CLEAR_RECENT_COMMAND, [this]() { return clearRecentScores(); });
    d->onRequest(this, PROJECT_CONTINUE_LAST_SESSION_COMMAND, [this]() { return continueLastSession(); });
    d->onRequest(this, PROJECT_PROPERTIES_COMMAND, [this]() { return openProjectProperties(); });

    saveProjectScenario()->busyChanged().onNotify(this, [this]() {
        m_busyChanged.notify();
    });

    openProjectScenario()->busyChanged().onNotify(this, [this]() {
        m_busyChanged.notify();
    });

    closeProjectScenario()->busyChanged().onNotify(this, [this]() {
        m_busyChanged.notify();
    });

    // listen changes
    projectSession()->projectChanged().onNotify(this, [this]() {
        auto project = projectSession()->project();
        if (project) {
            project->needSaveChanged().onNotify(this, [this]() {
                m_needSaveChanged.notify();
            });
        }

        auto notation = projectSession()->currentNotation();
        if (notation) {
            notation->interaction()->selectionChanged().onNotify(this, [this]() {
                m_hasSelectionChanged.notify();
            });
        }
    });
}

bool ProjectActionsController::hasProject() const
{
    return currentNotationProject() != nullptr;
}

muse::async::Notification ProjectActionsController::hasProjectChanged() const
{
    return projectSession()->projectChanged();
}

bool ProjectActionsController::needSave() const
{
    return currentNotationProject() ? currentNotationProject()->isNeedSave() : false;
}

muse::async::Notification ProjectActionsController::needSaveChanged() const
{
    return m_needSaveChanged;
}

bool ProjectActionsController::isBusy(BusyStatus status) const
{
    return m_busyStatuses.contains(status)
           || saveProjectScenario()->isBusy(status)
           || openProjectScenario()->isBusy(status)
           || closeProjectScenario()->isBusy(status);
}

void ProjectActionsController::setBusy(BusyStatus status, bool isBusy)
{
    bool wasBusy = m_busyStatuses.contains(status);
    if (wasBusy == isBusy) {
        return;
    }

    if (isBusy) {
        m_busyStatuses.insert(status);
    } else {
        m_busyStatuses.erase(status);
    }

    m_busyChanged.notify();
}

muse::async::Notification ProjectActionsController::busyChanged() const
{
    return m_busyChanged;
}

bool ProjectActionsController::hasSelection() const
{
    return currentNotationSelection() ? !currentNotationSelection()->isNone() : false;
}

muse::async::Notification ProjectActionsController::hasSelectionChanged() const
{
    return m_hasSelectionChanged;
}

INotationProjectPtr ProjectActionsController::currentNotationProject() const
{
    return projectSession()->project();
}

IMasterNotationPtr ProjectActionsController::currentMasterNotation() const
{
    return currentNotationProject() ? currentNotationProject()->masterNotation() : nullptr;
}

INotationPtr ProjectActionsController::currentNotation() const
{
    return currentMasterNotation() ? currentMasterNotation()->notation() : nullptr;
}

INotationInteractionPtr ProjectActionsController::currentInteraction() const
{
    return currentNotation() ? currentNotation()->interaction() : nullptr;
}

INotationSelectionPtr ProjectActionsController::currentNotationSelection() const
{
    return currentNotation() ? currentInteraction()->selection() : nullptr;
}

muse::Ret ProjectActionsController::openPageIfNeed(muse::Uri pageUri)
{
    if (!interactive()->isOpened(pageUri).val) {
        interactive()->open(pageUri);
    }
    return make_ret(Ret::Code::Ok);
}

muse::Ret ProjectActionsController::openProject(const muse::io::path_t& path, const QString& displayNameOverride)
{
    return runAsync(openProjectScenario()->openProject(path, displayNameOverride));
}

muse::Ret ProjectActionsController::openProject(const muse::rcommand::Params& params)
{
    return runAsync(openProjectScenario()->openProject(params));
}

bool ProjectActionsController::isProjectOpened(const muse::io::path_t& scorePath) const
{
    auto project = projectSession()->project();
    if (!project) {
        return false;
    }

    LOGD() << "project->path: " << project->path() << ", check path: " << scorePath;
    if (project->path() == scorePath) {
        return true;
    }

    return false;
}

bool ProjectActionsController::isAnyProjectOpened() const
{
    auto project = projectSession()->project();
    if (project) {
        return true;
    }
    return false;
}

muse::Ret ProjectActionsController::newProject()
{
    //! NOTE This method is synchronous,
    //! but inside `multiwindowsProvider` there can be an event loop
    //! to wait for the responses from other instances, accordingly,
    //! the events (like user click) can be executed and this method can be called several times,
    //! before the end of the current call.
    //! So we ignore all subsequent calls until the current one completes.
    if (isBusy(BusyStatus::Opening)) {
        return make_ret(Ret::Code::Busy);
    }
    setBusy(BusyStatus::Opening, true);

    DEFER {
        setBusy(BusyStatus::Opening, false);
    };

    if (projectSession()->project()) {
        if (multiwindowsProvider()->isHasWindowWithoutProject()) {
            multiwindowsProvider()->activateWindowWithoutProject({ QString::fromStdString(PROJECT_NEW_COMMAND.toString()) });
            return make_ok();
        }
        QStringList args;
        args << "--session-type" << "start-with-new";
        multiwindowsProvider()->openNewWindow(args);
        return make_ok();
    }

    auto promise = interactive()->open(NEW_SCORE_URI);
    promise.onResolve(this, [this](const Val&) {
        Ret ret = openProjectScenario()->finishOpening();

        if (!ret) {
            LOGE() << ret.toString();
        }
    });

    return make_ok();
}

muse::Ret ProjectActionsController::closeProject()
{
    bool anyInstanceWithoutProject = multiwindowsProvider()->isHasWindowWithoutProject();

    return runAsync(closeProjectScenario()->closeOpenedProject(true)
                    .then<Ret>(this, [this, anyInstanceWithoutProject](const Ret& ret, auto resolve) {
        if (ret && anyInstanceWithoutProject) {
            //! NOTE: we need to call `quit` in the next event loop due to controlling the lifecycle of this method
            async::Async::call(this, [this]() {
                commandDispatcher()->dispatch(appshell::APP_QUIT_COMMAND, { { "all_instances", muse::Val(false) } });
            });
            multiwindowsProvider()->activateWindowWithoutProject();
        }

        return resolve(ret);
    }));
}

//! Commands report that the flow has started; its outcome is shown to the user by the scenario itself
muse::Ret ProjectActionsController::runAsync(async::Promise<Ret> flow)
{
    flow.onResolve(this, [](const Ret& ret) {
        if (!ret) {
            LOGD() << ret.toString();
        }
    });

    return make_ok();
}

async::Promise<Ret> ProjectActionsController::saveProject(SaveMode saveMode, SaveLocationType saveLocationType, bool force)
{
    return saveProjectScenario()->saveProject(saveMode, saveLocationType, force);
}

async::Promise<Ret> ProjectActionsController::publish()
{
    return saveProjectScenario()->publish();
}

async::Promise<Ret> ProjectActionsController::sharedAudio()
{
    return saveProjectScenario()->shareAudio();
}

async::Promise<Ret> ProjectActionsController::saveProjectAt(const muse::rcommand::Params& params)
{
    return saveProjectScenario()->saveProjectAt(params);
}

muse::Ret ProjectActionsController::convertFileToScore()
{
    convertFileToScoreScenario()->convertFiles();
    return make_ok();
}

muse::Ret ProjectActionsController::clearRecentScores()
{
    recentFilesController()->clearRecentFiles();
    return make_ok();
}

muse::Ret ProjectActionsController::continueLastSession()
{
    const RecentFilesList& recentScorePaths = recentFilesController()->recentFilesList();

    if (recentScorePaths.empty()) {
        Ret ret = openPageIfNeed(HOME_PAGE_URI);
        if (!ret) {
            LOGE() << ret.toString();
        }
        return ret;
    }

    muse::io::path_t lastScorePath = recentScorePaths.front().path;
    return openProject(lastScorePath);
}

muse::Ret ProjectActionsController::exportScore()
{
    static const Uri EXPORT_URI("musescore://project/export");
    if (!interactive()->isOpened(EXPORT_URI).val) {
        interactive()->open(EXPORT_URI);
    }
    return make_ok();
}

muse::Ret ProjectActionsController::printScore()
{
    INotationPtr notation = projectSession()->currentNotation();
    if (!notation) {
        return make_ret(Ret::Code::InternalError);
    }

    printProvider()->printNotation(notation);
    return make_ok();
}

muse::Ret ProjectActionsController::openProjectProperties()
{
    interactive()->open(PROJECT_PROPERTIES_URI);
    return make_ok();
}
