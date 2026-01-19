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
#include <string>

#include "modularity/ioc.h"

#include "ui/iuiactionsregister.h"
#include "interactive/iinteractiveuriregister.h"

#include "internal/applicationuiactions.h"
#include "internal/applicationactioncontroller.h"
#include "internal/appshellconfiguration.h"
#include "internal/startupscenario.h"
#include "internal/applicationactioncontroller.h"
#include "internal/sessionsmanager.h"

#ifdef Q_OS_MAC
#include "internal/platform/macos/macosappmenumodelhook.h"
#else
#include "internal/iappmenumodelhook.h"
#endif

using namespace mu::appshell;
using namespace muse;
using namespace muse::modularity;
using namespace muse::dock;

static const std::string mname("appshell");

std::string AppShellModule::moduleName() const
{
    return mname;
}

void AppShellModule::registerExports()
{
    m_appShellConfiguration = std::make_shared<AppShellConfiguration>(iocContext());
    m_sessionsManager = std::make_shared<SessionsManager>(iocContext());

    ioc()->registerExport<IAppShellConfiguration>(moduleName(), m_appShellConfiguration);
    ioc()->registerExport<IStartupScenario>(moduleName(), new StartupScenario(iocContext()));
    ioc()->registerExport<ISessionsManager>(moduleName(), m_sessionsManager);

#ifdef Q_OS_MAC
    ioc()->registerExport<IAppMenuModelHook>(moduleName(), std::make_shared<MacOSAppMenuModelHook>());
#else
    ioc()->registerExport<IAppMenuModelHook>(moduleName(), std::make_shared<AppMenuModelHookStub>());
#endif
}

void AppShellModule::resolveImports()
{
    auto ir = ioc()->resolve<interactive::IInteractiveUriRegister>(moduleName());
    if (ir) {
        ir->registerPageUri(Uri("musescore://home"));
        ir->registerPageUri(Uri("musescore://notation"));
        ir->registerPageUri(Uri("musescore://sequencer"));
        ir->registerPageUri(Uri("musescore://publish"));
        ir->registerPageUri(Uri("musescore://devtools"));

        ir->registerQmlUri(Uri("musescore://about/musescore"), "MuseScore.AppShell", "AboutDialog");
        ir->registerQmlUri(Uri("musescore://about/musicxml"), "MuseScore.AppShell", "AboutMusicXMLDialog");
        ir->registerQmlUri(Uri("musescore://welcomedialog"), "MuseScore.AppShell", "WelcomeDialog");
        ir->registerQmlUri(Uri("musescore://firstLaunchSetup"), "MuseScore.AppShell", "FirstLaunchSetupDialog");
    }
}

void AppShellModule::onPreInit(const IApplication::RunMode&)
{
}

void AppShellModule::onInit(const IApplication::RunMode&)
{
    m_appShellConfiguration->init();
    m_sessionsManager->init();
}

void AppShellModule::onAllInited(const IApplication::RunMode&)
{
    //! NOTE: process QEvent::FileOpen as early as possible if it was postponed
#ifdef Q_OS_MACOS
    qApp->processEvents();
#endif
}

void AppShellModule::onDeinit()
{
    m_sessionsManager->deinit();
}

// Session
muse::modularity::IContextSetup* AppShellModule::newContext(const muse::modularity::ContextPtr& ctx) const
{
    return new AppShellContext(ctx);
}

void AppShellContext::registerExports()
{
    m_applicationActionController = std::make_shared<ApplicationActionController>(iocContext());
    m_applicationUiActions = std::make_shared<ApplicationUiActions>(m_applicationActionController, iocContext());
}

void AppShellContext::resolveImports()
{
    auto ar = ioc()->resolve<muse::ui::IUiActionsRegister>(mname);
    if (ar) {
        ar->reg(m_applicationUiActions);
    }
}

void AppShellContext::onPreInit(const muse::IApplication::RunMode&)
{
    m_applicationActionController->preInit();
}

void AppShellContext::onInit(const muse::IApplication::RunMode& mode)
{
    if (mode == IApplication::RunMode::GuiApp) {
        m_applicationUiActions->init();
        m_applicationActionController->init();
    }
}
