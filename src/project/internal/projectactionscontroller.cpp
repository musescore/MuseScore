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
#include <QTemporaryFile>
#include <QFileInfo>

#include "translation.h"
#include "defer.h"
#include "notation/notationerrors.h"
#include "projectconfiguration.h"
#include "engraving/infrastructure/mscio.h"
#include "engraving/engravingerrors.h"
#include "cloud/clouderrors.h"

#include "log.h"

using namespace mu;
using namespace mu::project;
using namespace mu::notation;
using namespace mu::framework;
using namespace mu::actions;

static mu::Uri NOTATION_PAGE_URI("musescore://notation");
static mu::Uri HOME_PAGE_URI("musescore://home");
static mu::Uri NEW_SCORE_URI("musescore://project/newscore");
static mu::Uri PROJECT_PROPERTIES_URI("musescore://project/properties");
static mu::Uri UPLOAD_PROGRESS_URI("musescore://project/upload/progress");

void ProjectActionsController::init()
{
    dispatcher()->reg(this, "file-open", this, &ProjectActionsController::openProject);
    dispatcher()->reg(this, "file-new", this, &ProjectActionsController::newProject);

    dispatcher()->reg(this, "file-close", [this]() {
        bool quitApp = multiInstancesProvider()->instances().size() > 1;
        closeOpenedProject(quitApp);
    });

    dispatcher()->reg(this, "file-save", [this]() { saveProject(); });
    dispatcher()->reg(this, "file-save-as", this, &ProjectActionsController::saveProjectAs);
    dispatcher()->reg(this, "file-save-a-copy", this, &ProjectActionsController::saveProjectCopy);
    dispatcher()->reg(this, "file-save-selection", this, &ProjectActionsController::saveSelection);
    dispatcher()->reg(this, "file-save-to-cloud", this, &ProjectActionsController::saveToCloud);
    dispatcher()->reg(this, "file-publish", this, &ProjectActionsController::publish);

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

bool ProjectActionsController::canReceiveAction(const actions::ActionCode& code) const
{
    if (m_isProjectUploading) {
        if (code == "file-save-to-cloud" || code == "file-publish") {
            return false;
        }
    }

    return true;
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
    io::path_t projectPath = !args.empty() ? args.arg<io::path_t>(0) : "";
    openProject(projectPath);
}

Ret ProjectActionsController::openProject(const io::path_t& projectPath_)
{
    //! NOTE This method is synchronous,
    //! but inside `multiInstancesProvider` there can be an event loop
    //! to wait for the responses from other instances, accordingly,
    //! the events (like user click) can be executed and this method can be called several times,
    //! before the end of the current call.
    //! So we ignore all subsequent calls until the current one completes.
    if (m_isProjectProcessing) {
        return make_ret(Ret::Code::InternalError);
    }
    m_isProjectProcessing = true;

    DEFER {
        m_isProjectProcessing = false;
    };

    //! Step 1. If no path is specified, ask the user to select a project
    io::path_t projectPath = fileSystem()->absoluteFilePath(projectPath_);
    if (projectPath.empty()) {
        projectPath = selectScoreOpeningFile();

        if (projectPath.empty()) {
            return make_ret(Ret::Code::Cancel);
        }
    }

    //! Step 2. If the project is already open in the current window, then just switch to showing the notation
    if (isProjectOpened(projectPath)) {
        return openPageIfNeed(NOTATION_PAGE_URI);
    }

    //! Step 3. Check, if the project already opened in another window, then activate the window with the project
    if (multiInstancesProvider()->isProjectAlreadyOpened(projectPath)) {
        multiInstancesProvider()->activateWindowWithProject(projectPath);
        return make_ret(Ret::Code::Ok);
    }

    //! Step 4. Check, if a any project is already open in the current window,
    //! then create a new instance
    if (globalContext()->currentProject()) {
        QStringList args;
        args << projectPath.toQString();
        multiInstancesProvider()->openNewAppInstance(args);
        return make_ret(Ret::Code::Ok);
    }

    //! Step 5. Open project in the current window
    return doOpenProject(projectPath);
}

Ret ProjectActionsController::doOpenProject(const io::path_t& filePath)
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

        if (checkCanIgnoreError(ret, filePath)) {
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
    } else {
        prependToRecentScoreList(filePath);
    }

    globalContext()->setCurrentProject(project);

    return openPageIfNeed(NOTATION_PAGE_URI);
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
    if (m_isProjectProcessing) {
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
            dispatcher()->dispatch("quit", actions::ActionData::make_arg1<bool>(false));
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

    IInteractive::Options options {
        IInteractive::Option::WithIcon
    };

    IInteractive::Result result = interactive()->warning(title, body, {
        IInteractive::Button::DontSave,
        IInteractive::Button::Cancel,
        IInteractive::Button::Save
    }, IInteractive::Button::Save, options);

    return result.standardButton();
}

bool ProjectActionsController::canSaveProject() const
{
    auto project = currentNotationProject();
    if (!project) {
        LOGW() << "no current project";
        return false;
    }

    if (!project->canSave()) {
        LOGW() << "project could not be saved";
        return false;
    }

    return true;
}

bool ProjectActionsController::saveProject(const io::path_t& path)
{
    if (!canSaveProject()) {
        warnSaveIsNotAvailable();
        return false;
    }

    auto project = currentNotationProject();

    if (!project->isNewlyCreated()) {
        if (project->isCloudProject()) {
            return saveProjectToCloud(project->cloudInfo());
        }

        return saveProjectLocally();
    }

    if (!path.empty()) {
        return saveProjectLocally(path);
    }

    RetVal<SaveLocation> location = saveProjectScenario()->askSaveLocation(project, SaveMode::Save);
    if (!location.ret) {
        return false;
    }

    return saveProjectAt(location.val, SaveMode::Save);
}

void ProjectActionsController::saveProjectAs()
{
    if (!canSaveProject()) {
        warnSaveIsNotAvailable();
        return;
    }

    auto project = currentNotationProject();

    RetVal<SaveLocation> location = saveProjectScenario()->askSaveLocation(project, SaveMode::SaveAs);
    if (!location.ret) {
        return;
    }

    saveProjectAt(location.val, SaveMode::SaveAs);
}

void ProjectActionsController::saveProjectCopy()
{
    if (!canSaveProject()) {
        warnSaveIsNotAvailable();
        return;
    }

    auto project = currentNotationProject();

    RetVal<SaveLocation> location = saveProjectScenario()->askSaveLocation(project, SaveMode::SaveCopy);
    if (!location.ret) {
        return;
    }

    saveProjectAt(location.val, SaveMode::SaveCopy);
}

void ProjectActionsController::saveSelection()
{
    if (!canSaveProject()) {
        warnSaveIsNotAvailable();
        return;
    }

    auto project = currentNotationProject();

    RetVal<io::path_t> path = saveProjectScenario()->askLocalPath(project, SaveMode::SaveSelection);
    if (!path.ret) {
        return;
    }

    Ret ret = currentNotationProject()->save(path.val, SaveMode::SaveSelection);
    if (!ret) {
        LOGE() << ret.toString();
    }
}

void ProjectActionsController::saveToCloud()
{
    if (!canSaveProject()) {
        warnSaveIsNotAvailable();
        return;
    }

    auto project = currentNotationProject();

    RetVal<SaveLocation> response = saveProjectScenario()->askSaveLocation(project, SaveMode::SaveAs, SaveLocationType::Cloud);
    if (!response.ret) {
        return;
    }

    saveProjectAt(response.val, SaveMode::SaveAs);
}

void ProjectActionsController::publish()
{
    if (!canSaveProject()) {
        warnSaveIsNotAvailable();
        return;
    }

    if (!authorizationService()->checkCloudIsAvailable()) {
        warnPublishIsNotAvailable();
        return;
    }

    auto project = currentNotationProject();

    RetVal<CloudProjectInfo> info = saveProjectScenario()->askPublishLocation(project);
    if (!info.ret) {
        return;
    }

    AudioFile audio = exportMp3(project->masterNotation()->notation());
    if (audio.isValid()) {
        uploadProject(info.val, audio, /*openEditUrl=*/ true, /*publishMode=*/ true);
    }
}

bool ProjectActionsController::saveProjectAt(const SaveLocation& location, SaveMode saveMode)
{
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
    Ret ret = currentNotationProject()->save(filePath, saveMode);
    if (!ret) {
        LOGE() << ret.toString();
        return false;
    }

    prependToRecentScoreList(filePath);
    return true;
}

bool ProjectActionsController::saveProjectToCloud(CloudProjectInfo info, SaveMode saveMode)
{
    if (m_isProjectUploading) {
        return true;
    }

    m_isProjectUploading = true;

    bool isUploadingFinished = true;

    DEFER {
        if (isUploadingFinished) {
            m_isProjectUploading = false;
        }
    };

    bool isCloudAvailable = authorizationService()->checkCloudIsAvailable();
    if (!isCloudAvailable) {
        warnCloudIsNotAvailable();
    } else {
        Ret ret = authorizationService()->ensureAuthorization(
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
        RetVal<cloud::ScoreInfo> scoreInfo = cloudProjectsService()->downloadScoreInfo(info.sourceUrl);
        if (scoreInfo.ret) {
            info.visibility = scoreInfo.val.visibility;
            isPublic = info.visibility == cloud::Visibility::Public;
        } else {
            LOGE() << "Failed to download up-to-date score info for " << info.sourceUrl
                   << "; falling back to last known visibility setting, namely " << static_cast<int>(info.visibility);
        }

        if (isPublic) {
            if (!saveProjectScenario()->warnBeforeSavingToExistingPubliclyVisibleCloudProject()) {
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

    project->setCloudInfo(info);

    io::path_t savingPath;

    if (project->isCloudProject()) {
        if (saveMode == SaveMode::Save || saveMode == SaveMode::AutoSave) {
            savingPath = project->path();
        }
    }

    if (savingPath.empty()) {
        savingPath = configuration()->cloudProjectSavingFilePath(info.name.toStdString());
    }

    if (!saveProjectLocally(savingPath)) {
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

    uploadProject(info, audio, /*openEditUrl=*/ isPublic, /*publishMode=*/ false);
    isUploadingFinished = false;

    m_numberOfSavesToCloud++;

    return true;
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

void ProjectActionsController::uploadProject(const CloudProjectInfo& info, const AudioFile& audio, bool openEditUrl, bool publishMode)
{
    INotationProjectPtr project = globalContext()->currentProject();
    if (!project) {
        return;
    }

    QBuffer* projectData = new QBuffer();
    projectData->open(QIODevice::WriteOnly);

    Ret ret = project->writeToDevice(projectData);
    if (!ret) {
        LOGE() << ret.toString();
        delete projectData;
        return;
    }

    projectData->close();
    projectData->open(QIODevice::ReadOnly);

    bool isFirstSave = info.sourceUrl.isEmpty();

    m_uploadingProjectProgress = cloudProjectsService()->uploadScore(*projectData, info.name, info.visibility, info.sourceUrl);

    m_uploadingProjectProgress->started.onNotify(this, [this]() {
        showUploadProgressDialog();
        LOGD() << "Uploading project started";
    });

    m_uploadingProjectProgress->progressChanged.onReceive(this, [](int64_t current, int64_t total, const std::string&) {
        if (total > 0) {
            LOGD() << "Uploading project progress: " << current << " / " << total << " bytes";
        }
    });

    m_uploadingProjectProgress->finished.onReceive(this, [this, project, projectData, audio, openEditUrl, publishMode,
                                                          isFirstSave](const ProgressResult& res) {
        projectData->deleteLater();

        if (!res.ret) {
            LOGE() << res.ret.toString();
            onProjectUploadFailed(res.ret, publishMode);
            return;
        }

        ValMap urlMap = res.val.toMap();
        QString newSourceUrl = urlMap["sourceUrl"].toQString();
        QString editUrl = openEditUrl ? urlMap["editUrl"].toQString() : QString();

        LOGD() << "Source url received: " << newSourceUrl;

        if (audio.isValid()) {
            uploadAudio(audio, newSourceUrl, editUrl, isFirstSave);
        } else {
            onProjectSuccessfullyUploaded(editUrl, isFirstSave);
        }

        CloudProjectInfo info = project->cloudInfo();
        if (info.sourceUrl == newSourceUrl) {
            return;
        }

        info.sourceUrl = newSourceUrl;
        project->setCloudInfo(info);

        if (!project->isNewlyCreated()) {
            project->save();
        }
    });
}

void ProjectActionsController::uploadAudio(const AudioFile& audio, const QUrl& sourceUrl, const QUrl& urlToOpen, bool isFirstSave)
{
    m_uploadingAudioProgress = cloudProjectsService()->uploadAudio(*audio.device, audio.format, sourceUrl);

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

    QUrl scoreManagerUrl = configuration()->scoreManagerUrl();

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
    okBtn.accent = true;

    std::string msg = trc("project/save", "All saved changes will now update to the cloud. "
                                          "You can manage this file in the score manager on musescore.com.");

    int btn = interactive()->info(trc("global", "Success!"), msg, { viewOnlineBtn, okBtn },
                                  static_cast<int>(IInteractive::Button::Ok)).button();

    if (btn == viewOnlineBtn.btn) {
        interactive()->openUrl(scoreManagerUrl);
    }
}

void ProjectActionsController::onProjectUploadFailed(const Ret& ret, bool publishMode)
{
    m_isProjectUploading = false;

    closeUploadProgressDialog();

    std::string title = publishMode
                        ? trc("project/save", "Your score could not be published")
                        : trc("project/save", "Your score could not be saved to the cloud");

    std::string msg;

    IInteractive::ButtonData okBtn = interactive()->buttonData(IInteractive::Button::Ok);
    okBtn.accent = true;

    IInteractive::ButtonData helpBtn { IInteractive::Button::CustomButton, trc("project/save", "Get help") };

    IInteractive::ButtonDatas buttons { helpBtn, okBtn };

    switch (ret.code()) {
    case int(cloud::Err::AccountNotActivated):
        msg = trc("project/save", "Your musescore.com account needs to be verified first. "
                                  "Please activate your account via the link in the activation email.");
        buttons = { okBtn };
        break;
    case int(cloud::Err::NetworkError):
        msg = cloud::cloudNetworkErrorUserDescription(ret);
        if (!msg.empty()) {
            msg += "\n\n" + trc("project/save", "Please try again later, or get help for this problem on musescore.org.");
            break;
        }
    // FALLTHROUGH
    default:
        msg = trc("project/save", "Please try again later, or get help for this problem on musescore.org.");
        break;
    }

    IInteractive::Result result = interactive()->warning(title, msg, buttons, okBtn.btn, IInteractive::Option::WithIcon);
    if (result.button() == helpBtn.btn) {
        interactive()->openUrl(configuration()->supportForumUrl());
    }
}

void ProjectActionsController::warnCloudIsNotAvailable()
{
    closeUploadProgressDialog();

    if (!configuration()->showCloudIsNotAvailableWarning()) {
        return;
    }

    std::string title = trc("project/save", "Unable to connect to the cloud");
    std::string msg = trc("project/save", "Your changes will be saved to a local file until the connection resumes.");

    IInteractive::ButtonData okBtn = interactive()->buttonData(IInteractive::Button::Ok);
    okBtn.accent = true;

    IInteractive::Result result = interactive()->warning(title, msg,
                                                         { okBtn }, okBtn.btn,
                                                         IInteractive::Option::WithIcon | IInteractive::Option::WithDontShowAgainCheckBox);

    configuration()->setShowCloudIsNotAvailableWarning(result.showAgain());
}

void ProjectActionsController::warnPublishIsNotAvailable()
{
    std::string title = trc("project/save", "Unable to connect to MuseScore.com");
    std::string msg = trc("project/save", "Please check your internet connection or try again later.");

    IInteractive::ButtonData okBtn = interactive()->buttonData(IInteractive::Button::Ok);
    okBtn.accent = true;

    interactive()->warning(title, msg, { okBtn }, okBtn.btn, IInteractive::Option::WithIcon);
}

void ProjectActionsController::warnSaveIsNotAvailable()
{
    std::string title = trc("project/save", "Your score could not be saved");
    std::string msg;

    if (!currentMasterNotation()->hasParts()) {
        msg = trc("project/save", "Please add at least one instrument to enable saving.");
    }

    IInteractive::ButtonData okBtn = interactive()->buttonData(IInteractive::Button::Ok);
    okBtn.accent = true;

    interactive()->warning(title, msg, { okBtn }, okBtn.btn, IInteractive::Option::WithIcon);
}

bool ProjectActionsController::checkCanIgnoreError(const Ret& ret, const io::path_t& filePath)
{
    static const QList<engraving::Err> ignorableErrors {
        engraving::Err::FileTooOld,
        engraving::Err::FileTooNew,
        engraving::Err::FileCorrupted,
        engraving::Err::FileOld300Format
    };

    //: an error that occurred while opening a file
    std::string title = trc("project", "Open error");
    std::string body = ret.text();

    if (body.empty()) {
        body = qtrc("project", "Cannot open file %1").arg(filePath.toQString()).toStdString();
    }

    IInteractive::Options options;
    options.setFlag(IInteractive::Option::WithIcon);

    bool canIgnore = ignorableErrors.contains(static_cast<engraving::Err>(ret.code()));

    if (!canIgnore) {
        interactive()->error(title, body, {
            IInteractive::Button::Ok
        }, IInteractive::Button::Ok, options);

        return false;
    }

    IInteractive::Result result = interactive()->warning(title, body, {
        IInteractive::Button::Cancel,
        IInteractive::Button::Ignore
    }, IInteractive::Button::Ignore, options);

    return result.standardButton() == IInteractive::Button::Ignore;
}

void ProjectActionsController::importPdf()
{
    interactive()->openUrl("https://musescore.com/import");
}

void ProjectActionsController::clearRecentScores()
{
    configuration()->setRecentProjectPaths({});
    platformRecentFilesController()->clearRecentFiles();
}

void ProjectActionsController::continueLastSession()
{
    io::paths_t recentScorePaths = configuration()->recentProjectPaths();

    if (recentScorePaths.empty()) {
        return;
    }

    io::path_t lastScorePath = recentScorePaths.front();
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
                         "*.ove *.scw *.bmw *.bww *.gtp *.gp3 *.gp4 *.gp5 *.gpx *.gp *.ptb *.mscx *.mscs *.mscz~";

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
                                      trc("project", "Uncompressed MuseScore folders (experimental)") + " (*.mscx)",
                                      trc("project", "MuseScore developer files") + " (*.mscs)",
                                      trc("project", "MuseScore backup files") + " (*.mscz~)" };

    io::path_t defaultDir = configuration()->lastOpenedProjectsPath();

    if (defaultDir.empty()) {
        defaultDir = configuration()->defaultProjectsPath();
    }

    io::path_t filePath = interactive()->selectOpeningFile(qtrc("project", "Open"), defaultDir, filter);

    if (!filePath.empty()) {
        configuration()->setLastOpenedProjectsPath(io::dirpath(filePath));
    }

    return filePath;
}

void ProjectActionsController::prependToRecentScoreList(const io::path_t& filePath)
{
    if (filePath.empty()) {
        return;
    }

    io::paths_t recentScorePaths = configuration()->recentProjectPaths();

    auto it = std::find(recentScorePaths.begin(), recentScorePaths.end(), filePath);
    if (it != recentScorePaths.end()) {
        recentScorePaths.erase(it);
    }

    recentScorePaths.insert(recentScorePaths.begin(), filePath);
    configuration()->setRecentProjectPaths(recentScorePaths);
    platformRecentFilesController()->addRecentFile(filePath);
}

bool ProjectActionsController::hasSelection() const
{
    return currentNotationSelection() ? !currentNotationSelection()->isNone() : false;
}

void ProjectActionsController::openProjectProperties()
{
    interactive()->open(PROJECT_PROPERTIES_URI);
}
