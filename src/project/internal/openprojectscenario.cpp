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

#include "openprojectscenario.h"

#include <QFile>
#include <QTimer>
#include <QUrlQuery>

#include "async/async.h"
#include "defer.h"
#include "translation.h"

#include "cloud/clouderrors.h"
#include "engraving/engravingerrors.h"
#include "engraving/infrastructure/mscio.h"

#include "notation/imasternotation.h"
#include "notation/inotation.h"

#include "inotationproject.h"
#include "projecterrors.h"

#include "log.h"

using namespace mu::project;
using namespace mu::notation;
using namespace muse;

static const muse::Uri NOTATION_PAGE_URI("musescore://notation");

static const QString MUSESCORE_URL_SCHEME("musescore");
static const QString OPEN_SCORE_URL_HOSTNAME("open-score");

bool OpenProjectScenario::isBusy(BusyStatus status) const
{
    return m_busyStatuses.contains(status);
}

void OpenProjectScenario::setBusy(BusyStatus status, bool isBusy)
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

muse::async::Notification OpenProjectScenario::busyChanged() const
{
    return m_busyChanged;
}

static std::string cloudStatusCodeErrorMessage(const Ret& ret, bool withHelp = false)
{
    std::string message;

    switch (ret.code()) {
    case int(cloud::Err::Status400_InvalidRequest):
        //: %1 will be replaced with the error code that MuseScore.com returned; this might contain english text
        //: that is deliberately not translated
        message = muse::qtrc("project/cloud", "MuseScore.com returned an error code: %1.")
                  .arg("400 Invalid request").toStdString();
        break;
    case int(cloud::Err::Status401_AuthorizationRequired):
        //: %1 will be replaced with the error code that MuseScore.com returned; this might contain english text
        //: that is deliberately not translated
        message = muse::qtrc("project/cloud", "MuseScore.com returned an error code: %1.")
                  .arg("401 Authorization required").toStdString();
        break;
    case int(cloud::Err::Status422_ValidationFailed):
        //: %1 will be replaced with the error code that MuseScore.com returned; this might contain english text
        //: that is deliberately not translated
        message = muse::qtrc("project/cloud", "MuseScore.com returned an error code: %1.")
                  .arg("422 Validation failed").toStdString();
        break;
    case int(cloud::Err::Status429_RateLimitExceeded):
        //: %1 will be replaced with the error code that MuseScore.com returned; this might contain english text
        //: that is deliberately not translated
        message = muse::qtrc("project/cloud", "MuseScore.com returned an error code: %1.")
                  .arg("429 Rate limit exceeded").toStdString();
        break;
    case int(cloud::Err::Status500_InternalServerError):
        //: %1 will be replaced with the error code that MuseScore.com returned; this might contain english text
        //: that is deliberately not translated
        message = muse::qtrc("project/cloud", "MuseScore.com returned an error code: %1.")
                  .arg("500 Internal server error").toStdString();
        break;
    case int(cloud::Err::UnknownStatusCode): {
        if (const auto status = ret.data<int>("status", -1); status != -1) {
            //: %1 will be replaced with the error code that MuseScore.com returned, which is a number.
            message = muse::qtrc("project/cloud", "MuseScore.com returned an unknown error code: %1.")
                      .arg(status).toStdString();
        } else {
            message = muse::trc("project/cloud", "MuseScore.com returned an unknown error code.");
        }
    } break;
    }

    if (withHelp) {
        message += "\n\n" + muse::trc("project/cloud", "Please try again later, or get help for this problem on MuseScore.com.");
    }

    return message;
}

