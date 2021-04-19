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
#ifndef MU_CLOUD_ACCOUNTCONTROLLER_H
#define MU_CLOUD_ACCOUNTCONTROLLER_H

#include <QScopedPointer>

#include "iaccountcontroller.h"

namespace Ms {
class CloudManager;
}

namespace mu {
namespace cloud {
class AccountController : public IAccountController
{
public:
    AccountController();

    void init();

    void createAccount() override;
    void signIn() override;
    void signOut() override;

    ValCh<bool> userAuthorized() const override;
    ValCh<AccountInfo> accountInfo() const override;

private:
    void setAccountInfo(const AccountInfo& info);

    QScopedPointer<Ms::CloudManager> m_cloudManager;

    ValCh<bool> m_userAuthorized;
    ValCh<AccountInfo> m_accountInfo;
};
}
}

#endif // MU_CLOUD_ACCOUNTCONTROLLER_H
