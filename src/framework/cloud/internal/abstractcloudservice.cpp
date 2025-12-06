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

#include "abstractcloudservice.h"

#include <QOAuth2AuthorizationCodeFlow>
#include <QJsonDocument>
#include <QJsonObject>
#include <QUrlQuery>
#include <QHttpMultiPart>
#include <QRandomGenerator>

#include "clouderrors.h"
#include "multiinstances/resourcelockguard.h"
#include "network/networkerrors.h"
#include "global/iapplication.h"
#include "global/io/ioretcodes.h"
#include "draw/types/color.h"

#include "oauthhttpserverreplyhandler.h"

#include "defer.h"
#include "log.h"

using namespace muse;
using namespace muse::cloud;
using namespace muse::network;
using namespace muse::async;

const QString muse::cloud::ACCESS_TOKEN_KEY("access_token");
const QString muse::cloud::REFRESH_TOKEN_KEY("refresh_token");

static const std::string CLOUD_ACCESS_TOKEN_RESOURCE_NAME("CLOUD_ACCESS_TOKEN");
static const std::string STATUS_KEY("status");

AbstractCloudService::AbstractCloudService(const modularity::ContextPtr& iocCtx, QObject* parent)
    : QObject(parent), Injectable(iocCtx)
{
    m_userAuthorized.val = false;
}

void AbstractCloudService::init()
{
    TRACEFUNC;

    m_serverConfig = serverConfig();
    m_networkManager = networkManagerCreator()->makeNetworkManager();

    multiInstancesProvider()->resourceChanged().onReceive(this, [this](const std::string& resourceName) {
        if (resourceName == CLOUD_ACCESS_TOKEN_RESOURCE_NAME) {
            readTokens();
        }
    });

    if (readTokens()) {
        executeAsyncRequest([this]() { return downloadAccountInfo(); });
    }
}

void AbstractCloudService::initOAuthIfNecessary()
{
    if (m_oauth2) {
        return;
    }

    // We initialize the OAuth etc lazily, to save resources, but also so that firewall warnings
    // will only appear at the moment that the user tries to log in, not on every launch.

    m_oauth2 = new QOAuth2AuthorizationCodeFlow(this);

    m_replyHandler = new OAuthHttpServerReplyHandler(iocContext(), this);
    m_replyHandler->setRedirectUrl(m_serverConfig.signInSuccessUrl);

    m_oauth2->setAuthorizationUrl(m_serverConfig.authorizationUrl);
#if QT_VERSION >= QT_VERSION_CHECK(6, 9, 0)
    m_oauth2->setTokenUrl(m_serverConfig.accessTokenUrl);
    m_oauth2->setPkceMethod(QOAuth2AuthorizationCodeFlow::PkceMethod::None);
#else
    m_oauth2->setAccessTokenUrl(m_serverConfig.accessTokenUrl);
#endif
    m_oauth2->setModifyParametersFunction([this](QAbstractOAuth::Stage, QMultiMap<QString, QVariant>* parameters) {
        for (const QString& key : m_serverConfig.authorizationParameters.keys()) {
            parameters->replace(key, m_serverConfig.authorizationParameters.value(key));
        }
    });

    m_oauth2->setReplyHandler(m_replyHandler);

    connect(m_oauth2, &QOAuth2AuthorizationCodeFlow::authorizeWithBrowser, this, &AbstractCloudService::openUrl);
    connect(m_oauth2, &QOAuth2AuthorizationCodeFlow::granted, this, &AbstractCloudService::onUserAuthorized);

#if QT_VERSION >= QT_VERSION_CHECK(6, 9, 0)
    connect(m_oauth2, &QOAuth2AuthorizationCodeFlow::serverReportedErrorOccurred,
            [](const QString& error, const QString& errorDescription, const QUrl& uri) {
        LOGE() << "Error during authorization: " << error << "\n Description: " << errorDescription << "\n URI: " << uri.toString();
    });
#else
    connect(m_oauth2, &QOAuth2AuthorizationCodeFlow::error, [](const QString& error, const QString& errorDescription, const QUrl& uri) {
        LOGE() << "Error during authorization: " << error << "\n Description: " << errorDescription << "\n URI: " << uri.toString();
    });
#endif
}

bool AbstractCloudService::readTokens()
{
    TRACEFUNC;

    io::path_t tokensPath = tokensFilePath();
    RetVal<ByteArray> tokensData;
    {
        mi::ReadResourceLockGuard resource_guard(multiInstancesProvider(), CLOUD_ACCESS_TOKEN_RESOURCE_NAME);
        tokensData = fileSystem()->readFile(tokensPath);
    }

    if (!tokensData.ret) {
        if (tokensData.ret.code() == (int)io::Err::FSNotExist) {
            LOGI() << "Could not find the tokens file: " << tokensPath;
        } else {
            LOGE() << tokensData.ret.toString();
        }

        return false;
    }

    QJsonParseError err;
    QJsonDocument tokensDoc = QJsonDocument::fromJson(tokensData.val.toQByteArrayNoCopy(), &err);
    if (err.error != QJsonParseError::NoError || !tokensDoc.isObject()) {
        LOGE() << "Error on parse tokens file: " << err.errorString();
        return false;
    }

    QJsonObject saveObject = tokensDoc.object();

    m_accessToken = saveObject[ACCESS_TOKEN_KEY].toString();
    m_refreshToken = saveObject[REFRESH_TOKEN_KEY].toString();

    return true;
}

