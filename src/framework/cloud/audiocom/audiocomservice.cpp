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

#include "audiocomservice.h"

#include <QJsonDocument>
#include <QJsonObject>
#include <QBuffer>
#include <QHttpMultiPart>

#include "async/async.h"
#include "containers.h"

#include "clouderrors.h"

#include "defer.h"
#include "log.h"

using namespace muse;
using namespace muse::cloud;
using namespace muse::network;
using namespace muse::async;

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

static RetVal<AccountInfo> parseAudioComAccountInfo(const QByteArray& data)
{
    QJsonParseError err;
    QJsonDocument doc = QJsonDocument::fromJson(data, &err);
    if (err.error != QJsonParseError::NoError || !doc.isObject()) {
        return muse::make_ret(Ret::Code::InternalError, err.errorString().toStdString());
    }

    QJsonObject user = doc.object();
    QString profileUrl = AUDIOCOM_CLOUD_URL + "/" + user.value("username").toString();

    AccountInfo info;
    info.id = user.value("id").toString();
    info.userName = user.value("profile").toObject().value("name").toString();
    info.profileUrl = QUrl(profileUrl);
    info.collectionUrl = info.profileUrl;
    info.avatarUrl = QUrl(user.value("avatar").toString());

    return RetVal<AccountInfo>::make_ok(info);
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
    return accountInfo().profileUrl.toString() + "/projects";
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
    RequestHeaders headers = defaultHeaders();
    headers.rawHeaders["Accept"] = "application/json";
    headers.rawHeaders["Content-Type"] = "application/json";
    headers.rawHeaders["Authorization"] = QString("Bearer " + (!token.isEmpty() ? token : accessToken())).toUtf8();

    return headers;
}

