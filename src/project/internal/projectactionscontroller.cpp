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
    return m_busyStatuses.contains(status) || saveProjectScenario()->isBusy(status);
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

muse::Ret ProjectActionsController::openProject(const muse::rcommand::Params& params)
{
    const QString url = params.at("url").toQString();
    const QString displayNameOverride = params.at("display_name").toQString();

    Ret ret = openProject(ProjectFile(QUrl(url), displayNameOverride));
    if (!ret) {
        LOGE() << ret.toString();
    }

    return ret;
}

Ret ProjectActionsController::openProject(const ProjectFile& file)
{
    LOGI() << "Try open project: url = " << file.url.toString() << ", displayNameOverride = " << file.displayNameOverride;

    if (file.isNull()) {
        auto promise = selectScoreOpeningFile();
        promise.onResolve(this, [this](const io::path_t& askedPath) {
            if (askedPath.empty()) {
                return;
            }

            configuration()->setLastOpenedProjectsPath(io::dirpath(askedPath));

            openProject(askedPath);
        });

        return muse::make_ok();
    }

    if (file.url.isLocalFile()) {
        return openProject(file.path(), file.displayNameOverride);
    }

    if (file.url.scheme() == MUSESCORE_URL_SCHEME) {
        return openMuseScoreUrl(file.url);
    }

    return make_ret(Err::UnsupportedUrl);
}

Ret ProjectActionsController::openProject(const muse::io::path_t& givenPath, const QString& displayNameOverride)
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

    //! Step 1. Take absolute path
    muse::io::path_t actualPath = fileSystem()->absoluteFilePath(givenPath);
    if (actualPath.empty()) {
        // We assume that a valid path has been specified to this method
        return make_ret(Ret::Code::UnknownError);
    }

    //! Step 2. If the project is already open in the current window, then just switch to showing the notation
    if (isProjectOpened(actualPath)) {
        return doFinishOpenProject();
    }

    //! Step 3. Check, if the project already opened in another window, then activate the window with the project
    if (multiwindowsProvider()->isProjectAlreadyOpened(actualPath)) {
        multiwindowsProvider()->activateWindowWithProject(actualPath);
        return make_ret(Ret::Code::Ok);
    }

    //! Step 4. Check, if a any project is already open in the current window,
    //! then create a new instance
    if (globalContext()->currentProject()) {
        QStringList args;
        args << actualPath.toQString();

        if (!displayNameOverride.isEmpty()) {
            args << "--score-display-name-override" << displayNameOverride;
        }

        multiwindowsProvider()->openNewWindow(args);
        return make_ret(Ret::Code::Ok);
    }

    //! Step 5. If it's a cloud project, download the latest version
    if (configuration()->isCloudProject(actualPath) && !configuration()->isLegacyCloudProject(actualPath)) {
        bool isCloudAvailable = museScoreComService()->authorization()->checkCloudIsAvailable();
        if (isCloudAvailable) {
            downloadAndOpenCloudProject(configuration()->cloudScoreIdFromPath(actualPath));
            return make_ret(Ret::Code::Ok);
        }

        if (fileSystem()->exists(actualPath)) {
            return doOpenCloudProjectOffline(actualPath, displayNameOverride);
        }

        Ret ret = make_ret(cloud::Err::NetworkError);
        openSaveProjectScenario()->showCloudOpenError(ret);
        return ret;
    }

    //! Step 6. Open project in the current window
    return doOpenProject(actualPath);
}

RetVal<INotationProjectPtr> ProjectActionsController::loadProject(const muse::io::path_t& filePath)
{
    TRACEFUNC;

    const auto project = projectCreator()->newProject(iocContext());
    IF_ASSERT_FAILED(project) {
        return make_ret(Ret::Code::InternalError);
    }

    const bool hasUnsavedChanges = projectAutoSaver()->projectHasUnsavedChanges(filePath);

    const muse::io::path_t loadPath = hasUnsavedChanges ? projectAutoSaver()->projectAutoSavePath(filePath) : filePath;
    const std::string format = io::suffix(filePath);

    if (Ret result = loadWithFallback(project, loadPath, format); !result) {
        return result;
    }

    if (hasUnsavedChanges) {
        //! NOTE: redirect the project to the original file path
        project->setPath(filePath);

        project->markAsUnsaved();
    }

    if (projectAutoSaver()->isAutosaveOfNewlyCreatedProject(filePath)) {
        project->markAsNewlyCreated();
    }

    return RetVal<INotationProjectPtr>::make_ok(project);
}

