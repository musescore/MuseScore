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
#include <QFile>
#include <QFileInfo>
#include <QTemporaryFile>
#include <QUrl>

#include "async/async.h"
#include "defer.h"
#include "translation.h"

#include "cloud/clouderrors.h"
#include "cloud/qml/Muse/Cloud/enums.h"
#include "engraving/infrastructure/mscio.h"

#include "notation/imasternotation.h"
#include "notation/inotation.h"
#include "notation/inotationinteraction.h"

#include "inotationproject.h"
#include "projecterrors.h"

#include "log.h"

using namespace mu::project;
using namespace mu::notation;
using namespace muse;
using muse::async::Promise;

template<typename T>
static Promise<T> resolvedPromise(const T& value)
{
    return async::make_promise<T>([value](auto resolve) {
        return resolve(value);
    });
}

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

Promise<Ret> SaveProjectScenario::runIfNotBusy(BusyStatus status, const std::function<Promise<Ret>()>& flow)
{
    if (isBusy(status)) {
        return resolvedPromise(make_ret(Ret::Code::Busy));
    }

    setBusy(status, true);

    return flow().then<Ret>(this, [this, status](const Ret& ret, auto resolve) {
        setBusy(status, false);
        return resolve(ret);
    });
}

Promise<RetVal<Val> > SaveProjectScenario::openDialog(const UriQuery& query) const
{
    return async::make_promise<RetVal<Val> >([this, query](auto resolve) {
        interactive()->open(query)
        .onResolve(this, [resolve](const Val& val) {
            (void)resolve(RetVal<Val>::make_ok(val));
        })
        .onReject(this, [resolve](int code, const std::string& err) {
            (void)resolve(RetVal<Val>(muse::make_ret(code, err)));
        });

        return Promise<RetVal<Val> >::dummy_result();
    });
}

Promise<Ret> SaveProjectScenario::shareAudio()
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

Promise<Ret> SaveProjectScenario::saveProject(const muse::io::path_t& path)
{
    if (path.empty()) {
        return saveProject(SaveMode::Save);
    }

    return runIfNotBusy(BusyStatus::Saving, [this, path]() {
        return saveProjectAt(SaveLocation(SaveLocationType::Local, path));
    });
}

Promise<Ret> SaveProjectScenario::saveProject(SaveMode saveMode, SaveLocationType saveLocationType, bool force)
{
    return runIfNotBusy(BusyStatus::Saving, [this, saveMode, saveLocationType, force]() -> Promise<Ret> {
        INotationProjectPtr project = currentNotationProject();
        if (!project) {
            LOGW() << "no current project";
            return resolvedPromise(make_ret(Err::NoProjectError));
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

        return askSaveLocation(project, saveMode, saveLocationType)
               .then<Ret>(this, [this, saveMode, force](const RetVal<SaveLocation>& response, auto resolve) {
            if (!response.ret) {
                LOGE() << response.ret.toString();
                return resolve(response.ret);
            }

            saveProjectAt(response.val, saveMode, force).onResolve(this, [resolve](const Ret& ret) {
                (void)resolve(ret);
            });

            return Promise<Ret>::dummy_result();
        });
    });
}

Promise<Ret> SaveProjectScenario::publish()
{
    return runIfNotBusy(BusyStatus::Publishing, [this]() -> Promise<Ret> {
        Ret ret = canSaveProject();
        if (!ret) {
            askIfUserAgreesToSaveProjectWithErrors(ret, SaveLocationType::Cloud);
            return resolvedPromise(ret);
        }

        auto project = currentNotationProject();

        return askPublishLocation(project)
               .then<Ret>(this, [this, project](const RetVal<CloudProjectInfo>& info, auto resolve) {
            if (info.ret.code() == RET_CODE_CHANGE_SAVE_LOCATION_TYPE) {
                return resolve(saveProjectLocallyInstead(project, SaveMode::Save));
            }

            if (!info.ret) {
                return resolve(info.ret);
            }

            AudioFile audio = exportMp3(project->masterNotation()->notation());
            if (!audio.isValid()) {
                return resolve(make_ret(Ret::Code::BadData));
            }

            uploadProject(info.val, audio, /*openEditUrl=*/ true, /*publishMode=*/ true)
            .onResolve(this, [resolve](const Ret& ret) {
                (void)resolve(ret);
            });

            return Promise<Ret>::dummy_result();
        });
    });
}

Promise<Ret> SaveProjectScenario::shareAudio(const AudioFile& existingAudio)
{
    return runIfNotBusy(BusyStatus::AudioSharing, [this, existingAudio]() -> Promise<Ret> {
        auto project = currentNotationProject();
        if (!project) {
            LOGW() << "no current project";
            return resolvedPromise(make_ret(Err::NoProjectError));
        }

        return askShareAudioLocation(project)
               .then<Ret>(this, [this, project, existingAudio](const RetVal<CloudAudioInfo>& info, auto resolve) {
            if (!info.ret) {
                return resolve(info.ret);
            }

            AudioFile audio = existingAudio.isValid() ? existingAudio : exportMp3(project->masterNotation()->notation());
            if (!audio.isValid()) {
                return resolve(make_ret(Ret::Code::BadData));
            }

            uploadAudioToAudioCom(audio, project, info.val).onResolve(this, [resolve](const Ret& ret) {
                (void)resolve(ret);
            });

            return Promise<Ret>::dummy_result();
        });
    });
}

Promise<Ret> SaveProjectScenario::uploadAudioToAudioCom(const AudioFile& audio, const INotationProjectPtr& project,
                                                        const CloudAudioInfo& info)
{
    return async::make_promise<Ret>([this, audio, project, info](auto resolve) {
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

        m_uploadingAudioProgress->finished().onReceive(this, [this, project, info, resolve](const ProgressResult& res) {
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

            (void)resolve(res.ret);
        });

        return Promise<Ret>::dummy_result();
    });
}

Promise<Ret> SaveProjectScenario::saveProjectAt(const muse::rcommand::Params& params)
{
    const std::string& path = params.at("path").toString();
    if (path.empty()) {
        return resolvedPromise(make_ret(Ret::Code::BadArgs));
    }

    return runIfNotBusy(BusyStatus::Saving, [this, path]() {
        return saveProjectAt(SaveLocation(muse::io::path_t(path)));
    });
}

Promise<Ret> SaveProjectScenario::saveProjectAt(const SaveLocation& location, SaveMode saveMode, bool force)
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
                return resolvedPromise(ret);
            }
        }
    }

    if (location.isLocal()) {
        return resolvedPromise(Ret(saveProjectLocally(location.localPath(), saveMode)));
    }

    if (location.isCloud()) {
        return saveProjectToCloud(location.cloudInfo(), saveMode);
    }

    return resolvedPromise(make_ret(Err::UnknownError));
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

