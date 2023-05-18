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
#ifndef MU_CLOUD_ACCOUNTMODEL_H
#define MU_CLOUD_ACCOUNTMODEL_H

#include <QObject>
#include <QVariant>

#include "modularity/ioc.h"
#include "async/asyncable.h"

#include "iauthorizationservice.h"

namespace mu::cloud {
class AccountModel : public QObject, async::Asyncable
{
    Q_OBJECT

    INJECT(IAuthorizationService, authorizationService)

    Q_PROPERTY(bool userAuthorized READ userAuthorized NOTIFY userAuthorizedChanged)
    Q_PROPERTY(QVariant accountInfo READ accountInfo NOTIFY accountInfoChanged)

public:
    explicit AccountModel(QObject* parent = nullptr);

    bool userAuthorized() const;
    QVariant accountInfo() const;

    Q_INVOKABLE void load();
    Q_INVOKABLE void createAccount();
    Q_INVOKABLE void signIn();
    Q_INVOKABLE void signOut();

signals:
    void userAuthorizedChanged();
    void accountInfoChanged();

private:
    void setUserAuthorized(bool authorized);
    void setAccountInfo(const AccountInfo& info);

    bool m_userAuthorized = false;
    AccountInfo m_accountInfo;
};
}

#endif // MU_CLOUD_ACCOUNTMODEL_H
