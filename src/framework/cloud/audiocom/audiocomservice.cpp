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

#include "audiocomservice.h"

#include <QJsonDocument>
#include <QJsonObject>
#include <QBuffer>
#include <QHttpMultiPart>

#include "async/async.h"
#include "containers.h"

#include "clouderrors.h"

#include "log.h"

using namespace muse;
using namespace muse::cloud;
using namespace muse::network;

static const QString AUDIOCOM_CLOUD_TITLE("Audio.com");
static const QString AUDIOCOM_CLOUD_URL("https://audio.com");
static const QString AUDIOCOM_API_ROOT_URL("https://api.audio.com");
static const QUrl AUDIOCOM_USER_INFO_API_URL(AUDIOCOM_API_ROOT_URL + "/me");

static const QUrl AUDIOCOM_UPLOAD_AUDIO_API_URL(AUDIOCOM_API_ROOT_URL + "/audio");

static const QString AUDIOCOM_LOGO_URL(AUDIOCOM_CLOUD_URL + "/img/mu-app-logo.svg");

static QString audioMime(const QString& audioFormat)
{
    if (audioFormat == "mp3") {
        return "audio/mpeg";
    }

    return "audio/x-wav";
}

AudioComService::AudioComService(const modularity::ContextPtr& iocCtx, QObject* parent)
    : AbstractCloudService(iocCtx, parent)
{
}

IAuthorizationServicePtr AudioComService::authorization()
{
    return shared_from_this();
}

CloudInfo AudioComService::cloudInfo() const
{
    return {
        AUDIO_COM_CLOUD_CODE,
        AUDIOCOM_CLOUD_TITLE,
        AUDIOCOM_CLOUD_URL,
        AUDIOCOM_LOGO_URL,
        logoColor()
    };
}

QUrl AudioComService::projectManagerUrl() const
{
    return accountInfo().val.profileUrl.toString() + "/projects";
}

AbstractCloudService::ServerConfig AudioComService::serverConfig() const
{
    ServerConfig serverConfig;
    serverConfig.serverCode = AUDIO_COM_CLOUD_CODE;
    serverConfig.serverUrl = AUDIOCOM_CLOUD_URL;

    serverConfig.serverAvailabilityUrl = AUDIOCOM_API_ROOT_URL + "/system/healthcheck";

    serverConfig.authorizationUrl = AUDIOCOM_CLOUD_URL + "/auth/sign-in";
    serverConfig.signUpUrl = AUDIOCOM_CLOUD_URL + "/auth/sign-up";
    serverConfig.signInSuccessUrl = AUDIOCOM_CLOUD_URL + "/my-audio?muAuthSuccess=true";

    serverConfig.accessTokenUrl = AUDIOCOM_API_ROOT_URL + "/auth/token";
    serverConfig.refreshApiUrl = AUDIOCOM_API_ROOT_URL + "/auth/token";

    serverConfig.headers = headers();

    static const QString CLIENT_ID = "1760627154191472";
    static const QString CLIENT_SECRET = "0qLugLJwBXsm8e-fOd3WgFx6TOTQkw74";

    serverConfig.authorizationParameters = {
        { "client_id", CLIENT_ID },
        { "client_secret", CLIENT_SECRET },
        { "grant_type", "authorization_code" },
        { "scope", "all" }
    };

    serverConfig.refreshParameters = {
        { "client_id", CLIENT_ID },
        { "client_secret", CLIENT_SECRET },
        { "grant_type", "refresh_token" }
    };

    return serverConfig;
}

RequestHeaders AudioComService::headers(const QString& token) const
{
    RequestHeaders headers;
    headers.rawHeaders["Accept"] = "application/json";
    headers.rawHeaders["Content-Type"] = "application/json";
    headers.rawHeaders["Authorization"] = QString("Bearer " + (!token.isEmpty() ? token : accessToken())).toUtf8();
    headers.knownHeaders[QNetworkRequest::UserAgentHeader] = userAgent();

    return headers;
}

