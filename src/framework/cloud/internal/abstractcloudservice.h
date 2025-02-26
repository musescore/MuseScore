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
#ifndef MUSE_CLOUD_ABSTRACTCLOUDSERVICE_H
#define MUSE_CLOUD_ABSTRACTCLOUDSERVICE_H

#include <QObject>
#include <QBuffer>

class QOAuth2AuthorizationCodeFlow;

#include "async/asyncable.h"

#include "modularity/ioc.h"
#include "icloudconfiguration.h"
#include "ui/iuiconfiguration.h"
#include "io/ifilesystem.h"
#include "network/inetworkmanagercreator.h"
#include "multiinstances/imultiinstancesprovider.h"
#include "iinteractive.h"

#include "iauthorizationservice.h"

namespace muse::cloud {
extern const QString ACCESS_TOKEN_KEY;
extern const QString REFRESH_TOKEN_KEY;

static constexpr int USER_UNAUTHORIZED_STATUS_CODE = 401;
static constexpr int FORBIDDEN_CODE = 403;
static constexpr int NOT_FOUND_STATUS_CODE = 404;
static constexpr int CONFLICT_STATUS_CODE = 409;

QString userAgent();
int generateFileNameNumber();

class OAuthHttpServerReplyHandler;

class AbstractCloudService : public QObject, public IAuthorizationService, public Injectable, public async::Asyncable
{
    Q_OBJECT
public:
    muse::Inject<ICloudConfiguration> configuration = { this };
    muse::Inject<ui::IUiConfiguration> uiConfig = { this };
    muse::Inject<io::IFileSystem> fileSystem = { this };
    muse::Inject<network::INetworkManagerCreator> networkManagerCreator = { this };
    muse::Inject<IInteractive> interactive = { this };
    muse::Inject<mi::IMultiInstancesProvider> multiInstancesProvider = { this };

public:
    explicit AbstractCloudService(const modularity::ContextPtr& iocCtx, QObject* parent = nullptr);

    void init();

    void signUp() override;
    void signIn() override;
    void signOut() override;

    RetVal<Val> ensureAuthorization(bool publishingScore, const std::string& text = {}) override;

    ValCh<bool> userAuthorized() const override;
    ValCh<AccountInfo> accountInfo() const override;

    Ret checkCloudIsAvailable() const override;

private slots:
    void onUserAuthorized();

protected:
    struct ServerConfig {
        QString serverCode;
        QUrl serverUrl;

        QUrl serverAvailabilityUrl;

        QUrl authorizationUrl;
        QUrl signUpUrl;
        QUrl signInSuccessUrl;
        QUrl accessTokenUrl;
        QUrl userInfoUrl;

        QUrl refreshApiUrl;
        QUrl logoutApiUrl;

        network::RequestHeaders headers;
        QVariantMap authorizationParameters;
        QVariantMap refreshParameters;
    };

    virtual ServerConfig serverConfig() const = 0;

    virtual Ret downloadAccountInfo() = 0;

    QString logoColor() const;

    void setAccountInfo(const AccountInfo& info);

    RetVal<QUrl> prepareUrlForRequest(QUrl apiUrl, const QVariantMap& params = QVariantMap()) const;

    using RequestCallback = std::function<Ret ()>;
    Ret executeRequest(const RequestCallback& requestCallback);

    Ret uploadingDownloadingRetFromRawRet(const Ret& rawRet, bool isAlreadyUploaded = false) const;
    int statusCode(const Ret& ret) const;
    void printServerReply(const QBuffer& reply) const;

    QString accessToken() const;
    void setAccessToken(const QString& token);

    QString refreshToken() const;
    void setRefreshToken(const QString& token);

    virtual bool doUpdateTokens();

private:
    void initOAuthIfNecessary();

    bool readTokens();
    bool saveTokens();
    bool updateTokens();
    void clearTokens();

    io::path_t tokensFilePath() const;

    void openUrl(const QUrl& url);

    network::RequestHeaders headers() const;

    QOAuth2AuthorizationCodeFlow* m_oauth2 = nullptr;
    OAuthHttpServerReplyHandler* m_replyHandler = nullptr;

    ValCh<bool> m_userAuthorized;
    ValCh<AccountInfo> m_accountInfo;

    QString m_accessToken;
    QString m_refreshToken;

    ServerConfig m_serverConfig;
};
}

#endif // MUSE_CLOUD_ABSTRACTCLOUDSERVICE_H
