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
#ifndef MU_ACCOUNT_ACCOUNTCONTROLLER_H
#define MU_ACCOUNT_ACCOUNTCONTROLLER_H

#include <QObject>

#include "iaccountcontroller.h"

namespace Ms {
class CloudManager;
}

namespace mu {
namespace account {
class AccountController : public QObject, public IAccountController
{
    Q_OBJECT

public:
    static AccountController* instance();

    void init();

    void logIn() override;
    void logOut() override;

    ValCh<AccountInfo> accountInfo() const override;

private slots:
    void updateAccountInfo();

private:
    AccountController();

    Ms::CloudManager* m_cloudManager = nullptr;
    ValCh<AccountInfo> m_accountInfo;
};
}
}

#endif MU_ACCOUNT_ACCOUNTCONTROLLER_H
