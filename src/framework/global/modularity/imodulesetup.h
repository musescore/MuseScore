/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-CLA-applies
 *
 * MuseScore
 * Music Composition & Notation
 *
 * Copyright (C) 2024 MuseScore BVBA and others
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

#ifndef MU_MODULARITY_IMODULESETUP_H
#define MU_MODULARITY_IMODULESETUP_H

#include <string>

#include "../iapplication.h"

namespace muse::modularity {
// `IModuleSetup` is the class used to register a MuseScore module.
// It is then loaded to the `IApplication` using `app->addModule(new YourModule());` in `appfactory.h`
// TL;DR;
// 
// - override `registerResources` for registering .qrc files
// - override `registerUiTypes` for injecting C++ types into the QML code
// - override `registerExports` to register types that are commonly used in the app (Singleton)
class IModuleSetup
{
public:

    virtual ~IModuleSetup() {}

    // What do we use this module name for?
    //
    // When you want to get the implementation of a type that is registered through `registerExports`,
    // you do `ioc()->resolve<T>("theModuleName");`.
    virtual std::string moduleName() const = 0;

    // Sometimes you want to register a C++ type and
    // access it from multiple parts of the code (like a Singleton pattern).
    //
    // **How to register**: `ioc()->registerExport<IUiEngine>(moduleName(), m_uiEngine);`
    // (where `m_uiEngine` is a `std::shared_ptr` that is previously initialized).
    // **How to access**: `ioc()->resolve<IUiEngine>(moduleName());`.
    virtual void registerExports() {}
    virtual void resolveImports() {}

    // Register .qrc files
    // **Use**: `Q_INIT_RESOURCE(file)`
    // where `file` refers to a file named `file.qrc`
    virtual void registerResources() {}
    // Sometimes you want to create a QtQuick type from C++ code
    // and use it in a .qml file.
    // **Use:** `qmlRegisterType<Type>("Namespace.Import", 1, 0, "Type");`.
    // there is also `qmlRegisterUncreatableType` and `qmlRegisterAnonymousType`
    virtual void registerUiTypes() {}
    virtual void registerApi() {}

    // NOTE ! What do we use these below here for?
    // I understand more or less what they mean but - what are they used for?

    virtual void onPreInit(const IApplication::RunMode& mode) { (void)mode; }
    virtual void onInit(const IApplication::RunMode& mode) { (void)mode; }
    virtual void onAllInited(const IApplication::RunMode& mode) { (void)mode; }
    virtual void onDelayedInit() {}
    virtual void onDeinit() {}
    virtual void onDestroy() {}

    virtual void onStartApp() {}

    void setApplication(std::shared_ptr<IApplication> app)
    {
        m_application = app;
    }

    std::shared_ptr<IApplication> application() const { return m_application; }

    const modularity::ContextPtr iocContext() const { return m_application ? m_application->iocContext() : muse::modularity::globalCtx(); }
    ModulesIoC* ioc() const { return m_application ? m_application->ioc() : muse::modularity::globalIoc(); }

protected:
    std::shared_ptr<IApplication> m_application;
};
}

#endif // MU_MODULARITY_IMODULESETUP_H