Promise<Ret> SaveProjectScenario::saveProjectToCloud(CloudProjectInfo info, SaveMode saveMode)
{
    return runIfNotBusy(BusyStatus::Uploading, [this, info, saveMode]() {
        bool isCloudAvailable = museScoreComService()->authorization()->checkCloudIsAvailable();
        if (!isCloudAvailable) {
            warnCloudIsNotAvailable();

            INotationProjectPtr project = currentNotationProject();
            if (!project) {
                return resolvedPromise(make_ret(Err::NoProjectError));
            }

            return resolvedPromise(Ret(saveCloudProjectLocally(project, info, saveMode)));
        }

        std::string dialogText = muse::trc("project/save", "Log in to MuseScore.com to save this score to the cloud.");

        return ensureAuthorization(muse::cloud::MUSESCORE_COM_CLOUD_CODE, true, dialogText)
               .then<Ret>(this, [this, info, saveMode](const RetVal<Val>& auth, auto resolve) {
            if (!auth.ret) {
                return resolve(auth.ret);
            }

            INotationProjectPtr project = currentNotationProject();
            if (!project) {
                return resolve(make_ret(Err::NoProjectError));
            }

            using Response = muse::cloud::SaveToCloudResponse::SaveToCloudResponse;
            if (static_cast<Response>(auth.val.toInt()) == Response::SaveLocallyInstead) {
                return resolve(saveProjectLocallyInstead(project, saveMode));
            }

            saveAndUploadProject(project, info, saveMode).onResolve(this, [resolve](const Ret& ret) {
                (void)resolve(ret);
            });

            return Promise<Ret>::dummy_result();
        });
    });
}

Promise<Ret> SaveProjectScenario::saveAndUploadProject(const INotationProjectPtr& project, CloudProjectInfo info, SaveMode saveMode)
{
    bool isPublic = info.visibility == muse::cloud::Visibility::Public;

    if (saveMode == SaveMode::Save) {
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

        if (isPublic && !warnBeforeSavingToExistingPubliclyVisibleCloudProject()) {
            return resolvedPromise(make_ret(Ret::Code::Cancel));
        }
    }

    return needGenerateAudio(isPublic)
           .then<Ret>(this, [this, project, info, saveMode, isPublic](const RetVal<bool>& need, auto resolve) {
        if (!need.ret) {
            return resolve(need.ret);
        }

        if (!saveCloudProjectLocally(project, info, saveMode)) {
            return resolve(make_ret(Ret::Code::UnknownError));
        }

        AudioFile audio;
        if (need.val) {
            audio = exportMp3(project->masterNotation()->notation());
            if (!audio.isValid()) {
                return resolve(make_ret(Ret::Code::BadData));
            }
        }

        uploadProject(info, audio, /*openEditUrl=*/ isPublic, /*publishMode=*/ false)
        .onResolve(this, [this, resolve](const Ret& ret) {
            if (ret) {
                m_numberOfSavesToCloud++;
            }
            (void)resolve(ret);
        });

        return Promise<Ret>::dummy_result();
    });
}

bool SaveProjectScenario::saveCloudProjectLocally(const INotationProjectPtr& project, const CloudProjectInfo& info, SaveMode saveMode)
{
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

    return saveProjectLocally(savingPath, saveMode);
}

Ret SaveProjectScenario::saveProjectLocallyInstead(const INotationProjectPtr& project, SaveMode saveMode)
{
    RetVal<muse::io::path_t> path = askLocalPath(project, saveMode);
    if (!path.ret) {
        LOGE() << path.ret.toString();
        return path.ret;
    }

    bool ok = saveProjectLocally(path.val, saveMode);
    configuration()->setLastUsedSaveLocationType(SaveLocationType::Local);

    return Ret(ok);
}

void SaveProjectScenario::alsoShareAudioCom(const AudioFile& audio)
{
    if (!configuration()->showAlsoShareAudioComDialog()) {
        shareAudio(audio);
        return;
    }

    UriQuery query("musescore://project/alsoshareaudiocom");
    query.addParam("rememberChoice", Val(!configuration()->hasAskedAlsoShareAudioCom()));

    openDialog(query).onResolve(this, [this, audio](const RetVal<Val>& rv) {
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
    });
}

