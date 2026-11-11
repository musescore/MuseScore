/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2021 MuseScore Limited and others
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

#include <qqml.h>

#include "modularity/ioc.h"

#include "interactive/iinteractiveuriregister.h"

#include "appshell/internal/appshellconfiguration.h"
#include "internal/startupscenario.h"

#include "view/navigableappmenumodel.h"

using namespace mu::appshell;
using namespace muse;
using namespace muse::modularity;
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
    m_appShellConfiguration = std::make_shared<AppShellConfiguration>(globalCtx());

    globalIoc()->registerExport<IAppShellConfiguration>(moduleName(), m_appShellConfiguration);
    globalIoc()->registerExport<IStartupScenario>(moduleName(), new StartupScenario(globalCtx()));
}

void AppShellModule::resolveImports()
{
    auto ir = globalIoc()->resolve<interactive::IInteractiveUriRegister>(moduleName());
    if (ir) {
        ir->registerPageUri(Uri("musescore://notation"));
        ir->registerPageUri(Uri("musescore://devtools"));
    }
}

void AppShellModule::registerResources()
{
    appshell_init_qrc();
}

void AppShellModule::registerUiTypes()
{
    qmlRegisterType<NavigableAppMenuModel>("MuseScore.AppShell", 1, 0, "AppMenuModel");
}

void AppShellModule::onPreInit(const IApplication::RunMode& mode)
{
    if (mode == IApplication::RunMode::AudioPluginRegistration) {
        return;
    }
}

void AppShellModule::onInit(const IApplication::RunMode& mode)
{
    if (mode == IApplication::RunMode::AudioPluginRegistration) {
        return;
    }

    m_appShellConfiguration->init();
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
