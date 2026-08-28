/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2026 MuseScore Limited and others
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

#include "saveprojectscenario.h"

#include <QBuffer>
#include <QEventLoop>
#include <QFileInfo>
#include <QTemporaryFile>
#include <QUrl>

#include "async/async.h"
#include "async/processevents.h"
#include "defer.h"
#include "translation.h"

#include "cloud/clouderrors.h"
#include "cloud/qml/Muse/Cloud/enums.h"

#include "notation/imasternotation.h"
#include "notation/inotationinteraction.h"

#include "inotationproject.h"
#include "projecterrors.h"

#include "log.h"

using namespace mu::project;
using namespace mu::notation;
using namespace muse;

static const muse::Uri UPLOAD_PROGRESS_URI("musescore://project/upload/progress");

static constexpr int RETRY_SAVE_BTN_ID = int(IInteractive::Button::CustomButton);
static constexpr int SAVE_AS_BTN_ID    = RETRY_SAVE_BTN_ID + 1;

INotationProjectPtr SaveProjectScenario::currentNotationProject() const
{
    return globalContext()->currentProject();
}

INotationInteractionPtr SaveProjectScenario::currentInteraction() const
{
    INotationProjectPtr project = currentNotationProject();
    if (!project || !project->masterNotation() || !project->masterNotation()->notation()) {
        return nullptr;
    }

    return project->masterNotation()->notation()->interaction();
}

bool SaveProjectScenario::isBusy(BusyStatus status) const
{
    return m_busyStatuses.contains(status);
}

void SaveProjectScenario::setBusy(BusyStatus status, bool isBusy)
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

muse::async::Notification SaveProjectScenario::busyChanged() const
{
    return m_busyChanged;
}

muse::async::Notification SaveProjectScenario::revertToLastSavedRequested() const
{
    return m_revertToLastSavedRequested;
}

muse::Ret SaveProjectScenario::shareAudio()
{
    return shareAudio(AudioFile());
}

Ret SaveProjectScenario::canSaveProject() const
{
    auto project = currentNotationProject();
    if (!project) {
        LOGW() << "no current project";
        return make_ret(Err::NoProjectError);
    }

    return project->canSave();
}

bool SaveProjectScenario::saveProject(const muse::io::path_t& path)
{
    if (!path.empty()) {
        if (isBusy(BusyStatus::Saving)) {
            return false;
        }

        setBusy(BusyStatus::Saving, true);
        DEFER {
            setBusy(BusyStatus::Saving, false);
        };

        return saveProjectAt(SaveLocation(SaveLocationType::Local, path));
    }

    return saveProject(SaveMode::Save);
}

muse::Ret SaveProjectScenario::saveProject(SaveMode saveMode, SaveLocationType saveLocationType, bool force)
{
    if (isBusy(BusyStatus::Saving)) {
        return make_ret(Ret::Code::Busy);
    }

    setBusy(BusyStatus::Saving, true);
    DEFER {
        setBusy(BusyStatus::Saving, false);
    };

    INotationProjectPtr project = currentNotationProject();
    if (!project) {
        LOGW() << "no current project";
        return make_ret(Err::NoProjectError);
    }

    const bool isExistingSave = saveMode == SaveMode::Save && !project->isNewlyCreated();
    const bool wantNewCloudSave = saveLocationType == SaveLocationType::Cloud && !project->isCloudProject();
    if (isExistingSave && !wantNewCloudSave) {
        // Under these conditions, we can save without asking...
        SaveLocation location;
        if (project->isCloudProject()) {
            location = SaveLocation(SaveLocationType::Cloud, project->cloudInfo());
        } else {
            location = SaveLocation(SaveLocationType::Local);
        }
        return saveProjectAt(location, saveMode, force);
    }

    RetVal<SaveLocation> response = openSaveProjectScenario()->askSaveLocation(project, saveMode, saveLocationType);
    if (!response.ret) {
        LOGE() << response.ret.toString();
        return response.ret;
    }

    return saveProjectAt(response.val, saveMode, force);
}

