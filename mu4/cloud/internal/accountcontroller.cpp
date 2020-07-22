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
#include "log.h"

using namespace mu::cloud;

AccountController::AccountController()
    : m_cloudManager(new Ms::CloudManager())
{
    QObject::connect(m_cloudManager.data(), &Ms::CloudManager::getUserSuccess, [this]() {
        updateAccountInfo();
    });
}

void AccountController::init()
{
    if (!m_cloudManager->init()) {
        LOGE() << "Error while init";
        return;
    }

    m_cloudManager->getUser();
}

void AccountController::createAccount()
{
    m_cloudManager->createAccount();
}

void AccountController::signIn()
{
    m_cloudManager->tryLogin();
}

void AccountController::signOut()
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
