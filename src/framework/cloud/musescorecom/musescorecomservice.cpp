/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-CLA-applies
 *
 * MuseScore
 * Music Composition & Notation
 *
 * Copyright (C) 2025 MuseScore Limited and others
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

#include "musescorecomservice.h"

#include <QBuffer>
#include <QHttpMultiPart>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QRandomGenerator>

#include "clouderrors.h"

#include "log.h"

using namespace muse;
using namespace muse::cloud;
using namespace muse::network;
using namespace muse::async;

static const QString MUSESCORECOM_CLOUD_TITLE("MuseScore.com");
static const QString MUSESCORECOM_CLOUD_URL("https://musescore.com");
static const QString MUSESCORECOM_API_ROOT_URL("https://desktop.musescore.com/editor/v1");
static const QUrl MUSESCORECOM_SCORE_MANAGER_URL(MUSESCORECOM_CLOUD_URL + "/my-scores");
static const QUrl MUSESCORECOM_USER_INFO_API_URL(MUSESCORECOM_API_ROOT_URL + "/me");

static const QUrl MUSESCORECOM_SCORE_INFO_API_URL(MUSESCORECOM_API_ROOT_URL + "/score/info");
static const QUrl MUSESCORECOM_SCORES_LIST_API_URL(MUSESCORECOM_API_ROOT_URL + "/collection/scores");
static const QUrl MUSESCORECOM_SCORE_DOWNLOAD_API_URL(MUSESCORECOM_API_ROOT_URL + "/score/download");
static const QUrl MUSESCORECOM_SCORE_DOWNLOAD_SHARED_API_URL(MUSESCORECOM_API_ROOT_URL + "/score/download-shared");
static const QUrl MUSESCORECOM_UPLOAD_SCORE_API_URL(MUSESCORECOM_API_ROOT_URL + "/score/upload");
static const QUrl MUSESCORECOM_UPLOAD_AUDIO_API_URL(MUSESCORECOM_API_ROOT_URL + "/score/audio");

static const QString MUSESCORE_TEXT_LOGO("https://musescore.com/static/public/musescore/img/logo/musescore-logo.svg");

static const QString SCORE_ID_KEY("score_id");
static const QString EDITOR_SOURCE_KEY("editor_source");
static const QString EDITOR_SOURCE_VALUE("Musescore Editor %1");
static const QString PLATFORM_KEY("platform");

static int generateFileNameNumber()
{
    return QRandomGenerator::global()->generate() % 100000;
}

static RetVal<AccountInfo> parseMuseScoreComAccountInfo(const QByteArray& data)
{
    QJsonParseError err;
    QJsonDocument doc = QJsonDocument::fromJson(data, &err);
    if (err.error != QJsonParseError::NoError || !doc.isObject()) {
        return muse::make_ret(Ret::Code::InternalError, err.errorString().toStdString());
    }

    QJsonObject user = doc.object();

    AccountInfo info;
    info.id = QString::number(user.value("id").toInt());
    info.userName = user.value("name").toString();
    QString profileUrl = user.value("profile_url").toString();
    info.profileUrl = QUrl(profileUrl);
    info.avatarUrl = QUrl(user.value("avatar_url").toString());
    info.collectionUrl = QUrl(profileUrl + "/sheetmusic");

    return RetVal<AccountInfo>::make_ok(info);
}

