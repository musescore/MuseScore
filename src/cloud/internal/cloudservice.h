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
#ifndef MU_CLOUD_CLOUDSERVICE_H
#define MU_CLOUD_CLOUDSERVICE_H

#include <QObject>

class QOAuth2AuthorizationCodeFlow;
class QOAuthHttpServerReplyHandler;

#include "iauthorizationservice.h"
#include "iuploadingservice.h"

#include "async/asyncable.h"

#include "modularity/ioc.h"
#include "icloudconfiguration.h"
#include "system/ifilesystem.h"
#include "network/inetworkmanagercreator.h"
#include "multiinstances/imultiinstancesprovider.h"
#include "iinteractive.h"

namespace mu::cloud {
class CloudService : public QObject, public IAuthorizationService, public IUploadingService, public async::Asyncable
{
    Q_OBJECT

    INJECT(cloud, ICloudConfiguration, configuration)
    INJECT(cloud, system::IFileSystem, fileSystem)
    INJECT(cloud, network::INetworkManagerCreator, networkManagerCreator)
    INJECT(cloud, framework::IInteractive, interactive)
    INJECT(cloud, mi::IMultiInstancesProvider, multiInstancesProvider)

public:
    CloudService(QObject* parent = nullptr);

    void init();

    void signIn() override;
    void signOut() override;

    ValCh<bool> userAuthorized() const override;
    ValCh<AccountInfo> accountInfo() const override;

    void uploadScore(io::Device& scoreSourceDevice, const QString& title, const QUrl& sourceUrl = QUrl()) override;

    async::Channel<QUrl> sourceUrlReceived() const override;
    framework::ProgressChannel progressChannel() const override;

private slots:
    void onUserAuthorized();

private:
    bool readTokens();
    bool saveTokens();
    bool updateTokens();
    void clearTokens();

    using OnUserAuthorizedCallback = std::function<void ()>;
    void authorize(const OnUserAuthorizedCallback& onUserAuthorizedCallback = OnUserAuthorizedCallback());

    void setAccountInfo(const AccountInfo& info);

    void openUrl(const QUrl& url);

    enum class RequestStatus {
        Ok,
        UserUnauthorized,
        Error
    };

    QUrl prepareUrlForRequest(QUrl apiUrl) const;
    network::RequestHeaders headers() const;

    RequestStatus downloadUserInfo();
    RequestStatus doUploadScore(io::Device& scoreSourceDevice, const QString& title, const QUrl& sourceUrl = QUrl());

    using RequestCallback = std::function<RequestStatus()>;
    void executeRequest(const RequestCallback& requestCallback);

    QOAuth2AuthorizationCodeFlow* m_oauth2 = nullptr;
    QOAuthHttpServerReplyHandler* m_replyHandler = nullptr;
    network::INetworkManagerPtr m_networkManager;

    ValCh<bool> m_userAuthorized;
    ValCh<AccountInfo> m_accountInfo;

    QString m_accessToken;
    QString m_refreshToken;

    OnUserAuthorizedCallback m_onUserAuthorizedCallback;
    async::Channel<QUrl> m_sourceUrlReceived;
};
}

#endif // MU_CLOUD_CLOUDSERVICE_H
