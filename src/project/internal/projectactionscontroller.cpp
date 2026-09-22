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
#include "rcommand/commandtypes.h"

#include "notation/imasternotation.h"
#include "notation/inotationinteraction.h"
#include "notation/inotationselection.h"

#include "inotationproject.h"

#include "../projectcommands.h"
#include "rcommand/actiontocommand.h"
#include "types/projecturis.h"

#include "log.h"
#include "types/ret.h"

using namespace mu;
using namespace mu::project;
using namespace mu::notation;
using namespace muse;
using namespace muse::actions;

auto openArgs = [](const rcommand::Command& command, const ActionData& args) -> muse::rcommand::CommandQuery {
    rcommand::CommandQuery query(command);
    if (args.count() > 0) {
        query.set("url", Val(args.arg<QUrl>(0).toString().toStdString()));
    }
    if (args.count() > 1) {
        query.set("display_name", Val(args.arg<QString>(1).toStdString()));
    }
    return query;
};

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

    // compat
    {
        using namespace muse::rcommand;
        static const std::vector<ActionToCommand> actionToCommand = {
            { "file-new", PROJECT_NEW_COMMAND, {} },
            { "file-open", PROJECT_OPEN_COMMAND, openArgs },
            { "file-close", PROJECT_CLOSE_COMMAND, {} },
            { "file-save", PROJECT_SAVE_COMMAND, {} },
            { "file-save-as", PROJECT_SAVE_AS_COMMAND, {} },
            { "file-save-a-copy", PROJECT_SAVE_A_COPY_COMMAND, {} },
            { "file-save-selection", PROJECT_SAVE_SELECTION_COMMAND, {} },
            { "file-save-to-cloud", PROJECT_SAVE_TO_CLOUD_COMMAND, {} },
            { "file-save-at", PROJECT_SAVE_AT_COMMAND, make_conv({ { "path", param<io::path_t> } }) },
            { "file-publish", PROJECT_PUBLISH_COMMAND, {} },
            { "file-share-audio", PROJECT_SHARE_AUDIO_COMMAND, {} },
            { "file-export", PROJECT_EXPORT_COMMAND, {} },
            { "file-convert-to-score", PROJECT_CONVERT_TO_SCORE_COMMAND, {} },
            { "export", PROJECT_EXPORT_COMMAND, {} },
            { "print", PROJECT_PRINT_COMMAND, {} },
            { "clear-recent", PROJECT_CLEAR_RECENT_COMMAND, {} },
            { "continue-last-session", PROJECT_CONTINUE_LAST_SESSION_COMMAND, {} },
            { "project-properties", PROJECT_PROPERTIES_COMMAND, {} },
        };

        rcommand::registerActionToCommand(this, actionToCommand, commandDispatcher(), dispatcher());
    }

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
    globalContext()->currentProjectChanged().onNotify(this, [this]() {
        auto project = globalContext()->currentProject();
        if (project) {
            project->needSaveChanged().onNotify(this, [this]() {
                m_needSaveChanged.notify();
            });
        }

        auto notation = globalContext()->currentNotation();
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
    return globalContext()->currentProjectChanged();
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
    return globalContext()->currentProject();
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

bool ProjectActionsController::canReceiveAction(const ActionCode& code) const
{
    if (!currentNotationProject()) {
        static const std::unordered_set<ActionCode> DONT_REQUIRE_OPEN_PROJECT {
            "file-new",
            "file-open",
            "file-convert-to-score",
            "continue-last-session",
            "clear-recent",
        };

        return muse::contains(DONT_REQUIRE_OPEN_PROJECT, code);
    }

    if (isBusy(BusyStatus::Uploading)) {
        if (code == "file-save-to-cloud" || code == "file-publish") {
            return false;
        }
    }

    if (interactive()->currentUri().val == NOTATION_REVIEW_PAGE_URI) {
        static const std::unordered_set<ActionCode> ALLOWED_ON_REVIEW_PAGE {
            "file-close",
        };

        return muse::contains(ALLOWED_ON_REVIEW_PAGE, code);
    }

    return true;
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
    auto project = globalContext()->currentProject();
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
    auto project = globalContext()->currentProject();
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

    if (globalContext()->currentProject()) {
        if (multiwindowsProvider()->isHasWindowWithoutProject()) {
            multiwindowsProvider()->activateWindowWithoutProject({ "file-new" });
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
                dispatcher()->dispatch("quit", ActionData::make_arg1<bool>(false));
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
    INotationPtr notation = globalContext()->currentNotation();
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
