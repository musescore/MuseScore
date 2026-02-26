/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-CLA-applies
 *
 * MuseScore
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

#include "environment.h"

#include "global/globalmodule.h"
#include "global/iapplication.h"

using namespace muse::testing;

Environment::Modules Environment::m_dependencyModules;
Environment::PreInit Environment::m_preInit;
Environment::PostInit Environment::m_postInit;
Environment::PostInit Environment::m_deInit;
muse::GlobalModule* Environment::m_globalModule = nullptr;

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

void Environment::setDeInit(const DeInit& deInit)
{
    m_deInit = deInit;
}

void Environment::setup()
{
    IApplication::RunMode runMode = IApplication::RunMode::GuiApp;

    m_globalModule = new GlobalModule();
    m_globalModule->registerResources();
    m_globalModule->registerExports();
    m_globalModule->registerUiTypes();

    for (modularity::IModuleSetup* m : m_dependencyModules) {
        m->setApplication(m_globalModule->application());
        m->registerResources();
    }

    for (modularity::IModuleSetup* m : m_dependencyModules) {
        m->registerExports();
    }

    m_globalModule->resolveImports();
    for (modularity::IModuleSetup* m : m_dependencyModules) {
        m->registerUiTypes();
        m->resolveImports();
    }

    m_globalModule->onPreInit(runMode);
    //! NOTE Now we can use logger and profiler

    for (modularity::IModuleSetup* m : m_dependencyModules) {
        m->onPreInit(runMode);
    }

    if (m_preInit) {
        m_preInit();
    }

    m_globalModule->onInit(runMode);
    for (modularity::IModuleSetup* m : m_dependencyModules) {
        m->onInit(runMode);
    }

    m_globalModule->onAllInited(runMode);
    for (modularity::IModuleSetup* m : m_dependencyModules) {
        m->onAllInited(runMode);
    }

    m_globalModule->onStartApp();
    for (modularity::IModuleSetup* m : m_dependencyModules) {
        m->onStartApp();
    }

    if (m_postInit) {
        m_postInit();
    }
}

void Environment::deinit()
{
    if (m_deInit) {
        m_deInit();
    }

    for (modularity::IModuleSetup* m : m_dependencyModules) {
        m->onDeinit();
    }

    m_globalModule->onDeinit();

    for (modularity::IModuleSetup* m : m_dependencyModules) {
        m->onDestroy();
    }

    m_globalModule->onDestroy();

    qDeleteAll(m_dependencyModules);
    m_dependencyModules.clear();

    delete m_globalModule;
    m_globalModule = nullptr;

    muse::modularity::resetAll();
}
