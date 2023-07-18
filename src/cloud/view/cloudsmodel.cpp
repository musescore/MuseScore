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

#include "cloudsmodel.h"

#include "translation.h"
#include "log.h"

using namespace mu::cloud;

static constexpr int INVALID_INDEX = -1;

namespace prv {
const QString CLOUD_CODE("cloudCode");
const QString CLOUD_TITLE("cloudTitle");
const QString USER_IS_AUTHORIZED("userIsAuthorized");
const QString USER_NAME("userName");
const QString USER_PROFILE_URL("userProfileUrl");
const QString USER_AVATAR_URL("userAvatarUrl");
const QString USER_COLLECTION_URL("userCollectionUrl");
}

static QVariantMap makeCloudInfoMap(const QString& title, const AccountInfo& accountInfo)
{
    QVariantMap cloudInfoMap;

    cloudInfoMap[prv::CLOUD_TITLE] = title;
    cloudInfoMap[prv::USER_NAME] = accountInfo.userName;
    cloudInfoMap[prv::USER_PROFILE_URL] = accountInfo.profileUrl;
    cloudInfoMap[prv::USER_AVATAR_URL] = accountInfo.avatarUrl;
    cloudInfoMap[prv::USER_COLLECTION_URL] = accountInfo.collectionUrl;

    return cloudInfoMap;
}

CloudsModel::CloudsModel(QObject* parent)
    : QAbstractListModel(parent)
{
}

QVariant CloudsModel::data(const QModelIndex& index, int role) const
{
    if (!index.isValid()) {
        return QVariant();
    }

    IAuthorizationServicePtr cloud = m_clouds[index.row()];

    const CloudInfo& cloudInfo = cloud->cloudInfo();
    const AccountInfo& accountInfo = cloud->accountInfo().val;
    const bool isAuthorized = cloud->userAuthorized().val;

    switch (role) {
    case rCloudCode:
        return cloudInfo.code;
    case rCloudTitle:
        return cloudInfo.title;
    case rUserIsAuthorized:
        return isAuthorized;
    case rUserName:
        return accountInfo.userName;
    case rUserProfileUrl:
        return accountInfo.profileUrl;
    case rUserAvatarUrl:
        return accountInfo.avatarUrl;
    case rUserCollectionUrl:
        return accountInfo.profileUrl.host() + accountInfo.profileUrl.path();
    }

    return QVariant();
}

int CloudsModel::rowCount(const QModelIndex&) const
{
    return static_cast<int>(m_clouds.size());
}

QHash<int, QByteArray> CloudsModel::roleNames() const
{
    static const QHash<int, QByteArray> roles = {
        { rCloudCode, prv::CLOUD_CODE.toUtf8() },
        { rCloudTitle, prv::CLOUD_TITLE.toUtf8() },
        { rUserIsAuthorized, prv::USER_IS_AUTHORIZED.toUtf8() },
        { rUserName, prv::USER_NAME.toUtf8() },
        { rUserProfileUrl, prv::USER_PROFILE_URL.toUtf8() },
        { rUserAvatarUrl, prv::USER_AVATAR_URL.toUtf8() },
        { rUserCollectionUrl, prv::USER_COLLECTION_URL.toUtf8() }
    };

    return roles;
}

bool CloudsModel::userAuthorized() const
{
    int index = indexOfFirstAuthorizedCloud();
    return index != INVALID_INDEX;
}

QVariant CloudsModel::firstAuthorizedCloudInfo() const
{
    int index = indexOfFirstAuthorizedCloud();
    if (index == INVALID_INDEX) {
        return QVariant();
    }

    QString cloudTitle = m_clouds[index]->cloudInfo().title;
    AccountInfo accountInfo = m_clouds[index]->accountInfo().val;

    return makeCloudInfoMap(cloudTitle, accountInfo);
}

QVariant CloudsModel::cloudInfo(const QString& cloudCode) const
{
    int index = indexByCode(cloudCode);
    if (index == INVALID_INDEX) {
        return QVariant();
    }

    QString cloudTitle = m_clouds[index]->cloudInfo().title;
    AccountInfo accountInfo = m_clouds[index]->accountInfo().val;

    return makeCloudInfoMap(cloudTitle, accountInfo);
}

void CloudsModel::load()
{
    beginResetModel();
    m_clouds.clear();

    m_clouds = {
        museScoreComService()->authorization(),
        audioComService()->authorization()
    };

    for (const IAuthorizationServicePtr& cloud : m_clouds) {
        QString cloudCode = cloud->cloudInfo().code;
        static const QVector<int> AUTHORIZATION_ROLES
            = { rUserIsAuthorized, rUserName, rUserProfileUrl, rUserAvatarUrl, rUserCollectionUrl };

        ValCh<AccountInfo> infoCh = cloud->accountInfo();
        infoCh.ch.onReceive(this, [this, cloudCode](const AccountInfo&) {
            QModelIndex index = createIndex(indexByCode(cloudCode), 0);
            emit dataChanged(index, index, AUTHORIZATION_ROLES);

            emit userAuthorizedChanged();
        });

        ValCh<bool> userAuthorizedCh = cloud->userAuthorized();
        userAuthorizedCh.ch.onReceive(this, [this, cloudCode](bool) {
            QModelIndex index = createIndex(indexByCode(cloudCode), 0);
            emit dataChanged(index, index, AUTHORIZATION_ROLES);

            emit userAuthorizedChanged();
        });
    }

    emit userAuthorizedChanged();

    endResetModel();
}

int CloudsModel::indexByCode(const QString& code) const
{
    for (size_t i = 0; i < m_clouds.size(); ++i) {
        const IAuthorizationServicePtr cloud = m_clouds[i];
        if (cloud->cloudInfo().code == code) {
            return static_cast<int>(i);
        }
    }

    return INVALID_INDEX;
}

int CloudsModel::indexOfFirstAuthorizedCloud() const
{
    for (size_t i = 0; i < m_clouds.size(); ++i) {
        const IAuthorizationServicePtr cloud = m_clouds[i];
        if (cloud->userAuthorized().val) {
            return static_cast<int>(i);
        }
    }

    return INVALID_INDEX;
}

void CloudsModel::createAccount(const QString& code)
{
    int index = indexByCode(code);
    if (index == INVALID_INDEX) {
        return;
    }

    m_clouds[index]->signUp();
}

void CloudsModel::signIn(const QString& code)
{
    int index = indexByCode(code);
    if (index == INVALID_INDEX) {
        return;
    }

    m_clouds[index]->signIn();
}

void CloudsModel::signOut(const QString& code)
{
    int index = indexByCode(code);
    if (index == INVALID_INDEX) {
        return;
    }

    m_clouds[index]->signOut();
}
