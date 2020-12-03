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

#include "dockwindow/docksetup.h"
#include "settings/settingslistmodel.h"

#include "modularity/ioc.h"
#include "ui/iinteractiveuriregister.h"

using namespace mu::appshell;
using namespace mu::framework;

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

void AppShellModule::resolveImports()
{
    auto ir = ioc()->resolve<IInteractiveUriRegister>(moduleName());
    if (ir) {
        ir->registerUri(Uri("musescore://home"), ContainerMeta(ContainerType::PrimaryPage));
        ir->registerUri(Uri("musescore://notation"), ContainerMeta(ContainerType::PrimaryPage));
        ir->registerUri(Uri("musescore://sequencer"), ContainerMeta(ContainerType::PrimaryPage));
        ir->registerUri(Uri("musescore://publish"), ContainerMeta(ContainerType::PrimaryPage));
        ir->registerUri(Uri("musescore://settings"), ContainerMeta(ContainerType::PrimaryPage));
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
}