Ret ProjectActionsController::loadWithFallback(const std::shared_ptr<INotationProject>& project,
                                               const muse::io::path_t& loadPath,
                                               const std::string& format)
{
    Ret result = project->load(loadPath, OpenParams(), format);

    if (result || result.code() == static_cast<int>(Ret::Code::Cancel)) {
        return result;
    }

    bool forceLoad = shouldRetryLoadAfterError(result, loadPath);
    if (forceLoad) {
        OpenParams params;
        params.forceMode = forceLoad;
        result = project->load(loadPath, params, format);
    }

    return result;
}

Ret ProjectActionsController::doOpenProject(const muse::io::path_t& filePath)
{
    TRACEFUNC;

    RetVal<INotationProjectPtr> rv = loadProject(filePath);
    if (!rv.ret) {
        return rv.ret;
    }

    INotationProjectPtr project = rv.val;

    bool isNewlyCreated = projectAutoSaver()->isAutosaveOfNewlyCreatedProject(filePath);
    if (!isNewlyCreated) {
        recentFilesController()->prependRecentFile(makeRecentFile(project));
    }

    globalContext()->setCurrentProject(project);

    return doFinishOpenProject();
}

Ret ProjectActionsController::doOpenCloudProject(const muse::io::path_t& filePath, const CloudProjectInfo& info, bool isOwner)
{
    RetVal<INotationProjectPtr> rv = loadProject(filePath);
    if (!rv.ret) {
        return rv.ret;
    }

    INotationProjectPtr project = rv.val;

    if (isOwner) {
        project->setCloudInfo(info);
    } else {
        project->markAsNewlyCreated();
        project->setCloudInfo(CloudProjectInfo());
    }

    bool isNewlyCreated = projectAutoSaver()->isAutosaveOfNewlyCreatedProject(filePath)
                          || !isOwner;
    if (!isNewlyCreated) {
        recentFilesController()->prependRecentFile(makeRecentFile(project));
    }

    globalContext()->setCurrentProject(project);

    return doFinishOpenProject();
}

muse::Ret ProjectActionsController::doOpenCloudProjectOffline(const muse::io::path_t& filePath, const QString& displayNameOverride)
{
    RetVal<INotationProjectPtr> rv = loadProject(filePath);
    if (!rv.ret) {
        return rv.ret;
    }

    INotationProjectPtr project = rv.val;
    CloudProjectInfo info = project->cloudInfo();
    info.name = displayNameOverride;
    project->setCloudInfo(info);

    recentFilesController()->prependRecentFile(makeRecentFile(project));
    globalContext()->setCurrentProject(project);

    return doFinishOpenProject();
}

Ret ProjectActionsController::doFinishOpenProject()
{
    //! Show MuseSounds / MuseSampler update if need
    auto showUpdateNotification = [this]() {
        QTimer::singleShot(1000, [this]() {
            if (museSoundsCheckUpdateScenario()->hasUpdate()) {
                museSoundsCheckUpdateScenario()->showUpdate();
            } else if (!museSamplerCheckUpdateScenario()->alreadyChecked()) {
                museSamplerCheckUpdateScenario()->checkAndShowUpdateIfNeed();
            }
        });
    };

    if (interactive()->isOpened(NOTATION_PAGE_URI).val) {
        showUpdateNotification();
    } else {
        async::Channel<Uri> opened = interactive()->opened();
        opened.onReceive(this, [this, opened, showUpdateNotification](const Uri&) {
            async::Async::call(this, [this, opened, showUpdateNotification]() {
                async::Channel<Uri> mut = opened;
                mut.disconnect(this);

                showUpdateNotification();
            });
        });
    }

    return openPageIfNeed(NOTATION_PAGE_URI);
}