static RetVal<ScoresList> parseScoreList(const QByteArray& data, int batchNumber)
{
    QJsonParseError err;
    QJsonDocument doc = QJsonDocument::fromJson(data);
    if (err.error != QJsonParseError::NoError || !doc.isObject()) {
        return RetVal<ScoresList>::make_ret((int)Ret::Code::InternalError, err.errorString().toStdString());
    }

    QJsonObject obj = doc.object();
    QJsonObject metaObj = obj.value("_meta").toObject();

    ScoresList result;
    result.meta.totalScoresCount = metaObj.value("totalCount").toInt();
    result.meta.batchesCount = metaObj.value("pageCount").toInt();
    result.meta.thisBatchNumber = metaObj.value("currentPage").toInt();
    result.meta.scoresPerBatch = metaObj.value("perPage").toInt();

    if (result.meta.thisBatchNumber < batchNumber) {
        // This happens when the requested page number was too high.
        // In this situation, the API just returns the last page and the items from that page.
        // We will return just an empty list, in order not to confuse the caller.
        return RetVal<ScoresList>::make_ok(result);
    }

    QJsonArray items = obj.value("items").toArray();
    result.items.reserve(items.size());

    for (const QJsonValue itemVal : items) {
        QJsonObject itemObj = itemVal.toObject();

        ScoresList::Item item;
        item.id = itemObj.value("id").toInt();
        item.title = itemObj.value("title").toString();
        item.lastModified = QDateTime::fromSecsSinceEpoch(itemObj.value("date_updated").toInt());
        item.fileSize = itemObj.value("current_revision").toObject().value("file_size").toInt();
        item.thumbnailUrl = itemObj.value("thumbnails").toObject().value("small").toString();
        item.visibility = static_cast<Visibility>(itemObj.value("privacy").toInt());
        item.viewCount = itemObj.value("view_count").toInt();

        result.items.push_back(item);
    }

    return RetVal<ScoresList>::make_ok(result);
}

static RetVal<ScoreInfo> parseScoreInfo(const QByteArray& data)
{
    QJsonParseError err;
    QJsonDocument doc = QJsonDocument::fromJson(data);
    if (err.error != QJsonParseError::NoError || !doc.isObject()) {
        return RetVal<ScoreInfo>::make_ret((int)Ret::Code::InternalError, err.errorString().toStdString());
    }

    QJsonObject scoreInfo = doc.object();
    QJsonObject owner = scoreInfo.value("user").toObject();

    ScoreInfo result;
    result.id = scoreInfo.value("id").toInt();
    result.revisionId = scoreInfo.value("revision_id").toInt();
    result.title = scoreInfo.value("title").toString();
    result.description = scoreInfo.value("description").toString();
    result.license = scoreInfo.value("license").toString();
    result.tags = scoreInfo.value("tags").toString().split(',');
    result.visibility = static_cast<Visibility>(scoreInfo.value("privacy").toInt());
    result.url = scoreInfo.value("custom_url").toString();
    result.owner.id = owner.value("uid").toInt();
    result.owner.userName = owner.value("username").toString();
    result.owner.profileUrl = owner.value("custom_url").toString();

    return RetVal<ScoreInfo>::make_ok(result);
}

static RetVal<Val> parseScoreUploadResponse(const QByteArray& data)
{
    QJsonParseError err;
    QJsonDocument doc = QJsonDocument::fromJson(data, &err);
    if (err.error != QJsonParseError::NoError || !doc.isObject()) {
        return muse::make_ret(Ret::Code::InternalError, err.errorString().toStdString());
    }

    QJsonObject scoreInfo = doc.object();
    QUrl newSourceUrl = QUrl(scoreInfo.value("permalink").toString());
    QUrl editUrl = QUrl(scoreInfo.value("edit_url").toString());
    int newRevisionId = scoreInfo.value("revision_id").toInt();

    if (!newSourceUrl.isValid()) {
        return make_ret(cloud::Err::CouldNotReceiveSourceUrl);
    }

    ValMap map;
    map["sourceUrl"] = Val(newSourceUrl.toString());
    map["editUrl"] = Val(editUrl.toString());
    map["revisionId"] = Val(newRevisionId);

    return RetVal<Val>::make_ok(Val(map));
}

