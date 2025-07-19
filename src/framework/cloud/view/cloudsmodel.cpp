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

using namespace muse::cloud;

static constexpr int INVALID_INDEX = -1;

namespace prv {
const QString CLOUD_CODE("cloudCode");
const QString CLOUD_TITLE("cloudTitle");
const QString USER_IS_AUTHORIZED("userIsAuthorized");
const QString USER_NAME("userName");
const QString USER_PROFILE_URL("userProfileUrl");
const QString USER_AVATAR_URL("userAvatarUrl");
const QString USER_COLLECTION_URL("userCollectionUrl");
const QString CLOUD_LOGO_URL("cloudLogoUrl");
const QString CLOUD_LOGO_COLOR("cloudLogoColor");

const QString DIALOG_TITLE_TEXT("titleText");
const QString REPLACE_BUTTON_TEXT("replaceButtonText");
const QString NEW_BUTTON_TEXT("newButtonText");
const QString SAVE_BUTTON_TEXT("saveButtonText");
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
    : QAbstractListModel(parent), Injectable(muse::iocCtxForQmlObject(this))
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

    QVariantMap cloudInfoMap = makeCloudInfoMap(cloudTitle, accountInfo);

    cloudInfoMap[prv::CLOUD_LOGO_URL] = m_clouds[index]->cloudInfo().logoUrl;
    cloudInfoMap[prv::CLOUD_LOGO_COLOR] = m_clouds[index]->cloudInfo().logoColor;

    return cloudInfoMap;
}

QVariantList CloudsModel::visibilityModel(const QString& cloudCode) const
{
    QVariantList visibilityTypes;

    QVariantMap publicVisibility;
    publicVisibility.insert("value", int(Visibility::Public));
    publicVisibility.insert("text", muse::qtrc("project/save", "Public"));
    visibilityTypes.append(publicVisibility);

    QVariantMap unlistedVisibility;
    unlistedVisibility.insert("value", int(Visibility::Unlisted));
    unlistedVisibility.insert("text", muse::qtrc("project/save", "Unlisted"));
    visibilityTypes.append(unlistedVisibility);

    if (cloudCode == cloud::MUSESCORE_COM_CLOUD_CODE) {
        QVariantMap privateVisibility;
        privateVisibility.insert("value", int(Visibility::Private));
        privateVisibility.insert("text", muse::qtrc("project/save", "Private"));
        visibilityTypes.append(privateVisibility);
    }

    return visibilityTypes;
}

QVariant CloudsModel::dialogText(const QString& cloudCode, const QString& existingScoreOrAudioUrl) const
{
    QVariantMap dialogTextMap;

    if (cloudCode == cloud::MUSESCORE_COM_CLOUD_CODE) {
        dialogTextMap[prv::DIALOG_TITLE_TEXT] = muse::qtrc("project/save", "Publish to MuseScore.com");

        if (!existingScoreOrAudioUrl.isEmpty()) {
            //: The text between `<a href=\"%1\">` and `</a>` will be a clickable link to the online score in question
            dialogTextMap[prv::REPLACE_BUTTON_TEXT] = muse::qtrc("project/save", "Replace the existing <a href=\"%1\">online score</a>")
                                                      .arg(existingScoreOrAudioUrl);

            dialogTextMap[prv::NEW_BUTTON_TEXT] = muse::qtrc("project/save", "Publish as new online score");
        }

        dialogTextMap[prv::SAVE_BUTTON_TEXT] = muse::qtrc("project/save", "Publish");

        return dialogTextMap;
    } else if (cloudCode == cloud::AUDIO_COM_CLOUD_CODE) {
        dialogTextMap[prv::DIALOG_TITLE_TEXT] = muse::qtrc("project/save", "Share on Audio.com");

        if (!existingScoreOrAudioUrl.isEmpty()) {
            //: The text between `<a href=\"%1\">` and `</a>` will be a clickable link to the online audio in question
            dialogTextMap[prv::REPLACE_BUTTON_TEXT] = muse::qtrc("project/save", "Replace the <a href=\"%1\">existing audio</a>")
                                                      .arg(existingScoreOrAudioUrl);

            dialogTextMap[prv::NEW_BUTTON_TEXT] = muse::qtrc("project/save", "Upload as new audio file");
        }

        dialogTextMap[prv::SAVE_BUTTON_TEXT] = muse::qtrc("project/save", "Share");

        return dialogTextMap;
    }

    LOGE() << "Unknown cloud code: " + cloudCode;

    return dialogTextMap;
}

void CloudsModel::load()
{
    beginResetModel();
    m_clouds.clear();

    m_clouds = {
#ifdef MUSE_MODULE_CLOUD_MUSESCORECOM
        museScoreComService()->authorization(),
#endif
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
