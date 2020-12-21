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

#include "environment.h"

#include "framework/global/globalmodule.h"

using namespace mu::testing;

Environment::Modules Environment::m_dependencyModules;
Environment::PostInit Environment::m_postInit;

void Environment::setDependency(const Modules& modules)
{
    m_dependencyModules = modules;
}

void Environment::setPostInit(const PostInit& postInit)
{
    m_postInit = postInit;
}

void Environment::setup()
{
    static mu::framework::GlobalModule globalModule;

    framework::IApplication::RunMode runMode = framework::IApplication::RunMode::Editor;

    globalModule.registerResources();
    globalModule.registerExports();
    globalModule.registerUiTypes();
    globalModule.onInit(runMode);

    //! NOTE Now we can use logger and profiler

    for (mu::framework::IModuleSetup* m : m_dependencyModules) {
        m->registerResources();
    }

    for (mu::framework::IModuleSetup* m : m_dependencyModules) {
        m->registerExports();
    }

    globalModule.resolveImports();
    for (mu::framework::IModuleSetup* m : m_dependencyModules) {
        m->registerUiTypes();
        m->resolveImports();
    }

    for (mu::framework::IModuleSetup* m : m_dependencyModules) {
        m->onInit(runMode);
    }

    globalModule.onStartApp();
    for (mu::framework::IModuleSetup* m : m_dependencyModules) {
        m->onStartApp();
    }

    if (m_postInit) {
        m_postInit();
    }
}