muse::Ret SaveProjectScenario::publish()
{
    if (isBusy(BusyStatus::Publishing)) {
        return make_ret(Ret::Code::Busy);
    }

    setBusy(BusyStatus::Publishing, true);
    DEFER {
        setBusy(BusyStatus::Publishing, false);
    };

    Ret ret = canSaveProject();
    if (!ret) {
        askIfUserAgreesToSaveProjectWithErrors(ret, SaveLocationType::Cloud);
        return ret;
    }

    auto project = currentNotationProject();

    RetVal<CloudProjectInfo> info = openSaveProjectScenario()->askPublishLocation(project);
    if (!info.ret) {
        return info.ret;
    }

    AudioFile audio = exportMp3(project->masterNotation()->notation());
    if (audio.isValid()) {
        uploadProject(info.val, audio, /*openEditUrl=*/ true, /*publishMode=*/ true);
    }

    return make_ok();
}

muse::Ret SaveProjectScenario::shareAudio(const AudioFile& existingAudio)
{
    if (isBusy(BusyStatus::AudioSharing)) {
        return make_ret(Ret::Code::Busy);
    }

    setBusy(BusyStatus::AudioSharing, true);

    bool isSharingFinished = true;
    DEFER {
        if (isSharingFinished) {
            setBusy(BusyStatus::AudioSharing, false);
        }
    };

    auto project = currentNotationProject();
    RetVal<CloudAudioInfo> retVal = openSaveProjectScenario()->askShareAudioLocation(project);
    if (!retVal.ret) {
        return retVal.ret;
    }

    AudioFile audio;
    if (existingAudio.isValid()) {
        audio = existingAudio;
    } else {
        audio = exportMp3(project->masterNotation()->notation());
        if (!audio.isValid()) {
            return make_ret(Ret::Code::BadData);
        }
    }

    uploadAudioToAudioCom(audio, project, retVal.val);

    isSharingFinished = false;

    return make_ok();
}

void SaveProjectScenario::uploadAudioToAudioCom(const AudioFile& audio, const INotationProjectPtr& project,
                                                     const CloudAudioInfo& info)
{
    m_uploadingAudioProgress = audioComService()->uploadAudio(audio.device, audio.format, info.name,
                                                              project->cloudAudioInfo().url, info.visibility,
                                                              info.replaceExisting);
    LOGD() << "Uploading audio started";
    showUploadProgressDialog();

    m_uploadingAudioProgress->progressChanged().onReceive(this, [](int64_t current, int64_t total, const std::string&) {
        if (total > 0) {
            LOGD() << "Uploading audio progress: " << current << " / " << total << " bytes";
        }
    });

    m_uploadingAudioProgress->finished().onReceive(this, [this, project, info](const ProgressResult& res) {
        LOGD() << "Uploading audio finished";

        if (!res.ret) {
            LOGE() << res.ret.toString();
            onAudioUploadFailed(res.ret);
        } else {
            ValMap resMap = res.val.toMap();
            onAudioSuccessfullyUploaded(resMap["editUrl"].toQString());
            if (!info.replaceExisting) {
                CloudAudioInfo newInfo = project->cloudAudioInfo();
                newInfo.url = QUrl(resMap["url"].toQString());
                project->setCloudAudioInfo(newInfo);
            }
        }

        m_uploadingAudioProgress->started().disconnect(this);
        m_uploadingAudioProgress->progressChanged().disconnect(this);
        m_uploadingAudioProgress->finished().disconnect(this);
    });
}

muse::Ret SaveProjectScenario::saveProjectAt(const muse::rcommand::Params& params)
{
    const std::string& path = params.at("path").toString();
    if (!path.empty()) {
        return saveProjectAt(SaveLocation(muse::io::path_t(path)));
    }
    return make_ret(Ret::Code::BadArgs);
}

muse::Ret SaveProjectScenario::saveProjectAt(const SaveLocation& location, SaveMode saveMode, bool force)
{
    INotationInteractionPtr interaction = currentInteraction();
    if (interaction && interaction->isTextEditingStarted()) {
        interaction->endEditText();
    }

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

    return make_ret(Err::UnknownError);
}

