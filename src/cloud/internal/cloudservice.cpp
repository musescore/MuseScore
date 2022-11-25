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

#include "cloudservice.h"
#include "config.h"

#include <QOAuth2AuthorizationCodeFlow>
#include <QJsonDocument>
#include <QJsonObject>
#include <QUrlQuery>
#include <QBuffer>
#include <QHttpMultiPart>
#include <QRandomGenerator>

#include "async/async.h"
#include "containers.h"
#include "types/translatablestring.h"

#include "clouderrors.h"
#include "multiinstances/resourcelockguard.h"
#include "network/networkerrors.h"

#include "oauthhttpserverreplyhandler.h"

#include "log.h"

using namespace mu;
using namespace mu::cloud;
using namespace mu::network;
using namespace mu::framework;

static const QString ACCESS_TOKEN_KEY("access_token");
static const QString REFRESH_TOKEN_KEY("refresh_token");
static const QString DEVICE_ID_KEY("device_id");
static const QString SCORE_ID_KEY("score_id");
static const QString EDITOR_SOURCE_KEY("editor_source");
static const QString EDITOR_SOURCE_VALUE(QString("Musescore Editor %1").arg(VERSION));
static const QString PLATFORM_KEY("platform");

static const std::string CLOUD_ACCESS_TOKEN_RESOURCE_NAME("CLOUD_ACCESS_TOKEN");

static constexpr int USER_UNAUTHORIZED_STATUS_CODE = 401;
static constexpr int FORBIDDEN_CODE = 403;
static constexpr int NOT_FOUND_STATUS_CODE = 404;

static constexpr int INVALID_SCORE_ID = 0;

static int statusCode(const mu::Ret& ret)
{
    std::any status = ret.data("status");
    return status.has_value() ? std::any_cast<int>(status) : 0;
}

static int scoreIdFromSourceUrl(const QUrl& sourceUrl)
{
    QStringList parts = sourceUrl.toString().split("/");
    if (parts.isEmpty()) {
        return INVALID_SCORE_ID;
    }

    return parts.last().toInt();
}

static int generateFileNameNumber()
{
    return QRandomGenerator::global()->generate() % 100000;
}

static void printServerReply(const QBuffer& reply)
{
    const QByteArray& data = reply.data();

    if (!data.isEmpty()) {
        LOGD() << "Server reply: " << data;
    }
}

CloudService::CloudService(QObject* parent)
    : QObject(parent)
{
    m_userAuthorized.val = false;
}

void CloudService::init()
{
    TRACEFUNC;

    m_oauth2 = new QOAuth2AuthorizationCodeFlow(this);
    m_replyHandler = new OAuthHttpServerReplyHandler(this);

    m_oauth2->setAuthorizationUrl(configuration()->authorizationUrl());
    m_oauth2->setAccessTokenUrl(configuration()->accessTokenUrl());
    m_oauth2->setModifyParametersFunction([](QAbstractOAuth::Stage, QVariantMap* parameters) {
        parameters->insert(EDITOR_SOURCE_KEY, EDITOR_SOURCE_VALUE);
        parameters->insert(PLATFORM_KEY, QString("%1 %2 %3")
                           .arg(QSysInfo::productType())
                           .arg(QSysInfo::productVersion())
                           .arg(QSysInfo::currentCpuArchitecture())
                           );
    });
    m_oauth2->setReplyHandler(m_replyHandler);

    connect(m_oauth2, &QOAuth2AuthorizationCodeFlow::authorizeWithBrowser, this, &CloudService::openUrl);
    connect(m_oauth2, &QOAuth2AuthorizationCodeFlow::granted, this, &CloudService::onUserAuthorized);

    connect(m_oauth2, &QOAuth2AuthorizationCodeFlow::error, [](const QString& error, const QString& errorDescription, const QUrl& uri) {
        LOGE() << "Error during authorization: " << error << "\n Description: " << errorDescription << "\n URI: " << uri.toString();
    });

    multiInstancesProvider()->resourceChanged().onReceive(this, [this](const std::string& resourceName) {
        if (resourceName == CLOUD_ACCESS_TOKEN_RESOURCE_NAME) {
            readTokens();
        }
    });

    if (readTokens()) {
        executeRequest([this]() { return downloadAccountInfo(); });
    }
}

