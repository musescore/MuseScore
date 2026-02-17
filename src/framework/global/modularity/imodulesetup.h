/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-CLA-applies
 *
 * MuseScore
 * Music Composition & Notation
 *
 * Copyright (C) 2024 MuseScore Limited and others
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

#include "ioc.h"
#include "../iapplication.h"

namespace muse::modularity {
class IContextSetup
{
public:

    IContextSetup(const modularity::ContextPtr& ctx)
        : m_ctx(ctx) {}

    virtual ~IContextSetup() = default;

    const modularity::ContextPtr iocContext() const { return m_ctx; }
    ModulesIoC* ioc() const { return modularity::ioc(iocContext()); }

    virtual void registerExports() {}
    virtual void resolveImports() {}
    virtual void onPreInit(const IApplication::RunMode& mode) { (void)mode; }
    virtual void onInit(const IApplication::RunMode& mode) { (void)mode; }
    virtual void onAllInited(const IApplication::RunMode& mode) { (void)mode; }
    virtual void onDeinit() {}

private:
    ContextPtr m_ctx;
};

class IModuleSetup
{
public:

    virtual ~IModuleSetup() {}

    virtual std::string moduleName() const = 0;

    ModulesIoC* globalIoc() const { return muse::modularity::globalIoc(); }

    virtual void registerExports() {}
    virtual void resolveImports() {}

    virtual void registerResources() {}
    virtual void registerUiTypes() {}
    virtual void registerApi() {}

    virtual void onPreInit(const IApplication::RunMode& mode) { (void)mode; }
    virtual void onInit(const IApplication::RunMode& mode) { (void)mode; }
    virtual void onAllInited(const IApplication::RunMode& mode) { (void)mode; }
    virtual void onDelayedInit() {}
    virtual void onDeinit() {}
    virtual void onDestroy() {}

    virtual void onStartApp() {}

    // Context
    virtual IContextSetup* newContext(const muse::modularity::ContextPtr& ctx) const { (void)ctx; return nullptr; }

    // to remove
    void setApplication(std::shared_ptr<IApplication> app)
    {
        m_application = app;
    }

    std::shared_ptr<IApplication> application() const { return m_application; }

protected:
    std::shared_ptr<IApplication> m_application;
};
}

#endif // MU_MODULARITY_IMODULESETUP_H
