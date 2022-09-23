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

#include "accountmodel.h"

#include "iauthorizationservice.h"

#include "log.h"

using namespace mu::cloud;

namespace {
const QString USER_NAME("userName");
const QString PROFILE_URL("profileUrl");
const QString AVATAR_URL("avatarUrl");
const QString SHEETMUSIC_URL("sheetmusicUrl");
}

AccountModel::AccountModel(QObject* parent)
    : QObject(parent)
{
}

void AccountModel::load()
{
    ValCh<AccountInfo> infoCh = authorizationService()->accountInfo();
    setAccountInfo(infoCh.val);

    infoCh.ch.onReceive(this, [this](const AccountInfo& info) {
        setAccountInfo(info);
    });

    ValCh<bool> userAuthorizedCh = authorizationService()->userAuthorized();
    setUserAuthorized(userAuthorizedCh.val);

    userAuthorizedCh.ch.onReceive(this, [this](bool authorized) {
        setUserAuthorized(authorized);
    });
}

void AccountModel::setUserAuthorized(bool authorized)
{
    if (m_userAuthorized == authorized) {
        return;
    }

    m_userAuthorized = authorized;
    emit userAuthorizedChanged();
}

void AccountModel::setAccountInfo(const AccountInfo& info)
{
    if (m_accountInfo == info) {
        return;
    }

    m_accountInfo = info;
    emit accountInfoChanged();
}

void AccountModel::createAccount()
{
    authorizationService()->signUp();
}

void AccountModel::signIn()
{
    authorizationService()->signIn();
}

void AccountModel::signOut()
{
    authorizationService()->signOut();
}

bool AccountModel::userAuthorized() const
{
    return m_userAuthorized;
}

QVariant AccountModel::accountInfo() const
{
    QVariantMap accountInfo;

    accountInfo[USER_NAME] = m_accountInfo.userName;
    accountInfo[PROFILE_URL] = m_accountInfo.profileUrl;
    accountInfo[AVATAR_URL] = m_accountInfo.avatarUrl;
    accountInfo[SHEETMUSIC_URL] = m_accountInfo.sheetmusicUrl;

    return accountInfo;
}
