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

#include "accountcontroller.h"

#include "internal/loginmanager.h"

using namespace mu::account;

AccountController* AccountController::instance()
{
    static AccountController controller;
    return &controller;
}

AccountController::AccountController():
    m_loginManager(new Ms::LoginManager(this))
{
}

void AccountController::init()
{
    connect(m_loginManager, &Ms::LoginManager::getUserSuccess, this, &AccountController::updateAccountInfo);

    m_loginManager->getUser();
}

void AccountController::logIn()
{
    m_loginManager->tryLogin();
}

void AccountController::logOut()
{
    m_loginManager->logout();
}

mu::ValCh<AccountInfo> AccountController::accountInfo() const
{
    return m_accountInfo;
}

void AccountController::updateAccountInfo()
{
    AccountInfo newAccountInfo;

    newAccountInfo.userName = m_loginManager->userName();
    newAccountInfo.avatarUrl = m_loginManager->avatar();

    m_accountInfo.set(newAccountInfo);
}