static QHttpMultiPartPtr makeMultiPartForScoreUpload(QIODevice* scoreData, int scoreId, const QString& title,
                                                     Visibility visibility, int revisionId,
                                                     const QString& licence, bool isScoreAlreadyUploaded)
{
    auto multiPart = std::make_shared<QHttpMultiPart>(QHttpMultiPart::FormDataType);

    QHttpPart filePart;
    filePart.setHeader(QNetworkRequest::ContentTypeHeader, QVariant("application/octet-stream"));
    QString contentDisposition = QString("form-data; name=\"score_data\"; filename=\"temp_%1.mscz\"").arg(generateFileNameNumber());
    filePart.setHeader(QNetworkRequest::ContentDispositionHeader, QVariant(contentDisposition));

    filePart.setBodyDevice(scoreData);
    multiPart->append(filePart);

    if (isScoreAlreadyUploaded) {
        QHttpPart scoreIdPart;
        scoreIdPart.setHeader(QNetworkRequest::ContentDispositionHeader, QVariant("form-data; name=\"score_id\""));
        scoreIdPart.setBody(QString::number(scoreId).toLatin1());
        multiPart->append(scoreIdPart);

        if (revisionId) {
            scoreIdPart.setHeader(QNetworkRequest::ContentDispositionHeader, QVariant("form-data; name=\"last_revision_id\""));
            scoreIdPart.setBody(QByteArray::number(revisionId));
            multiPart->append(scoreIdPart);
        }
    }

    QHttpPart titlePart;
    titlePart.setHeader(QNetworkRequest::ContentDispositionHeader, QVariant("form-data; name=\"title\""));
    titlePart.setBody(title.toUtf8());
    multiPart->append(titlePart);

    QHttpPart privacyPart;
    privacyPart.setHeader(QNetworkRequest::ContentDispositionHeader, QVariant("form-data; name=\"privacy\""));
    privacyPart.setBody(QByteArray::number(int(visibility)));
    multiPart->append(privacyPart);

    QHttpPart licensePart;
    licensePart.setHeader(QNetworkRequest::ContentDispositionHeader, QVariant("form-data; name=\"license\""));
    licensePart.setBody(licence.toUtf8());
    multiPart->append(licensePart);

    return multiPart;
}

static QHttpMultiPartPtr makeMultiPartForAudioUpload(QIODevice* audioData, const QString& audioFormat, const QUrl& sourceUrl)
{
    auto multiPart = std::make_shared<QHttpMultiPart>(QHttpMultiPart::FormDataType);

    QHttpPart audioPart;
    audioPart.setHeader(QNetworkRequest::ContentTypeHeader, QVariant("application/octet-stream"));
    QString contentDisposition = QString("form-data; name=\"audio_data\"; filename=\"temp_%1.%2\"")
                                 .arg(generateFileNameNumber())
                                 .arg(audioFormat);
    audioPart.setHeader(QNetworkRequest::ContentDispositionHeader, contentDisposition);
    audioPart.setBodyDevice(audioData);
    multiPart->append(audioPart);

    QHttpPart scoreIdPart;
    scoreIdPart.setHeader(QNetworkRequest::ContentDispositionHeader, QVariant("form-data; name=\"score_id\""));
    scoreIdPart.setBody(QString::number(idFromCloudUrl(sourceUrl).toUint64()).toLatin1());
    multiPart->append(scoreIdPart);

    return multiPart;
}

MuseScoreComService::MuseScoreComService(const modularity::ContextPtr& iocCtx, QObject* parent)
    : AbstractCloudService(iocCtx, parent)
{
}

IAuthorizationServicePtr MuseScoreComService::authorization()
{
    return shared_from_this();
}

CloudInfo MuseScoreComService::cloudInfo() const
{
    return {
        MUSESCORE_COM_CLOUD_CODE,
        MUSESCORECOM_CLOUD_TITLE,
        MUSESCORECOM_CLOUD_URL,
        MUSESCORE_TEXT_LOGO,
        logoColor()
    };
}

QUrl MuseScoreComService::scoreManagerUrl() const
{
    return MUSESCORECOM_CLOUD_URL + "/my-scores";
}

