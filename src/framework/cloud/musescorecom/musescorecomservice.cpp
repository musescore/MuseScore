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

#include "musescorecomservice.h"

#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QBuffer>
#include <QHttpMultiPart>

#include "async/async.h"

#include "clouderrors.h"

#include "log.h"

using namespace muse;
using namespace muse::cloud;
using namespace muse::network;

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
    RequestHeaders headers;
    headers.rawHeaders["Accept"] = "application/json";
    headers.rawHeaders["X-MS-CLIENT-ID"] = QByteArray::fromStdString(configuration()->clientId());
    headers.knownHeaders[QNetworkRequest::UserAgentHeader] = userAgent();

    return headers;
}

Ret MuseScoreComService::downloadAccountInfo()
{
    TRACEFUNC;

    RetVal<QUrl> userInfoUrl = prepareUrlForRequest(MUSESCORECOM_USER_INFO_API_URL);
    if (!userInfoUrl.ret) {
        return userInfoUrl.ret;
    }

    QBuffer receivedData;
    INetworkManagerPtr manager = networkManagerCreator()->makeNetworkManager();
    Ret ret = manager->get(userInfoUrl.val, &receivedData, headers());

    if (!ret) {
        printServerReply(receivedData);
        return ret;
    }

    QJsonDocument document = QJsonDocument::fromJson(receivedData.data());
    QJsonObject user = document.object();

    AccountInfo info;
    info.id = QString::number(user.value("id").toInt());
    info.userName = user.value("name").toString();
    QString profileUrl = user.value("profile_url").toString();
    info.profileUrl = QUrl(profileUrl);
    info.avatarUrl = QUrl(user.value("avatar_url").toString());
    info.collectionUrl = QUrl(profileUrl + "/sheetmusic");

    if (info.isValid()) {
        setAccountInfo(info);
    } else {
        setAccountInfo(AccountInfo());
    }

    return muse::make_ok();
}

bool MuseScoreComService::doUpdateTokens()
{
    TRACEFUNC;

    QHttpPart refreshTokenPart;
    refreshTokenPart.setHeader(QNetworkRequest::ContentDispositionHeader, QString("form-data; name=\"refresh_token\""));
    refreshTokenPart.setBody(refreshToken().toUtf8());

    QHttpMultiPart multiPart(QHttpMultiPart::FormDataType);
    multiPart.append(refreshTokenPart);

    QBuffer receivedData;
    OutgoingDevice device(&multiPart);

    INetworkManagerPtr manager = networkManagerCreator()->makeNetworkManager();
    Ret ret = manager->post(serverConfig().refreshApiUrl, &device, &receivedData, headers());

    if (!ret) {
        printServerReply(receivedData);
        LOGE() << ret.toString();
        return false;
    }

    QJsonDocument document = QJsonDocument::fromJson(receivedData.data());
    QJsonObject tokens = document.object();

    setAccessToken(tokens.value(ACCESS_TOKEN_KEY).toString());
    setRefreshToken(tokens.value(REFRESH_TOKEN_KEY).toString());

    return true;
}

RetVal<ScoreInfo> MuseScoreComService::downloadScoreInfo(const QUrl& sourceUrl)
{
    return downloadScoreInfo(idFromCloudUrl(sourceUrl).toUint64());
}