bool AbstractCloudService::saveTokens()
{
    TRACEFUNC;

    QJsonObject tokensObject;
    tokensObject[ACCESS_TOKEN_KEY] = m_accessToken;
    tokensObject[REFRESH_TOKEN_KEY] = m_refreshToken;
    QJsonDocument tokensDoc(tokensObject);
    QByteArray json = tokensDoc.toJson();

    Ret ret;
    {
        mi::WriteResourceLockGuard resource_guard(multiInstancesProvider(), CLOUD_ACCESS_TOKEN_RESOURCE_NAME);
        ret = fileSystem()->writeFile(tokensFilePath(), ByteArray::fromQByteArrayNoCopy(json));
    }

    if (!ret) {
        LOGE() << ret.toString();
    }

    return ret;
}

void AbstractCloudService::removeTokens()
{
    {
        mi::WriteResourceLockGuard resource_guard(multiInstancesProvider(), CLOUD_ACCESS_TOKEN_RESOURCE_NAME);
        Ret ret = fileSystem()->remove(tokensFilePath());
        if (!ret) {
            LOGE() << ret.toString();
        }
    }

    clearTokens();
}

void AbstractCloudService::clearTokens()
{
    m_accessToken.clear();
    m_refreshToken.clear();
    setAccountInfo(AccountInfo());
}

io::path_t AbstractCloudService::tokensFilePath() const
{
    return configuration()->tokensFilePath(m_serverConfig.serverCode.toStdString());
}

void AbstractCloudService::onUserAuthorized()
{
    TRACEFUNC;

    initOAuthIfNecessary();

    m_accessToken = m_oauth2->token();
    m_refreshToken = m_oauth2->refreshToken();

    LOGD() << "========== access " << m_accessToken << " ========= refresh " << m_refreshToken;

    saveTokens();

    downloadAccountInfo().onResolve(this, [](const Ret& ret) {
        if (!ret) {
            LOGE() << ret.toString();
        }
    });
}

RequestHeaders AbstractCloudService::defaultHeaders() const
{
    return configuration()->headers();
}

RetVal<QUrl> AbstractCloudService::prepareUrlForRequest(QUrl apiUrl, const QVariantMap& params) const
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

void AbstractCloudService::signIn()
{
    if (m_userAuthorized.val) {
        return;
    }

    initOAuthIfNecessary();

    m_oauth2->setAuthorizationUrl(m_serverConfig.authorizationUrl);
    m_oauth2->grant();
}

void AbstractCloudService::signUp()
{
    if (m_userAuthorized.val) {
        return;
    }

    initOAuthIfNecessary();

    m_oauth2->setAuthorizationUrl(m_serverConfig.signUpUrl);
    m_oauth2->grant();
}

void AbstractCloudService::signOut()
{
    if (!m_userAuthorized.val) {
        return;
    }

    TRACEFUNC;

    QVariantMap params;
    params[REFRESH_TOKEN_KEY] = m_refreshToken;

    RetVal<QUrl> signOutUrl = prepareUrlForRequest(m_serverConfig.logoutApiUrl, params);
    if (!signOutUrl.ret) {
        LOGE() << signOutUrl.ret.toString();
        return;
    }

    if (signOutUrl.val.isEmpty()) {
        removeTokens();
        return;
    }

    auto receivedData = std::make_shared<QBuffer>();
    RetVal<Progress> progress = m_networkManager->get(signOutUrl.val, receivedData, m_serverConfig.headers);
    if (!progress.ret) {
        LOGE() << progress.ret.toString();
        removeTokens();
        return;
    }

    progress.val.finished().onReceive(this, [this, receivedData](const ProgressResult& res) {
        if (!res.ret) {
            LOGE() << res.ret;
            printServerReply(*receivedData);
        }

        removeTokens();
    });
}

RetVal<Val> AbstractCloudService::ensureAuthorization(bool publishingScore, const std::string& text)
{
    if (m_userAuthorized.val) {
        return muse::make_ok();
    }

    UriQuery query("muse://cloud/requireauthorization");
    query.addParam("text", Val(text));
    query.addParam("cloudCode", Val(cloudInfo().code));
    query.addParam("publishingScore", Val(publishingScore));
    return interactive()->openSync(query);
}

ValCh<bool> AbstractCloudService::userAuthorized() const
{
    return m_userAuthorized;
}

const AccountInfo& AbstractCloudService::accountInfo() const
{
    return m_accountInfo;
}