bool SaveProjectScenario::saveProjectLocally(const muse::io::path_t& filePath, SaveMode saveMode, bool createBackup)
{
    INotationProjectPtr project = currentNotationProject();
    if (!project) {
        return false;
    }

    Ret ret = project->save(filePath, saveMode, createBackup);

    if (!ret) {
        LOGE() << ret.toString();
        if (ret.code() != (int)Err::CorruptionUponSavingError) {
            warnScoreCouldnotBeSaved(ret);
        } else {
            switch (warnScoreHasBecomeCorruptedAfterSave(ret)) {
            case RETRY_SAVE_BTN_ID:
                async::Async::call(this, [this, filePath, saveMode]() {
                    // Retry the save. Do not create a backup this time because the target file has been corrupted
                    // already. Creating a backup file of a corrupted file now makes no sense and will corrupt
                    // the healthy backup file created on the first save attempt.
                    saveProjectLocally(filePath, saveMode, false /*createBackup*/);
                });
                break;

            case SAVE_AS_BTN_ID:
                async::Async::call(this, [this]() {
                    saveProject(SaveMode::SaveAs);
                });
                break;
            }
        }
        return false;
    }

    recentFilesController()->prependRecentFile(makeRecentFile(project));
    return true;
}

bool SaveProjectScenario::saveProjectToCloud(CloudProjectInfo info, SaveMode saveMode)
{
    if (isBusy(BusyStatus::Uploading)) {
        return true;
    }

    setBusy(BusyStatus::Uploading, true);

    DEFER {
        setBusy(BusyStatus::Uploading, false);
    };

    INotationProjectPtr project = currentNotationProject();

    bool isCloudAvailable = museScoreComService()->authorization()->checkCloudIsAvailable();
    if (!isCloudAvailable) {
        warnCloudIsNotAvailable();
    } else {
        std::string dialogText = muse::trc("project/save", "Log in to MuseScore.com to save this score to the cloud.");
        RetVal<Val> retVal = museScoreComService()->authorization()->ensureAuthorization(true, dialogText);
        if (!retVal.ret) {
            return false;
        }

        using Response = muse::cloud::SaveToCloudResponse::SaveToCloudResponse;
        bool saveLocally = static_cast<Response>(retVal.val.toInt()) == Response::SaveLocallyInstead;
        if (saveLocally && project) {
            RetVal<muse::io::path_t> rv = openSaveProjectScenario()->askLocalPath(project, saveMode);
            if (!rv.ret) {
                LOGE() << rv.ret.toString();
                return false;
            }

            saveProjectLocally(rv.val, saveMode);
            configuration()->setLastUsedSaveLocationType(SaveLocationType::Local);

            return false;
        }
    }

    if (!project) {
        return false;
    }

    bool isPublic = info.visibility == muse::cloud::Visibility::Public;
    bool generateAudio = false;

    if (saveMode == SaveMode::Save && isCloudAvailable) {
        // Get up-to-date visibility information
        RetVal<muse::cloud::ScoreInfo> scoreInfo = museScoreComService()->downloadScoreInfo(info.sourceUrl);
        if (scoreInfo.ret) {
            info.name = scoreInfo.val.title;
            info.visibility = scoreInfo.val.visibility;
            isPublic = info.visibility == muse::cloud::Visibility::Public;
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

    muse::io::path_t savingPath;

    if (project->isCloudProject()) {
        if (saveMode == SaveMode::Save || saveMode == SaveMode::AutoSave) {
            savingPath = project->path();
        }
    }

    if (savingPath.empty()) {
        ID scoreId = muse::cloud::idFromCloudUrl(info.sourceUrl);

        savingPath = configuration()->cloudProjectSavingPath(scoreId.toUint64());
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

void SaveProjectScenario::alsoShareAudioCom(const AudioFile& audio)
{
    if (!configuration()->showAlsoShareAudioComDialog()) {
        shareAudio(audio);
        return;
    }

    UriQuery query("musescore://project/alsoshareaudiocom");
    query.addParam("rememberChoice", Val(!configuration()->hasAskedAlsoShareAudioCom()));
    RetVal<Val> rv = interactive()->openSync(query);

    if (!rv.val.isNull()) {
        QVariantMap vals = rv.val.toQVariant().toMap();
        bool shareAudioCom = vals["share"].toBool();
        bool rememberChoice = vals["remember"].toBool();

        if (shareAudioCom) {
            shareAudio(audio);
        }

        configuration()->setShowAlsoShareAudioComDialog(!rememberChoice);
        configuration()->setAlsoShareAudioCom(shareAudioCom);
    }

    configuration()->setHasAskedAlsoShareAudioCom(true);
}

Ret SaveProjectScenario::askAudioGenerationSettings() const
{
    RetVal<Val> res = interactive()->openSync("musescore://project/audiogenerationsettings");
    if (!res.ret) {
        return res.ret;
    }

    configuration()->setHasAskedAudioGenerationSettings(true);

    return muse::make_ok();
}

RetVal<bool> SaveProjectScenario::needGenerateAudio(bool isPublicUpload) const
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

SaveProjectScenario::AudioFile SaveProjectScenario::exportMp3(const INotationPtr notation) const
{
    auto tempFile = std::make_shared<QTemporaryFile>(configuration()->temporaryMp3FilePathTemplate().toQString());
    if (!tempFile->open()) {
        LOGE() << "Could not open a temp file";
        return AudioFile();
    }

    QString mp3Path = QFileInfo(*tempFile).absoluteFilePath();
    LOGD() << "mp3 path: " << mp3Path;

    if (mp3Path.isEmpty()) {
        LOGE() << "mp3 path is empty";
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
        return AudioFile();
    }

    AudioFile audio;
    audio.format = "mp3";
    audio.device = tempFile;
    audio.device->seek(0);

    return audio;
}

void SaveProjectScenario::showUploadProgressDialog()
{
    if (interactive()->isOpened(UPLOAD_PROGRESS_URI).val) {
        return;
    }

    interactive()->open(UPLOAD_PROGRESS_URI);
}

void SaveProjectScenario::closeUploadProgressDialog()
{
    if (interactive()->isOpened(UPLOAD_PROGRESS_URI).val) {
        interactive()->closeSync(UriQuery(UPLOAD_PROGRESS_URI));
    }
}

Ret SaveProjectScenario::uploadProject(const CloudProjectInfo& info, const AudioFile& audio, bool openEditUrl, bool publishMode)
{
    INotationProjectPtr project = globalContext()->currentProject();
    if (!project) {
        return false;
    }

    auto projectData = std::make_shared<QBuffer>();
    projectData->open(QIODevice::WriteOnly);

    Ret ret = project->writeToDevice(projectData.get());
    if (!ret) {
        LOGE() << ret.toString();
        return ret;
    }

    projectData->close();
    projectData->open(QIODevice::ReadOnly);

    bool isFirstSave = info.sourceUrl.isEmpty();

    // The method must not return until the saving is complete, to prevent the app from being quit prematurely
    QEventLoop eventLoop;

    m_uploadingProjectProgress = museScoreComService()->uploadScore(projectData, info.name, info.visibility, info.sourceUrl,
                                                                    info.revisionId);
    showUploadProgressDialog();
    LOGD() << "Uploading project started";

    m_uploadingProjectProgress->progressChanged().onReceive(this, [](int64_t current, int64_t total, const std::string&) {
        if (total > 0) {
            LOGD() << "Uploading project progress: " << current << " / " << total << " bytes";
        }
    });

    m_uploadingProjectProgress->finished().onReceive(this, [this, project, info, audio, openEditUrl, publishMode,
                                                            isFirstSave, &ret, &eventLoop](const ProgressResult& res) {
        DEFER {
            m_uploadingProjectProgress->progressChanged().disconnect(this);
            m_uploadingProjectProgress->finished().disconnect(this);
            eventLoop.quit();
        };

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
                moveProject(project, configuration()->cloudProjectPath(muse::cloud::idFromCloudUrl(cpinfo.sourceUrl).toUint64()), true);
            }
        }

        if (audio.isValid()) {
            uploadAudioToMuseScoreCom(audio, newSourceUrl, editUrl, isFirstSave, publishMode);
        } else {
            onProjectSuccessfullyUploaded(editUrl, isFirstSave);

            if (publishMode && (configuration()->alsoShareAudioCom() || configuration()->showAlsoShareAudioComDialog())) {
                alsoShareAudioCom(audio);
            }
        }
    });

    muse::async::processMessages();
    eventLoop.exec();

    return ret;
}

void SaveProjectScenario::uploadAudioToMuseScoreCom(const AudioFile& audio, const QUrl& sourceUrl, const QUrl& urlToOpen,
                                                         bool isFirstSave,
                                                         bool publishMode)
{
    m_uploadingAudioProgress = museScoreComService()->uploadAudio(audio.device, audio.format, sourceUrl);

    m_uploadingAudioProgress->progressChanged().onReceive(this, [](int64_t current, int64_t total, const std::string&) {
        if (total > 0) {
            LOGD() << "Uploading audio progress: " << current << " / " << total << " bytes";
        }
    });

    m_uploadingAudioProgress->finished().onReceive(this, [this, audio, urlToOpen, isFirstSave, publishMode](const ProgressResult& res) {
        LOGD() << "Uploading audio finished";

        if (!res.ret) {
            LOGE() << res.ret.toString();
        }

        onProjectSuccessfullyUploaded(urlToOpen, isFirstSave);

        m_uploadingAudioProgress->progressChanged().disconnect(this);
        m_uploadingAudioProgress->finished().disconnect(this);

        if (publishMode && (configuration()->alsoShareAudioCom() || configuration()->showAlsoShareAudioComDialog())) {
            alsoShareAudioCom(audio);
        }
    });
}

void SaveProjectScenario::onProjectSuccessfullyUploaded(const QUrl& urlToOpen, bool isFirstSave)
{
    setBusy(BusyStatus::Uploading, false);

    closeUploadProgressDialog();

    if (!urlToOpen.isEmpty()) {
        platformInteractive()->openUrl(urlToOpen);
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

    IInteractive::ButtonData viewOnlineBtn(IInteractive::Button::CustomButton, muse::trc("project/save", "View online"));
    IInteractive::ButtonData okBtn = interactive()->buttonData(IInteractive::Button::Ok);

    std::string msg = muse::trc("project/save", "All saved changes will now update to the cloud. "
                                                "You can manage this file in the score manager on MuseScore.com.");

    interactive()->info(muse::trc("global", "Success!"), msg, { viewOnlineBtn, okBtn },
                        static_cast<int>(IInteractive::Button::Ok))
    .onResolve(this, [this, viewOnlineBtn, scoreManagerUrl](const IInteractive::Result& res) {
        if (res.isButton(viewOnlineBtn.btn)) {
            platformInteractive()->openUrl(scoreManagerUrl);
        }
    });
}

Ret SaveProjectScenario::onProjectUploadFailed(const Ret& ret, const CloudProjectInfo& info, const AudioFile& audio,
                                                    bool openEditUrl,
                                                    bool publishMode)
{
    setBusy(BusyStatus::Uploading, false);

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
        RetVal<muse::cloud::ScoreInfo> scoreInfo = museScoreComService()->downloadScoreInfo(info.sourceUrl);
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

void SaveProjectScenario::onAudioSuccessfullyUploaded(const QUrl& urlToOpen)
{
    setBusy(BusyStatus::AudioSharing, false);

    closeUploadProgressDialog();

    platformInteractive()->openUrl(urlToOpen);
}

void SaveProjectScenario::onAudioUploadFailed(const Ret& ret)
{
    setBusy(BusyStatus::AudioSharing, false);

    closeUploadProgressDialog();

    openSaveProjectScenario()->showAudioCloudShareError(ret);
}

void SaveProjectScenario::warnCloudIsNotAvailable()
{
    closeUploadProgressDialog();

    if (!configuration()->showCloudIsNotAvailableWarning()) {
        return;
    }

    std::string title = muse::trc("project/save", "Unable to connect to the cloud");
    std::string msg = muse::trc("project/save", "Your changes will be saved to a local file until the connection resumes.");

    auto result = interactive()->warning(title, msg,
                                         { IInteractive::Button::Ok }, IInteractive::Button::Ok,
                                         IInteractive::Option::WithIcon | IInteractive::Option::WithDontShowAgainCheckBox);

    result.onResolve(this, [this](const IInteractive::Result& res) {
        configuration()->setShowCloudIsNotAvailableWarning(res.showAgain());
    });
}

bool SaveProjectScenario::askIfUserAgreesToSaveProjectWithErrors(const Ret& ret, const SaveLocation& location)
{
    switch (static_cast<Err>(ret.code())) {
    case Err::NoPartsError:
        warnScoreCouldnotBeSaved(muse::trc("project/save", "Please add at least one instrument to enable saving."));
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

bool SaveProjectScenario::askIfUserAgreesToSaveCorruptedScore(const SaveLocation& location, const std::string& errorText,
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
    case SaveLocationType::Undefined:     // fallthrough
    default:
        return false;
    }
}

void SaveProjectScenario::warnCorruptedScoreCannotBeSavedOnCloud(const std::string& errorText, bool canRevert)
{
    std::string title = muse::trc("project", "Your score cannot be uploaded to the cloud");

    IInteractive::Text text;
    text.text = muse::trc("project", "This score has become corrupted and contains errors. "
                                     "You can fix the errors manually, or save the score to your computer "
                                     "and get help for this issue on MuseScore.org.");
    text.detailedText = errorText;

    IInteractive::ButtonDatas buttons;
    buttons.push_back(interactive()->buttonData(IInteractive::Button::Cancel));

    IInteractive::ButtonData saveCopyBtn(IInteractive::Button::CustomButton, muse::trc("project", "Save as…"), !canRevert /*accent*/);
    buttons.push_back(saveCopyBtn);

    int defaultBtn = saveCopyBtn.btn;

    IInteractive::ButtonData revertToLastSavedBtn(saveCopyBtn.btn + 1, muse::trc("project", "Revert to last saved"),
                                                  true /*accent*/);

    if (canRevert) {
        buttons.push_back(revertToLastSavedBtn);
        defaultBtn = revertToLastSavedBtn.btn;
    }

    interactive()->error(title, text, buttons, defaultBtn)
    .onResolve(this, [this, saveCopyBtn, revertToLastSavedBtn](const IInteractive::Result& res) {
        int btn = res.button();
        if (btn == saveCopyBtn.btn) {
            setBusy(BusyStatus::Saving, false);
            saveProject(SaveMode::SaveAs, SaveLocationType::Local, true /*force*/);
        } else if (btn == revertToLastSavedBtn.btn) {
            askToRevertCorruptedScoreToLastSaved();
        }
    });
}

bool SaveProjectScenario::askIfUserAgreesToSaveCorruptedScoreLocally(const std::string& errorText,
                                                                          bool canRevert)
{
    std::string title = muse::trc("project", "This score has become corrupted and contains errors");

    IInteractive::Text text;
    text.text = !canRevert
                ? muse::trc("project", "You can continue saving it locally, although the file may become unusable. "
                                       "You can try to fix the errors manually, or get help for this issue on MuseScore.org.")
                : muse::trc("project", "You can continue saving it locally, although the file may become unusable. "
                                       "To preserve your score, revert to the last saved version, or fix the errors manually. "
                                       "You can also get help for this issue on MuseScore.org.");
    text.detailedText = errorText;

    IInteractive::ButtonDatas buttons;
    buttons.push_back(interactive()->buttonData(IInteractive::Button::Cancel));

    IInteractive::ButtonData saveAnywayBtn(IInteractive::Button::CustomButton, muse::trc("project", "Save anyway"),
                                           !canRevert /*accent*/);
    buttons.push_back(saveAnywayBtn);

    int defaultBtn = saveAnywayBtn.btn;

    IInteractive::ButtonData revertToLastSavedBtn(saveAnywayBtn.btn + 1, muse::trc("project", "Revert to last saved"),
                                                  true /*accent*/);
    if (canRevert) {
        buttons.push_back(revertToLastSavedBtn);
        defaultBtn = revertToLastSavedBtn.btn;
    }

    int btn = interactive()->errorSync(title, text, buttons, defaultBtn).button();

    if (btn == revertToLastSavedBtn.btn) {
        askToRevertCorruptedScoreToLastSaved();
    }

    return btn == saveAnywayBtn.btn;
}

bool SaveProjectScenario::askIfUserAgreesToSaveCorruptedScoreUponOpenning(const SaveLocation& location,
                                                                               const std::string& errorText)
{
    switch (location.type) {
    case SaveLocationType::Cloud:
        showErrCorruptedScoreCannotBeSaved(location, errorText);
        return false;
    case SaveLocationType::Local:
        return askIfUserAgreesToSaveCorruptedScoreLocally(errorText, false /*canRevert*/);
    case SaveLocationType::Undefined:     // fallthrough
    default:
        return false;
    }
}

void SaveProjectScenario::showErrCorruptedScoreCannotBeSaved(const SaveLocation& location, const std::string& errorText)
{
    std::string title = location.isLocal()
                        ? muse::trc("project", "Your score cannot be saved")
                        : muse::trc("project", "Your score cannot be uploaded to the cloud");

    IInteractive::Text text;
    text.text = muse::trc("project", "This score is corrupted. You can get help for this issue on MuseScore.org.");
    text.detailedText = errorText;

    IInteractive::ButtonData getHelpBtn(IInteractive::Button::CustomButton, muse::trc("project", "Get help"));

    interactive()->error(title, text, {
        getHelpBtn,
        interactive()->buttonData(IInteractive::Button::Ok)
    }).onResolve(this, [this, getHelpBtn](const IInteractive::Result& res) {
        if (res.isButton(getHelpBtn.btn)) {
            platformInteractive()->openUrl(configuration()->supportForumUrl());
        }
    });
}

void SaveProjectScenario::warnScoreCouldnotBeSaved(const Ret& ret)
{
    std::string message = ret.text();
    if (message.empty()) {
        message = muse::trc("project/save", "An unknown error occurred while saving this file.");
    }

    warnScoreCouldnotBeSaved(message);
}

void SaveProjectScenario::warnScoreCouldnotBeSaved(const std::string& errorText)
{
    interactive()->warning(muse::trc("project/save", "Your score could not be saved"), errorText);
}

int SaveProjectScenario::warnScoreHasBecomeCorruptedAfterSave(const Ret& ret)
{
    const QString errDetailsMessage = QString::fromStdString(ret.toString()).toHtmlEscaped();

    const QString supportForumLink = String("<a href=\"%1\" style=\"text-decoration: none\">MuseScore.org</a>")
                                     .arg(configuration()->supportForumUrl().toString());

    const std::string title = muse::trc("project/save", "An error occurred while saving your score");

    const std::string body = muse::qtrc("project/save",
                                        "To preserve your score, try saving it again. "
                                        "If this message still appears, please save your score as new copy. "
                                        "You can also get help for this issue on %1.<br/><br/>"
                                        "Error details (please cite when asking for support): %2")
                             .arg(supportForumLink, errDetailsMessage)
                             .toStdString();

    IInteractive::ButtonDatas buttons;

    IInteractive::ButtonData saveAsBtn(SAVE_AS_BTN_ID, muse::trc("project/save", "Save as…"));
    saveAsBtn.role = IInteractive::ButtonRole::ContinueRole;
    buttons.push_back(saveAsBtn);

    IInteractive::ButtonData retryBtn(RETRY_SAVE_BTN_ID, muse::trc("project", "Try again"), true /*accent*/);
    retryBtn.role = IInteractive::ButtonRole::ContinueRole;
    buttons.push_back(retryBtn);

    IInteractive::ButtonData cancelBtn = interactive()->buttonData(IInteractive::Button::Cancel);
    buttons.push_back(cancelBtn);

    return interactive()->errorSync(title, IInteractive::Text(body, IInteractive::TextFormat::RichText),
                                    buttons, retryBtn.btn).button();
}

void SaveProjectScenario::askToRevertCorruptedScoreToLastSaved()
{
    TRACEFUNC;

    std::string title = muse::trc("project", "Revert to last saved?");
    std::string body = muse::trc("project", "Your changes will be lost. This action cannot be undone.");

    auto promise = interactive()->warning(title, body, {
        { IInteractive::Button::No, IInteractive::Button::Yes }
    }, IInteractive::Button::Yes, IInteractive::Option::WithIcon);

    promise.onResolve(this, [this](const IInteractive::Result& res) {
        if (res.isButton(IInteractive::Button::No)) {
            return;
        }

        //! NOTE Reopening the file is part of the open flow, so it is left to the listener
        m_revertToLastSavedRequested.notify();
    });
}

RecentFile SaveProjectScenario::makeRecentFile(INotationProjectPtr project)
{
    RecentFile file;
    file.path = project->path();

    if (project->isCloudProject()) {
        file.displayNameOverride = project->cloudInfo().name;
    }

    return file;
}

void SaveProjectScenario::moveProject(INotationProjectPtr project, const muse::io::path_t& newPath, bool replace)
{
    muse::io::path_t oldPath = project->path();
    if (oldPath == newPath) {
        return;
    }

    fileSystem()->move(oldPath, newPath, replace);
    project->setPath(newPath);

    recentFilesController()->moveRecentFile(oldPath, makeRecentFile(project));
}

QUrl SaveProjectScenario::scoreManagerUrl() const
{
    return museScoreComService()->scoreManagerUrl();
}
