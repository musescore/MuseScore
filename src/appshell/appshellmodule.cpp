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

#include "appshellmodule.h"

#include <QQmlEngine>

#include "modularity/ioc.h"

#include "actions/iactionsregister.h"
#include "ui/iinteractiveuriregister.h"

#include "internal/applicationactions.h"
#include "internal/applicationactioncontroller.h"
#include "internal/appshellconfiguration.h"
#include "internal/notationpagestate.h"
#include "view/dockwindow/docksetup.h"
#include "view/settings/settingslistmodel.h"
#include "view/appmenumodel.h"
#include "view/notationpagemodel.h"

using namespace mu::appshell;
using namespace mu::framework;
using namespace mu::ui;

static std::shared_ptr<ApplicationActionController> s_applicationActionController = std::make_shared<ApplicationActionController>();
static std::shared_ptr<AppShellConfiguration> s_appShellConfiguration = std::make_shared<AppShellConfiguration>();
static std::shared_ptr<NotationPageState> s_notationPageState = std::make_shared<NotationPageState>();

static void appshell_init_qrc()
{
    Q_INIT_RESOURCE(appshell);
}

AppShellModule::AppShellModule()
{
}

std::string AppShellModule::moduleName() const
{
    return "appshell";
}

void AppShellModule::registerExports()
{
    ioc()->registerExport<IAppShellConfiguration>(moduleName(), s_appShellConfiguration);
    ioc()->registerExport<IApplicationActionController>(moduleName(), s_applicationActionController);
    ioc()->registerExport<INotationPageState>(moduleName(), s_notationPageState);
}

void AppShellModule::resolveImports()
{
    auto ar = ioc()->resolve<actions::IActionsRegister>(moduleName());
    if (ar) {
        ar->reg(std::make_shared<ApplicationActions>());
    }

    auto ir = ioc()->resolve<IInteractiveUriRegister>(moduleName());
    if (ir) {
        ir->registerUri(Uri("musescore://home"), ContainerMeta(ContainerType::PrimaryPage));
        ir->registerUri(Uri("musescore://notation"), ContainerMeta(ContainerType::PrimaryPage));
        ir->registerUri(Uri("musescore://sequencer"), ContainerMeta(ContainerType::PrimaryPage));
        ir->registerUri(Uri("musescore://publish"), ContainerMeta(ContainerType::PrimaryPage));
        ir->registerUri(Uri("musescore://devtools"), ContainerMeta(ContainerType::PrimaryPage));
    }
}

void AppShellModule::registerResources()
{
    appshell_init_qrc();
}

void AppShellModule::registerUiTypes()
{
    dock::DockSetup::registerQmlTypes();

    qmlRegisterType<SettingListModel>("MuseScore.Settings", 1, 0, "SettingListModel");
    qmlRegisterType<AppMenuModel>("MuseScore.AppMenu", 1, 0, "AppMenuModel");
    qmlRegisterType<NotationPageModel>("MuseScore.PageState", 1, 0, "NotationPageModel");
}

void AppShellModule::onInit(const IApplication::RunMode&)
{
    s_appShellConfiguration->init();
    s_applicationActionController->init();
    s_notationPageState->init();
}