void OpenProjectScenario::showCloudOpenError(const Ret& ret) const
{
    std::string title = muse::trc("project", "Your score could not be opened");
    std::string message;

    switch (ret.code()) {
    case int(Err::InvalidCloudScoreId):
        message = muse::trc("project", "This score is invalid.");
        break;
    case int(Err::FileOpenError):
        message = muse::trc("project/cloud", "The file could not be downloaded to your disk.");
        break;
    case int(cloud::Err::Status403_AccountNotActivated):
        message = muse::trc("project/cloud", "Your MuseScore.com account needs to be verified first. "
                                             "Please activate your account via the link in the activation email.");
        break;
    case int(cloud::Err::Status403_NotOwner):
        message = muse::trc("project/cloud", "This score does not belong to this account. To access this score, make sure you are logged in "
                                             "to the desktop app with the account to which this score belongs.");
        break;
    case int(cloud::Err::Status404_NotFound):
        message = muse::trc("project/cloud", "The score could not be found, or cannot be accessed by your account.");
        break;

    case int(cloud::Err::Status400_InvalidRequest):
    case int(cloud::Err::Status401_AuthorizationRequired):
    case int(cloud::Err::Status422_ValidationFailed):
    case int(cloud::Err::Status429_RateLimitExceeded):
    case int(cloud::Err::Status500_InternalServerError):
    case int(cloud::Err::UnknownStatusCode):
        message = cloudStatusCodeErrorMessage(ret);
        break;

    case int(cloud::Err::NetworkError):
        message = muse::mtrc("project/cloud", "Could not connect to <a href=\"%1\">MuseScore.com</a>. "
                                              "Please check your internet connection or try again later.")
                  .arg(u"https://musescore.com").toStdString();
        break;
    default:
        message = muse::trc("project/cloud", "Please try again later.");
        break;
    }

    interactive()->warning(title, message);
}

bool OpenProjectScenario::isUrlSupported(const QUrl& url) const
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

bool OpenProjectScenario::isFileSupported(const muse::io::path_t& path) const
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

muse::Ret OpenProjectScenario::openProject(const muse::rcommand::Params& params)
{
    const QString url = params.at("url").toQString();
    const QString displayNameOverride = params.at("display_name").toQString();

    Ret ret = openProject(ProjectFile(QUrl(url), displayNameOverride));
    if (!ret) {
        LOGE() << ret.toString();
    }

    return ret;
}

Ret OpenProjectScenario::openProject(const ProjectFile& file)
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

Ret OpenProjectScenario::openProject(const muse::io::path_t& givenPath, const QString& displayNameOverride)
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
        return finishOpening();
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
        showCloudOpenError(ret);
        return ret;
    }

    //! Step 6. Open project in the current window
    return doOpenProject(actualPath);
}

RetVal<INotationProjectPtr> OpenProjectScenario::loadProject(const muse::io::path_t& filePath)
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

Ret OpenProjectScenario::loadWithFallback(const std::shared_ptr<INotationProject>& project,
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

Ret OpenProjectScenario::doOpenProject(const muse::io::path_t& filePath)
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

    return finishOpening();
}

Ret OpenProjectScenario::doOpenCloudProject(const muse::io::path_t& filePath, const CloudProjectInfo& info, bool isOwner)
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

    return finishOpening();
}

muse::Ret OpenProjectScenario::doOpenCloudProjectOffline(const muse::io::path_t& filePath, const QString& displayNameOverride)
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

    return finishOpening();
}

Ret OpenProjectScenario::finishOpening()
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

void OpenProjectScenario::downloadAndOpenCloudProject(int scoreId, const QString& hash, const QString& secret, bool isOwner)
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
        showCloudOpenError(make_ret(Err::InvalidCloudScoreId));
        return;
    }

    Ret ret = ensureAuthorization().ret;
    if (!ret) {
        return;
    }

    CloudProjectInfo info;
    muse::io::path_t localPath = configuration()->cloudProjectPath(scoreId);

    if (isOwner) {
        RetVal<muse::cloud::ScoreInfo> scoreInfo = museScoreComService()->downloadScoreInfo(scoreId);
        if (!scoreInfo.ret) {
            LOGE() << "Error while downloading score info: " << scoreInfo.ret.toString();
            showCloudOpenError(scoreInfo.ret);
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
        showCloudOpenError(make_ret(Err::FileOpenError));
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
            showCloudOpenError(res.ret);
            return;
        }

        doOpenCloudProject(localPath, info, isOwner);
    });

    m_projectBeingDownloadedChanged.notify();
    isDownloadingFinished = false;
}

Ret OpenProjectScenario::openMuseScoreUrl(const QUrl& url)
{
    if (url.host() == OPEN_SCORE_URL_HOSTNAME) {
        return openScoreFromMuseScoreCom(url);
    }

    return make_ret(Err::UnsupportedUrl);
}

