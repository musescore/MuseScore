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

#include "environment.h"

#include "framework/global/globalmodule.h"

using namespace mu::testing;

Environment::Modules Environment::m_dependencyModules;
Environment::PreInit Environment::m_preInit;
Environment::PostInit Environment::m_postInit;

void Environment::setDependency(const Modules& modules)
{
    m_dependencyModules = modules;
}

void Environment::setPreInit(const PreInit& preInit)
{
    m_preInit = preInit;
}

void Environment::setPostInit(const PostInit& postInit)
{
    m_postInit = postInit;
}

void Environment::setup()
{
    static mu::framework::GlobalModule globalModule;

    framework::IApplication::RunMode runMode = framework::IApplication::RunMode::GuiApp;

    globalModule.registerResources();
    globalModule.registerExports();
    globalModule.registerUiTypes();

    for (mu::modularity::IModuleSetup* m : m_dependencyModules) {
        m->registerResources();
    }

    for (mu::modularity::IModuleSetup* m : m_dependencyModules) {
        m->registerExports();
    }

    globalModule.resolveImports();
    for (mu::modularity::IModuleSetup* m : m_dependencyModules) {
        m->registerUiTypes();
        m->resolveImports();
    }

    globalModule.onPreInit(runMode);
    //! NOTE Now we can use logger and profiler

    for (mu::modularity::IModuleSetup* m : m_dependencyModules) {
        m->onPreInit(runMode);
    }

    if (m_preInit) {
        m_preInit();
    }

    globalModule.onInit(runMode);
    for (mu::modularity::IModuleSetup* m : m_dependencyModules) {
        m->onInit(runMode);
    }

    globalModule.onAllInited(runMode);
    for (mu::modularity::IModuleSetup* m : m_dependencyModules) {
        m->onAllInited(runMode);
    }

    globalModule.onStartApp();
    for (mu::modularity::IModuleSetup* m : m_dependencyModules) {
        m->onStartApp();
    }

    if (m_postInit) {
        m_postInit();
    }
}