RetVal<ScoreInfo> MuseScoreComService::downloadScoreInfo(int scoreId)
{
    TRACEFUNC;

    RetVal<ScoreInfo> result = RetVal<ScoreInfo>::make_ok(ScoreInfo());

    QVariantMap params;
    params[SCORE_ID_KEY] = scoreId;

    RetVal<QUrl> scoreInfoUrl = prepareUrlForRequest(MUSESCORECOM_SCORE_INFO_API_URL, params);
    if (!scoreInfoUrl.ret) {
        result.ret = scoreInfoUrl.ret;
        return result;
    }

    QBuffer receivedData;
    INetworkManagerPtr manager = networkManagerCreator()->makeNetworkManager();
    Ret ret = manager->get(scoreInfoUrl.val, &receivedData, headers());

    if (!ret) {
        printServerReply(receivedData);
        result.ret = uploadingDownloadingRetFromRawRet(ret);
        return result;
    }

    QJsonDocument document = QJsonDocument::fromJson(receivedData.data());
    QJsonObject scoreInfo = document.object();

    result.val.id = scoreInfo.value("id").toInt();
    result.val.revisionId = scoreInfo.value("revision_id").toInt();
    result.val.title = scoreInfo.value("title").toString();
    result.val.description = scoreInfo.value("description").toString();
    result.val.license = scoreInfo.value("license").toString();
    result.val.tags = scoreInfo.value("tags").toString().split(',');
    result.val.visibility = static_cast<Visibility>(scoreInfo.value("privacy").toInt());
    result.val.url = scoreInfo.value("custom_url").toString();

    QJsonObject owner = scoreInfo.value("user").toObject();

    result.val.owner.id = owner.value("uid").toInt();
    result.val.owner.userName = owner.value("username").toString();
    result.val.owner.profileUrl = owner.value("custom_url").toString();

    return result;
}

async::Promise<ScoresList> MuseScoreComService::downloadScoresList(int scoresPerBatch, int batchNumber)
{
    return async::Promise<ScoresList>([this, scoresPerBatch, batchNumber](auto resolve, auto reject) {
        QVariantMap params;
        params["per-page"] = scoresPerBatch;
        params["page"] = batchNumber;

        RetVal<QUrl> scoresListUrl = prepareUrlForRequest(MUSESCORECOM_SCORES_LIST_API_URL, params);
        if (!scoresListUrl.ret) {
            return reject(scoresListUrl.ret.code(), scoresListUrl.ret.toString());
        }

        QBuffer receivedData;
        INetworkManagerPtr manager = networkManagerCreator()->makeNetworkManager();
        Ret ret = manager->get(scoresListUrl.val, &receivedData, headers());

        if (!ret) {
            printServerReply(receivedData);
            return reject(ret.code(), ret.toString());
        }

        ScoresList result;

        QJsonDocument document = QJsonDocument::fromJson(receivedData.data());
        QJsonObject obj = document.object();

        QJsonObject metaObj = obj.value("_meta").toObject();
        result.meta.totalScoresCount = metaObj.value("totalCount").toInt();
        result.meta.batchesCount = metaObj.value("pageCount").toInt();
        result.meta.thisBatchNumber = metaObj.value("currentPage").toInt();
        result.meta.scoresPerBatch = metaObj.value("perPage").toInt();

        if (result.meta.thisBatchNumber < batchNumber) {
            // This happens when the requested page number was too high.
            // In this situation, the API just returns the last page and the items from that page.
            // We will return just an empty list, in order not to confuse the caller.
            return resolve(result);
        }

        QJsonArray items = obj.value("items").toArray();

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

        return resolve(result);
    });
}

ProgressPtr MuseScoreComService::downloadScore(int scoreId, QIODevice& scoreData, const QString& hash, const QString& secret)
{
    ProgressPtr progress = std::make_shared<Progress>();

    INetworkManagerPtr manager = networkManagerCreator()->makeNetworkManager();
    manager->progress().progressChanged().onReceive(this, [progress](int64_t current, int64_t total, const std::string& message) {
        progress->progress(current, total, message);
    });

    async::Async::call(this, [this, manager, scoreId, &scoreData, hash, secret, progress]() {
        progress->start();

        ProgressResult result;
        result.ret = executeRequest([this, manager, scoreId, &scoreData, hash, secret]() {
            return doDownloadScore(manager, scoreId, scoreData, hash, secret);
        });

        progress->finish(result);
    });

    return progress;
}

Ret MuseScoreComService::doDownloadScore(network::INetworkManagerPtr downloadManager, int scoreId, QIODevice& scoreData,
                                         const QString& hash, const QString& secret)
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

    RetVal<QUrl> downloadUrl = prepareUrlForRequest(baseDownloadUrl, params);
    if (!downloadUrl.ret) {
        return downloadUrl.ret;
    }

    Ret ret = downloadManager->get(downloadUrl.val, &scoreData, headers());

    return uploadingDownloadingRetFromRawRet(ret);
}

