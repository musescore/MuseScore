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
#include "projectactionscontroller.h"

#include <QBuffer>
#include <QEventLoop>
#include <QFileInfo>
#include <QTemporaryFile>
#include <QUrl>
#include <QUrlQuery>

#include "async/async.h"
#include "defer.h"
#include "translation.h"

#include "cloud/clouderrors.h"
#include "engraving/infrastructure/mscio.h"
#include "engraving/engravingerrors.h"
#include "network/networkerrors.h"
#include "projecterrors.h"

#include "log.h"

using namespace mu;
using namespace mu::project;
using namespace mu::notation;
using namespace mu::framework;
using namespace mu::actions;

static const mu::Uri NOTATION_PAGE_URI("musescore://notation");
static const mu::Uri HOME_PAGE_URI("musescore://home");
static const mu::Uri NEW_SCORE_URI("musescore://project/newscore");
static const mu::Uri PROJECT_PROPERTIES_URI("musescore://project/properties");
static const mu::Uri UPLOAD_PROGRESS_URI("musescore://project/upload/progress");

static const QString MUSESCORE_URL_SCHEME("musescore");
static const QString OPEN_SCORE_URL_HOSTNAME("open-score");

void ProjectActionsController::init()
{
    dispatcher()->reg(this, "file-new", this, &ProjectActionsController::newProject);
    dispatcher()->reg(this, "file-open", this, &ProjectActionsController::openProject);

    dispatcher()->reg(this, "file-close", [this]() {
        bool quitApp = multiInstancesProvider()->instances().size() > 1;
        closeOpenedProject(quitApp);
    });

    dispatcher()->reg(this, "file-save", [this]() { saveProject(SaveMode::Save); });
    dispatcher()->reg(this, "file-save-as", [this]() { saveProject(SaveMode::SaveAs); });
    dispatcher()->reg(this, "file-save-a-copy", [this]() { saveProject(SaveMode::SaveCopy); });
    dispatcher()->reg(this, "file-save-selection", [this]() { saveProject(SaveMode::SaveSelection, SaveLocationType::Local); });
    dispatcher()->reg(this, "file-save-to-cloud", [this]() { saveProject(SaveMode::SaveAs, SaveLocationType::Cloud); });

    dispatcher()->reg(this, "file-publish", this, &ProjectActionsController::publish);
    dispatcher()->reg(this, "file-share-audio", this, &ProjectActionsController::shareAudio);

    dispatcher()->reg(this, "file-export", this, &ProjectActionsController::exportScore);
    dispatcher()->reg(this, "file-import-pdf", this, &ProjectActionsController::importPdf);

    dispatcher()->reg(this, "print", this, &ProjectActionsController::printScore);

    dispatcher()->reg(this, "clear-recent", this, &ProjectActionsController::clearRecentScores);

    dispatcher()->reg(this, "continue-last-session", this, &ProjectActionsController::continueLastSession);

    dispatcher()->reg(this, "project-properties", this, &ProjectActionsController::openProjectProperties);
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
            "continue-last-session",
            "clear-recent",
        };

        return mu::contains(DONT_REQUIRE_OPEN_PROJECT, code);
    }

    if (m_isProjectUploading) {
        if (code == "file-save-to-cloud" || code == "file-publish") {
            return false;
        }
    }

    return true;
}

bool ProjectActionsController::isUrlSupported(const QUrl& url) const
{
    if (url.isLocalFile()) {
        return isFileSupported(io::path_t(url));
    }

    if (url.scheme() == MUSESCORE_URL_SCHEME) {
        if (url.host() == OPEN_SCORE_URL_HOSTNAME) {
            return true;
        }
    }

    return false;
}

bool ProjectActionsController::isFileSupported(const io::path_t& path) const
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

void ProjectActionsController::openProject(const actions::ActionData& args)
{
    QUrl url = !args.empty() ? args.arg<QUrl>(0) : QUrl();
    QString displayNameOverride = args.count() >= 2 ? args.arg<QString>(1) : QString();

    openProject(ProjectFile(url, displayNameOverride));
}

Ret ProjectActionsController::openProject(const ProjectFile& file)
{
    LOGI() << "Try open project: url = " << file.url.toString() << ", displayNameOverride = " << file.displayNameOverride;

    if (file.url.isLocalFile()) {
        return openProject(file.path(), file.displayNameOverride);
    }

    if (file.url.scheme() == MUSESCORE_URL_SCHEME) {
        return openMuseScoreUrl(file.url);
    }

    return make_ret(Err::UnsupportedUrl);
}

Ret ProjectActionsController::openProject(const io::path_t& givenPath, const QString& displayNameOverride)
{
    //! NOTE This method is synchronous,
    //! but inside `multiInstancesProvider` there can be an event loop
    //! to wait for the responses from other instances, accordingly,
    //! the events (like user click) can be executed and this method can be called several times,
    //! before the end of the current call.
    //! So we ignore all subsequent calls until the current one completes.
    if (m_isProjectProcessing || m_isProjectDownloading) {
        return make_ret(Ret::Code::InternalError);
    }
    m_isProjectProcessing = true;

    DEFER {
        m_isProjectProcessing = false;
    };

    //! Step 1. If no path is specified, ask the user to select a project
    io::path_t actualPath = fileSystem()->absoluteFilePath(givenPath);
    if (actualPath.empty()) {
        actualPath = selectScoreOpeningFile();

        if (actualPath.empty()) {
            return make_ret(Ret::Code::Cancel);
        }
    }

    //! Step 2. If the project is already open in the current window, then just switch to showing the notation
    if (isProjectOpened(actualPath)) {
        return openPageIfNeed(NOTATION_PAGE_URI);
    }

    //! Step 3. Check, if the project already opened in another window, then activate the window with the project
    if (multiInstancesProvider()->isProjectAlreadyOpened(actualPath)) {
        multiInstancesProvider()->activateWindowWithProject(actualPath);
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

        multiInstancesProvider()->openNewAppInstance(args);
        return make_ret(Ret::Code::Ok);
    }

    //! Step 5. If it's a cloud project, download the latest version
    if (configuration()->isCloudProject(actualPath) && !configuration()->isLegacyCloudProject(actualPath)) {
        downloadAndOpenCloudProject(configuration()->cloudScoreIdFromPath(actualPath));
        return make_ret(Ret::Code::Ok);
    }

    //! Step 6. Open project in the current window
    return doOpenProject(actualPath);
}

