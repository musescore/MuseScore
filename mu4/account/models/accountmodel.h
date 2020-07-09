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
#ifndef MU_ACCOUNT_ACCOUNTMODEL_H
#define MU_ACCOUNT_ACCOUNTMODEL_H

#include <QObject>

#include "modularity/ioc.h"
#include "controllers/iaccountcontroller.h"
#include "async/asyncable.h"

namespace mu {
namespace account {
class AccountModel : public QObject, async::Asyncable
{
    Q_OBJECT

    INJECT(account, IAccountController, accountController)

    Q_PROPERTY(QVariant accountInfo READ accountInfo NOTIFY accountInfoChanged)

public:
    explicit AccountModel(QObject *parent = nullptr);

    QVariant accountInfo() const;

    Q_INVOKABLE void load();
    Q_INVOKABLE void logIn();
    Q_INVOKABLE void logOut();

signals:
    void accountInfoChanged();

private:
    AccountInfo m_accountInfo;
};
}
}

#endif // MU_ACCOUNT_ACCOUNTMODEL_H