ProgressPtr MuseScoreComService::uploadScore(QIODevice& scoreData, const QString& title, Visibility visibility, const QUrl& sourceUrl,
                                             int revisionId)
{
    ProgressPtr progress = std::make_shared<Progress>();

    INetworkManagerPtr manager = networkManagerCreator()->makeNetworkManager();
    manager->progress().progressChanged().onReceive(this, [progress](int64_t current, int64_t total, const std::string& message) {
        progress->progress(current, total, message);
    });

    std::shared_ptr<ValMap> scoreUrlMap = std::make_shared<ValMap>();

    auto uploadCallback = [this, manager, &scoreData, title, visibility, sourceUrl, revisionId, scoreUrlMap]() {
        RetVal<ValMap> urlMap = doUploadScore(manager, scoreData, title, visibility, sourceUrl, revisionId);
        *scoreUrlMap = urlMap.val;

        return urlMap.ret;
    };

    async::Async::call(this, [this, progress, uploadCallback, scoreUrlMap]() {
        progress->start();

        ProgressResult result;
        result.ret = executeRequest(uploadCallback);
        result.val = Val(*scoreUrlMap);

        progress->finish(result);
    });

    return progress;
}

ProgressPtr MuseScoreComService::uploadAudio(QIODevice& audioData, const QString& audioFormat, const QUrl& sourceUrl)
{
    ProgressPtr progress = std::make_shared<Progress>();

    INetworkManagerPtr manager = networkManagerCreator()->makeNetworkManager();
    manager->progress().progressChanged().onReceive(this, [progress](int64_t current, int64_t total, const std::string& message) {
        progress->progress(current, total, message);
    });

    auto uploadCallback = [this, manager, &audioData, audioFormat, sourceUrl]() {
        return doUploadAudio(manager, audioData, audioFormat, sourceUrl);
    };

    async::Async::call(this, [this, progress, uploadCallback]() {
        progress->start();
        Ret ret = executeRequest(uploadCallback);
        progress->finish(ret);
    });

    return progress;
}