Promise<Ret> SaveProjectScenario::askAudioGenerationSettings() const
{
    return openDialog(UriQuery("musescore://project/audiogenerationsettings"))
           .then<Ret>(this, [this](const RetVal<Val>& res, auto resolve) {
        if (!res.ret) {
            return resolve(res.ret);
        }

        configuration()->setHasAskedAudioGenerationSettings(true);

        return resolve(make_ok());
    });
}

Promise<RetVal<bool> > SaveProjectScenario::needGenerateAudio(bool isPublicUpload) const
{
    if (isPublicUpload) {
        return resolvedPromise(RetVal<bool>::make_ok(true));
    }

    if (configuration()->hasAskedAudioGenerationSettings()) {
        return resolvedPromise(RetVal<bool>::make_ok(needGenerateAudioAccordingToSettings()));
    }

    return askAudioGenerationSettings().then<RetVal<bool> >(this, [this](const Ret& ret, auto resolve) {
        if (!ret) {
            return resolve(RetVal<bool>(ret));
        }

        return resolve(RetVal<bool>::make_ok(needGenerateAudioAccordingToSettings()));
    });
}

bool SaveProjectScenario::needGenerateAudioAccordingToSettings() const
{
    switch (configuration()->generateAudioTimePeriodType()) {
    case GenerateAudioTimePeriodType::Never:
        return false;
    case GenerateAudioTimePeriodType::Always:
        return true;
    case GenerateAudioTimePeriodType::AfterCertainNumberOfSaves: {
        int requiredNumberOfSaves = configuration()->numberOfSavesToGenerateAudio();
        if (requiredNumberOfSaves <= 0) {
            LOGW() << "invalid number of saves to generate audio: " << requiredNumberOfSaves;
            return true;
        }

        return m_numberOfSavesToCloud % requiredNumberOfSaves == 0;
    }
    }

    return false;
}

SaveProjectScenario::AudioFile SaveProjectScenario::exportMp3(const INotationPtr notation) const
{
    QString mp3Path;
    {
        QTemporaryFile tempFile(configuration()->temporaryMp3FilePathTemplate().toQString());
        if (!tempFile.open()) {
            LOGE() << "Could not create a temp file";
            return AudioFile();
        }

        mp3Path = QFileInfo(tempFile).absoluteFilePath();
    }

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
        fileSystem()->remove(mp3Path);
        return AudioFile();
    }

    std::shared_ptr<QFile> exportedFile(new QFile(mp3Path), [this](QFile* file) {
        file->close();
        fileSystem()->remove(file->fileName());
        delete file;
    });

    if (!exportedFile->open(QIODevice::ReadOnly)) {
        LOGE() << "Could not reopen exported mp3: " << mp3Path;
        return AudioFile();
    }

    AudioFile audio;
    audio.format = "mp3";
    audio.device = exportedFile;

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