AbstractCloudService::ServerConfig MuseScoreComService::serverConfig() const
{
    ServerConfig serverConfig;
    serverConfig.serverCode = MUSESCORE_COM_CLOUD_CODE;
    serverConfig.serverUrl = MUSESCORECOM_CLOUD_URL;
    serverConfig.serverAvailabilityUrl = MUSESCORECOM_API_ROOT_URL + "/system/healthcheck";
    serverConfig.authorizationUrl = MUSESCORECOM_CLOUD_URL + "/oauth/authorize";
    serverConfig.signUpUrl = MUSESCORECOM_CLOUD_URL + "/oauth/authorize-new";
    serverConfig.signInSuccessUrl = MUSESCORECOM_CLOUD_URL + "/desktop-signin-success";
    serverConfig.accessTokenUrl = MUSESCORECOM_API_ROOT_URL + "/oauth/token";
    serverConfig.refreshApiUrl = MUSESCORECOM_API_ROOT_URL + "/oauth/refresh";
    serverConfig.logoutApiUrl = MUSESCORECOM_API_ROOT_URL + "/oauth/logout";
    serverConfig.headers = headers();

    serverConfig.authorizationParameters = {
        { EDITOR_SOURCE_KEY, EDITOR_SOURCE_VALUE.arg(application()->version().toString()) },
        { PLATFORM_KEY, QString("%1 %2 %3")
          .arg(QSysInfo::productType())
          .arg(QSysInfo::productVersion())
          .arg(QSysInfo::currentCpuArchitecture()) }
    };

    return serverConfig;
}

RequestHeaders MuseScoreComService::headers() const
{
    RequestHeaders headers = defaultHeaders();
    headers.rawHeaders["Accept"] = "application/json";
    headers.rawHeaders["X-MS-CLIENT-ID"] = QByteArray::fromStdString(configuration()->clientId());

    return headers;
}

Promise<Ret> MuseScoreComService::downloadAccountInfo()
{
    TRACEFUNC;

    return make_promise<Ret>([this](auto resolve, auto) {
        RetVal<QUrl> userInfoUrl = prepareUrlForRequest(MUSESCORECOM_USER_INFO_API_URL);
        if (!userInfoUrl.ret) {
            return resolve(userInfoUrl.ret);
        }

        auto receivedData = std::make_shared<QBuffer>();
        RetVal<Progress> progress = m_networkManager->get(userInfoUrl.val, receivedData, headers());
        if (!progress.ret) {
            return resolve(progress.ret);
        }

        progress.val.finished().onReceive(this, [this, receivedData, resolve](const ProgressResult& res) {
            if (!res.ret) {
                printServerReply(*receivedData);
                (void)resolve(res.ret);
                return;
            }

            RetVal<AccountInfo> info = parseMuseScoreComAccountInfo(receivedData->data());
            if (!info.ret) {
                (void)resolve(info.ret);
                return;
            }

            if (info.val.isValid()) {
                setAccountInfo(info.val);
            } else {
                setAccountInfo(AccountInfo());
            }

            (void)resolve(make_ok());
        });

        return Promise<Ret>::dummy_result();
    });
}