Ret OpenProjectScenario::openScoreFromMuseScoreCom(const QUrl& url)
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
    Ret ret = ensureAuthorization().ret;
    if (!ret) {
        return ret;
    }

    // Check if user is owner
    RetVal<muse::cloud::ScoreInfo> scoreInfo = museScoreComService()->downloadScoreInfo(scoreId);
    if (!scoreInfo.ret) {
        LOGE() << "Error while downloading score info: " << scoreInfo.ret.toString();
        showCloudOpenError(scoreInfo.ret);

        return scoreInfo.ret;
    }

    bool isOwner = QString::number(scoreInfo.val.owner.id) == museScoreComService()->authorization()->accountInfo().id;

    // If yes, score will be opened as regular cloud score; check if not yet opened
    if (isOwner) {
        muse::io::path_t projectPath = configuration()->cloudProjectPath(scoreId);

        // either in this instance
        if (isProjectOpened(projectPath)) {
            return finishOpening();
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

const ProjectBeingDownloaded& OpenProjectScenario::projectBeingDownloaded() const
{
    return m_projectBeingDownloaded;
}

muse::async::Notification OpenProjectScenario::projectBeingDownloadedChanged() const
{
    return m_projectBeingDownloadedChanged;
}

Ret OpenProjectScenario::openPageIfNeed(Uri pageUri)
{
    if (!interactive()->isOpened(pageUri).val) {
        interactive()->open(pageUri);
    }
    return make_ret(Ret::Code::Ok);
}

bool OpenProjectScenario::isProjectOpened(const muse::io::path_t& scorePath) const
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

RecentFile OpenProjectScenario::makeRecentFile(INotationProjectPtr project)
{
    RecentFile file;
    file.path = project->path();

    if (project->isCloudProject()) {
        file.displayNameOverride = project->cloudInfo().name;
    }

    return file;
}

void OpenProjectScenario::revertToLastSaved()
{
    TRACEFUNC;

    auto currentProject = globalContext()->currentProject();
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

bool OpenProjectScenario::shouldRetryLoadAfterError(const Ret& ret, const muse::io::path_t& filepath)
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

bool OpenProjectScenario::askIfUserAgreesToOpenProjectWithIncompatibleVersion(const std::string& errorText)
{
    IInteractive::ButtonData openAnywayBtn(IInteractive::Button::CustomButton, muse::trc("project", "Open anyway"), true /*accent*/);

    int btn = interactive()->warningSync(errorText, "", {
        interactive()->buttonData(IInteractive::Button::Cancel),
        openAnywayBtn
    }, openAnywayBtn.btn).button();

    return btn == openAnywayBtn.btn;
}

void OpenProjectScenario::warnFileTooNew(const muse::io::path_t& filepath)
{
    interactive()->error(muse::qtrc("project", "Cannot read file %1").arg(io::toNativeSeparators(filepath).toQString()).toStdString(),
                         muse::mtrc("project", "This file was saved using a newer version of MuseScore Studio. "
                                               "Please visit <a href=\"%1\">MuseScore.org</a> to obtain the latest version.")
                         .arg(u"https://musescore.org").toStdString());
}

bool OpenProjectScenario::askIfUserAgreesToOpenCorruptedProject(const String& projectName, const std::string& errorText)
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

void OpenProjectScenario::warnProjectCriticallyCorrupted(const String& projectName, const std::string& errorText)
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

void OpenProjectScenario::warnProjectCannotBeOpened(const Ret& ret, const muse::io::path_t& filepath)
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

async::Promise<io::path_t> OpenProjectScenario::selectScoreOpeningFile() const
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

muse::RetVal<Val> OpenProjectScenario::ensureAuthorization() const
{
    bool userAuthorized = museScoreComService()->authorization()->userAuthorized().val;

    if (userAuthorized) {
        return muse::make_ok();
    }

    const std::string dialogText = muse::trc("project/save", "Log in or create a free account on MuseScore.com to open this score.");

    UriQuery query("muse://cloud/requireauthorization");
    query.addParam("text", Val(dialogText));
    query.addParam("cloudCode", Val(muse::cloud::MUSESCORE_COM_CLOUD_CODE));
    return interactive()->openSync(query);
}