Ret AbstractCloudService::checkCloudIsAvailable() const
{
    QBuffer receivedData;
    deprecated::INetworkManagerPtr manager = networkManagerCreator()->makeDeprecatedNetworkManager();
    Ret ret = manager->get(m_serverConfig.serverAvailabilityUrl, &receivedData, m_serverConfig.headers);

    if (!ret) {
        printServerReply(receivedData);
    }

    return ret;
}

void AbstractCloudService::setAccountInfo(const AccountInfo& info)
{
    if (m_accountInfo == info) {
        return;
    }

    m_accountInfo = info;
    m_userAuthorized.set(info.isValid());
}

Ret AbstractCloudService::executeRequest(const RequestCallback& requestCallback)
{
    DEPRECATED_USE("executeAsyncRequest(callback)");

    Ret ret = requestCallback();
    if (ret) {
        return muse::make_ok();
    }

    if (statusCode(ret) != USER_UNAUTHORIZED_STATUS_CODE) {
        return ret;
    }

    QEventLoop loop;
    updateTokens().onResolve(this, [this, &loop, &ret](const Ret& updateTokensRet) {
        DEFER {
            loop.quit();
        };

        if (!updateTokensRet) {
            ret = updateTokensRet;
            clearTokens();
            return;
        }

        if (!saveTokens()) {
            ret = false;
            return;
        }
    });
    loop.exec();

    if (ret) {
        ret = requestCallback();
    }

    return ret;
}

void AbstractCloudService::executeAsyncRequest(const AsyncRequestCallback& requestCallback)
{
    requestCallback().onResolve(this, [this, requestCallback](const Ret& ret) {
        if (ret) {
            return;
        }

        if (statusCode(ret) != USER_UNAUTHORIZED_STATUS_CODE) {
            LOGE() << ret.toString();
            return;
        }

        // Update tokens and retry request
        updateTokens().onResolve(this, [this, requestCallback](const Ret& ret) {
            if (!ret) {
                LOGE() << ret.toString();
                clearTokens();
                return;
            }

            if (!saveTokens()) {
                return;
            }

            requestCallback().onResolve(this, [](const Ret& ret) {
                if (!ret) {
                    LOGE() << ret.toString();
                }
            });
        });
    });
}

Ret AbstractCloudService::uploadingDownloadingRetFromRawRet(const Ret& rawRet, bool isAlreadyUploaded) const
{
    if (rawRet) {
        return rawRet; // OK
    }

    int status = statusCode(rawRet);
    if (status) {
        switch (status) {
        case 400: return make_ret(Err::Status400_InvalidRequest);
        case 401: return make_ret(Err::Status401_AuthorizationRequired);
        case 403:
            if (isAlreadyUploaded) {
                return make_ret(Err::Status403_AccountNotActivated);
            } else {
                return make_ret(Err::Status403_NotOwner);
            }
        case 404: return make_ret(Err::Status404_NotFound);
        case 409: return make_ret(Err::Status409_Conflict);
        case 422: return make_ret(Err::Status422_ValidationFailed);
        case 429: return make_ret(Err::Status429_RateLimitExceeded);
        case 500: return make_ret(Err::Status500_InternalServerError);
        default: break;
        }

        Ret ret = make_ret(Err::UnknownStatusCode);
        ret.setText("Unknown status code: " + std::to_string(status));
        ret.setData("status", status);
        return ret;
    }

    switch (rawRet.code()) {
    case int(network::Err::NetworkError):
    case int(network::Err::Timeout):
    case int(network::Err::Abort):
        return make_ret(Err::NetworkError);
    }

    return rawRet;
}

int AbstractCloudService::statusCode(const Ret& ret) const
{
    return ret.data<int>(STATUS_KEY, 0);
}

void AbstractCloudService::printServerReply(const QBuffer& reply) const
{
    const QByteArray& data = reply.data();

    if (!data.isEmpty()) {
        LOGD() << "Server reply: " << data;
    }
}

const QString& AbstractCloudService::accessToken() const
{
    return m_accessToken;
}

void AbstractCloudService::setAccessToken(const QString& token)
{
    m_accessToken = token;
}

const QString& AbstractCloudService::refreshToken() const
{
    return m_refreshToken;
}

void AbstractCloudService::setRefreshToken(const QString& token)
{
    m_refreshToken = token;
}

void AbstractCloudService::openUrl(const QUrl& url)
{
    Ret ret = interactive()->openUrl(url);
    if (!ret) {
        LOGE() << ret.toString();
    }
}

QString AbstractCloudService::logoColor() const
{
    const ui::ThemeList& themens = uiConfig()->themes();
    bool isDarkMode = uiConfig()->isDarkMode();

    for (const ui::ThemeInfo& theme : themens) {
        if ((isDarkMode && theme.codeKey == ui::DARK_THEME_CODE)
            || (!isDarkMode && theme.codeKey == ui::LIGHT_THEME_CODE)) {
            return theme.values[ui::FONT_PRIMARY_COLOR].toString();
        }
    }

    return QString::fromStdString(draw::Color::BLACK.toString());
}