Promise<Ret> AudioComService::downloadAccountInfo()
{
    TRACEFUNC;

    return make_promise<Ret>([this](auto resolve, auto) {
        auto receivedData = std::make_shared<QBuffer>();
        RetVal<Progress> progress = m_networkManager->get(AUDIOCOM_USER_INFO_API_URL, receivedData, headers());
        if (!progress.ret) {
            return resolve(progress.ret);
        }

        progress.val.finished().onReceive(this, [this, receivedData, resolve](const ProgressResult& res) {
            if (!res.ret) {
                printServerReply(*receivedData);
                (void)resolve(res.ret);
                return;
            }

            RetVal<AccountInfo> info = parseAudioComAccountInfo(receivedData->data());
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

Promise<Ret> AudioComService::updateTokens()
{
    TRACEFUNC;

    return make_promise<Ret>([this](auto resolve, auto) {
        ServerConfig serverConfig = this->serverConfig();

        QJsonObject json;
        json["refresh_token"] = refreshToken();

        for (const QString& key : serverConfig.authorizationParameters.keys()) {
            json.insert(key, serverConfig.refreshParameters.value(key).toString());
        }

        QByteArray jsonData = QString::fromStdString(QJsonDocument(json).toJson(QJsonDocument::JsonFormat::Compact).toStdString()).toUtf8();
        auto outgoingData = std::make_shared<QBuffer>();
        outgoingData->setData(jsonData);
        auto receivedData = std::make_shared<QBuffer>();

        RetVal<Progress> progress = m_networkManager->post(serverConfig.refreshApiUrl, outgoingData, receivedData, headers());
        if (!progress.ret) {
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

ProgressPtr AudioComService::uploadAudio(DevicePtr audioData, const QString& audioFormat, const QString& title, const QUrl& existingUrl,
                                         Visibility visibility, bool replaceExisting)
{
    ProgressPtr progress = std::make_shared<Progress>();
    progress->start();

    auto finishProgress = [this, progress](const Ret& ret) {
        ProgressResult result(ret);

        DEFER {
            m_currentUploadingAudioSlug.clear();
            m_currentUploadingAudioId.clear();
            m_currentUploadingAudioInfo = {};
            progress->finish(result);
        };

        if (!ret) {
            return;
        }

        ValMap audioMap;
        audioMap["editUrl"] = Val(QString("%2/audio/%3/edit").arg(
                                      accountInfo().collectionUrl.toString(), m_currentUploadingAudioSlug));
        audioMap["url"] = Val(AUDIOCOM_CLOUD_URL + "/audio/" + m_currentUploadingAudioId);
        result.val = Val(audioMap);
    };

    if (replaceExisting) {
        executeAsyncRequest([this, audioData, audioFormat, title, existingUrl, visibility, progress]() {
            return replaceExistingAudio(audioData, audioFormat, title, existingUrl, visibility, progress);
        }).onResolve(this, [finishProgress](const Ret& ret) {
            finishProgress(ret);
        });
    } else {
        executeAsyncRequest([this, audioData, audioFormat, title, existingUrl, visibility, progress]() {
            return uploadNewAudio(audioData, audioFormat, title, existingUrl, visibility, progress);
        }).onResolve(this, [finishProgress](const Ret& ret) {
            finishProgress(ret);
        });
    }

    return progress;
}

Promise<Ret> AudioComService::uploadNewAudio(DevicePtr audioData, const QString& audioFormat, const QString& title, const QUrl& url,
                                             Visibility visibility, ProgressPtr progress)
{
    std::weak_ptr<QIODevice> audioDataWeakPtr = audioData; // prevents memory leak

    // Create audio info -> upload audio
    return doCreateAudio(title, audioData->size(), audioFormat, url, visibility, false /*replaceExisting*/)
           .then<Ret>(this, [this, audioDataWeakPtr, audioFormat, progress](const Ret& ret, auto resolve) {
        DevicePtr audioData = audioDataWeakPtr.lock();
        if (!ret || !audioData) {
            return resolve(ret);
        }

        doUploadAudio(audioData, audioFormat, progress).onResolve(this, [resolve](const Ret& ret) {
            (void)resolve(ret);
        });

        return Promise<Ret>::dummy_result();
    });
}

Promise<Ret> AudioComService::replaceExistingAudio(DevicePtr audioData, const QString& audioFormat, const QString& title, const QUrl& url,
                                                   Visibility visibility, ProgressPtr progress)
{
    std::weak_ptr<QIODevice> audioDataWeakPtr = audioData; // prevents memory leak

    // Update visibility -> update audio info -> upload audio
    return doUpdateVisibility(url, visibility).then<Ret>(this,
                                                         [this, audioDataWeakPtr, audioFormat, title, url, visibility,
                                                          progress](const Ret& ret, auto resolve) {
        DevicePtr audioData = audioDataWeakPtr.lock();
        if (!ret || !audioData) {
            return resolve(ret);
        }

        doCreateAudio(title, audioData->size(), audioFormat, url, visibility, true /*replaceExisting*/)
        .onResolve(this, [this, audioData, audioFormat, progress, resolve](const Ret& ret) {
            if (!ret) {
                (void)resolve(ret);
                return;
            }

            doUploadAudio(audioData, audioFormat, progress).onResolve(this, [resolve](const Ret& ret) {
                (void)resolve(ret);
            });
        });

        return Promise<Ret>::dummy_result();
    });
}

Promise<Ret> AudioComService::doUploadAudio(DevicePtr audioData, const QString& audioFormat, ProgressPtr progress)
{
    TRACEFUNC;

    return make_promise<Ret>([this, audioData, audioFormat, progress](auto resolve, auto) {
        IF_FAILED(!m_currentUploadingAudioInfo.isEmpty()) {
            return resolve(make_ret(Err::UnknownError));
        }

        QUrl url = QUrl(m_currentUploadingAudioInfo.value("url").toString());
        QUrl success = QUrl(m_currentUploadingAudioInfo.value("success").toString());
        QUrl fail = QUrl(m_currentUploadingAudioInfo.value("fail").toString());
        QUrl progressUrl = QUrl(m_currentUploadingAudioInfo.value("progress").toString());

        if (!url.isValid() || !success.isValid() || !fail.isValid() || !progressUrl.isValid()) {
            return resolve(make_ret(Err::UnknownError));
        }

        audioData->seek(0);

        QString token;

        if (m_currentUploadingAudioInfo.contains("extra")) {
            QJsonObject extra = m_currentUploadingAudioInfo.value("extra").toObject();
            QJsonObject audio = extra.value("audio").toObject();
            m_currentUploadingAudioSlug = audio.value("slug").toString();
            m_currentUploadingAudioId = audio.value("id").toString();
            token = extra.value("token").toString();
        }

        RequestHeaders headers = defaultHeaders();
        headers.knownHeaders[QNetworkRequest::ContentTypeHeader] = audioMime(audioFormat);

        RetVal<Progress> putProgress = m_networkManager->put(url, audioData, nullptr, headers);
        if (!putProgress.ret) {
            return resolve(putProgress.ret);
        }

        putProgress.val.progressChanged().onReceive(this, [progress](int64_t current, int64_t total, const std::string& msg) {
            progress->progress(current, total, msg);
        });

        putProgress.val.finished().onReceive(this, [this, token, success, fail, resolve](const ProgressResult& res) {
            if (res.ret) {
                notifyServerAboutSuccessUpload(success, token);
                (void)resolve(make_ok());
            } else {
                notifyServerAboutFailUpload(fail, token);
                (void)resolve(uploadingDownloadingRetFromRawRet(res.ret));
            }
        });

        return Promise<Ret>::dummy_result();
    });
}

async::Promise<Ret> AudioComService::doUpdateVisibility(const QUrl& url, Visibility visibility)
{
    return make_promise<Ret>([this, url, visibility](auto resolve, auto) {
        QUrl patchUrl(AUDIOCOM_API_ROOT_URL + "/audio/" + idFromCloudUrl(url).toQString());

        QJsonObject json;
        json["public"] = visibility == Visibility::Public;
        QByteArray jsonData = QString::fromStdString(QJsonDocument(json).toJson(QJsonDocument::JsonFormat::Compact).toStdString()).toUtf8();

        auto outgoingData = std::make_shared<QBuffer>();
        outgoingData->setData(jsonData);

        RetVal<Progress> progress = m_networkManager->patch(patchUrl, outgoingData, nullptr, headers());
        if (!progress.ret) {
            return resolve(progress.ret);
        }

        progress.val.finished().onReceive(this, [resolve](const ProgressResult& res) {
            (void)resolve(res.ret);
        });

        return Promise<Ret>::dummy_result();
    });
}

Promise<Ret> AudioComService::doCreateAudio(const QString& title, int size,
                                            const QString& audioFormat,
                                            const QUrl& existingUrl, Visibility visibility, bool replaceExisting)
{
    TRACEFUNC;

    return make_promise<Ret>([this, title, size, audioFormat, existingUrl, visibility, replaceExisting](auto resolve, auto) {
        QJsonObject json;
        QString mime = audioMime(audioFormat);
        json["mime"] = mime;
        json["download_mime"] = mime;
        json["size"] = size;
        json["name"] = title;
        json["method"] = "PUT";

        QUrl postUrl;
        if (replaceExisting) {
            postUrl = QUrl(AUDIOCOM_API_ROOT_URL + "/audio/" + idFromCloudUrl(existingUrl).toQString() + "/source");
        } else {
            json["public"] = visibility == Visibility::Public;
            postUrl = AUDIOCOM_UPLOAD_AUDIO_API_URL;
        }

        QByteArray jsonData = QString::fromStdString(QJsonDocument(json).toJson(QJsonDocument::JsonFormat::Compact).toStdString()).toUtf8();
        auto outgoingData = std::make_shared<QBuffer>();
        outgoingData->setData(jsonData);
        auto receivedData = std::make_shared<QBuffer>();

        RetVal<Progress> progress = m_networkManager->post(postUrl, outgoingData, receivedData, headers());
        if (!progress.ret) {
            return resolve(progress.ret);
        }

        progress.val.finished().onReceive(this, [this, receivedData, resolve](const ProgressResult& res) {
            if (!res.ret) {
                (void)resolve(uploadingDownloadingRetFromRawRet(res.ret));
                return;
            }

            QJsonParseError err;
            QJsonDocument doc = QJsonDocument::fromJson(receivedData->data(), &err);
            if (err.error != QJsonParseError::NoError || !doc.isObject()) {
                (void)resolve(muse::make_ret(Ret::Code::InternalError, err.errorString().toStdString()));
                return;
            }

            m_currentUploadingAudioInfo = doc.object();
            (void)resolve(make_ok());
        });

        return Promise<Ret>::dummy_result();
    });
}

void AudioComService::notifyServerAboutFailUpload(const QUrl& failUrl, const QString& token)
{
    RetVal<Progress> progress = m_networkManager->del(failUrl, nullptr, headers(token));
    if (!progress.ret) {
        LOGE() << progress.ret.toString();
        return;
    }

    progress.val.finished().onReceive(this, [](const ProgressResult& res) {
        if (!res.ret) {
            LOGE() << res.ret.toString();
        }
    });
}

void AudioComService::notifyServerAboutSuccessUpload(const QUrl& successUrl, const QString& token)
{
    auto outData = std::make_shared<QBuffer>();
    RetVal<Progress> progress = m_networkManager->post(successUrl, outData, nullptr, headers(token));
    if (!progress.ret) {
        LOGE() << progress.ret.toString();
        return;
    }

    progress.val.finished().onReceive(this, [](const ProgressResult& res) {
        if (!res.ret) {
            LOGE() << res.ret.toString();
        }
    });
}