Promise<Ret> SaveProjectScenario::uploadProject(const CloudProjectInfo& info, const AudioFile& audio, bool openEditUrl, bool publishMode)
{
    return async::make_promise<Ret>([this, info, audio, openEditUrl, publishMode](auto resolve) {
        INotationProjectPtr project = globalContext()->currentProject();
        if (!project) {
            return resolve(make_ret(Err::NoProjectError));
        }

        auto projectData = std::make_shared<QBuffer>();
        projectData->open(QIODevice::WriteOnly);

        Ret ret = project->writeToDevice(projectData.get());
        if (!ret) {
            LOGE() << ret.toString();
            return resolve(ret);
        }

        projectData->close();
        projectData->open(QIODevice::ReadOnly);

        bool isFirstSave = info.sourceUrl.isEmpty();

        ProgressPtr progress = museScoreComService()->uploadScore(projectData, info.name, info.visibility, info.sourceUrl,
                                                                  info.revisionId);
        m_uploadingProjectProgress = progress;

        showUploadProgressDialog();
        LOGD() << "Uploading project started";

        progress->progressChanged().onReceive(this, [](int64_t current, int64_t total, const std::string&) {
            if (total > 0) {
                LOGD() << "Uploading project progress: " << current << " / " << total << " bytes";
            }
        });

        progress->finished().onReceive(this, [this, project, info, audio, openEditUrl, publishMode,
                                              isFirstSave, progress, resolve](const ProgressResult& res) {
            progress->progressChanged().disconnect(this);
            progress->finished().disconnect(this);

            if (!res.ret) {
                LOGE() << res.ret.toString();
                onProjectUploadFailed(res.ret, info, audio, openEditUrl, publishMode).onResolve(this, [resolve](const Ret& failRet) {
                    (void)resolve(failRet);
                });
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

            (void)resolve(res.ret);
        });

        return Promise<Ret>::dummy_result();
    });
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

Promise<Ret> SaveProjectScenario::onProjectUploadFailed(const Ret& ret, const CloudProjectInfo& info, const AudioFile& audio,
                                                        bool openEditUrl, bool publishMode)
{
    setBusy(BusyStatus::Uploading, false);

    closeUploadProgressDialog();

    Ret userResponse = showCloudSaveError(ret, info, publishMode, true);
    switch (userResponse.code()) {
    case RET_CODE_CONFLICT_RESPONSE_SAVE_AS: {
        // The save that started this upload still holds the status; the new flow takes it over
        setBusy(BusyStatus::Saving, false);
        return saveProject(SaveMode::SaveAs);
    }
    case RET_CODE_CONFLICT_RESPONSE_PUBLISH_AS_NEW_SCORE: {
        CloudProjectInfo newInfo = info;
        newInfo.sourceUrl = QUrl();
        return uploadProject(newInfo, audio, openEditUrl, publishMode);
    }
    case RET_CODE_CONFLICT_RESPONSE_REPLACE: {
        RetVal<muse::cloud::ScoreInfo> scoreInfo = museScoreComService()->downloadScoreInfo(info.sourceUrl);
        if (!scoreInfo.ret) {
            LOGE() << scoreInfo.ret.toString();
            showCloudSaveError(scoreInfo.ret, info, publishMode, false);
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

    return resolvedPromise(ret);
}

void SaveProjectScenario::onAudioSuccessfullyUploaded(const QUrl& urlToOpen)
{
    closeUploadProgressDialog();

    platformInteractive()->openUrl(urlToOpen);
}

void SaveProjectScenario::onAudioUploadFailed(const Ret& ret)
{
    closeUploadProgressDialog();

    showAudioCloudShareError(ret);
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

        openProjectScenario()->revertToLastSaved();
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

static std::string saveCloudStatusCodeErrorMessage(const Ret& ret, bool withHelp = false)
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

Promise<RetVal<SaveLocation> > SaveProjectScenario::askSaveLocation(INotationProjectPtr project, SaveMode mode,
                                                                    SaveLocationType preselectedType) const
{
    if (preselectedType != SaveLocationType::Undefined) {
        return askSaveLocationOfType(project, mode, preselectedType);
    }

    return saveLocationType()
           .then<RetVal<SaveLocation> >(this, [this, project, mode](const RetVal<SaveLocationType>& type, auto resolve) {
        if (!type.ret) {
            return resolve(RetVal<SaveLocation>(type.ret));
        }

        askSaveLocationOfType(project, mode, type.val).onResolve(this, [resolve](const RetVal<SaveLocation>& location) {
            (void)resolve(location);
        });

        return Promise<RetVal<SaveLocation> >::dummy_result();
    });
}

Promise<RetVal<SaveLocation> > SaveProjectScenario::askSaveLocationOfType(INotationProjectPtr project, SaveMode mode,
                                                                          SaveLocationType type) const
{
    IF_ASSERT_FAILED(type != SaveLocationType::Undefined) {
        return resolvedPromise(RetVal<SaveLocation>(make_ret(Ret::Code::UnknownError)));
    }

    // The user may switch between Local and Cloud as often as they want
    configuration()->setLastUsedSaveLocationType(type);

    if (type == SaveLocationType::Local) {
        RetVal<muse::io::path_t> path = askLocalPath(project, mode);
        switch (path.ret.code()) {
        case int(Ret::Code::Ok):
            return resolvedPromise(RetVal<SaveLocation>::make_ok(SaveLocation(path.val)));
        case RET_CODE_CHANGE_SAVE_LOCATION_TYPE:
            return askSaveLocationOfType(project, mode, SaveLocationType::Cloud);
        default:
            return resolvedPromise(RetVal<SaveLocation>(path.ret));
        }
    }

    return askCloudLocation(project, mode)
           .then<RetVal<SaveLocation> >(this, [this, project, mode](const RetVal<CloudProjectInfo>& info, auto resolve) {
        switch (info.ret.code()) {
            case int(Ret::Code::Ok):
                return resolve(RetVal<SaveLocation>::make_ok(SaveLocation(info.val)));
            case RET_CODE_CHANGE_SAVE_LOCATION_TYPE:
                askSaveLocationOfType(project, mode, SaveLocationType::Local)
                .onResolve(this, [resolve](const RetVal<SaveLocation>& location) {
                (void)resolve(location);
            });
                return Promise<RetVal<SaveLocation> >::dummy_result();
            default:
                return resolve(RetVal<SaveLocation>(info.ret));
        }
    });
}

RetVal<muse::io::path_t> SaveProjectScenario::askLocalPath(INotationProjectPtr project, SaveMode saveMode) const
{
    std::string dialogTitle = muse::trc("project/save", "Save score");
    std::string filenameAddition;

    if (saveMode == SaveMode::SaveCopy) {
        //: used to form a filename suggestion, like "originalFile - copy"
        filenameAddition = " - " + muse::trc("project/save", "copy", "a copy of a file");
    } else if (saveMode == SaveMode::SaveSelection) {
        //: used to form a filename suggestion, like "originalFile - selection"
        filenameAddition = " - " + muse::trc("project/save", "selection");
    }

    muse::io::path_t defaultPath = configuration()->defaultSavingFilePath(project, filenameAddition);

    std::vector<std::string> filter {
        muse::trc("project", "MuseScore file") + " (*.mscz)",
        muse::trc("project", "Uncompressed MuseScore folder [experimental]")
#ifdef Q_OS_MAC
        + " (*)"
#else
        + " (*.)"
#endif
    };

    muse::io::path_t selectedPath = interactive()->selectSavingFileSync(dialogTitle, defaultPath, filter);

    if (selectedPath.empty()) {
        return make_ret(Ret::Code::Cancel);
    }

    if (!engraving::isMuseScoreFile(io::suffix(selectedPath))) {
        // Then it must be that the user is trying to save a mscx file.
        // At the selected path, a folder will be created,
        // and inside the folder, a mscx file will be created.
        // We should return the path to the mscx file.
        selectedPath = selectedPath.appendingComponent(io::filename(selectedPath)).appendingSuffix(engraving::MSCX);
    }

    configuration()->setLastSavedProjectsPath(io::dirpath(selectedPath));

    return RetVal<muse::io::path_t>::make_ok(selectedPath);
}

Promise<RetVal<SaveLocationType> > SaveProjectScenario::saveLocationType() const
{
    bool shouldAsk = configuration()->shouldAskSaveLocationType();
    SaveLocationType lastUsed = configuration()->lastUsedSaveLocationType();
    if (!shouldAsk && lastUsed != SaveLocationType::Undefined) {
        return resolvedPromise(RetVal<SaveLocationType>::make_ok(lastUsed));
    }

    return askSaveLocationType();
}

Promise<RetVal<SaveLocationType> > SaveProjectScenario::askSaveLocationType() const
{
    UriQuery query("musescore://project/asksavelocationtype");
    bool shouldAsk = configuration()->shouldAskSaveLocationType();
    query.addParam("askAgain", Val(shouldAsk));

    return openDialog(query).then<RetVal<SaveLocationType> >(this, [this](const RetVal<Val>& rv, auto resolve) {
        if (!rv.ret) {
            return resolve(RetVal<SaveLocationType>(rv.ret));
        }

        QVariantMap vals = rv.val.toQVariant().toMap();

        bool askAgain = vals["askAgain"].toBool();
        configuration()->setShouldAskSaveLocationType(askAgain);

        SaveLocationType type = static_cast<SaveLocationType>(vals["saveLocationType"].toInt());
        return resolve(RetVal<SaveLocationType>::make_ok(type));
    });
}

Promise<RetVal<CloudProjectInfo> > SaveProjectScenario::askCloudLocation(INotationProjectPtr project, SaveMode mode) const
{
    return doAskCloudLocation(project, mode, false);
}

Promise<RetVal<CloudProjectInfo> > SaveProjectScenario::askPublishLocation(INotationProjectPtr project) const
{
    return doAskCloudLocation(project, SaveMode::Save, true);
}

Promise<RetVal<CloudAudioInfo> > SaveProjectScenario::askShareAudioLocation(INotationProjectPtr project) const
{
    bool isCloudAvailable = audioComService()->authorization()->checkCloudIsAvailable();
    if (!isCloudAvailable) {
        return resolvedPromise(RetVal<CloudAudioInfo>(warnCloudNotAvailableForSharingAudio()));
    }

    std::string dialogText = muse::trc("project/save", "Log in or create a new account on Audio.com to share your music.");

    return ensureAuthorization(muse::cloud::AUDIO_COM_CLOUD_CODE, false, dialogText)
           .then<RetVal<CloudAudioInfo> >(this, [this, project](const RetVal<Val>& auth, auto resolve) {
        if (!auth.ret) {
            return resolve(RetVal<CloudAudioInfo>(auth.ret));
        }

        QString defaultName = project->displayName();
        QUrl uploadUrl = project->cloudAudioInfo().url;
        cloud::Visibility defaultVisibility = cloud::Visibility::Public;

        UriQuery query("musescore://project/savetocloud");
        query.addParam("isPublishShare", Val(true));
        query.addParam("name", Val(defaultName));
        query.addParam("visibility", Val(defaultVisibility));
        query.addParam("cloudCode", Val(cloud::AUDIO_COM_CLOUD_CODE));

        if (!uploadUrl.isEmpty()) {
            query.addParam("existingScoreOrAudioUrl", Val(uploadUrl.toString()));
        }

        openDialog(query).onResolve(this, [resolve, uploadUrl](const RetVal<Val>& rv) {
            if (!rv.ret) {
                (void)resolve(RetVal<CloudAudioInfo>(rv.ret));
                return;
            }

            QVariantMap vals = rv.val.toQVariant().toMap();
            using Response = cloud::SaveToCloudResponse::SaveToCloudResponse;
            auto response = static_cast<Response>(vals["response"].toInt());
            switch (response) {
                case Response::Cancel:
                case Response::SaveLocallyInstead:
                    (void)resolve(RetVal<CloudAudioInfo>(make_ret(Ret::Code::Cancel)));
                    return;
                case Response::Ok:
                    break;
            }

            CloudAudioInfo result;
            result.name = vals["name"].toString();
            result.visibility = static_cast<cloud::Visibility>(vals["visibility"].toInt());
            result.replaceExisting = vals["replaceExisting"].toBool() && !uploadUrl.isEmpty();

            (void)resolve(RetVal<CloudAudioInfo>::make_ok(result));
        });

        return Promise<RetVal<CloudAudioInfo> >::dummy_result();
    });
}

Promise<RetVal<CloudProjectInfo> > SaveProjectScenario::doAskCloudLocation(INotationProjectPtr project, SaveMode mode,
                                                                           bool isPublishShare) const
{
    bool isCloudAvailable = museScoreComService()->authorization()->checkCloudIsAvailable();
    if (!isCloudAvailable) {
        return resolvedPromise(RetVal<CloudProjectInfo>(warnCloudNotAvailableForUploading(isPublishShare)));
    }

    std::string dialogText = isPublishShare
                             ? muse::trc("project/save", "Log in to MuseScore.com to publish this score.")
                             : muse::trc("project/save", "Log in to MuseScore.com to save this score to the cloud.");

    return ensureAuthorization(muse::cloud::MUSESCORE_COM_CLOUD_CODE, true, dialogText)
           .then<RetVal<CloudProjectInfo> >(this, [this, project, mode, isPublishShare](const RetVal<Val>& auth, auto resolve) {
        if (!auth.ret) {
            return resolve(RetVal<CloudProjectInfo>(auth.ret));
        }

        using Response = cloud::SaveToCloudResponse::SaveToCloudResponse;
        if (static_cast<Response>(auth.val.toInt()) == Response::SaveLocallyInstead) {
            return resolve(RetVal<CloudProjectInfo>(Ret(RET_CODE_CHANGE_SAVE_LOCATION_TYPE)));
        }

        askCloudProjectInfo(project, mode, isPublishShare).onResolve(this, [resolve](const RetVal<CloudProjectInfo>& info) {
            (void)resolve(info);
        });

        return Promise<RetVal<CloudProjectInfo> >::dummy_result();
    });
}

Promise<RetVal<CloudProjectInfo> > SaveProjectScenario::askCloudProjectInfo(INotationProjectPtr project, SaveMode mode,
                                                                            bool isPublishShare) const
{
    QString defaultName = project->displayName();
    cloud::Visibility defaultVisibility = isPublishShare ? cloud::Visibility::Public : cloud::Visibility::Private;
    const CloudProjectInfo existingProjectInfo = project->cloudInfo();

    QUrl existingScoreUrl = existingProjectInfo.sourceUrl;

    if (!existingScoreUrl.isEmpty()) {
        RetVal<cloud::ScoreInfo> scoreInfo = museScoreComService()->downloadScoreInfo(existingScoreUrl);

        if (scoreInfo.val.isValid()) {
            const cloud::AccountInfo& accountInfo = museScoreComService()->authorization()->accountInfo();
            if (accountInfo.id.toInt() != scoreInfo.val.owner.id) {
                existingScoreUrl = QUrl();
            }
        }

        switch (scoreInfo.ret.code()) {
        case int(Ret::Code::Ok):
            defaultName = scoreInfo.val.title;
            if (!isPublishShare) {
                defaultVisibility = scoreInfo.val.visibility;
            }
            break;

        case int(cloud::Err::Status400_InvalidRequest):
        case int(cloud::Err::Status403_AccountNotActivated):
        case int(cloud::Err::Status422_ValidationFailed):
        case int(cloud::Err::Status429_RateLimitExceeded):
        case int(cloud::Err::Status500_InternalServerError):
        case int(cloud::Err::UnknownStatusCode):
        case int(cloud::Err::NetworkError):
            return resolvedPromise(RetVal<CloudProjectInfo>(showCloudSaveError(scoreInfo.ret, project->cloudInfo(), isPublishShare,
                                                                               false)));

        // It's possible the source URL is invalid or points to a score on a different user's account.
        // In this situation we shouldn't show an error.
        default: break;
        }
    }

    UriQuery query("musescore://project/savetocloud");
    query.addParam("isPublishShare", Val(isPublishShare));
    query.addParam("name", Val(defaultName));
    query.addParam("visibility", Val(defaultVisibility));
    query.addParam("existingScoreOrAudioUrl", Val(existingScoreUrl.toString()));
    query.addParam("cloudCode", Val(cloud::MUSESCORE_COM_CLOUD_CODE));

    return openDialog(query)
           .then<RetVal<CloudProjectInfo> >(this, [this, mode, isPublishShare, existingProjectInfo](const RetVal<Val>& rv, auto resolve) {
        if (!rv.ret) {
            return resolve(RetVal<CloudProjectInfo>(rv.ret));
        }

        QVariantMap vals = rv.val.toQVariant().toMap();
        using Response = cloud::SaveToCloudResponse::SaveToCloudResponse;
        auto response = static_cast<Response>(vals["response"].toInt());
        switch (response) {
            case Response::Cancel:
                return resolve(RetVal<CloudProjectInfo>(make_ret(Ret::Code::Cancel)));
            case Response::SaveLocallyInstead:
                return resolve(RetVal<CloudProjectInfo>(Ret(RET_CODE_CHANGE_SAVE_LOCATION_TYPE)));
            case Response::Ok:
                break;
        }

        CloudProjectInfo result;

        if ((mode == SaveMode::Save || isPublishShare) && vals["replaceExisting"].toBool()) {
            result = existingProjectInfo;
        }

        result.name = vals["name"].toString();
        result.visibility = static_cast<cloud::Visibility>(vals["visibility"].toInt());

        if (!warnBeforePublishing(isPublishShare, result.visibility)) {
            return resolve(RetVal<CloudProjectInfo>(make_ret(Ret::Code::Cancel)));
        }

        return resolve(RetVal<CloudProjectInfo>::make_ok(result));
    });
}

bool SaveProjectScenario::warnBeforePublishing(bool isPublishShare, cloud::Visibility visibility) const
{
    if (isPublishShare) {
        if (!configuration()->shouldWarnBeforePublish()) {
            return true;
        }
    } else {
        if (!configuration()->shouldWarnBeforeSavingPubliclyToCloud()) {
            return true;
        }
    }

    std::string title, message;

    IInteractive::ButtonDatas buttons = {
        IInteractive::ButtonData(IInteractive::Button::Cancel, muse::trc("global", "Cancel")),
        IInteractive::ButtonData(IInteractive::Button::Ok, muse::trc("project/save", "Publish"), true)
    };

    IInteractive::Options options = IInteractive::Option::WithIcon | IInteractive::Option::WithDontShowAgainCheckBox;

    if (isPublishShare) {
        title = muse::trc("project/save", "Publish changes online?");
        message = muse::trc("project/save", "We will need to generate a new MP3 for web playback.");
    } else if (visibility == cloud::Visibility::Public) {
        title = muse::trc("project/save", "Publish this score online?"),
        message = muse::trc("project/save", "All saved changes will be publicly visible on MuseScore.com. "
                                            "If you want to make frequent changes, we recommend saving this "
                                            "score privately until you’re ready to share it to the world.");
    } else {
        return true;
    }

    IInteractive::Result result = interactive()->warningSync(title, message, buttons, int(IInteractive::Button::Ok), options);

    bool ok = result.standardButton() == IInteractive::Button::Ok;
    if (ok && !result.showAgain()) {
        if (isPublishShare) {
            configuration()->setShouldWarnBeforePublish(false);
        } else {
            configuration()->setShouldWarnBeforeSavingPubliclyToCloud(false);
        }
    }

    return ok;
}

bool SaveProjectScenario::warnBeforeSavingToExistingPubliclyVisibleCloudProject() const
{
    IInteractive::ButtonDatas buttons = {
        IInteractive::ButtonData(IInteractive::Button::Cancel, muse::trc("global", "Cancel")),
        IInteractive::ButtonData(IInteractive::Button::Ok, muse::trc("project/save", "Publish"), true)
    };

    IInteractive::Result result = interactive()->warningSync(
        muse::trc("project/save", "Publish changes online?"),
        muse::trc("project/save", "Your saved changes will be publicly visible. We will also "
                                  "need to generate a new MP3 for public playback."),
        buttons, int(IInteractive::Button::Ok));

    return result.standardButton() == IInteractive::Button::Ok;
}

Ret SaveProjectScenario::warnCloudNotAvailableForUploading(bool isPublishShare) const
{
    if (isPublishShare) {
        interactive()->warningSync(muse::trc("project/save", "Unable to connect to MuseScore.com"),
                                   muse::trc("project/save", "Please check your internet connection or try again later."));
        return make_ret(Ret::Code::Cancel);
    }

    IInteractive::ButtonDatas buttons = {
        IInteractive::ButtonData(IInteractive::Button::Cancel, muse::trc("global", "Cancel")),
        IInteractive::ButtonData(IInteractive::Button::Ok, muse::trc("project/save", "Save to computer"), true)
    };

    IInteractive::Result result = interactive()->warningSync(muse::trc("project/save", "Unable to connect to the cloud"),
                                                             muse::trc("project/save",
                                                                       "Please check your internet connection or try again later."),
                                                             buttons, int(IInteractive::Button::Ok));

    if (result.standardButton() == IInteractive::Button::Ok) {
        return Ret(RET_CODE_CHANGE_SAVE_LOCATION_TYPE);
    }

    return make_ret(Ret::Code::Cancel);
}

Ret SaveProjectScenario::warnCloudNotAvailableForSharingAudio() const
{
    interactive()->warningSync(muse::trc("project/save", "Unable to connect to Audio.com"),
                               muse::trc("project/save", "Please check your internet connection or try again later."));
    return make_ret(Ret::Code::Cancel);
}

Promise<RetVal<Val> > SaveProjectScenario::ensureAuthorization(const QString& cloudCode, bool publishingScore,
                                                               const std::string& text) const
{
    IF_ASSERT_FAILED(cloudCode == muse::cloud::MUSESCORE_COM_CLOUD_CODE || cloudCode == muse::cloud::AUDIO_COM_CLOUD_CODE) {
        return resolvedPromise(RetVal<Val>(make_ret(Err::UnknownError)));
    }

    bool isMuseScoreCom = cloudCode == muse::cloud::MUSESCORE_COM_CLOUD_CODE;
    bool userAuthorized = isMuseScoreCom ? museScoreComService()->authorization()->userAuthorized().val
                          : audioComService()->authorization()->userAuthorized().val;

    if (userAuthorized) {
        return resolvedPromise(RetVal<Val>::make_ok(Val()));
    }

    UriQuery query("muse://cloud/requireauthorization");
    query.addParam("text", Val(text));
    query.addParam("cloudCode", Val(cloudCode));
    query.addParam("publishingScore", Val(publishingScore));
    return openDialog(query);
}

Ret SaveProjectScenario::showCloudSaveError(const Ret& ret, const CloudProjectInfo& info, bool isPublishShare,
                                            bool alreadyAttempted) const
{
    std::string title;
    if (alreadyAttempted) {
        title = isPublishShare
                ? muse::trc("project/save", "Your score could not be published")
                : muse::trc("project/save", "Your score could not be saved to the cloud");
    } else {
        title = isPublishShare
                ? muse::trc("project/save", "Your score cannot be published")
                : muse::trc("project/save", "Your score cannot be saved to the cloud");
    }

    std::string msg;

    static constexpr int helpBtnCode = int(IInteractive::Button::CustomButton) + 1;
    static constexpr int saveLocallyBtnCode = int(IInteractive::Button::CustomButton) + 2;
    static constexpr int saveAsBtnCode = int(IInteractive::Button::CustomButton) + 3;
    static constexpr int publishAsNewScoreBtnCode = int(IInteractive::Button::CustomButton) + 4;
    static constexpr int replaceBtnCode = int(IInteractive::Button::CustomButton) + 5;

    IInteractive::ButtonData okBtn = interactive()->buttonData(IInteractive::Button::Ok);
    IInteractive::ButtonData saveLocallyBtn { saveLocallyBtnCode, muse::trc("project/save", "Save to computer") };
    IInteractive::ButtonData helpBtn { helpBtnCode, muse::trc("project/save", "Get help") };

    IInteractive::ButtonDatas buttons = (alreadyAttempted || isPublishShare)
                                        ? (IInteractive::ButtonDatas { helpBtn, okBtn })
                                        : (IInteractive::ButtonDatas { helpBtn, saveLocallyBtn, okBtn });

    int defaultButtonCode = okBtn.btn;

    switch (ret.code()) {
    case int(cloud::Err::Status403_AccountNotActivated):
        msg = muse::trc("project/cloud", "Your MuseScore.com account needs to be verified first. "
                                         "Please activate your account via the link in the activation email.");
        buttons = { okBtn };
        break;
    case int(cloud::Err::Status409_Conflict):
        title = muse::trc("project/save", "There are conflicting changes in the online score");
        if (isPublishShare) {
            msg = muse::qtrc("project/save", "You can replace the <a href=\"%1\">online score</a>, or publish this as a new score "
                                             "to avoid losing changes in the current online version.")
                  .arg(info.sourceUrl.toString())
                  .toStdString();
            buttons = {
                interactive()->buttonData(IInteractive::Button::Cancel),
                IInteractive::ButtonData { publishAsNewScoreBtnCode, muse::trc("project/save", "Publish as new score") },
                IInteractive::ButtonData { replaceBtnCode, muse::trc("project/save", "Replace") }
            };
            defaultButtonCode = replaceBtnCode;
        } else {
            msg = muse::qtrc("project/save", "You can replace the <a href=\"%1\">online score</a>, or save this as a new file "
                                             "to avoid losing changes in the current online version.")
                  .arg(info.sourceUrl.toString())
                  .toStdString();
            buttons = {
                interactive()->buttonData(IInteractive::Button::Cancel),
                IInteractive::ButtonData { saveAsBtnCode, muse::trc("project/save", "Save as…") },
                IInteractive::ButtonData { replaceBtnCode, muse::trc("project/save", "Replace") }
            };
            defaultButtonCode = replaceBtnCode;
        }
        break;

    case int(cloud::Err::Status400_InvalidRequest):
    case int(cloud::Err::Status401_AuthorizationRequired):
    case int(cloud::Err::Status422_ValidationFailed):
    case int(cloud::Err::Status429_RateLimitExceeded):
    case int(cloud::Err::Status500_InternalServerError):
    case int(cloud::Err::UnknownStatusCode):
        msg = saveCloudStatusCodeErrorMessage(ret, /*withHelp=*/ true);
        break;

    case int(cloud::Err::NetworkError):
        msg = muse::mtrc("project/cloud", "Could not connect to <a href=\"%1\">MuseScore.com</a>. "
                                          "Please check your internet connection or try again later.")
              .arg(u"https://musescore.com").toStdString();
        break;
    default:
        msg = muse::trc("project/cloud", "Please try again later, or get help for this problem on MuseScore.com.");
        break;
    }

    IInteractive::Result result = interactive()->warningSync(title, msg, buttons, defaultButtonCode);
    switch (result.button()) {
    case helpBtnCode:
        platformInteractive()->openUrl(configuration()->dotComBugReportUrl());
        break;
    case saveLocallyBtnCode:
        return Ret(RET_CODE_CHANGE_SAVE_LOCATION_TYPE);
    case saveAsBtnCode:
        return Ret(RET_CODE_CONFLICT_RESPONSE_SAVE_AS);
    case publishAsNewScoreBtnCode:
        return Ret(RET_CODE_CONFLICT_RESPONSE_PUBLISH_AS_NEW_SCORE);
    case replaceBtnCode:
        return Ret(RET_CODE_CONFLICT_RESPONSE_REPLACE);
    }

    return make_ret(Ret::Code::Cancel);
}

Ret SaveProjectScenario::showAudioCloudShareError(const Ret& ret) const
{
    std::string title= muse::trc("project/share", "Your audio could not be shared");
    std::string msg;

    IInteractive::ButtonData okBtn = interactive()->buttonData(IInteractive::Button::Ok);
    IInteractive::ButtonDatas buttons = IInteractive::ButtonDatas { okBtn };

    switch (ret.code()) {
    case int(cloud::Err::Status403_AccountNotActivated):
        msg = muse::trc("project/share", "Your Audio.com account needs to be verified first. "
                                         "Please activate your account via the link in the activation email.");
        break;
    case int(cloud::Err::UnknownStatusCode): {
        if (const auto status = ret.data<int>("status", -1); status != -1) {
            //: %1 will be replaced with the error code that audio.com returned, which is a number.
            msg = muse::qtrc("project/share", "Audio.com returned an unknown error code: %1.")
                  .arg(status).toStdString();
        } else {
            msg = muse::trc("project/share", "Audio.com returned an unknown error code.");
        }
        msg += "\n\n" + muse::trc("project/share", "Please try again later, or get help for this problem on Audio.com.");
    } break;
    case int(cloud::Err::NetworkError):
        msg = muse::trc("project/share", "Could not connect to Audio.com. "
                                         "Please check your internet connection or try again later.");
        break;
    default:
        msg = muse::trc("project/share", "Please try again later, or get help for this problem on Audio.com.");
        break;
    }

    interactive()->warning(title, msg, buttons);

    return muse::make_ok();
}