Ret AudioComService::downloadAccountInfo()
{
    TRACEFUNC;

    QBuffer receivedData;
    INetworkManagerPtr manager = networkManagerCreator()->makeNetworkManager();
    Ret ret = manager->get(AUDIOCOM_USER_INFO_API_URL, &receivedData, headers());

    if (!ret) {
        printServerReply(receivedData);
        return ret;
    }

    QJsonDocument document = QJsonDocument::fromJson(receivedData.data());
    QJsonObject user = document.object();

    AccountInfo info;
    info.id = user.value("id").toString();
    info.userName = user.value("profile").toObject().value("name").toString();

    QString profileUrl = AUDIOCOM_CLOUD_URL + "/" + user.value("username").toString();
    info.profileUrl = QUrl(profileUrl);
    info.collectionUrl = info.profileUrl;

    info.avatarUrl = QUrl(user.value("avatar").toString());

    if (info.isValid()) {
        setAccountInfo(info);
    } else {
        setAccountInfo(AccountInfo());
    }

    return muse::make_ok();
}

bool AudioComService::doUpdateTokens()
{
    TRACEFUNC;

    ServerConfig serverConfig = this->serverConfig();

    QJsonObject json;
    json["refresh_token"] = refreshToken();

    for (const QString& key : serverConfig.authorizationParameters.keys()) {
        json.insert(key, serverConfig.refreshParameters.value(key).toString());
    }

    QByteArray jsonData = QString::fromStdString(QJsonDocument(json).toJson(QJsonDocument::JsonFormat::Compact).toStdString()).toUtf8();
    QBuffer receivedData(&jsonData);
    OutgoingDevice device(&receivedData);

    INetworkManagerPtr manager = networkManagerCreator()->makeNetworkManager();
    Ret ret = manager->post(serverConfig.refreshApiUrl, &device, &receivedData, headers());

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

ProgressPtr AudioComService::uploadAudio(QIODevice& audioData, const QString& audioFormat, const QString& title, const QUrl& existingUrl,
                                         Visibility visibility, bool replaceExisting)
{
    ProgressPtr progress = std::make_shared<Progress>();

    INetworkManagerPtr manager = networkManagerCreator()->makeNetworkManager();
    manager->progress().progressChanged().onReceive(this, [progress](int64_t current, int64_t total, const std::string& message) {
        progress->progress(current, total, message);
    });

    auto createAudioCallback = [this, manager, &audioData, title, audioFormat, existingUrl, visibility, replaceExisting]() {
        qint64 size = audioData.size();
        return doCreateAudio(manager, title, size, audioFormat, existingUrl, visibility, replaceExisting);
    };

    auto uploadCallback = [this, manager, &audioData, audioFormat]() {
        return doUploadAudio(manager, audioData, audioFormat);
    };

    async::Async::call(this, [this, progress, createAudioCallback, uploadCallback]() {
        progress->start();

        ProgressResult result;

        Ret ret = executeRequest(createAudioCallback);
        if (ret) {
            ret = executeRequest(uploadCallback);
            if (ret) {
                ValMap audioMap;
                audioMap["editUrl"] = Val(QString("%2/audio/%3/edit").arg(
                                              accountInfo().val.collectionUrl.toString(), m_currentUploadingAudioSlug));
                audioMap["url"] = Val(AUDIOCOM_CLOUD_URL + "/audio/" + m_currentUploadingAudioId);
                result.val = Val(audioMap);
            }
        }

        result.ret = ret;

        m_currentUploadingAudioSlug.clear();
        m_currentUploadingAudioId.clear();
        m_currentUploadingAudioInfo = {};

        progress->finish(result);
    });

    return progress;
}

Ret AudioComService::doUploadAudio(network::INetworkManagerPtr uploadManager, QIODevice& audioData, const QString& audioFormat)
{
    TRACEFUNC;

    IF_FAILED(!m_currentUploadingAudioInfo.isEmpty()) {
        return make_ret(Err::UnknownError);
    }

    QUrl url = QUrl(m_currentUploadingAudioInfo.value("url").toString());
    QUrl success = QUrl(m_currentUploadingAudioInfo.value("success").toString());
    QUrl fail = QUrl(m_currentUploadingAudioInfo.value("fail").toString());
    QUrl progress = QUrl(m_currentUploadingAudioInfo.value("progress").toString());

    if (!url.isValid() || !success.isValid() || !fail.isValid() || !progress.isValid()) {
        return make_ret(Err::UnknownError);
    }

    audioData.seek(0);

    QString token;

    if (m_currentUploadingAudioInfo.contains("extra")) {
        QJsonObject extra = m_currentUploadingAudioInfo.value("extra").toObject();
        QJsonObject audio = extra.value("audio").toObject();
        m_currentUploadingAudioSlug = audio.value("slug").toString();
        m_currentUploadingAudioId = audio.value("id").toString();
        token = extra.value("token").toString();
    }

    RequestHeaders headers;
    headers.knownHeaders[QNetworkRequest::UserAgentHeader] = userAgent();
    headers.knownHeaders[QNetworkRequest::ContentTypeHeader] = audioMime(audioFormat);

    Ret ret(true);
    QBuffer receivedData;
    OutgoingDevice device(&audioData);

    ret = uploadManager->put(url, &device, &receivedData, headers);

    if (!ret) {
        printServerReply(receivedData);
        notifyServerAboutFailUpload(fail, token);
        ret = uploadingDownloadingRetFromRawRet(ret);
    } else {
        notifyServerAboutSuccessUpload(success, token);
    }

    return ret;
}

Ret AudioComService::doUpdateVisibility(network::INetworkManagerPtr manager, const QUrl& url, Visibility visibility)
{
    QUrl patchUrl(AUDIOCOM_API_ROOT_URL + "/audio/" + idFromCloudUrl(url).toQString());

    QJsonObject json;
    json["public"] = visibility == Visibility::Public;
    QByteArray jsonData = QString::fromStdString(QJsonDocument(json).toJson(QJsonDocument::JsonFormat::Compact).toStdString()).toUtf8();
    QBuffer receivedData(&jsonData);
    OutgoingDevice device(&receivedData);

    Ret ret = manager->patch(patchUrl, &device, &receivedData, headers());

    if (!ret) {
        printServerReply(receivedData);
    }

    return ret;
}

Ret AudioComService::doCreateAudio(network::INetworkManagerPtr manager, const QString& title, int size, const QString& audioFormat,
                                   const QUrl& existingUrl, Visibility visibility, bool replaceExisting)
{
    TRACEFUNC;

    QUrl postUrl;
    Ret ret;

    QJsonObject json;
    QString mime = audioMime(audioFormat);
    json["mime"] = mime;
    json["download_mime"] = mime;

    json["size"] = size;
    json["name"] = title;

    json["method"] = "PUT";

    if (replaceExisting) {
        ret = doUpdateVisibility(manager, existingUrl, visibility);

        if (!ret) {
            ret = uploadingDownloadingRetFromRawRet(ret);
            return ret;
        }
        postUrl = QUrl(AUDIOCOM_API_ROOT_URL + "/audio/" + idFromCloudUrl(existingUrl).toQString() + "/source");
    } else {
        json["public"] = visibility == Visibility::Public;
        postUrl = AUDIOCOM_UPLOAD_AUDIO_API_URL;
    }

    QByteArray jsonData = QString::fromStdString(QJsonDocument(json).toJson(QJsonDocument::JsonFormat::Compact).toStdString()).toUtf8();
    QBuffer receivedData(&jsonData);
    OutgoingDevice device(&receivedData);

    ret = manager->post(postUrl, &device, &receivedData, headers());

    if (!ret) {
        printServerReply(receivedData);
        ret = uploadingDownloadingRetFromRawRet(ret);
        return ret;
    }

    m_currentUploadingAudioInfo = QJsonDocument::fromJson(receivedData.data()).object();

    return ret;
}

void AudioComService::notifyServerAboutFailUpload(const QUrl& failUrl, const QString& token)
{
    INetworkManagerPtr manager = networkManagerCreator()->makeNetworkManager();

    QBuffer receivedData;

    Ret ret = manager->del(failUrl, &receivedData, headers(token));
    if (!ret) {
        printServerReply(receivedData);
    }
}

void AudioComService::notifyServerAboutSuccessUpload(const QUrl& successUrl, const QString& token)
{
    INetworkManagerPtr manager = networkManagerCreator()->makeNetworkManager();

    QBuffer receivedData;
    QBuffer outData;
    OutgoingDevice device(&outData);

    Ret ret = manager->post(successUrl, &device, &receivedData, headers(token));
    if (!ret) {
        printServerReply(receivedData);
    }
}
