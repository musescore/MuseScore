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

#include "iauthorizationservice.h"
#include "icloudprojectsservice.h"

#include "async/asyncable.h"

#include "modularity/ioc.h"
#include "icloudconfiguration.h"
#include "io/ifilesystem.h"
#include "network/inetworkmanagercreator.h"
#include "multiinstances/imultiinstancesprovider.h"
#include "iinteractive.h"

namespace mu::cloud {
class OAuthHttpServerReplyHandler;

class CloudService : public QObject, public IAuthorizationService, public ICloudProjectsService, public async::Asyncable
{
    Q_OBJECT

    INJECT(cloud, ICloudConfiguration, configuration)
    INJECT(cloud, io::IFileSystem, fileSystem)
    INJECT(cloud, network::INetworkManagerCreator, networkManagerCreator)
    INJECT(cloud, framework::IInteractive, interactive)
    INJECT(cloud, mi::IMultiInstancesProvider, multiInstancesProvider)

public:
    CloudService(QObject* parent = nullptr);

    void init();

    void signUp() override;
    void signIn() override;
    void signOut() override;

    Ret ensureAuthorization(const std::string& text = {}) override;

    ValCh<bool> userAuthorized() const override;
    ValCh<AccountInfo> accountInfo() const override;

    Ret checkCloudIsAvailable() const override;

    framework::ProgressPtr uploadScore(QIODevice& scoreData, const QString& title, Visibility visibility = Visibility::Private,
                                       const QUrl& sourceUrl = QUrl()) override;
    framework::ProgressPtr uploadAudio(QIODevice& audioData, const QString& audioFormat, const QUrl& sourceUrl) override;

    RetVal<ScoreInfo> downloadScoreInfo(const QUrl& sourceUrl) override;

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

    RetVal<QUrl> prepareUrlForRequest(QUrl apiUrl, const QVariantMap& params = QVariantMap()) const;
    network::RequestHeaders headers() const;

    Ret downloadAccountInfo();
    RetVal<ScoreInfo> downloadScoreInfo(int scoreId);

    mu::RetVal<mu::ValMap> doUploadScore(network::INetworkManagerPtr uploadManager, QIODevice& scoreData, const QString& title,
                                         Visibility visibility, const QUrl& sourceUrl = QUrl());
    Ret doUploadAudio(network::INetworkManagerPtr uploadManager, QIODevice& audioData, const QString& audioFormat, const QUrl& sourceUrl);

    using RequestCallback = std::function<Ret()>;
    void executeRequest(const RequestCallback& requestCallback);

    QOAuth2AuthorizationCodeFlow* m_oauth2 = nullptr;
    OAuthHttpServerReplyHandler* m_replyHandler = nullptr;

    ValCh<bool> m_userAuthorized;
    ValCh<AccountInfo> m_accountInfo;

    QString m_accessToken;
    QString m_refreshToken;

    OnUserAuthorizedCallback m_onUserAuthorizedCallback;
};
}

#endif // MU_CLOUD_CLOUDSERVICE_H