void ProjectActionsController::downloadAndOpenCloudProject(int scoreId, const QString& hash, const QString& secret, bool isOwner)
{
    if (isBusy(BusyStatus::Downloading)) {
        return;
    }
    setBusy(BusyStatus::Downloading, true);

    bool isDownloadingFinished = true;
    DEFER {
        if (isDownloadingFinished) {
            setBusy(BusyStatus::Downloading, false);
        }
    };

    if (!scoreId) {
        // Might happen when user tries to open score that saved as a cloud score but upload did not fully succeed
        LOGE() << "invalid cloud score id";
        openSaveProjectScenario()->showCloudOpenError(make_ret(Err::InvalidCloudScoreId));
        return;
    }

    std::string dialogText = muse::trc("project/save", "Log in or create a free account on MuseScore.com to open this score.");
    Ret ret = museScoreComService()->authorization()->ensureAuthorization(false, dialogText).ret;
    if (!ret) {
        return;
    }

    CloudProjectInfo info;
    muse::io::path_t localPath = configuration()->cloudProjectPath(scoreId);

    if (isOwner) {
        RetVal<muse::cloud::ScoreInfo> scoreInfo = museScoreComService()->downloadScoreInfo(scoreId);
        if (!scoreInfo.ret) {
            LOGE() << "Error while downloading score info: " << scoreInfo.ret.toString();
            openSaveProjectScenario()->showCloudOpenError(scoreInfo.ret);
            return;
        }

        info.name = scoreInfo.val.title;
        info.visibility = scoreInfo.val.visibility;
        info.sourceUrl = scoreInfo.val.url;
        info.revisionId = scoreInfo.val.revisionId;

        RetVal<CloudProjectInfo> localInfo = mscMetaReader()->readCloudProjectInfo(localPath);

        if (localInfo.ret) {
            if (localInfo.val.revisionId == scoreInfo.val.revisionId) {
                doOpenCloudProject(localPath, info, isOwner);
                return;
            }
        } else {
            LOGE() << localInfo.ret;
        }
    }

    // TODO(cloud): conflict checking (don't recklessly overwrite the existing file)
    auto projectData = std::make_shared<QFile>(localPath.toQString());
    if (!projectData->open(QIODevice::WriteOnly)) {
        openSaveProjectScenario()->showCloudOpenError(make_ret(Err::FileOpenError));
        return;
    }

    m_projectBeingDownloaded.scoreId = scoreId;
    m_projectBeingDownloaded.progress = museScoreComService()->downloadScore(scoreId, projectData, hash, secret);

    m_projectBeingDownloaded.progress->finished().onReceive(this, [this, localPath, info, isOwner](const ProgressResult& res) {
        m_projectBeingDownloaded = {};
        m_projectBeingDownloadedChanged.notify();

        setBusy(BusyStatus::Downloading, false);

        if (!res.ret) {
            LOGE() << res.ret.toString();
            openSaveProjectScenario()->showCloudOpenError(res.ret);
            return;
        }

        doOpenCloudProject(localPath, info, isOwner);
    });

    m_projectBeingDownloadedChanged.notify();
    isDownloadingFinished = false;
}

Ret ProjectActionsController::openMuseScoreUrl(const QUrl& url)
{
    if (url.host() == OPEN_SCORE_URL_HOSTNAME) {
        return openScoreFromMuseScoreCom(url);
    }

    return make_ret(Err::UnsupportedUrl);
}

Ret ProjectActionsController::openScoreFromMuseScoreCom(const QUrl& url)
{
    //! NOTE See explanation in `openProject(const muse::io::path_t& _path, const QString& displayNameOverride)`
    if (isBusy(BusyStatus::Downloading) || isBusy(BusyStatus::Opening)) {
        // TODO: instead of ignoring the open request, queue it?
        return make_ret(Ret::Code::Busy);
    }
    setBusy(BusyStatus::Opening, true);

    DEFER {
        setBusy(BusyStatus::Opening, false);
    };

    // Retrieve score id from URL
    bool ok = false;
    int scoreId = url.fileName().toInt(&ok);
    if (!ok || scoreId <= 0) {
        return make_ret(Err::MalformedOpenScoreUrl);
    }

    // Ensure logged in
    std::string dialogText = muse::trc("project/save", "Log in or create a free account on MuseScore.com to open this score.");
    Ret ret = museScoreComService()->authorization()->ensureAuthorization(false, dialogText).ret;
    if (!ret) {
        return ret;
    }

    // Check if user is owner
    RetVal<muse::cloud::ScoreInfo> scoreInfo = museScoreComService()->downloadScoreInfo(scoreId);
    if (!scoreInfo.ret) {
        LOGE() << "Error while downloading score info: " << scoreInfo.ret.toString();
        openSaveProjectScenario()->showCloudOpenError(scoreInfo.ret);

        return scoreInfo.ret;
    }

    bool isOwner = QString::number(scoreInfo.val.owner.id) == museScoreComService()->authorization()->accountInfo().id;

    // If yes, score will be opened as regular cloud score; check if not yet opened
    if (isOwner) {
        muse::io::path_t projectPath = configuration()->cloudProjectPath(scoreId);

        // either in this instance
        if (isProjectOpened(projectPath)) {
            return doFinishOpenProject();
        }

        // or in another one
        if (multiwindowsProvider()->isProjectAlreadyOpened(projectPath)) {
            multiwindowsProvider()->activateWindowWithProject(projectPath);
            return muse::make_ok();
        }
    }

    // Check if this instance already has an open project
    if (globalContext()->currentProject()) {
        QStringList args;
        args << url.toString();

        if (!scoreInfo.val.title.isEmpty()) {
            args << "--score-display-name-override" << scoreInfo.val.title;
        }

        multiwindowsProvider()->openNewWindow(args);
        return muse::make_ok();
    }

    QUrlQuery query(url);
    QString hash = query.queryItemValue("h");
    QString secret = query.queryItemValue("secret");

    downloadAndOpenCloudProject(scoreId, hash, secret, isOwner);

    return muse::make_ok();
}

