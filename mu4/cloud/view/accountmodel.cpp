//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2020 MuseScore BVBA and others
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
//=============================================================================

#include "accountmodel.h"

#include "iaccountcontroller.h"

using namespace mu::cloud;

namespace {
const QString USER_NAME("userName");
const QString AVATAR_URL("avatarUrl");
}

AccountModel::AccountModel(QObject *parent)
    : QObject(parent)
{

}

void AccountModel::load()
{
    ValCh<AccountInfo> infoCh = accountController()->accountInfo();

    infoCh.ch.onReceive(this, [this](const AccountInfo& info) {
        m_accountInfo = info;
        emit accountInfoChanged();
    });
}

void AccountModel::createAccount()
{
    accountController()->createAccount();
}

void AccountModel::signIn()
{
    accountController()->signIn();
}

void AccountModel::signOut()
{
    accountController()->signOut();
}

QVariant AccountModel::accountInfo() const
{
    QVariantMap accountInfo;

    accountInfo[USER_NAME] = m_accountInfo.userName;
    accountInfo[AVATAR_URL] = m_accountInfo.avatarUrl;

    return accountInfo;
}
