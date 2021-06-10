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

#include "modularity/ioc.h"
#include "icloudconfiguration.h"
#include "system/ifilesystem.h"
#include "network/inetworkmanagercreator.h"

namespace mu::cloud {
class CloudService : public QObject, public IAuthorizationService, public IUploadingService
{
    Q_OBJECT

    INJECT(cloud, ICloudConfiguration, configuration)
    INJECT(cloud, system::IFileSystem, fileSystem)
    INJECT(cloud, network::INetworkManagerCreator, networkManagerCreator)

public:
    CloudService(QObject* parent = nullptr);

    void init();

    void signIn() override;
    void signOut() override;

    ValCh<bool> userAuthorized() const override;
    ValCh<AccountInfo> accountInfo() const override;

    framework::ProgressChannel uploadScore(notation::INotation& notation) override;

private slots:
    void onUserAuthorized();

private:
    bool readTokens();
    bool saveTokens();
    bool updateTokens();
    void clearTokens();

    enum class RequestStatus {
        Ok,
        UserUnauthorized,
        Error
    };

    QUrl prepareUrlForRequest(QUrl apiUrl) const;

    RequestStatus downloadUserInfo();

    void setAccountInfo(const AccountInfo& info);

    QOAuth2AuthorizationCodeFlow* m_oauth2 = nullptr;
    QOAuthHttpServerReplyHandler* m_replyHandler = nullptr;
    network::INetworkManagerPtr m_networkManager;

    ValCh<bool> m_userAuthorized;
    ValCh<AccountInfo> m_accountInfo;

    QString m_accessToken;
    QString m_refreshToken;
};
}

#endif // MU_CLOUD_CLOUDSERVICE_H