const ProjectBeingDownloaded& ProjectActionsController::projectBeingDownloaded() const
{
    return m_projectBeingDownloaded;
}

muse::async::Notification ProjectActionsController::projectBeingDownloadedChanged() const
{
    return m_projectBeingDownloadedChanged;
}

Ret ProjectActionsController::openPageIfNeed(Uri pageUri)
{
    if (!interactive()->isOpened(pageUri).val) {
        interactive()->open(pageUri);
    }
    return make_ret(Ret::Code::Ok);
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
        Ret ret = doFinishOpenProject();

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
    TRACEFUNC;

    auto currentProject = currentNotationProject();
    if (!currentProject) {
        return;
    }

    muse::io::path_t filePath = currentProject->path();

    bool hasUnsavedChanges = projectAutoSaver()->projectHasUnsavedChanges(filePath);
    if (hasUnsavedChanges) {
        muse::io::path_t autoSavePath = projectAutoSaver()->projectAutoSavePath(filePath);
        fileSystem()->remove(autoSavePath);
    }

    Ret ret = doOpenProject(filePath);
    if (!ret) {
        LOGE() << ret.toString();
    }
}

RecentFile ProjectActionsController::makeRecentFile(INotationProjectPtr project)
{
    RecentFile file;
    file.path = project->path();

    if (project->isCloudProject()) {
        file.displayNameOverride = project->cloudInfo().name;
    }

    return file;
}

bool ProjectActionsController::shouldRetryLoadAfterError(const Ret& ret, const muse::io::path_t& filepath)
{
    if (ret) {
        return true;
    }

    switch (static_cast<engraving::Err>(ret.code())) {
    case engraving::Err::FileTooOld:
    case engraving::Err::FileOld300Format:
        return askIfUserAgreesToOpenProjectWithIncompatibleVersion(ret.text());
    case engraving::Err::FileTooNew:
        warnFileTooNew(filepath);
        return configuration()->disableVersionChecking();
    case engraving::Err::FileCorrupted:
        return askIfUserAgreesToOpenCorruptedProject(io::filename(filepath).toString(), ret.text());
    case engraving::Err::FileCriticallyCorrupted:
        warnProjectCriticallyCorrupted(io::filename(filepath).toString(), ret.text());
        return false;
    default:
        warnProjectCannotBeOpened(ret, filepath);
        break;
    }

    return false;
}

bool ProjectActionsController::askIfUserAgreesToOpenProjectWithIncompatibleVersion(const std::string& errorText)
{
    IInteractive::ButtonData openAnywayBtn(IInteractive::Button::CustomButton, muse::trc("project", "Open anyway"), true /*accent*/);

    int btn = interactive()->warningSync(errorText, "", {
        interactive()->buttonData(IInteractive::Button::Cancel),
        openAnywayBtn
    }, openAnywayBtn.btn).button();

    return btn == openAnywayBtn.btn;
}

void ProjectActionsController::warnFileTooNew(const muse::io::path_t& filepath)
{
    interactive()->error(muse::qtrc("project", "Cannot read file %1").arg(io::toNativeSeparators(filepath).toQString()).toStdString(),
                         muse::mtrc("project", "This file was saved using a newer version of MuseScore Studio. "
                                               "Please visit <a href=\"%1\">MuseScore.org</a> to obtain the latest version.")
                         .arg(u"https://musescore.org").toStdString());
}

bool ProjectActionsController::askIfUserAgreesToOpenCorruptedProject(const String& projectName, const std::string& errorText)
{
    std::string title = muse::mtrc("project", "File “%1” is corrupted").arg(projectName).toStdString();
    IInteractive::Text text;
    text.text = muse::trc("project", "This file contains errors that could cause MuseScore Studio to malfunction.");
    text.detailedText = errorText;

    IInteractive::ButtonData openAnywayBtn(IInteractive::Button::CustomButton, muse::trc("project", "Open anyway"), true /*accent*/);

    int btn = interactive()->warningSync(title, text, {
        interactive()->buttonData(IInteractive::Button::Cancel),
        openAnywayBtn
    }, openAnywayBtn.btn).button();

    return btn == openAnywayBtn.btn;
}