Promise<Ret> MuseScoreComService::updateTokens()
{
    TRACEFUNC;

    return make_promise<Ret>([this](auto resolve, auto) {
        QHttpPart refreshTokenPart;
        refreshTokenPart.setHeader(QNetworkRequest::ContentDispositionHeader, QString("form-data; name=\"refresh_token\""));
        refreshTokenPart.setBody(refreshToken().toUtf8());

        auto multiPart = std::make_shared<QHttpMultiPart>(QHttpMultiPart::FormDataType);
        multiPart->append(refreshTokenPart);
        auto receivedData = std::make_shared<QBuffer>();

        RetVal<Progress> progress = m_networkManager->post(serverConfig().refreshApiUrl, multiPart, receivedData, headers());
        if (!progress.ret) {
            printServerReply(*receivedData);
            return resolve(progress.ret);
        }

        progress.val.finished().onReceive(this, [this, receivedData, resolve](const ProgressResult& res) {
            if (!res.ret) {
                printServerReply(*receivedData);
                (void)resolve(res.ret);
                return;
            }

            QJsonParseError err;
            QJsonDocument doc = QJsonDocument::fromJson(receivedData->data(), &err);
            if (err.error != QJsonParseError::NoError || !doc.isObject()) {
                (void)resolve(Ret((int)Err::UnknownError, err.errorString().toStdString()));
                return;
            }

            QJsonObject tokens = doc.object();
            setAccessToken(tokens.value(ACCESS_TOKEN_KEY).toString());
            setRefreshToken(tokens.value(REFRESH_TOKEN_KEY).toString());
            (void)resolve(make_ok());
        });

        return Promise<Ret>::dummy_result();
    });
}

RetVal<ScoreInfo> MuseScoreComService::downloadScoreInfo(const QUrl& sourceUrl)
{
    return downloadScoreInfo(idFromCloudUrl(sourceUrl).toUint64());
}

RetVal<ScoreInfo> MuseScoreComService::downloadScoreInfo(int scoreId)
{
    TRACEFUNC;

    RetVal<ScoreInfo> result = RetVal<ScoreInfo>::make_ok(ScoreInfo());

    QEventLoop loop;
    doDownloadScoreInfo(scoreId).onResolve(this, [&result, &loop](const RetVal<ScoreInfo>& info) {
        result = info;
        loop.quit();
    });
    loop.exec();

    return result;
}

Promise<RetVal<ScoreInfo> > MuseScoreComService::doDownloadScoreInfo(int scoreId)
{
    return Promise<RetVal<ScoreInfo> >([this, scoreId](auto resolve, auto) {
        QVariantMap params;
        params[SCORE_ID_KEY] = scoreId;

        RetVal<QUrl> scoreInfoUrl = prepareUrlForRequest(MUSESCORECOM_SCORE_INFO_API_URL, params);
        if (!scoreInfoUrl.ret) {
            return resolve(scoreInfoUrl.ret);
        }

        auto receivedData = std::make_shared<QBuffer>();
        RetVal<Progress> progress = m_networkManager->get(scoreInfoUrl.val, receivedData, headers());
        if (!progress.ret) {
            return resolve(progress.ret);
        }

        progress.val.finished().onReceive(this, [this, receivedData, resolve](const ProgressResult& res) {
            if (res.ret) {
                (void)resolve(parseScoreInfo(receivedData->data()));
            } else {
                (void)resolve(uploadingDownloadingRetFromRawRet(res.ret));
            }
        });

        return Promise<RetVal<ScoreInfo> >::dummy_result();
    });
}

Promise<ScoresList> MuseScoreComService::downloadScoresList(int scoresPerBatch, int batchNumber)
{
    return Promise<ScoresList>([this, scoresPerBatch, batchNumber](auto resolve, auto reject) {
        QVariantMap params;
        params["per-page"] = scoresPerBatch;
        params["page"] = batchNumber;

        RetVal<QUrl> scoresListUrl = prepareUrlForRequest(MUSESCORECOM_SCORES_LIST_API_URL, params);
        if (!scoresListUrl.ret) {
            return reject(scoresListUrl.ret.code(), scoresListUrl.ret.toString());
        }

        auto receivedData = std::make_shared<QBuffer>();
        RetVal<Progress> progress = m_networkManager->get(scoresListUrl.val, receivedData, headers());
        if (!progress.ret) {
            return reject(progress.ret.code(), progress.ret.toString());
        }

        progress.val.finished().onReceive(this, [batchNumber, receivedData, resolve, reject](const ProgressResult& res) {
            if (!res.ret) {
                (void)reject(res.ret.code(), res.ret.toString());
                return;
            }

            RetVal<ScoresList> list = parseScoreList(receivedData->data(), batchNumber);
            if (list.ret) {
                (void)resolve(list.val);
            } else {
                (void)reject(list.ret.code(), list.ret.toString());
            }
        });

        return Promise<ScoresList>::dummy_result();
    });
}

