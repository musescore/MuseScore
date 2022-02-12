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

#include <QOAuth2AuthorizationCodeFlow>
#include <QOAuthHttpServerReplyHandler>
#include <QJsonDocument>
#include <QJsonObject>
#include <QUrlQuery>
#include <QBuffer>
#include <QHttpMultiPart>
#include <QRandomGenerator>

#include "multiinstances/resourcelockguard.h"

#include "log.h"

using namespace mu::cloud;
using namespace mu::network;
using namespace mu::framework;

static const QString ACCESS_TOKEN_KEY("token");
static const QString REFRESH_TOKEN_KEY("refresh_token");
static const QString DEVICE_ID_KEY("device_id");

static const std::string CLOUD_ACCESS_TOKEN_RESOURCE_NAME("CLOUD_ACCESS_TOKEN");

constexpr int USER_UNAUTHORIZED_ERR_CODE = 401;
constexpr int INVALID_SCORE_ID = 0;

static int scoreIdFromSourceUrl(const QUrl& sourceUrl)
{
    QStringList parts = sourceUrl.toString().split("/");
    if (parts.isEmpty()) {
        return INVALID_SCORE_ID;
    }

    return parts.last().toInt();
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
    m_replyHandler = new QOAuthHttpServerReplyHandler(this);
    m_networkManager = networkManagerCreator()->makeNetworkManager();

    m_oauth2->setAuthorizationUrl(configuration()->authorizationUrl());
    m_oauth2->setAccessTokenUrl(configuration()->accessTokenUrl());
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
        executeRequest([this]() { return downloadUserInfo(); });
    }
}

bool CloudService::readTokens()
{
    TRACEFUNC;

    mi::ReadResourceLockGuard resource_guard(multiInstancesProvider(), CLOUD_ACCESS_TOKEN_RESOURCE_NAME);

    io::path tokensPath = configuration()->tokensFilePath();
    if (!fileSystem()->exists(tokensPath)) {
        return false;
    }

    RetVal<QByteArray> tokensData = fileSystem()->readFile(tokensPath);
    if (!tokensData.ret) {
        LOGE() << tokensData.ret.toString();
        return false;
    }

    QJsonDocument tokensDoc = QJsonDocument::fromJson(tokensData.val);
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

    Ret ret = fileSystem()->writeToFile(configuration()->tokensFilePath(), tokensDoc.toJson());
    if (!ret) {
        LOGE() << ret.toString();
    }

    return ret;
}

