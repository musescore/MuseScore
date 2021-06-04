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

#include "authorizationservice.h"

#include "log.h"
#include "config.h"

#include <QOAuth2AuthorizationCodeFlow>
#include <QOAuthHttpServerReplyHandler>
#include <QDesktopServices>
#include <QJsonDocument>
#include <QJsonObject>
#include <QUrlQuery>
#include <QBuffer>

using namespace mu::cloud;
using namespace mu::network;

static const QString ACCESS_TOKEN_KEY("token");
static const QString REFRESH_TOKEN_KEY("refreshToken");

static QByteArray generateClientId()
{
    QByteArray qtGeneratedId(QSysInfo::machineUniqueId());
    if (!qtGeneratedId.isEmpty()) {
        return qtGeneratedId;
    }

    long long randId = qrand();
    constexpr size_t randBytes = sizeof(decltype(qrand()));

    for (size_t bytes = randBytes; bytes < sizeof(randId); bytes += randBytes) {
        randId <<= 8 * randBytes;
        randId += qrand();
    }

    return QString::number(randId, 16).toLatin1();
}

static QString userAgent()
{
    QStringList systemInfo;
    systemInfo << QSysInfo::kernelType() << QSysInfo::kernelVersion()
         << QSysInfo::productType() << QSysInfo::productVersion()
         << QSysInfo::currentCpuArchitecture();

    return QString("MS_EDITOR/%1.%2 (%3)")
            .arg(VERSION)
            .arg(BUILD_NUMBER)
            .arg(systemInfo.join(' ')).toLatin1();
}

static RequestHeaders buildHeaders()
{
    static QByteArray clientId = generateClientId();

    RequestHeaders header;
    header.rawHeaders["Accept"] = "application/json";
    header.rawHeaders["X-MS-API-KEY"] = "0b19809bab331d70fb9983a0b9866290";
    header.rawHeaders["X-MS-CLIENT-ID"] = clientId;
    header.knownHeaders[QNetworkRequest::UserAgentHeader] = userAgent();

    return header;
}

AuthorizationService::AuthorizationService(QObject* parent)
    : QObject(parent)
{
    m_userAuthorized.val = false; 
}

void AuthorizationService::init()
{
    TRACEFUNC;

    m_oauth2 = new QOAuth2AuthorizationCodeFlow(this);
    m_replyHandler = new QOAuthHttpServerReplyHandler(this);
    m_networkManager = networkManagerCreator()->makeNetworkManager();

    m_oauth2->setAuthorizationUrl(configuration()->authorizationUrl());
    m_oauth2->setAccessTokenUrl(configuration()->accessTokenUrl());
    m_oauth2->setReplyHandler(m_replyHandler);

    connect(m_oauth2, &QOAuth2AuthorizationCodeFlow::authorizeWithBrowser, &QDesktopServices::openUrl);
    connect(m_oauth2, &QOAuth2AuthorizationCodeFlow::granted, this, &AuthorizationService::onUserAuthorized);

    connect(m_oauth2, &QOAuth2AuthorizationCodeFlow::error, [](const QString& error, const QString& errorDescription, const QUrl& uri){
        LOGE() << "Error during authorization: " << error << "\n Description: " << errorDescription << "\n URI: " << uri.toString();
    });

    RetVal<QByteArray> tokensData = fileSystem()->readFile(configuration()->tokensFilePath());
    if (!tokensData.ret) {
        LOGE() << tokensData.ret.toString();
        return;
    }

    QJsonDocument tokensDoc = QJsonDocument::fromBinaryData(tokensData.val);
    QJsonObject saveObject = tokensDoc.object();

    m_accessToken = saveObject[ACCESS_TOKEN_KEY].toString();
    m_refreshToken = saveObject[REFRESH_TOKEN_KEY].toString();

    downloadUserInfo();
}

void AuthorizationService::onUserAuthorized()
{
    TRACEFUNC;

    m_accessToken = m_oauth2->token();
    m_refreshToken = m_oauth2->refreshToken();

    QJsonObject tokensObject;
    tokensObject[ACCESS_TOKEN_KEY] = m_accessToken;
    tokensObject[REFRESH_TOKEN_KEY] = m_refreshToken;
    QJsonDocument tokensDoc(tokensObject);

    Ret ret = fileSystem()->writeToFile(configuration()->tokensFilePath(), tokensDoc.toBinaryData());
    if (!ret) {
        LOGE() << ret.toString();
        //return;
    }

    downloadUserInfo();
}

void AuthorizationService::downloadUserInfo()
{
    if (m_accessToken.isEmpty() || m_refreshToken.isEmpty()) {
        return;
    }

    QUrl userInfoUrl = prepareUrlForRequest(configuration()->userInfoApiUrl());
    QBuffer receivedData;
    Ret ret = m_networkManager->get(userInfoUrl, &receivedData, buildHeaders());

    if (!ret) {
        LOGE() << ret.toString();
        return;
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
}

void AuthorizationService::createAccount()
{
    NOT_IMPLEMENTED;
}

void AuthorizationService::signIn()
{
    if (m_userAuthorized.val) {
        return;
    }

    m_oauth2->grant();
}

void AuthorizationService::signOut()
{
    if (!m_userAuthorized.val) {
        return;
    }

    QUrl signOutUrl = prepareUrlForRequest(configuration()->signOutApiUrl());
    if (!signOutUrl.isEmpty()) {
        QBuffer receivedData;
        Ret ret = m_networkManager->del(signOutUrl, &receivedData, buildHeaders());

        if (!ret) {
            LOGE() << ret.toString();
        }
    }

    Ret ret = fileSystem()->remove(configuration()->tokensFilePath());
    if (!ret) {
        LOGE() << ret.toString();
    }

    m_accessToken.clear();
    m_refreshToken.clear();

    setAccountInfo(AccountInfo());
}

QUrl AuthorizationService::prepareUrlForRequest(QUrl apiUrl) const
{
    if (m_accessToken.isEmpty()) {
        return QUrl();
    }

    QUrlQuery query;
    query.addQueryItem(ACCESS_TOKEN_KEY, m_accessToken);
    apiUrl.setQuery(query);

    return apiUrl;
}

mu::ValCh<bool> AuthorizationService::userAuthorized() const
{
    return m_userAuthorized;
}

mu::ValCh<AccountInfo> AuthorizationService::accountInfo() const
{
    return m_accountInfo;
}

void AuthorizationService::setAccountInfo(const AccountInfo& info)
{
    if (m_accountInfo.val == info) {
        return;
    }

    m_accountInfo.set(info);
    m_userAuthorized.set(info.isValid());
}