bool CloudService::readTokens()
{
    TRACEFUNC;

    mi::ReadResourceLockGuard resource_guard(multiInstancesProvider(), CLOUD_ACCESS_TOKEN_RESOURCE_NAME);

    io::path_t tokensPath = configuration()->tokensFilePath();
    if (!fileSystem()->exists(tokensPath)) {
        LOGW() << "Could not find the tokens file: " << tokensPath;
        return false;
    }

    RetVal<ByteArray> tokensData = fileSystem()->readFile(tokensPath);
    if (!tokensData.ret) {
        LOGE() << tokensData.ret.toString();
        return false;
    }

    QJsonDocument tokensDoc = QJsonDocument::fromJson(tokensData.val.toQByteArrayNoCopy());
    QJsonObject saveObject = tokensDoc.object();

    m_accessToken = saveObject[ACCESS_TOKEN_KEY].toString();
    m_refreshToken = saveObject[REFRESH_TOKEN_KEY].toString();

    return true;
}

bool CloudService::saveTokens()
{
    TRACEFUNC;

    mi::WriteResourceLockGuard resource_guard(multiInstancesProvider(), CLOUD_ACCESS_TOKEN_RESOURCE_NAME);

    QJsonObject tokensObject;
    tokensObject[ACCESS_TOKEN_KEY] = m_accessToken;
    tokensObject[REFRESH_TOKEN_KEY] = m_refreshToken;
    QJsonDocument tokensDoc(tokensObject);

    QByteArray json = tokensDoc.toJson();
    Ret ret = fileSystem()->writeFile(configuration()->tokensFilePath(), ByteArray::fromQByteArrayNoCopy(json));
    if (!ret) {
        LOGE() << ret.toString();
    }

    return ret;
}

bool CloudService::updateTokens()
{
    TRACEFUNC;

    QHttpPart refreshTokenPart;
    refreshTokenPart.setHeader(QNetworkRequest::ContentDispositionHeader, QString("form-data; name=\"refresh_token\""));
    refreshTokenPart.setBody(m_refreshToken.toUtf8());

    QHttpMultiPart multiPart(QHttpMultiPart::FormDataType);
    multiPart.append(refreshTokenPart);

    QBuffer receivedData;
    OutgoingDevice device(&multiPart);

    INetworkManagerPtr manager = networkManagerCreator()->makeNetworkManager();
    Ret ret = manager->post(configuration()->refreshApiUrl(), &device, &receivedData, headers());

    if (!ret) {
        printServerReply(receivedData);
        LOGE() << ret.toString();
        clearTokens();
        return false;
    }

    QJsonDocument document = QJsonDocument::fromJson(receivedData.data());
    QJsonObject tokens = document.object();

    m_accessToken = tokens.value(ACCESS_TOKEN_KEY).toString();
    m_refreshToken = tokens.value(REFRESH_TOKEN_KEY).toString();

    return saveTokens();
}

void CloudService::clearTokens()
{
    m_accessToken.clear();
    m_refreshToken.clear();
    setAccountInfo(AccountInfo());
}

void CloudService::onUserAuthorized()
{
    TRACEFUNC;

    m_accessToken = m_oauth2->token();
    m_refreshToken = m_oauth2->refreshToken();

    saveTokens();

    Ret ret = downloadAccountInfo();
    if (!ret) {
        LOGE() << ret.toString();
        return;
    }

    if (m_onUserAuthorizedCallback) {
        m_onUserAuthorizedCallback();
        m_onUserAuthorizedCallback = OnUserAuthorizedCallback();
    }
}