bool CloudService::updateTokens()
{
    TRACEFUNC;

    QUrlQuery query;
    query.addQueryItem(REFRESH_TOKEN_KEY, m_refreshToken);
    query.addQueryItem(DEVICE_ID_KEY, configuration()->clientId());

    QUrl refreshApiUrl = configuration()->refreshApiUrl();
    refreshApiUrl.setQuery(query);

    QBuffer receivedData;
    Ret ret = m_networkManager->post(refreshApiUrl, nullptr, &receivedData, headers());

    if (!ret) {
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

    if (downloadUserInfo() == RequestStatus::Ok && m_onUserAuthorizedCallback) {
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
    m_oauth2->grant();
}

QUrl CloudService::prepareUrlForRequest(QUrl apiUrl) const
{
    if (m_accessToken.isEmpty()) {
        return QUrl();
    }

    QUrlQuery query;
    query.addQueryItem(ACCESS_TOKEN_KEY, m_accessToken);
    apiUrl.setQuery(query);

    return apiUrl;
}

RequestHeaders CloudService::headers() const
{
    return configuration()->headers();
}

CloudService::RequestStatus CloudService::downloadUserInfo()
{
    TRACEFUNC;

    QUrl userInfoUrl = prepareUrlForRequest(configuration()->userInfoApiUrl());
    if (userInfoUrl.isEmpty()) {
        return RequestStatus::Error;
    }

    QBuffer receivedData;
    Ret ret = m_networkManager->get(userInfoUrl, &receivedData, headers());

    if (ret.code() == USER_UNAUTHORIZED_ERR_CODE) {
        return RequestStatus::UserUnauthorized;
    }

    if (!ret) {
        LOGE() << ret.toString();
        return RequestStatus::Error;
    }

    QJsonDocument document = QJsonDocument::fromJson(receivedData.data());
    QJsonObject user = document.object();

    AccountInfo info;
    info.id = user.value("id").toString().toInt();
    info.userName = user.value("name").toString();
    QString profileUrl = user.value("permalink").toString();
    info.profileUrl = QUrl(profileUrl);
    info.avatarUrl = QUrl(user.value("avatar_url").toString());
    info.sheetmusicUrl = QUrl(profileUrl + "/sheetmusic");

    setAccountInfo(info);

    return RequestStatus::Ok;
}

void CloudService::signIn()
{
    authorize();
}

void CloudService::signOut()
{
    if (!m_userAuthorized.val) {
        return;
    }

    TRACEFUNC;

    QUrl signOutUrl = prepareUrlForRequest(configuration()->loginApiUrl());
    if (!signOutUrl.isEmpty()) {
        QBuffer receivedData;
        Ret ret = m_networkManager->del(signOutUrl, &receivedData, headers());

        if (!ret) {
            LOGE() << ret.toString();
        }
    }

    mi::WriteResourceLockGuard resource_guard(multiInstancesProvider(), CLOUD_ACCESS_TOKEN_RESOURCE_NAME);

    Ret ret = fileSystem()->remove(configuration()->tokensFilePath());
    if (!ret) {
        LOGE() << ret.toString();
    }

    clearTokens();
}

mu::ValCh<bool> CloudService::userAuthorized() const
{
    return m_userAuthorized;
}

mu::ValCh<AccountInfo> CloudService::accountInfo() const
{
    return m_accountInfo;
}

void CloudService::setAccountInfo(const AccountInfo& info)
{
    if (m_accountInfo.val == info) {
        return;
    }

    m_accountInfo.set(info);
    m_userAuthorized.set(info.isValid());
}

void CloudService::uploadScore(io::Device& scoreSourceDevice, const QString& title, const QUrl& sourceUrl)
{
    auto uploadCallback = [this, &scoreSourceDevice, title, sourceUrl]() {
        return doUploadScore(scoreSourceDevice, title, sourceUrl);
    };

    if (!m_userAuthorized.val) {
        authorize(uploadCallback);
        return;
    }

    executeRequest(uploadCallback);
}

CloudService::RequestStatus CloudService::doUploadScore(io::Device& scoreSourceDevice, const QString& title, const QUrl& sourceUrl)
{
    QUrl uploadUrl = prepareUrlForRequest(configuration()->uploadingApiUrl());
    if (uploadUrl.isEmpty()) {
        return RequestStatus::Error;
    }

    int scoreId = scoreIdFromSourceUrl(sourceUrl);
    bool isScoreAlreadyUploaded = scoreId != INVALID_SCORE_ID;

    QHttpMultiPart multiPart(QHttpMultiPart::FormDataType);

    QHttpPart filePart;
    filePart.setHeader(QNetworkRequest::ContentTypeHeader, QVariant("application/octet-stream"));
    int fileNameNumber = QRandomGenerator::global()->generate() % 100000;
    QString contentDisposition = QString("form-data; name=\"score_data\"; filename=\"temp_%1.mscz\"").arg(fileNameNumber);
    filePart.setHeader(QNetworkRequest::ContentDispositionHeader, QVariant(contentDisposition));

    filePart.setBodyDevice(&scoreSourceDevice);
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

    QHttpPart licensePart;
    licensePart.setHeader(QNetworkRequest::ContentDispositionHeader, QVariant("form-data; name=\"license\""));
    licensePart.setBody(configuration()->uploadingLicense());
    multiPart.append(licensePart);

    Ret ret(true);
    QBuffer receivedData;
    OutgoingDevice device(&multiPart);

    if (isScoreAlreadyUploaded) { // score exists, update
        ret = m_networkManager->put(uploadUrl, &device, &receivedData, headers());
    } else { // score doesn't exist, post a new score
        ret = m_networkManager->post(uploadUrl, &device, &receivedData, headers());
    }

    if (ret.code() == USER_UNAUTHORIZED_ERR_CODE) {
        return RequestStatus::UserUnauthorized;
    }

    if (!ret) {
        LOGE() << ret.toString();
        return RequestStatus::Error;
    }

    QJsonObject scoreInfo = QJsonDocument::fromJson(receivedData.data()).object();
    QUrl newSourceUrl = QUrl(scoreInfo.value("permalink").toString());
    QUrl editUrl = QUrl(scoreInfo.value("edit_url").toString());

    if (newSourceUrl.isValid()) {
        m_sourceUrlReceived.send(newSourceUrl);
    }

    openUrl(editUrl);

    return RequestStatus::Ok;
}

mu::async::Channel<QUrl> CloudService::sourceUrlReceived() const
{
    return m_sourceUrlReceived;
}

ProgressChannel CloudService::progressChannel() const
{
    return m_networkManager->progressChannel();
}

void CloudService::executeRequest(const RequestCallback& requestCallback)
{
    if (requestCallback() != RequestStatus::UserUnauthorized) {
        return;
    }

    if (updateTokens()) {
        requestCallback();
    }
}

void CloudService::openUrl(const QUrl& url)
{
    Ret ret = interactive()->openUrl(url);
    if (!ret) {
        LOGE() << ret.toString();
    }
}