ProgressPtr MuseScoreComService::downloadScore(int scoreId, DevicePtr scoreData, const QString& hash, const QString& secret)
{
    ProgressPtr progress = std::make_shared<Progress>();
    progress->start();

    executeAsyncRequest([this, scoreId, scoreData, hash, secret, progress]() {
        return doDownloadScore(scoreId, scoreData, hash, secret, progress);
    }).onResolve(this, [progress](const Ret& ret) {
        progress->finish(ret);
    });

    return progress;
}

Promise<Ret> MuseScoreComService::doDownloadScore(int scoreId, DevicePtr scoreData,
                                                  const QString& hash, const QString& secret, ProgressPtr progress)
{
    TRACEFUNC;

    QUrl baseDownloadUrl = MUSESCORECOM_SCORE_DOWNLOAD_API_URL;

    QVariantMap params;
    params["score_id"] = scoreId;

    if (!hash.isEmpty()) {
        baseDownloadUrl = MUSESCORECOM_SCORE_DOWNLOAD_SHARED_API_URL;

        params["h"] = hash;

        if (!secret.isEmpty()) {
            params["secret"] = secret;
        }
    }

    return make_promise<Ret>([this, baseDownloadUrl, params, scoreData, progress](auto resolve, auto) {
        RetVal<QUrl> downloadUrl = prepareUrlForRequest(baseDownloadUrl, params);
        if (!downloadUrl.ret) {
            return resolve(downloadUrl.ret);
        }

        RetVal<Progress> getProgress = m_networkManager->get(downloadUrl.val, scoreData, headers());
        if (!getProgress.ret) {
            return resolve(getProgress.ret);
        }

        getProgress.val.progressChanged().onReceive(this, [progress](int64_t current, int64_t total, const std::string& msg) {
            progress->progress(current, total, msg);
        });

        getProgress.val.finished().onReceive(this, [this, resolve](const ProgressResult& res) {
            (void)resolve(uploadingDownloadingRetFromRawRet(res.ret));
        });

        return Promise<Ret>::dummy_result();
    });
}

ProgressPtr MuseScoreComService::uploadScore(DevicePtr scoreData, const QString& title, Visibility visibility, const QUrl& sourceUrl,
                                             int revisionId)
{
    ProgressPtr progress = std::make_shared<Progress>();
    progress->start();

    executeAsyncRequest([this, scoreData, title, visibility, sourceUrl, revisionId, progress]() {
        return doUploadScore(scoreData, title, visibility, sourceUrl, revisionId, progress);
    }).onResolve(this, [progress](const Ret& ret) {
        if (progress->isStarted()) {
            progress->finish(ret);
        }
    });

    return progress;
}

async::Promise<RetVal<bool> > MuseScoreComService::checkScoreAlreadyUploaded(const ID& scoreId)
{
    if (scoreId == INVALID_ID) {
        return Promise<RetVal<bool> >([](auto resolve, auto) {
            return resolve(RetVal<bool>::make_ok(false));
        });
    }

    return doDownloadScoreInfo(scoreId.toUint64()).then<RetVal<bool> >(this, [this](const RetVal<ScoreInfo>& info, auto resolve) {
        if (!info.ret) {
            if (statusCode(info.ret) == NOT_FOUND_STATUS_CODE) {
                return resolve(RetVal<bool>::make_ok(false));
            }
            return resolve(info.ret);
        }

        const bool accountOwnsScore = info.val.owner.id == accountInfo().id.toInt();
        return resolve(RetVal<bool>::make_ok(accountOwnsScore));
    });
}

