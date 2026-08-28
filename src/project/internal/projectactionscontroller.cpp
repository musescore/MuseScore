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
#include <QEventLoop>
#include <QFileInfo>
#include <QTemporaryFile>
#include <QUrl>
#include <QUrlQuery>
#include <QTimer>
#include <qobject.h>

#include "async/async.h"
#include "async/processevents.h"
#include "defer.h"
#include "rcommand/commandtypes.h"
#include "translation.h"

#include "cloud/clouderrors.h"
#include "cloud/qml/Muse/Cloud/enums.h"
#include "engraving/infrastructure/mscio.h"
#include "engraving/engravingerrors.h"

#include "notation/imasternotation.h"
#include "notation/inotationinteraction.h"
#include "notation/inotationselection.h"

#include "projecterrors.h"

#include "../projectcommands.h"
#include "rcommand/actiontocommand.h"

#include "log.h"
#include "types/ret.h"

using namespace mu;
using namespace mu::project;
using namespace mu::notation;
using namespace muse;
using namespace muse::actions;

static const muse::Uri NOTATION_PAGE_URI("musescore://notation");
static const muse::Uri HOME_PAGE_URI("musescore://home");
static const muse::Uri NEW_SCORE_URI("musescore://project/newscore");
static const muse::Uri PROJECT_PROPERTIES_URI("musescore://project/properties");
static const muse::Uri UPLOAD_PROGRESS_URI("musescore://project/upload/progress");

static const QString MUSESCORE_URL_SCHEME("musescore");
static const QString OPEN_SCORE_URL_HOSTNAME("open-score");

