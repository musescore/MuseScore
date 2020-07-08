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

#include "interfaces/iaccountcontroller.h"

using namespace mu::account;

namespace {
const QString USER_NAME("userName");
const QString AVATAR_URL("avatarUrl");
}

AccountModel::AccountModel(QObject *parent):
    QObject(parent)
{

}

void AccountModel::load()
{
    ValCh<AccountInfo> info = accountController()->accountInfo();

    info.ch.onReceive(this, [this](const AccountInfo& info) {
        m_accountInfo = info;
        emit accountInfoChanged();
    });
}

void AccountModel::logIn()
{
    accountController()->logIn();
}

void AccountModel::logOut()
{
    accountController()->logOut();
}

QVariant AccountModel::accountInfo() const
{
    QVariantMap accountInfo;

    accountInfo[USER_NAME] = m_accountInfo.userName;
    accountInfo[AVATAR_URL] = m_accountInfo.avatarUrl;

    return accountInfo;
}