RetVal<ValMap> MuseScoreComService::doUploadScore(INetworkManagerPtr uploadManager, QIODevice& scoreData, const QString& title,
                                                  Visibility visibility, const QUrl& sourceUrl, int revisionId)
{
    TRACEFUNC;

    RetVal<ValMap> result = RetVal<ValMap>::make_ok(ValMap());

    RetVal<QUrl> uploadUrl = prepareUrlForRequest(MUSESCORECOM_UPLOAD_SCORE_API_URL);
    if (!uploadUrl.ret) {
        result.ret = uploadUrl.ret;
        return result;
    }

    ID scoreId = idFromCloudUrl(sourceUrl);
    bool isScoreAlreadyUploaded = scoreId != INVALID_ID;

    if (isScoreAlreadyUploaded) {
        RetVal<ScoreInfo> scoreInfo = downloadScoreInfo(scoreId.toUint64());

        if (!scoreInfo.ret) {
            if (statusCode(scoreInfo.ret) == NOT_FOUND_STATUS_CODE) {
                isScoreAlreadyUploaded = false;
            } else {
                result.ret = scoreInfo.ret;
                return result;
            }
        }

        if (scoreInfo.val.owner.id != accountInfo().val.id.toInt()) {
            isScoreAlreadyUploaded = false;
        }
    }

    scoreData.seek(0);

    QHttpMultiPart multiPart(QHttpMultiPart::FormDataType);

    QHttpPart filePart;
    filePart.setHeader(QNetworkRequest::ContentTypeHeader, QVariant("application/octet-stream"));
    QString contentDisposition = QString("form-data; name=\"score_data\"; filename=\"temp_%1.mscz\"").arg(generateFileNameNumber());
    filePart.setHeader(QNetworkRequest::ContentDispositionHeader, QVariant(contentDisposition));

    filePart.setBodyDevice(&scoreData);
    multiPart.append(filePart);

    if (isScoreAlreadyUploaded) {
        QHttpPart scoreIdPart;
        scoreIdPart.setHeader(QNetworkRequest::ContentDispositionHeader, QVariant("form-data; name=\"score_id\""));
        scoreIdPart.setBody(QString::number(scoreId.toUint64()).toLatin1());
        multiPart.append(scoreIdPart);

        if (revisionId) {
            QHttpPart revisionIdPart;
            scoreIdPart.setHeader(QNetworkRequest::ContentDispositionHeader, QVariant("form-data; name=\"last_revision_id\""));
            scoreIdPart.setBody(QByteArray::number(revisionId));
            multiPart.append(scoreIdPart);
        }
    }

    QHttpPart titlePart;
    titlePart.setHeader(QNetworkRequest::ContentDispositionHeader, QVariant("form-data; name=\"title\""));
    titlePart.setBody(title.toUtf8());
    multiPart.append(titlePart);

    QHttpPart privacyPart;
    privacyPart.setHeader(QNetworkRequest::ContentDispositionHeader, QVariant("form-data; name=\"privacy\""));
    privacyPart.setBody(QByteArray::number(int(visibility)));
    multiPart.append(privacyPart);

    QHttpPart licensePart;
    licensePart.setHeader(QNetworkRequest::ContentDispositionHeader, QVariant("form-data; name=\"license\""));
    licensePart.setBody(configuration()->uploadingLicense());
    multiPart.append(licensePart);

    Ret ret(true);
    QBuffer receivedData;
    OutgoingDevice device(&multiPart);

    if (isScoreAlreadyUploaded) { // score exists, update
        ret = uploadManager->put(uploadUrl.val, &device, &receivedData, headers());
    } else { // score doesn't exist, post a new score
        ret = uploadManager->post(uploadUrl.val, &device, &receivedData, headers());
    }

    if (!ret) {
        printServerReply(receivedData);

        result.ret = uploadingDownloadingRetFromRawRet(ret, isScoreAlreadyUploaded);

        return result;
    }

    QJsonObject scoreInfo = QJsonDocument::fromJson(receivedData.data()).object();
    QUrl newSourceUrl = QUrl(scoreInfo.value("permalink").toString());
    QUrl editUrl = QUrl(scoreInfo.value("edit_url").toString());
    int newRevisionId = scoreInfo.value("revision_id").toInt();

    if (!newSourceUrl.isValid()) {
        result.ret = make_ret(cloud::Err::CouldNotReceiveSourceUrl);
        return result;
    }

    result.val["sourceUrl"] = Val(newSourceUrl.toString());
    result.val["editUrl"] = Val(editUrl.toString());
    result.val["revisionId"] = Val(newRevisionId);

    return result;
}

Ret MuseScoreComService::doUploadAudio(network::INetworkManagerPtr uploadManager, QIODevice& audioData, const QString& audioFormat,
                                       const QUrl& sourceUrl)
{
    TRACEFUNC;

    RetVal<QUrl> uploadUrl = prepareUrlForRequest(MUSESCORECOM_UPLOAD_AUDIO_API_URL);
    if (!uploadUrl.ret) {
        return uploadUrl.ret;
    }

    audioData.seek(0);

    QHttpMultiPart multiPart(QHttpMultiPart::FormDataType);

    QHttpPart audioPart;
    audioPart.setHeader(QNetworkRequest::ContentTypeHeader, QVariant("application/octet-stream"));
    QString contentDisposition = QString("form-data; name=\"audio_data\"; filename=\"temp_%1.%2\"")
                                 .arg(generateFileNameNumber())
                                 .arg(audioFormat);
    audioPart.setHeader(QNetworkRequest::ContentDispositionHeader, contentDisposition);
    audioPart.setBodyDevice(&audioData);
    multiPart.append(audioPart);

    QHttpPart scoreIdPart;
    scoreIdPart.setHeader(QNetworkRequest::ContentDispositionHeader, QVariant("form-data; name=\"score_id\""));
    scoreIdPart.setBody(QString::number(idFromCloudUrl(sourceUrl).toUint64()).toLatin1());
    multiPart.append(scoreIdPart);

    QBuffer receivedData;
    OutgoingDevice device(&multiPart);

    Ret ret = uploadManager->post(uploadUrl.val, &device, &receivedData, headers());

    if (!ret) {
        printServerReply(receivedData);
    }

    return ret;
}