void CloudService::authorize(const OnUserAuthorizedCallback& onUserAuthorizedCallback)
{
    if (m_userAuthorized.val) {
        return;
    }

    m_onUserAuthorizedCallback = onUserAuthorizedCallback;
    m_oauth2->setAuthorizationUrl(configuration()->authorizationUrl());
    m_oauth2->grant();
}

mu::RetVal<QUrl> CloudService::prepareUrlForRequest(QUrl apiUrl, const QVariantMap& params) const
{
    if (m_accessToken.isEmpty()) {
        return make_ret(cloud::Err::AccessTokenIsEmpty);
    }

    QUrlQuery query;
    query.addQueryItem(ACCESS_TOKEN_KEY, m_accessToken);

    for (auto it = params.cbegin(); it != params.cend(); ++it) {
        query.addQueryItem(it.key(), it.value().toString());
    }

    apiUrl.setQuery(query);

    return RetVal<QUrl>::make_ok(apiUrl);
}

RequestHeaders CloudService::headers() const
{
    return configuration()->headers();
}

mu::Ret CloudService::downloadAccountInfo()
{
    TRACEFUNC;

    RetVal<QUrl> userInfoUrl = prepareUrlForRequest(configuration()->userInfoApiUrl());
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
    info.id = user.value("id").toInt();
    info.userName = user.value("name").toString();
    QString profileUrl = user.value("profile_url").toString();
    info.profileUrl = QUrl(profileUrl);
    info.avatarUrl = QUrl(user.value("avatar_url").toString());
    info.sheetmusicUrl = QUrl(profileUrl + "/sheetmusic");

    if (info.isValid()) {
        setAccountInfo(info);
    } else {
        setAccountInfo(AccountInfo());
    }

    return make_ok();
}

mu::RetVal<ScoreInfo> CloudService::downloadScoreInfo(const QUrl& sourceUrl)
{
    return downloadScoreInfo(scoreIdFromSourceUrl(sourceUrl));
}

