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

#include "internal/cloudmanager.h"

using namespace mu::cloud;

AccountController* AccountController::instance()
{
    static AccountController controller;
    return &controller;
}

AccountController::AccountController()
    : m_cloudManager(new Ms::CloudManager(this))
{
}

void AccountController::init()
{
    connect(m_cloudManager, &Ms::CloudManager::getUserSuccess, this, &AccountController::updateAccountInfo);

    m_cloudManager->getUser();
}

void AccountController::logIn()
{
    m_cloudManager->tryLogin();
}

void AccountController::logOut()
{
    m_cloudManager->logout();
}

mu::ValCh<AccountInfo> AccountController::accountInfo() const
{
    return m_accountInfo;
}

void AccountController::updateAccountInfo()
{
    AccountInfo newAccountInfo;

    newAccountInfo.userName = m_cloudManager->userName();
    newAccountInfo.avatarUrl = m_cloudManager->avatar();

    m_accountInfo.set(newAccountInfo);
}