static constexpr int RETRY_SAVE_BTN_ID = int(IInteractive::Button::CustomButton);
static constexpr int SAVE_AS_BTN_ID    = RETRY_SAVE_BTN_ID + 1;

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

    d->onRequest(this, PROJECT_SAVE_COMMAND, [this]() { return saveProject(SaveMode::Save); });
    d->onRequest(this, PROJECT_SAVE_AS_COMMAND, [this]() { return saveProject(SaveMode::SaveAs); });
    d->onRequest(this, PROJECT_SAVE_A_COPY_COMMAND, [this]() { return saveProject(SaveMode::SaveCopy); });
    d->onRequest(this, PROJECT_SAVE_SELECTION_COMMAND, [this]() { return saveProject(SaveMode::SaveSelection, SaveLocationType::Local); });
    d->onRequest(this, PROJECT_SAVE_TO_CLOUD_COMMAND, [this]() { return saveProject(SaveMode::Save, SaveLocationType::Cloud); });
    d->onRequest(this, PROJECT_SAVE_AT_COMMAND, [this](const rcommand::Params& params) { return saveProjectAt(params); });

    d->onRequest(this, PROJECT_PUBLISH_COMMAND, [this]() { return publish(); });
    d->onRequest(this, PROJECT_SHARED_AUDIO_COMMAND, [this]() { return sharedAudio(); });

    d->onRequest(this, PROJECT_EXPORT_COMMAND, [this]() { return exportScore(); });
    d->onRequest(this, PROJECT_IMPORT_PDF_COMMAND, [this]() { return importPdf(); });
    d->onRequest(this, PROJECT_IMPORT_AUDIO_TO_SCORE_COMMAND, [this]() { return importAudioToScore(); });

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
            { "file-share-audio", PROJECT_SHARED_AUDIO_COMMAND, {} },
            { "file-export", PROJECT_EXPORT_COMMAND, {} },
            { "file-import-pdf", PROJECT_IMPORT_PDF_COMMAND, {} },
            { "file-import-audio-to-score", PROJECT_IMPORT_AUDIO_TO_SCORE_COMMAND, {} },
            { "export", PROJECT_EXPORT_COMMAND, {} },
            { "import-pdf", PROJECT_IMPORT_PDF_COMMAND, {} },
            { "import-audio-to-score", PROJECT_IMPORT_AUDIO_TO_SCORE_COMMAND, {} },
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

    saveProjectScenario()->revertToLastSavedRequested().onNotify(this, [this]() {
        revertCorruptedScoreToLastSaved();
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
           || openProjectScenario()->isBusy(status);
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
            "file-import-pdf",
            "file-import-audio-to-score",
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

    return true;
}

bool ProjectActionsController::isUrlSupported(const QUrl& url) const
{
    if (url.isLocalFile()) {
        return isFileSupported(muse::io::path_t(url));
    }

    if (url.scheme() == MUSESCORE_URL_SCHEME) {
        if (url.host() == OPEN_SCORE_URL_HOSTNAME) {
            return true;
        }
    }

    return false;
}

bool ProjectActionsController::isFileSupported(const muse::io::path_t& path) const
{
    std::string suffix = io::suffix(path);
    if (engraving::isMuseScoreFile(suffix)) {
        return true;
    }

    if (readers()->reader(suffix)) {
        return true;
    }

    return false;
}

muse::Ret ProjectActionsController::openPageIfNeed(muse::Uri pageUri)
{
    if (!interactive()->isOpened(pageUri).val) {
        interactive()->open(pageUri);
    }
    return make_ret(Ret::Code::Ok);
}

muse::Ret ProjectActionsController::openProject(const ProjectFile& file)
{
    return openProjectScenario()->openProject(file);
}

muse::Ret ProjectActionsController::openProject(const muse::io::path_t& path, const QString& displayNameOverride)
{
    return openProjectScenario()->openProject(path, displayNameOverride);
}

muse::Ret ProjectActionsController::openProject(const muse::rcommand::Params& params)
{
    return openProjectScenario()->openProject(params);
}

const ProjectBeingDownloaded& ProjectActionsController::projectBeingDownloaded() const
{
    return openProjectScenario()->projectBeingDownloaded();
}

muse::async::Notification ProjectActionsController::projectBeingDownloadedChanged() const
{
    return openProjectScenario()->projectBeingDownloadedChanged();
}

bool ProjectActionsController::isProjectOpened(const muse::io::path_t& scorePath) const
{
    return openProjectScenario()->isProjectOpened(scorePath);
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
    auto anyInstanceWithoutProject = multiwindowsProvider()->isHasWindowWithoutProject();
    bool ok = closeOpenedProject();
    if (ok && anyInstanceWithoutProject) {
        //! NOTE: we need to call `quit` in the next event loop due to controlling the lifecycle of this method
        async::Async::call(this, [this]() {
            dispatcher()->dispatch("quit", ActionData::make_arg1<bool>(false));
        });
        multiwindowsProvider()->activateWindowWithoutProject();
    }

    return ok ? make_ok() : make_ret(Ret::Code::UnknownError);
}

bool ProjectActionsController::closeOpenedProject(bool goToHome)
{
    if (isBusy(BusyStatus::Closing)) {
        return false;
    }

    setBusy(BusyStatus::Closing, true);
    DEFER {
        setBusy(BusyStatus::Closing, false);
    };

    INotationProjectPtr project = currentNotationProject();
    if (!project) {
        return true;
    }

    if (globalContext()->playbackState()->isPlaying()) {
        commandDispatcher()->dispatch(rcommand::Command("command://playback/stop"));
    }

    bool result = true;

    if (project->isNeedSave()) {
        IInteractive::Button btn = askAboutSavingScore(project);

        if (btn == IInteractive::Button::Cancel) {
            result = false;
        } else if (btn == IInteractive::Button::Save) {
            result = saveProject();
        } else if (btn == IInteractive::Button::DontSave) {
            result = true;
        }
    }

    if (result) {
        interactive()->closeAllDialogsSync();
        globalContext()->setCurrentProject(nullptr);

        if (goToHome) {
            Ret ret = openPageIfNeed(HOME_PAGE_URI);
            if (!ret) {
                LOGE() << ret.toString();
            }
        }
    }

    return result;
}

IInteractive::Button ProjectActionsController::askAboutSavingScore(INotationProjectPtr project)
{
    std::string title = muse::qtrc("project", "Do you want to save changes to the score “%1” before closing?")
                        .arg(project->displayName()).toStdString();

    std::string body = muse::trc("project", "Your changes will be lost if you don’t save them.");

    IInteractive::Result result = interactive()->warningSync(title, body, {
        IInteractive::Button::DontSave,
        IInteractive::Button::Cancel,
        IInteractive::Button::Save
    }, IInteractive::Button::Save);

    return result.standardButton();
}

muse::Ret ProjectActionsController::saveProject(SaveMode saveMode, SaveLocationType saveLocationType, bool force)
{
    return saveProjectScenario()->saveProject(saveMode, saveLocationType, force);
}

muse::Ret ProjectActionsController::publish()
{
    return saveProjectScenario()->publish();
}

muse::Ret ProjectActionsController::sharedAudio()
{
    return saveProjectScenario()->shareAudio();
}

bool ProjectActionsController::saveProject(const muse::io::path_t& path)
{
    return saveProjectScenario()->saveProject(path);
}

muse::Ret ProjectActionsController::saveProjectAt(const muse::rcommand::Params& params)
{
    return saveProjectScenario()->saveProjectAt(params);
}

bool ProjectActionsController::saveProjectLocally(const muse::io::path_t& path, SaveMode saveMode, bool createBackup)
{
    return saveProjectScenario()->saveProjectLocally(path, saveMode, createBackup);
}

void ProjectActionsController::revertCorruptedScoreToLastSaved()
{
    openProjectScenario()->revertToLastSaved();
}

muse::Ret ProjectActionsController::importPdf()
{
    platformInteractive()->openUrl("https://musescore.com/import");
    return make_ok();
}

muse::Ret ProjectActionsController::importAudioToScore()
{
    platformInteractive()->openUrl("https://musescore.com/upload?format=audio2score");
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