mu::RetVal<ScoreInfo> CloudService::downloadScoreInfo(int scoreId)
{
    TRACEFUNC;

    RetVal<ScoreInfo> result = RetVal<ScoreInfo>::make_ok(ScoreInfo());

    QVariantMap params;
    params[SCORE_ID_KEY] = scoreId;

    RetVal<QUrl> scoreInfoUrl = prepareUrlForRequest(configuration()->scoreInfoApiUrl(), params);
    if (!scoreInfoUrl.ret) {
        result.ret = scoreInfoUrl.ret;
        return result;
    }

    QBuffer receivedData;
    INetworkManagerPtr manager = networkManagerCreator()->makeNetworkManager();
    Ret ret = manager->get(scoreInfoUrl.val, &receivedData, headers());

    if (!ret) {
        printServerReply(receivedData);
        result.ret = ret;
        return result;
    }

    QJsonDocument document = QJsonDocument::fromJson(receivedData.data());
    QJsonObject scoreInfo = document.object();

    result.val.id = scoreInfo.value("id").toInt();
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

void CloudService::signIn()
{
    authorize();
}

void CloudService::signUp()
{
    if (m_userAuthorized.val) {
        return;
    }
    m_oauth2->setAuthorizationUrl(configuration()->signUpUrl());
    m_oauth2->grant();
}

void CloudService::signOut()
{
    if (!m_userAuthorized.val) {
        return;
    }

    TRACEFUNC;

    QVariantMap params;
    params[REFRESH_TOKEN_KEY] = m_refreshToken;

    RetVal<QUrl> signOutUrl = prepareUrlForRequest(configuration()->logoutApiUrl(), params);
    if (!signOutUrl.ret) {
        LOGE() << signOutUrl.ret.toString();
        return;
    }

    QBuffer receivedData;
    INetworkManagerPtr manager = networkManagerCreator()->makeNetworkManager();
    Ret ret = manager->get(signOutUrl.val, &receivedData, headers());
    if (!ret) {
        printServerReply(receivedData);
        LOGE() << ret.toString();
    }

    mi::WriteResourceLockGuard resource_guard(multiInstancesProvider(), CLOUD_ACCESS_TOKEN_RESOURCE_NAME);

    ret = fileSystem()->remove(configuration()->tokensFilePath());
    if (!ret) {
        LOGE() << ret.toString();
    }

    clearTokens();
}

mu::Ret CloudService::ensureAuthorization(const std::string& text)
{
    if (m_userAuthorized.val) {
        return make_ok();
    }

    UriQuery query("musescore://cloud/requireauthorization");
    query.addParam("text", Val(text));
    return interactive()->open(query).ret;
}

mu::ValCh<bool> CloudService::userAuthorized() const
{
    return m_userAuthorized;
}

mu::ValCh<AccountInfo> CloudService::accountInfo() const
{
    return m_accountInfo;
}

mu::Ret CloudService::checkCloudIsAvailable() const
{
    QBuffer receivedData;
    INetworkManagerPtr manager = networkManagerCreator()->makeNetworkManager();
    Ret ret = manager->get(configuration()->cloudUrl(), &receivedData, headers());

    if (!ret) {
        printServerReply(receivedData);
    }

    return ret;
}

void CloudService::setAccountInfo(const AccountInfo& info)
{
    if (m_accountInfo.val == info) {
        return;
    }

    m_accountInfo.set(info);
    m_userAuthorized.set(info.isValid());
}

ProgressPtr CloudService::uploadScore(QIODevice& scoreData, const QString& title, Visibility visibility, const QUrl& sourceUrl)
{
    ProgressPtr progress = std::make_shared<Progress>();

    auto uploadCallback = [this, progress, &scoreData, title, visibility, sourceUrl]() {
        progress->started.notify();

        INetworkManagerPtr manager = networkManagerCreator()->makeNetworkManager();
        manager->progress().progressChanged.onReceive(this, [progress](int64_t current, int64_t total, const std::string& message) {
            progress->progressChanged.send(current, total, message);
        });

        RetVal<ValMap> urlMap = doUploadScore(manager, scoreData, title, visibility, sourceUrl);

        ProgressResult result;
        result.ret = urlMap.ret;
        result.val = Val(urlMap.val);
        progress->finished.send(result);

        return result.ret;
    };

    async::Async::call(this, [this, uploadCallback]() {
        if (!m_userAuthorized.val) {
            authorize(uploadCallback);
            return;
        }

        executeRequest(uploadCallback);
    });

    return progress;
}

ProgressPtr CloudService::uploadAudio(QIODevice& audioData, const QString& audioFormat, const QUrl& sourceUrl)
{
    ProgressPtr progress = std::make_shared<Progress>();

    auto uploadCallback = [this, progress, &audioData, audioFormat, sourceUrl]() {
        progress->started.notify();

        INetworkManagerPtr manager = networkManagerCreator()->makeNetworkManager();
        manager->progress().progressChanged.onReceive(this, [progress](int64_t current, int64_t total, const std::string& message) {
            progress->progressChanged.send(current, total, message);
        });

        Ret ret = doUploadAudio(manager, audioData, audioFormat, sourceUrl);
        progress->finished.send(ret);

        return ret;
    };

    async::Async::call(this, [this, uploadCallback]() {
        if (!m_userAuthorized.val) {
            authorize(uploadCallback);
            return;
        }

        executeRequest(uploadCallback);
    });

    return progress;
}

static Ret uploadingRetFromRawUploadingRet(const Ret& rawRet, bool isScoreAlreadyUploaded)
{
    int code = statusCode(rawRet);

    if (!isScoreAlreadyUploaded && code == FORBIDDEN_CODE) {
        return make_ret(cloud::Err::AccountNotActivated);
    }

    static const std::map<int, mu::TranslatableString> codes {
        { 400, mu::TranslatableString("cloud", "Invalid request") },
        { 403, mu::TranslatableString("cloud", "Forbidden. User is not owner of the score.") },
        { 422, mu::TranslatableString("cloud", "Validation is failed") },
        { 500, mu::TranslatableString("cloud", "Internal server error") },
    };

    std::string userDescription = qtrc("cloud", "Error %1: %2")
                                  .arg(code)
                                  .arg(mu::value(codes, code, TranslatableString("cloud", "Unknown error")).qTranslated())
                                  .toStdString();

    Ret ret = make_ret(cloud::Err::NetworkError);
    ret.setData(CLOUD_NETWORK_ERROR_USER_DESCRIPTION_KEY, userDescription);
    return ret;
}

mu::RetVal<mu::ValMap> CloudService::doUploadScore(INetworkManagerPtr uploadManager, QIODevice& scoreData, const QString& title,
                                                   Visibility visibility, const QUrl& sourceUrl)
{
    TRACEFUNC;

    RetVal<ValMap> result = RetVal<ValMap>::make_ok(ValMap());

    RetVal<QUrl> uploadUrl = prepareUrlForRequest(configuration()->uploadScoreApiUrl());
    if (!uploadUrl.ret) {
        result.ret = uploadUrl.ret;
        return result;
    }

    int scoreId = scoreIdFromSourceUrl(sourceUrl);
    bool isScoreAlreadyUploaded = scoreId != INVALID_SCORE_ID;

    if (isScoreAlreadyUploaded) {
        RetVal<ScoreInfo> scoreInfo = downloadScoreInfo(scoreId);

        if (!scoreInfo.ret) {
            if (statusCode(scoreInfo.ret) == NOT_FOUND_STATUS_CODE) {
                isScoreAlreadyUploaded = false;
            } else {
                result.ret = scoreInfo.ret;
                return result;
            }
        }

        if (scoreInfo.val.owner.id != m_accountInfo.val.id) {
            isScoreAlreadyUploaded = false;
        }
    }

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
        scoreIdPart.setBody(QString::number(scoreId).toLatin1());
        multiPart.append(scoreIdPart);
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

        result.ret = uploadingRetFromRawUploadingRet(ret, isScoreAlreadyUploaded);

        return result;
    }

    QJsonObject scoreInfo = QJsonDocument::fromJson(receivedData.data()).object();
    QUrl newSourceUrl = QUrl(scoreInfo.value("permalink").toString());
    QUrl editUrl = QUrl(scoreInfo.value("edit_url").toString());

    if (!newSourceUrl.isValid()) {
        result.ret = make_ret(cloud::Err::CouldNotReceiveSourceUrl);
        return result;
    }

    result.val["sourceUrl"] = Val(newSourceUrl.toString());
    result.val["editUrl"] = Val(editUrl.toString());

    return result;
}