Promise<Ret> MuseScoreComService::doUploadScore(DevicePtr scoreData, const QString& title,
                                                Visibility visibility, const QUrl& sourceUrl, int revisionId,
                                                ProgressPtr progress)
{
    TRACEFUNC;

    const ID scoreId = idFromCloudUrl(sourceUrl);

    return checkScoreAlreadyUploaded(scoreId).then<Ret>(this, [=](const RetVal<bool>& alreadyUploaded, auto resolve) {
        if (!alreadyUploaded.ret) {
            return resolve(alreadyUploaded.ret);
        }

        RetVal<QUrl> uploadUrl = prepareUrlForRequest(MUSESCORECOM_UPLOAD_SCORE_API_URL);
        if (!uploadUrl.ret) {
            return resolve(uploadUrl.ret);
        }

        scoreData->seek(0);

        auto multiPart = makeMultiPartForScoreUpload(scoreData.get(), scoreId.toUint64(), title, visibility, revisionId,
                                                     configuration()->uploadingLicense(), alreadyUploaded.val);
        auto receivedData = std::make_shared<QBuffer>();

        RetVal<Progress> uploadProgress;

        if (alreadyUploaded.val) { // score exists, update
            uploadProgress = m_networkManager->put(uploadUrl.val, multiPart, receivedData, headers());
        } else { // score doesn't exist, post a new score
            uploadProgress = m_networkManager->post(uploadUrl.val, multiPart, receivedData, headers());
        }

        if (!uploadProgress.ret) {
            return resolve(uploadProgress.ret);
        }

        uploadProgress.val.progressChanged().onReceive(this, [progress](int64_t current, int64_t total, const std::string& msg) {
            progress->progress(current, total, msg);
        });

        uploadProgress.val.finished().onReceive(this, [this, alreadyUploaded, receivedData, resolve, progress](const ProgressResult& res) {
            if (!res.ret) {
                (void)resolve(uploadingDownloadingRetFromRawRet(res.ret, alreadyUploaded.val));
                return;
            }

            ProgressResult result = parseScoreUploadResponse(receivedData->data());
            progress->finish(result);
            (void)resolve(make_ok());
        });

        return Promise<Ret>::dummy_result();
    });
}

ProgressPtr MuseScoreComService::uploadAudio(DevicePtr audioData, const QString& audioFormat, const QUrl& sourceUrl)
{
    ProgressPtr progress = std::make_shared<Progress>();
    progress->start();

    executeAsyncRequest([this, audioData, audioFormat, sourceUrl, progress]() {
        return doUploadAudio(audioData, audioFormat, sourceUrl, progress);
    }).onResolve(this, [progress](const Ret& ret) {
        progress->finish(ret);
    });

    return progress;
}

Promise<Ret> MuseScoreComService::doUploadAudio(DevicePtr audioData, const QString& audioFormat, const QUrl& sourceUrl,
                                                ProgressPtr progress)
{
    TRACEFUNC;

    return make_promise<Ret>([this, audioData, audioFormat, sourceUrl, progress](auto resolve, auto) {
        RetVal<QUrl> uploadUrl = prepareUrlForRequest(MUSESCORECOM_UPLOAD_AUDIO_API_URL);
        if (!uploadUrl.ret) {
            return resolve(uploadUrl.ret);
        }

        audioData->seek(0);

        auto multiPart = makeMultiPartForAudioUpload(audioData.get(), audioFormat, sourceUrl);
        RetVal<Progress> postProgress = m_networkManager->post(uploadUrl.val, multiPart, nullptr, headers());
        if (!postProgress.ret) {
            return resolve(postProgress.ret);
        }

        postProgress.val.progressChanged().onReceive(this, [progress](int64_t current, int64_t total, const std::string& msg) {
            progress->progress(current, total, msg);
        });

        postProgress.val.finished().onReceive(this, [resolve](const ProgressResult& res) {
            (void)resolve(res.ret);
        });

        return Promise<Ret>::dummy_result();
    });
}