RetVal<INotationProjectPtr> ProjectActionsController::loadProject(const io::path_t& filePath)
{
    TRACEFUNC;

    auto project = projectCreator()->newProject();
    IF_ASSERT_FAILED(project) {
        return make_ret(Ret::Code::InternalError);
    }

    bool hasUnsavedChanges = projectAutoSaver()->projectHasUnsavedChanges(filePath);

    io::path_t loadPath = hasUnsavedChanges ? projectAutoSaver()->projectAutoSavePath(filePath) : filePath;
    std::string format = io::suffix(filePath);

    Ret ret = project->load(loadPath, "" /*stylePath*/, false /*forceMode*/, format);

    if (!ret) {
        if (ret.code() == static_cast<int>(Ret::Code::Cancel)) {
            return ret;
        }

        if (checkCanIgnoreError(ret, loadPath)) {
            ret = project->load(loadPath, "" /*stylePath*/, true /*forceMode*/, format);
        }

        if (!ret) {
            return ret;
        }
    }

    if (hasUnsavedChanges) {
        //! NOTE: redirect the project to the original file path
        project->setPath(filePath);

        project->markAsUnsaved();
    }

    bool isNewlyCreated = projectAutoSaver()->isAutosaveOfNewlyCreatedProject(filePath);
    if (isNewlyCreated) {
        project->markAsNewlyCreated();
    }

    return RetVal<INotationProjectPtr>::make_ok(project);
}

Ret ProjectActionsController::doOpenProject(const io::path_t& filePath)
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

    return openPageIfNeed(NOTATION_PAGE_URI);
}

Ret ProjectActionsController::doOpenCloudProject(const io::path_t& filePath, const CloudProjectInfo& info, bool isOwner)
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

    return openPageIfNeed(NOTATION_PAGE_URI);
}