void ProjectActionsController::warnProjectCriticallyCorrupted(const String& projectName, const std::string& errorText)
{
    std::string title = muse::mtrc("project", "File “%1” is corrupted and cannot be opened").arg(projectName).toStdString();
    IInteractive::Text text;
    text.text = muse::trc("project", "Get help for this issue on MuseScore.org.");
    text.detailedText = errorText;

    IInteractive::ButtonData getHelpBtn(IInteractive::Button::CustomButton, muse::trc("project", "Get help"), true /*accent*/);

    interactive()->error(title, text, {
        interactive()->buttonData(IInteractive::Button::Cancel),
        getHelpBtn
    }, getHelpBtn.btn).onResolve(this, [this, getHelpBtn](const IInteractive::Result& res) {
        if (res.isButton(getHelpBtn.btn)) {
            platformInteractive()->openUrl(configuration()->supportForumUrl());
        }
    });
}

void ProjectActionsController::warnProjectCannotBeOpened(const Ret& ret, const muse::io::path_t& filepath)
{
    std::string title = muse::mtrc("project", "Cannot read file %1").arg(io::toNativeSeparators(filepath).toString()).toStdString();
    std::string body;

    switch (ret.code()) {
    case int(engraving::Err::FileNotFound):
        body = muse::trc("project", "This file does not exist or cannot be accessed at the moment.");
        break;
    case int(engraving::Err::FileOpenError):
        body = muse::trc("project",
                         "This file could not be opened. Please make sure that MuseScore Studio has permission to read this file.");
        break;
    default:
        if (!ret.text().empty()) {
            body = ret.text();
        } else {
            body = muse::trc("project", "An error occurred while reading this file.");
        }
    }

    interactive()->error(title, body);
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

async::Promise<io::path_t> ProjectActionsController::selectScoreOpeningFile() const
{
    std::string allExt = "*.mscz *.mxl *.musicxml *.xml *.mid *.midi *.kar *.md *.mgu *.sgu *.cap *.capx "
                         "*.ove *.scw *.bmw *.bww *.gtp *.gp3 *.gp4 *.gp5 *.gpx *.gp *.ptb *.mei *.mnx *.json *.tef *.mscx *.mscs *.mscz~";

    std::vector<std::string> filter { muse::trc("project", "All supported files") + " (" + allExt + ")",
                                      muse::trc("project", "MuseScore files") + " (*.mscz)",
                                      muse::trc("project", "MusicXML files") + " (*.mxl *.musicxml *.xml)",
                                      muse::trc("project", "MIDI files") + " (*.mid *.midi *.kar)",
                                      muse::trc("project", "MNX files [experimental]") + " (*.mnx *.json)",
                                      muse::trc("project", "MuseData files") + " (*.md)",
                                      muse::trc("project", "Capella files") + " (*.cap *.capx)",
                                      muse::trc("project", "BB files [experimental]") + " (*.mgu *.sgu)",
                                      muse::trc("project", "Overture / Score Writer files [experimental]") + " (*.ove *.scw)",
                                      muse::trc("project", "Bagpipe Music Writer files [experimental]") + " (*.bmw *.bww)",
                                      muse::trc("project", "Guitar Pro files") + " (*.gtp *.gp3 *.gp4 *.gp5 *.gpx *.gp)",
                                      muse::trc("project", "Power Tab Editor files [experimental]") + " (*.ptb)",
                                      muse::trc("project", "MEI files") + " (*.mei)",
                                      muse::trc("project", "TablEdit files [experimental]") + " (*.tef)",
                                      muse::trc("project", "Uncompressed MuseScore folders [experimental]") + " (*.mscx)",
                                      muse::trc("project", "MuseScore developer files") + " (*.mscs)",
                                      muse::trc("project", "MuseScore backup files") + " (*.mscz~)" };

    muse::io::path_t defaultDir = configuration()->lastOpenedProjectsPath();

    if (defaultDir.empty()) {
        defaultDir = configuration()->userProjectsPath();
    }

    if (defaultDir.empty()) {
        defaultDir = configuration()->defaultUserProjectsPath();
    }

    return interactive()->selectOpeningFile(muse::trc("project", "Open"), defaultDir, filter);
}

muse::Ret ProjectActionsController::openProjectProperties()
{
    interactive()->open(PROJECT_PROPERTIES_URI);
    return make_ok();
}