mu::Ret CloudService::doUploadAudio(network::INetworkManagerPtr uploadManager, QIODevice& audioData, const QString& audioFormat,
                                    const QUrl& sourceUrl)
{
    TRACEFUNC;

    RetVal<QUrl> uploadUrl = prepareUrlForRequest(configuration()->uploadAudioApiUrl());
    if (!uploadUrl.ret) {
        return uploadUrl.ret;
    }

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
    scoreIdPart.setBody(QString::number(scoreIdFromSourceUrl(sourceUrl)).toLatin1());
    multiPart.append(scoreIdPart);

    QBuffer receivedData;
    OutgoingDevice device(&multiPart);

    Ret ret = uploadManager->post(uploadUrl.val, &device, &receivedData, headers());

    if (!ret) {
        printServerReply(receivedData);
    }

    return ret;
}

void CloudService::executeRequest(const RequestCallback& requestCallback)
{
    Ret ret = requestCallback();
    if (ret) {
        return;
    }

    if (statusCode(ret) == USER_UNAUTHORIZED_STATUS_CODE) {
        if (updateTokens()) {
            ret = requestCallback();
        }
    }

    if (!ret) {
        LOGE() << ret.toString();
    }
}

void CloudService::openUrl(const QUrl& url)
{
    Ret ret = interactive()->openUrl(url);
    if (!ret) {
        LOGE() << ret.toString();
    }
}