void ProjectActionsController::downloadAndOpenCloudProject(int scoreId, const QString& hash, const QString& secret, bool isOwner)
{
    if (m_isProjectDownloading) {
        return;
    }
    m_isProjectDownloading = true;

    bool isDownloadingFinished = true;
    DEFER {
        if (isDownloadingFinished) {
            m_isProjectDownloading = false;
        }
    };

    if (!scoreId) {
        // Might happen when user tries to open score that saved as a cloud score but upload did not fully succeed
        LOGE() << "invalid cloud score id";
        openSaveProjectScenario()->showCloudOpenError(make_ret(Err::InvalidCloudScoreId));
        return;
    }

    Ret ret = museScoreComService()->authorization()->ensureAuthorization(
        trc("project/save", "Login or create a free account on musescore.com to open this score."));
    if (!ret) {
        return;
    }

    CloudProjectInfo info;

    if (isOwner) {
        RetVal<cloud::ScoreInfo> scoreInfo = museScoreComService()->downloadScoreInfo(scoreId);
        if (!scoreInfo.ret) {
            LOGE() << "Error while downloading score info: " << scoreInfo.ret.toString();
            openSaveProjectScenario()->showCloudOpenError(scoreInfo.ret);
            return;
        }

        info.name = scoreInfo.val.title;
        info.visibility = scoreInfo.val.visibility;
        info.sourceUrl = scoreInfo.val.url;
        info.revisionId = scoreInfo.val.revisionId;
    }

    // TODO(cloud): conflict checking (don't recklessly overwrite the existing file)
    io::path_t localPath = configuration()->cloudProjectPath(scoreId);
    QFile* projectData = new QFile(localPath.toQString());
    if (!projectData->open(QIODevice::WriteOnly)) {
        openSaveProjectScenario()->showCloudOpenError(make_ret(Err::FileOpenError));

        delete projectData;
        return;
    }

    m_projectBeingDownloaded.scoreId = scoreId;
    m_projectBeingDownloaded.progress = museScoreComService()->downloadScore(scoreId, *projectData, hash, secret);

    m_projectBeingDownloaded.progress->finished.onReceive(this, [this, localPath, info, isOwner, projectData](const ProgressResult& res) {
        projectData->deleteLater();

        m_projectBeingDownloaded = {};
        m_projectBeingDownloadedChanged.notify();

        m_isProjectDownloading = false;

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
    //! NOTE See explanation in `openProject(const io::path_t& _path, const QString& displayNameOverride)`
    if (m_isProjectProcessing || m_isProjectDownloading) {
        // TODO: instead of ignoring the open request, queue it?
        return make_ret(Ret::Code::InternalError);
    }
    m_isProjectProcessing = true;

    DEFER {
        m_isProjectProcessing = false;
    };

    // Retrieve score id from URL
    bool ok = false;
    int scoreId = url.fileName().toInt(&ok);
    if (!ok || scoreId <= 0) {
        return make_ret(Err::MalformedOpenScoreUrl);
    }

    // Ensure logged in
    Ret ret = museScoreComService()->authorization()->ensureAuthorization(
        trc("project/save", "Login or create a free account on musescore.com to open this score."));
    if (!ret) {
        return ret;
    }

    // Check if user is owner
    bool isOwner = true;
    RetVal<cloud::ScoreInfo> scoreInfo = museScoreComService()->downloadScoreInfo(scoreId);
    if (!scoreInfo.ret) {
        if (scoreInfo.ret.code() == int(cloud::Err::Status403_NotOwner)) {
            LOGI() << "Downloading score info returned 403, so not owner";
            isOwner = false;
        } else {
            LOGE() << "Error while downloading score info: " << scoreInfo.ret.toString();
            openSaveProjectScenario()->showCloudOpenError(scoreInfo.ret);

            return scoreInfo.ret;
        }
    }

    // If yes, score will be opened as regular cloud score; check if not yet opened
    if (isOwner) {
        io::path_t projectPath = configuration()->cloudProjectPath(scoreId);

        // either in this instance
        if (isProjectOpened(projectPath)) {
            return openPageIfNeed(NOTATION_PAGE_URI);
        }

        // or in another one
        if (multiInstancesProvider()->isProjectAlreadyOpened(projectPath)) {
            multiInstancesProvider()->activateWindowWithProject(projectPath);
            return make_ok();
        }
    }

    // Check if this instance already has an open project
    if (globalContext()->currentProject()) {
        QStringList args;
        args << url.toString();

        if (!scoreInfo.val.title.isEmpty()) {
            args << "--score-display-name-override" << scoreInfo.val.title;
        }

        multiInstancesProvider()->openNewAppInstance(args);
        return make_ok();
    }

    QUrlQuery query(url);
    QString hash = query.queryItemValue("h");
    QString secret = query.queryItemValue("secret");

    downloadAndOpenCloudProject(scoreId, hash, secret, isOwner);

    return make_ok();
}

const ProjectBeingDownloaded& ProjectActionsController::projectBeingDownloaded() const
{
    return m_projectBeingDownloaded;
}

async::Notification ProjectActionsController::projectBeingDownloadedChanged() const
{
    return m_projectBeingDownloadedChanged;
}

Ret ProjectActionsController::openPageIfNeed(Uri pageUri)
{
    if (interactive()->isOpened(pageUri).val) {
        return make_ret(Ret::Code::Ok);
    }

    return interactive()->open(pageUri).ret;
}

bool ProjectActionsController::isProjectOpened(const io::path_t& scorePath) const
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

void ProjectActionsController::newProject()
{
    //! NOTE This method is synchronous,
    //! but inside `multiInstancesProvider` there can be an event loop
    //! to wait for the responses from other instances, accordingly,
    //! the events (like user click) can be executed and this method can be called several times,
    //! before the end of the current call.
    //! So we ignore all subsequent calls until the current one completes.
    if (m_isProjectProcessing || m_isProjectDownloading) {
        return;
    }
    m_isProjectProcessing = true;

    DEFER {
        m_isProjectProcessing = false;
    };

    if (globalContext()->currentProject()) {
        //! Check, if any project is already open in the current window
        //! and there is already a created instance without a project, then activate it
        if (multiInstancesProvider()->isHasAppInstanceWithoutProject()) {
            multiInstancesProvider()->activateWindowWithoutProject();
            return;
        }

        //! Otherwise, we will create a new instance
        QStringList args;
        args << "--session-type" << "start-with-new";
        multiInstancesProvider()->openNewAppInstance(args);
        return;
    }

    Ret ret = interactive()->open(NEW_SCORE_URI).ret;

    if (ret) {
        ret = openPageIfNeed(NOTATION_PAGE_URI);
    }

    if (!ret) {
        LOGE() << ret.toString();
    }
}

bool ProjectActionsController::closeOpenedProject(bool quitApp)
{
    if (m_isProjectClosing) {
        return false;
    }

    m_isProjectClosing = true;
    DEFER {
        m_isProjectClosing = false;
    };

    INotationProjectPtr project = currentNotationProject();
    if (!project) {
        return true;
    }

    if (playbackController()->isPlaying()) {
        playbackController()->reset();
    }

    bool result = true;

    if (project->needSave().val) {
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
        interactive()->closeAllDialogs();
        globalContext()->setCurrentProject(nullptr);

        if (quitApp) {
            //! NOTE: we need to call `quit` in the next event loop due to controlling the lifecycle of this method
            async::Async::call(this, [this](){
                dispatcher()->dispatch("quit", actions::ActionData::make_arg1<bool>(false));
            });
        } else {
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
    std::string title = qtrc("project", "Do you want to save changes to the score “%1” before closing?")
                        .arg(project->displayName()).toStdString();

    std::string body = trc("project", "Your changes will be lost if you don’t save them.");

    IInteractive::Result result = interactive()->warning(title, body, {
        IInteractive::Button::DontSave,
        IInteractive::Button::Cancel,
        IInteractive::Button::Save
    }, IInteractive::Button::Save);

    return result.standardButton();
}

Ret ProjectActionsController::canSaveProject() const
{
    auto project = currentNotationProject();
    if (!project) {
        LOGW() << "no current project";
        return make_ret(Err::NoProjectError);
    }

    return project->canSave();
}

bool ProjectActionsController::saveProject(const io::path_t& path)
{
    if (!path.empty()) {
        if (m_isProjectSaving) {
            return false;
        }

        m_isProjectSaving = true;
        DEFER {
            m_isProjectSaving = false;
        };

        return saveProjectAt(SaveLocation(SaveLocationType::Local, path));
    }

    return saveProject(SaveMode::Save);
}

bool ProjectActionsController::saveProject(SaveMode saveMode, SaveLocationType saveLocationType, bool force)
{
    if (m_isProjectSaving) {
        return false;
    }

    m_isProjectSaving = true;
    DEFER {
        m_isProjectSaving = false;
    };

    INotationProjectPtr project = currentNotationProject();

    if (saveMode == SaveMode::Save && !project->isNewlyCreated()) {
        if (project->isCloudProject()) {
            return saveProjectAt(SaveLocation(SaveLocationType::Cloud, project->cloudInfo()));
        }

        return saveProjectAt(SaveLocation(SaveLocationType::Local));
    }

    RetVal<SaveLocation> response = openSaveProjectScenario()->askSaveLocation(project, saveMode, saveLocationType);
    if (!response.ret) {
        LOGE() << response.ret.toString();
        return false;
    }

    return saveProjectAt(response.val, saveMode, force);
}

void ProjectActionsController::publish()
{
    if (m_isProjectPublishing) {
        return;
    }

    m_isProjectPublishing = true;
    DEFER {
        m_isProjectPublishing = false;
    };

    Ret ret = canSaveProject();
    if (!ret) {
        askIfUserAgreesToSaveProjectWithErrors(ret, SaveLocationType::Cloud);
        return;
    }

    auto project = currentNotationProject();

    RetVal<CloudProjectInfo> info = openSaveProjectScenario()->askPublishLocation(project);
    if (!info.ret) {
        return;
    }

    AudioFile audio = exportMp3(project->masterNotation()->notation());
    if (audio.isValid()) {
        uploadProject(info.val, audio, /*openEditUrl=*/ true, /*publishMode=*/ true);
    }
}

void ProjectActionsController::shareAudio()
{
    if (m_isAudioSharing) {
        return;
    }

    m_isAudioSharing = true;

    bool isSharingFinished = true;
    DEFER {
        if (isSharingFinished) {
            m_isAudioSharing = false;
        }
    };

    auto project = currentNotationProject();
    RetVal<CloudAudioInfo> retVal = openSaveProjectScenario()->askShareAudioLocation(project);
    if (!retVal.ret) {
        return;
    }

    CloudAudioInfo cloudAudioInfo = retVal.val;

    AudioFile audio = exportMp3(project->masterNotation()->notation());
    if (!audio.isValid()) {
        return;
    }

    m_uploadingAudioProgress = audioComService()->uploadAudio(*audio.device, audio.format, cloudAudioInfo.name,
                                                              cloudAudioInfo.visibility);

    m_uploadingAudioProgress->started.onNotify(this, [this]() {
        LOGD() << "Uploading audio started";
        showUploadProgressDialog();
    });

    m_uploadingAudioProgress->progressChanged.onReceive(this, [](int64_t current, int64_t total, const std::string&) {
        if (total > 0) {
            LOGD() << "Uploading audio progress: " << current << " / " << total << " bytes";
        }
    });

    m_uploadingAudioProgress->finished.onReceive(this, [this, audio](const ProgressResult& res) {
        LOGD() << "Uploading audio finished";

        audio.device->deleteLater();

        if (!res.ret) {
            LOGE() << res.ret.toString();
            onAudioUploadFailed(res.ret);
        } else {
            onAudioSuccessfullyUploaded(res.val.toMap()["editUrl"].toQString());
        }
    });

    isSharingFinished = false;
}

bool ProjectActionsController::saveProjectAt(const SaveLocation& location, SaveMode saveMode, bool force)
{
    if (!force) {
        Ret ret = canSaveProject();
        if (!ret) {
            ret = askIfUserAgreesToSaveProjectWithErrors(ret, location);
            if (!ret) {
                return ret;
            }
        }
    }

    if (location.isLocal()) {
        return saveProjectLocally(location.localPath(), saveMode);
    }

    if (location.isCloud()) {
        return saveProjectToCloud(location.cloudInfo(), saveMode);
    }

    return false;
}

bool ProjectActionsController::saveProjectLocally(const io::path_t& filePath, SaveMode saveMode)
{
    INotationProjectPtr project = currentNotationProject();
    if (!project) {
        return false;
    }

    Ret ret = project->save(filePath, saveMode);
    if (!ret) {
        LOGE() << ret.toString();
        warnScoreCouldnotBeSaved(ret);
        return false;
    }

    recentFilesController()->prependRecentFile(makeRecentFile(project));
    return true;
}

bool ProjectActionsController::saveProjectToCloud(CloudProjectInfo info, SaveMode saveMode)
{
    if (m_isProjectUploading) {
        return true;
    }

    m_isProjectUploading = true;

    DEFER {
        m_isProjectUploading = false;
    };

    bool isCloudAvailable = museScoreComService()->authorization()->checkCloudIsAvailable();
    if (!isCloudAvailable) {
        warnCloudIsNotAvailable();
    } else {
        Ret ret = museScoreComService()->authorization()->ensureAuthorization(
            trc("project/save", "Login or create a free account on musescore.com to save this score to the cloud."));
        if (!ret) {
            return false;
        }
    }

    INotationProjectPtr project = currentNotationProject();
    if (!project) {
        return false;
    }

    bool isPublic = info.visibility == cloud::Visibility::Public;
    bool generateAudio = false;

    if (saveMode == SaveMode::Save && isCloudAvailable) {
        // Get up-to-date visibility information
        RetVal<cloud::ScoreInfo> scoreInfo = museScoreComService()->downloadScoreInfo(info.sourceUrl);
        if (scoreInfo.ret) {
            info.name = scoreInfo.val.title;
            info.visibility = scoreInfo.val.visibility;
            isPublic = info.visibility == cloud::Visibility::Public;
        } else {
            LOGE() << "Failed to download up-to-date score info for " << info.sourceUrl
                   << "; falling back to last known name and visibility setting, namely "
                   << info.name << " and " << static_cast<int>(info.visibility);
        }

        if (isPublic) {
            if (!openSaveProjectScenario()->warnBeforeSavingToExistingPubliclyVisibleCloudProject()) {
                return false;
            }
        }
    }

    if (isCloudAvailable) {
        RetVal<bool> need = needGenerateAudio(isPublic);
        if (!need.ret) {
            return false;
        }

        generateAudio = need.val;
    }

    // TODO(cloud): is this correct for all save modes?
    project->setCloudInfo(info);

    io::path_t savingPath;

    if (project->isCloudProject()) {
        if (saveMode == SaveMode::Save || saveMode == SaveMode::AutoSave) {
            savingPath = project->path();
        }
    }

    if (savingPath.empty()) {
        int scoreId = cloud::scoreIdFromSourceUrl(info.sourceUrl);

        savingPath = configuration()->cloudProjectSavingPath(scoreId);
    }

    if (!saveProjectLocally(savingPath, saveMode)) {
        return false;
    }

    if (!isCloudAvailable) {
        return true;
    }

    AudioFile audio;

    if (generateAudio) {
        audio = exportMp3(project->masterNotation()->notation());
        if (!audio.isValid()) {
            return false;
        }
    }

    Ret ret = uploadProject(info, audio, /*openEditUrl=*/ isPublic, /*publishMode=*/ false);

    m_numberOfSavesToCloud++;

    return ret;
}

Ret ProjectActionsController::askAudioGenerationSettings() const
{
    RetVal<Val> res = interactive()->open("musescore://project/audiogenerationsettings");
    if (!res.ret) {
        return res.ret;
    }

    configuration()->setHasAskedAudioGenerationSettings(true);

    return make_ok();
}

RetVal<bool> ProjectActionsController::needGenerateAudio(bool isPublicUpload) const
{
    if (isPublicUpload) {
        return RetVal<bool>::make_ok(true);
    }

    if (!configuration()->hasAskedAudioGenerationSettings()) {
        Ret ret = askAudioGenerationSettings();
        if (!ret) {
            return ret;
        }
    }

    switch (configuration()->generateAudioTimePeriodType()) {
    case GenerateAudioTimePeriodType::Never:
        return RetVal<bool>::make_ok(false);
    case GenerateAudioTimePeriodType::Always:
        return RetVal<bool>::make_ok(true);
    case GenerateAudioTimePeriodType::AfterCertainNumberOfSaves: {
        int requiredNumberOfSaves = configuration()->numberOfSavesToGenerateAudio();
        return RetVal<bool>::make_ok(m_numberOfSavesToCloud % requiredNumberOfSaves == 0);
    }
    }

    return RetVal<bool>::make_ok(false);
}

ProjectActionsController::AudioFile ProjectActionsController::exportMp3(const INotationPtr notation) const
{
    QTemporaryFile* tempFile = new QTemporaryFile(configuration()->temporaryMp3FilePathTemplate().toQString());
    if (!tempFile->open()) {
        LOGE() << "Could not open a temp file";
        delete tempFile;
        return AudioFile();
    }

    QString mp3Path = QFileInfo(*tempFile).absoluteFilePath();
    LOGD() << "mp3 path: " << mp3Path;

    if (mp3Path.isEmpty()) {
        LOGE() << "mp3 path is empty";
        delete tempFile;
        return AudioFile();
    }

    // In the uploaded audio file, the repeats need to be expanded
    bool wasExpandRepeats = notationConfiguration()->isPlayRepeatsEnabled();
    if (!wasExpandRepeats) {
        notationConfiguration()->setIsPlayRepeatsEnabled(true);
    }

    DEFER {
        if (!wasExpandRepeats) {
            notationConfiguration()->setIsPlayRepeatsEnabled(false);
        }
    };

    if (!exportProjectScenario()->exportScores({ notation }, mp3Path)) {
        LOGE() << "Could not export an mp3";
        delete tempFile;
        return AudioFile();
    }

    AudioFile audio;
    audio.format = "mp3";
    audio.device = tempFile;
    audio.device->seek(0);

    return audio;
}

void ProjectActionsController::showUploadProgressDialog()
{
    if (interactive()->isOpened(UPLOAD_PROGRESS_URI).val) {
        return;
    }

    UriQuery uriQuery(UPLOAD_PROGRESS_URI);
    uriQuery.addParam("sync", Val(false));
    interactive()->open(uriQuery);
}

void ProjectActionsController::closeUploadProgressDialog()
{
    if (interactive()->isOpened(UPLOAD_PROGRESS_URI).val) {
        interactive()->close(UPLOAD_PROGRESS_URI);
    }
}

Ret ProjectActionsController::uploadProject(const CloudProjectInfo& info, const AudioFile& audio, bool openEditUrl, bool publishMode)
{
    INotationProjectPtr project = globalContext()->currentProject();
    if (!project) {
        return false;
    }

    QBuffer* projectData = new QBuffer();
    projectData->open(QIODevice::WriteOnly);

    Ret ret = project->writeToDevice(projectData);
    if (!ret) {
        LOGE() << ret.toString();
        delete projectData;
        return ret;
    }

    projectData->close();
    projectData->open(QIODevice::ReadOnly);

    bool isFirstSave = info.sourceUrl.isEmpty();

    // The method must not return until the saving is complete, to prevent the app from being quit prematurely
    QEventLoop eventLoop;

    m_uploadingProjectProgress = museScoreComService()->uploadScore(*projectData, info.name, info.visibility, info.sourceUrl,
                                                                    info.revisionId);

    m_uploadingProjectProgress->started.onNotify(this, [this]() {
        showUploadProgressDialog();
        LOGD() << "Uploading project started";
    });

    m_uploadingProjectProgress->progressChanged.onReceive(this, [](int64_t current, int64_t total, const std::string&) {
        if (total > 0) {
            LOGD() << "Uploading project progress: " << current << " / " << total << " bytes";
        }
    });

    m_uploadingProjectProgress->finished.onReceive(this, [this, project, projectData, info, audio, openEditUrl, publishMode,
                                                          isFirstSave, &ret, &eventLoop](const ProgressResult& res) {
        DEFER {
            eventLoop.quit();
        };

        projectData->deleteLater();

        ret = res.ret;

        if (!res.ret) {
            LOGE() << res.ret.toString();
            ret = onProjectUploadFailed(res.ret, info, audio, openEditUrl, publishMode);
            return;
        }

        ValMap urlMap = res.val.toMap();
        QString newSourceUrl = urlMap["sourceUrl"].toQString();
        QString editUrl = openEditUrl ? urlMap["editUrl"].toQString() : QString();
        int newRevisionId = urlMap["revisionId"].toInt();

        LOGD() << "Source url received: " << newSourceUrl;

        CloudProjectInfo cpinfo = project->cloudInfo();
        if (cpinfo.sourceUrl != newSourceUrl || cpinfo.revisionId != newRevisionId) {
            // TODO(cloud): does this work correctly with different save modes?
            cpinfo.sourceUrl = newSourceUrl;
            cpinfo.revisionId = newRevisionId;
            project->setCloudInfo(cpinfo);

            if (!project->isNewlyCreated()) {
                project->save();
            }

            if (project->isCloudProject()) {
                moveProject(project, configuration()->cloudProjectPath(cloud::scoreIdFromSourceUrl(cpinfo.sourceUrl)), true);
            }
        }

        if (audio.isValid()) {
            uploadAudio(audio, newSourceUrl, editUrl, isFirstSave);
        } else {
            onProjectSuccessfullyUploaded(editUrl, isFirstSave);
        }
    });

    eventLoop.exec();

    return ret;
}

void ProjectActionsController::uploadAudio(const AudioFile& audio, const QUrl& sourceUrl, const QUrl& urlToOpen, bool isFirstSave)
{
    m_uploadingAudioProgress = museScoreComService()->uploadAudio(*audio.device, audio.format, sourceUrl);

    m_uploadingAudioProgress->started.onNotify(this, []() {
        LOGD() << "Uploading audio started";
    });

    m_uploadingAudioProgress->progressChanged.onReceive(this, [](int64_t current, int64_t total, const std::string&) {
        if (total > 0) {
            LOGD() << "Uploading audio progress: " << current << " / " << total << " bytes";
        }
    });

    m_uploadingAudioProgress->finished.onReceive(this, [this, audio, urlToOpen, isFirstSave](const ProgressResult& res) {
        LOGD() << "Uploading audio finished";

        audio.device->deleteLater();

        if (!res.ret) {
            LOGE() << res.ret.toString();
        }

        onProjectSuccessfullyUploaded(urlToOpen, isFirstSave);
    });
}

void ProjectActionsController::onProjectSuccessfullyUploaded(const QUrl& urlToOpen, bool isFirstSave)
{
    m_isProjectUploading = false;

    closeUploadProgressDialog();

    if (!urlToOpen.isEmpty()) {
        interactive()->openUrl(urlToOpen);
        return;
    }

    QUrl scoreManagerUrl = this->scoreManagerUrl();

    if (configuration()->openDetailedProjectUploadedDialog()) {
        UriQuery query("musescore://project/upload/success");
        query.addParam("scoreManagerUrl", Val(scoreManagerUrl.toString()));
        interactive()->open(query);
        configuration()->setOpenDetailedProjectUploadedDialog(false);
        return;
    }

    if (!isFirstSave) {
        return;
    }

    IInteractive::ButtonData viewOnlineBtn(IInteractive::Button::CustomButton, trc("project/save", "View online"));
    IInteractive::ButtonData okBtn = interactive()->buttonData(IInteractive::Button::Ok);

    std::string msg = trc("project/save", "All saved changes will now update to the cloud. "
                                          "You can manage this file in the score manager on musescore.com.");

    int btn = interactive()->info(trc("global", "Success!"), msg, { viewOnlineBtn, okBtn },
                                  static_cast<int>(IInteractive::Button::Ok)).button();

    if (btn == viewOnlineBtn.btn) {
        interactive()->openUrl(scoreManagerUrl);
    }
}

Ret ProjectActionsController::onProjectUploadFailed(const Ret& ret, const CloudProjectInfo& info, const AudioFile& audio, bool openEditUrl,
                                                    bool publishMode)
{
    m_isProjectUploading = false;

    closeUploadProgressDialog();

    Ret userResponse = openSaveProjectScenario()->showCloudSaveError(ret, info, publishMode, true);
    switch (userResponse.code()) {
    case IOpenSaveProjectScenario::RET_CODE_CONFLICT_RESPONSE_SAVE_AS: {
        return saveProject(SaveMode::SaveAs);
    }
    case IOpenSaveProjectScenario::RET_CODE_CONFLICT_RESPONSE_PUBLISH_AS_NEW_SCORE: {
        CloudProjectInfo newInfo = info;
        newInfo.sourceUrl = QUrl();
        return uploadProject(newInfo, audio, openEditUrl, publishMode);
    }
    case IOpenSaveProjectScenario::RET_CODE_CONFLICT_RESPONSE_REPLACE: {
        RetVal<cloud::ScoreInfo> scoreInfo = museScoreComService()->downloadScoreInfo(info.sourceUrl);
        if (!scoreInfo.ret) {
            LOGE() << scoreInfo.ret.toString();
            openSaveProjectScenario()->showCloudSaveError(scoreInfo.ret, info, publishMode, false);
            break;
        }

        int cloudRevisionId = scoreInfo.val.revisionId;
        CloudProjectInfo newInfo = info;
        newInfo.revisionId = cloudRevisionId;
        return uploadProject(newInfo, audio, openEditUrl, publishMode);
    }
    default:
        break;
    }

    return ret;
}

void ProjectActionsController::onAudioSuccessfullyUploaded(const QUrl& urlToOpen)
{
    m_isAudioSharing = false;

    closeUploadProgressDialog();

    interactive()->openUrl(urlToOpen);
}

void ProjectActionsController::onAudioUploadFailed(const Ret& ret)
{
    m_isAudioSharing = false;

    closeUploadProgressDialog();

    openSaveProjectScenario()->showAudioCloudShareError(ret);
}

void ProjectActionsController::warnCloudIsNotAvailable()
{
    closeUploadProgressDialog();

    if (!configuration()->showCloudIsNotAvailableWarning()) {
        return;
    }

    std::string title = trc("project/save", "Unable to connect to the cloud");
    std::string msg = trc("project/save", "Your changes will be saved to a local file until the connection resumes.");

    IInteractive::Result result = interactive()->warning(title, msg,
                                                         { IInteractive::Button::Ok }, IInteractive::Button::Ok,
                                                         IInteractive::Option::WithIcon | IInteractive::Option::WithDontShowAgainCheckBox);

    configuration()->setShowCloudIsNotAvailableWarning(result.showAgain());
}

bool ProjectActionsController::askIfUserAgreesToSaveProjectWithErrors(const Ret& ret, const SaveLocation& location)
{
    switch (static_cast<Err>(ret.code())) {
    case Err::NoPartsError:
        warnScoreCouldnotBeSaved(trc("project/save", "Please add at least one instrument to enable saving."));
        return false;
    case Err::CorruptionUponOpenningError:
        return askIfUserAgreesToSaveCorruptedScoreUponOpenning(location, ret.text());
    case Err::CorruptionError: {
        auto project = currentNotationProject();
        return askIfUserAgreesToSaveCorruptedScore(location, ret.text(), project->isNewlyCreated());
    }
    default:
        return false;
    }
}

bool ProjectActionsController::askIfUserAgreesToSaveCorruptedScore(const SaveLocation& location, const std::string& errorText,
                                                                   bool newlyCreated)
{
    switch (location.type) {
    case SaveLocationType::Cloud: {
        if (newlyCreated) {
            showErrCorruptedScoreCannotBeSaved(location, errorText);
        } else {
            warnCorruptedScoreCannotBeSavedOnCloud(errorText, !newlyCreated);
        }

        return false;
    }
    case SaveLocationType::Local:
        return askIfUserAgreesToSaveCorruptedScoreLocally(errorText, !newlyCreated);
    case SaveLocationType::Undefined: // fallthrough
    default:
        return false;
    }
}

void ProjectActionsController::warnCorruptedScoreCannotBeSavedOnCloud(const std::string& errorText, bool canRevert)
{
    std::string title = trc("project", "Your score cannot be uploaded to the cloud");
    std::string body = trc("project", "This score has become corrupted and contains errors. "
                                      "You can fix the errors manually, or save the score to your computer "
                                      "and get help for this issue on musescore.org.");

    IInteractive::ButtonDatas buttons;
    buttons.push_back(interactive()->buttonData(IInteractive::Button::Cancel));

    IInteractive::ButtonData saveCopyBtn(IInteractive::Button::CustomButton, trc("project", "Save as…"), !canRevert /*accent*/);
    buttons.push_back(saveCopyBtn);

    int defaultBtn = saveCopyBtn.btn;

    IInteractive::ButtonData revertToLastSavedBtn(saveCopyBtn.btn + 1, trc("project", "Revert to last saved"),
                                                  true /*accent*/);

    if (canRevert) {
        buttons.push_back(revertToLastSavedBtn);
        defaultBtn = revertToLastSavedBtn.btn;
    }

    int btn = interactive()->error(title, body, errorText, buttons, defaultBtn).button();

    if (btn == saveCopyBtn.btn) {
        m_isProjectSaving = false;
        saveProject(SaveMode::SaveAs, SaveLocationType::Local, true /*force*/);
    } else if (btn == revertToLastSavedBtn.btn) {
        revertCorruptedScoreToLastSaved();
    }
}

bool ProjectActionsController::askIfUserAgreesToSaveCorruptedScoreLocally(const std::string& errorText,
                                                                          bool canRevert)
{
    std::string title = trc("project", "This score has become corrupted and contains errors");
    std::string body = !canRevert ? trc("project", "You can continue saving it locally, although the file may become unusable. "
                                                   "You can try to fix the errors manually, or get help for this issue on musescore.org.")
                       : trc("project", "You can continue saving it locally, although the file may become unusable. "
                                        "To preserve your score, revert to the last saved version, or fix the errors manually. "
                                        "You can also get help for this issue on musescore.org.");

    IInteractive::ButtonDatas buttons;
    buttons.push_back(interactive()->buttonData(IInteractive::Button::Cancel));

    IInteractive::ButtonData saveAnywayBtn(IInteractive::Button::CustomButton, trc("project", "Save anyway"), !canRevert /*accent*/);
    buttons.push_back(saveAnywayBtn);

    int defaultBtn = saveAnywayBtn.btn;

    IInteractive::ButtonData revertToLastSavedBtn(saveAnywayBtn.btn + 1, trc("project", "Revert to last saved"),
                                                  true /*accent*/);
    if (canRevert) {
        buttons.push_back(revertToLastSavedBtn);
        defaultBtn = revertToLastSavedBtn.btn;
    }

    int btn = interactive()->error(title, body, errorText, buttons, defaultBtn).button();

    if (btn == revertToLastSavedBtn.btn) {
        revertCorruptedScoreToLastSaved();
    }

    return btn == saveAnywayBtn.btn;
}

bool ProjectActionsController::askIfUserAgreesToSaveCorruptedScoreUponOpenning(const SaveLocation& location, const std::string& errorText)
{
    switch (location.type) {
    case SaveLocationType::Cloud:
        showErrCorruptedScoreCannotBeSaved(location, errorText);
        return false;
    case SaveLocationType::Local:
        return askIfUserAgreesToSaveCorruptedScoreLocally(errorText, false /*canRevert*/);
    case SaveLocationType::Undefined: // fallthrough
    default:
        return false;
    }
}

void ProjectActionsController::showErrCorruptedScoreCannotBeSaved(const SaveLocation& location, const std::string& errorText)
{
    std::string title = location.isLocal() ? trc("project", "Your score cannot be saved")
                        : trc("project", "Your score cannot be uploaded to the cloud");
    std::string body = trc("project", "This score is corrupted. You can get help for this issue on musescore.org.");

    IInteractive::ButtonData getHelpBtn(IInteractive::Button::CustomButton, trc("project", "Get help"));

    int btn = interactive()->error(title, body, errorText, {
        getHelpBtn,
        interactive()->buttonData(IInteractive::Button::Ok)
    }).button();

    if (btn == getHelpBtn.btn) {
        interactive()->openUrl(configuration()->supportForumUrl());
    }
}

void ProjectActionsController::warnScoreCouldnotBeSaved(const Ret& ret)
{
    std::string message = ret.text();
    if (message.empty()) {
        message = trc("project/save", "An unknown error occurred while saving this file.");
    }

    warnScoreCouldnotBeSaved(message);
}

void ProjectActionsController::warnScoreCouldnotBeSaved(const std::string& errorText)
{
    interactive()->warning(trc("project/save", "Your score could not be saved"), errorText);
}

void ProjectActionsController::revertCorruptedScoreToLastSaved()
{
    TRACEFUNC;

    std::string title = trc("project", "Revert to last saved?");
    std::string body = trc("project", "Your changes will be lost. This action cannot be undone.");

    int btn = interactive()->warning(title, body, {
        { IInteractive::Button::No, IInteractive::Button::Yes }
    }, IInteractive::Button::Yes, IInteractive::Option::WithIcon).button();

    if (btn == static_cast<int>(IInteractive::Button::No)) {
        return;
    }

    auto currentProject = currentNotationProject();
    io::path_t filePath = currentProject->path();

    bool hasUnsavedChanges = projectAutoSaver()->projectHasUnsavedChanges(filePath);
    if (hasUnsavedChanges) {
        io::path_t autoSavePath = projectAutoSaver()->projectAutoSavePath(filePath);
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

void ProjectActionsController::moveProject(INotationProjectPtr project, const io::path_t& newPath, bool replace)
{
    io::path_t oldPath = project->path();
    if (oldPath == newPath) {
        return;
    }

    fileSystem()->move(oldPath, newPath, replace);
    project->setPath(newPath);

    recentFilesController()->moveRecentFile(oldPath, makeRecentFile(project));
}

bool ProjectActionsController::checkCanIgnoreError(const Ret& ret, const io::path_t& filepath)
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
        return false;
    case engraving::Err::FileCorrupted:
        return askIfUserAgreesToOpenCorruptedProject(io::filename(filepath).toString(), ret.text());
    case engraving::Err::FileCriticallyCorrupted:
        warnProjectCriticallyCorrupted(io::filename(filepath).toString(), ret.text());
        return false;
    default:
        break;
    }

    warnProjectCannotBeOpened(ret, filepath);
    return false;
}

bool ProjectActionsController::askIfUserAgreesToOpenProjectWithIncompatibleVersion(const std::string& errorText)
{
    IInteractive::ButtonData openAnywayBtn(IInteractive::Button::CustomButton, trc("project", "Open anyway"), true /*accent*/);

    int btn = interactive()->warning(errorText, "", {
        interactive()->buttonData(IInteractive::Button::Cancel),
        openAnywayBtn
    }, openAnywayBtn.btn).button();

    return btn == openAnywayBtn.btn;
}

void ProjectActionsController::warnFileTooNew(const io::path_t& filepath)
{
    interactive()->error(qtrc("project", "Cannot read file %1").arg(io::toNativeSeparators(filepath).toQString()).toStdString(),
                         trc("project", "This file was saved using a newer version of MuseScore. "
                                        "Please visit <a href=\"https://musescore.org\">musescore.org</a> to obtain the latest version."));
}

bool ProjectActionsController::askIfUserAgreesToOpenCorruptedProject(const String& projectName, const std::string& errorText)
{
    std::string title = mtrc("project", "File “%1” is corrupted").arg(projectName).toStdString();
    std::string body = trc("project", "This file contains errors that could cause MuseScore to malfunction.");

    IInteractive::ButtonData openAnywayBtn(IInteractive::Button::CustomButton, trc("project", "Open anyway"), true /*accent*/);

    int btn = interactive()->warning(title, body, errorText, {
        interactive()->buttonData(IInteractive::Button::Cancel),
        openAnywayBtn
    }, openAnywayBtn.btn).button();

    return btn == openAnywayBtn.btn;
}

void ProjectActionsController::warnProjectCriticallyCorrupted(const String& projectName, const std::string& errorText)
{
    std::string title = mtrc("project", "File “%1” is corrupted and cannot be opened").arg(projectName).toStdString();
    std::string body = trc("project", "Get help for this issue on musescore.org.");

    IInteractive::ButtonData getHelpBtn(IInteractive::Button::CustomButton, trc("project", "Get help"), true /*accent*/);

    int btn = interactive()->error(title, body, errorText, {
        interactive()->buttonData(IInteractive::Button::Cancel),
        getHelpBtn
    }, getHelpBtn.btn).button();

    if (btn == getHelpBtn.btn) {
        interactive()->openUrl(configuration()->supportForumUrl());
    }
}

void ProjectActionsController::warnProjectCannotBeOpened(const Ret& ret, const io::path_t& filepath)
{
    std::string title = mtrc("project", "Cannot read file %1").arg(io::toNativeSeparators(filepath).toString()).toStdString();
    std::string body;

    switch (ret.code()) {
    case int(engraving::Err::FileNotFound):
        body = trc("project", "This file does not exist or cannot be accessed at the moment.");
        break;
    case int(engraving::Err::FileOpenError):
        body = trc("project", "This file could not be opened. Please make sure that MuseScore has permission to read this file.");
        break;
    default:
        if (!ret.text().empty()) {
            body = ret.text();
        } else {
            body = trc("project", "An error occurred while reading this file.");
        }
    }

    interactive()->error(title, body);
}

void ProjectActionsController::importPdf()
{
    interactive()->openUrl("https://musescore.com/import");
}

void ProjectActionsController::clearRecentScores()
{
    recentFilesController()->clearRecentFiles();
}

void ProjectActionsController::continueLastSession()
{
    const RecentFilesList& recentScorePaths = recentFilesController()->recentFilesList();

    if (recentScorePaths.empty()) {
        Ret ret = openPageIfNeed(HOME_PAGE_URI);
        if (!ret) {
            LOGE() << ret.toString();
        }
        return;
    }

    io::path_t lastScorePath = recentScorePaths.front().path;
    openProject(lastScorePath);
}

void ProjectActionsController::exportScore()
{
    interactive()->open("musescore://project/export");
}

void ProjectActionsController::printScore()
{
    INotationPtr notation = globalContext()->currentNotation();
    if (!notation) {
        return;
    }

    printProvider()->printNotation(notation);
}

io::path_t ProjectActionsController::selectScoreOpeningFile()
{
    std::string allExt = "*.mscz *.mxl *.musicxml *.xml *.mid *.midi *.kar *.md *.mgu *.sgu *.cap *.capx "
                         "*.ove *.scw *.bmw *.bww *.gtp *.gp3 *.gp4 *.gp5 *.gpx *.gp *.ptb *.mei *.mscx *.mscs *.mscz~";

    std::vector<std::string> filter { trc("project", "All supported files") + " (" + allExt + ")",
                                      trc("project", "MuseScore files") + " (*.mscz)",
                                      trc("project", "MusicXML files") + " (*.mxl *.musicxml *.xml)",
                                      trc("project", "MIDI files") + " (*.mid *.midi *.kar)",
                                      trc("project", "MuseData files") + " (*.md)",
                                      trc("project", "Capella files") + " (*.cap *.capx)",
                                      trc("project", "BB files (experimental)") + " (*.mgu *.sgu)",
                                      trc("project", "Overture / Score Writer files (experimental)") + " (*.ove *.scw)",
                                      trc("project", "Bagpipe Music Writer files (experimental)") + " (*.bmw *.bww)",
                                      trc("project", "Guitar Pro files") + " (*.gtp *.gp3 *.gp4 *.gp5 *.gpx *.gp)",
                                      trc("project", "Power Tab Editor files (experimental)") + " (*.ptb)",
                                      trc("project", "MEI files") + " (*.mei)",
                                      trc("project", "Uncompressed MuseScore folders (experimental)") + " (*.mscx)",
                                      trc("project", "MuseScore developer files") + " (*.mscs)",
                                      trc("project", "MuseScore backup files") + " (*.mscz~)" };

    io::path_t defaultDir = configuration()->lastOpenedProjectsPath();

    if (defaultDir.empty()) {
        defaultDir = configuration()->userProjectsPath();
    }

    if (defaultDir.empty()) {
        defaultDir = configuration()->defaultUserProjectsPath();
    }

    io::path_t filePath = interactive()->selectOpeningFile(qtrc("project", "Open"), defaultDir, filter);

    if (!filePath.empty()) {
        configuration()->setLastOpenedProjectsPath(io::dirpath(filePath));
    }

    return filePath;
}

bool ProjectActionsController::hasSelection() const
{
    return currentNotationSelection() ? !currentNotationSelection()->isNone() : false;
}

QUrl ProjectActionsController::scoreManagerUrl() const
{
    return museScoreComService()->scoreManagerUrl();
}

void ProjectActionsController::openProjectProperties()
{
    interactive()->open(PROJECT_PROPERTIES_URI);
}
