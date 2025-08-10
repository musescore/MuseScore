/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2021 MuseScore Limited
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

#include "appshellmodule.h"

#include <QQmlEngine>

#include "modularity/ioc.h"

#include "ui/iuiactionsregister.h"
#include "ui/iinteractiveuriregister.h"

#include "internal/applicationuiactions.h"
#include "internal/applicationactioncontroller.h"
#include "internal/appshellconfiguration.h"
#include "internal/applicationactioncontroller.h"
#include "internal/startupscenario.h"

#include "view/notationpagemodel.h"
#include "view/notationstatusbarmodel.h"
#include "view/navigableappmenumodel.h"

using namespace mu::appshell;
using namespace muse;
using namespace muse::modularity;
using namespace muse::ui;
using namespace muse::dock;

static void appshell_init_qrc()
{
    Q_INIT_RESOURCE(appshell);
}

std::string AppShellModule::moduleName() const
{
    return "appshell";
}

void AppShellModule::registerExports()
{
    m_applicationActionController = std::make_shared<ApplicationActionController>(iocContext());
    m_applicationUiActions = std::make_shared<ApplicationUiActions>(m_applicationActionController, iocContext());
    m_appShellConfiguration = std::make_shared<AppShellConfiguration>(iocContext());

    ioc()->registerExport<IAppShellConfiguration>(moduleName(), m_appShellConfiguration);
    ioc()->registerExport<IStartupScenario>(moduleName(), new StartupScenario(iocContext()));
}

void AppShellModule::resolveImports()
{
    auto ar = ioc()->resolve<muse::ui::IUiActionsRegister>(moduleName());
    if (ar) {
        ar->reg(m_applicationUiActions);
    }

    auto ir = ioc()->resolve<IInteractiveUriRegister>(moduleName());
    if (ir) {
        ir->registerUri(Uri("musescore://notation"), ContainerMeta(ContainerType::PrimaryPage));
        ir->registerUri(Uri("musescore://devtools"), ContainerMeta(ContainerType::PrimaryPage));
    }
}

void AppShellModule::registerResources()
{
    appshell_init_qrc();
}

void AppShellModule::registerUiTypes()
{
    qmlRegisterType<NavigableAppMenuModel>("MuseScore.AppShell", 1, 0, "AppMenuModel");
    qmlRegisterType<NotationPageModel>("MuseScore.AppShell", 1, 0, "NotationPageModel");
    qmlRegisterType<NotationStatusBarModel>("MuseScore.AppShell", 1, 0, "NotationStatusBarModel");
}

void AppShellModule::onPreInit(const IApplication::RunMode& mode)
{
    if (mode == IApplication::RunMode::AudioPluginRegistration) {
        return;
    }

    m_applicationActionController->preInit();
}

void AppShellModule::onInit(const IApplication::RunMode& mode)
{
    if (mode == IApplication::RunMode::AudioPluginRegistration) {
        return;
    }

    m_appShellConfiguration->init();
    m_applicationActionController->init();
    m_applicationUiActions->init();
}

void AppShellModule::onAllInited(const IApplication::RunMode& mode)
{
    if (mode == IApplication::RunMode::AudioPluginRegistration) {
        return;
    }
}

void AppShellModule::onDeinit()
{
}
