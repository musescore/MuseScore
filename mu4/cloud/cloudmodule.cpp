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
#include "cloudmodule.h"

#include <QQmlEngine>
#include "modularity/ioc.h"
#include "ui/iuiengine.h"

#include "internal/accountcontroller.h"
#include "view/accountmodel.h"

using namespace mu::cloud;

static AccountController* m_accountController = new AccountController();

static void cloud_init_qrc()
{
    Q_INIT_RESOURCE(cloud);
}

std::string CloudModule::moduleName() const
{
    return "cloud";
}

void CloudModule::registerExports()
{
    framework::ioc()->registerExport<IAccountController>(moduleName(), m_accountController);
}

void CloudModule::registerResources()
{
    cloud_init_qrc();
}

void CloudModule::registerUiTypes()
{
    qmlRegisterType<AccountModel>("MuseScore.Cloud", 1, 0, "AccountModel");

    framework::ioc()->resolve<framework::IUiEngine>(moduleName())->addSourceImportPath(cloud_QML_IMPORT);
}

void CloudModule::onInit()
{
    m_accountController->init();
}
