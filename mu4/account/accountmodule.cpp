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
#include "accountmodule.h"

#include "modularity/ioc.h"

#include "controllers/accountcontroller.h"
#include "models/accountmodel.h"

using namespace mu::account;

std::string AccountModule::moduleName() const
{
    return "account";
}

void AccountModule::registerExports()
{
    framework::ioc()->registerExport<IAccountController>(moduleName(), new AccountController());
}

static void account_init_qrc()
{
    Q_INIT_RESOURCE(account);
}

void AccountModule::registerResources()
{
    account_init_qrc();
}

void AccountModule::registerUiTypes()
{
    qmlRegisterType<AccountModel>("MuseScore.Account", 1, 0, "AccountModel");
}
