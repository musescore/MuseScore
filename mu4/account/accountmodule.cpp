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
#include "internal/mu4loginmanageradapter.h"

using namespace mu::account;

static void account_init_qrc()
{
    Q_INIT_RESOURCE(account);
}

std::string AccountModule::moduleName() const
{
    return "account";
}

void AccountModule::registerExports()
{
    framework::ioc()->registerExport<IAccountController>(moduleName(), AccountController::instance());

#ifdef BUILD_UI_MU4
    framework::ioc()->registerExport<IPaletteAdapter>(moduleName(), new MU4LoginManagerAdapter());
#endif
}

void AccountModule::registerResources()
{
    account_init_qrc();
}

void AccountModule::registerUiTypes()
{
    qmlRegisterType<AccountModel>("MuseScore.Account", 1, 0, "AccountModel");
}

void AccountModule::onInit()
{
    AccountController::instance()->init();
}
