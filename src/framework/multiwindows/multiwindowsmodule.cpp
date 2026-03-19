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

#include "multiwindowsmodule.h"

#include "internal/multiwindowsuiactions.h"

#include "modularity/ioc.h"
#include "interactive/iinteractiveuriregister.h"
#include "ui/iuiactionsregister.h"

#include "muse_framework_config.h"

#ifdef MUSE_MODULE_MULTIWINDOWS_SINGLEPROC_MODE
#include "internal/singleprocess/singleprocessprovider.h"
#else
#include "internal/multiprocess/multiprocessprovider.h"
#endif

using namespace muse::mi;
using namespace muse::modularity;

static const std::string mname("multiwindows");

std::string MultiWindowsModule::moduleName() const
{
    return mname;
}

void MultiWindowsModule::registerExports()
{
#ifdef MUSE_MODULE_MULTIWINDOWS_SINGLEPROC_MODE
    m_windowsProvider = std::make_shared<SingleProcessProvider>();
#else
    m_windowsProvider = std::make_shared<MultiProcessProvider>();
    globalIoc()->registerExport<IMultiProcessProvider>(mname, m_windowsProvider);
#endif

    globalIoc()->registerExport<IMultiWindowsProvider>(mname, m_windowsProvider);
}

void MultiWindowsModule::resolveImports()
{
    auto ir = globalIoc()->resolve<muse::interactive::IInteractiveUriRegister>(mname);
    if (ir) {
        ir->registerQmlUri(Uri("muse://devtools/multiwindows/info"), "Muse.MultiWindows", "MultiInstancesDevDialog");
    }
}

void MultiWindowsModule::onPreInit(const IApplication::RunMode& mode)
{
    if (mode != IApplication::RunMode::GuiApp) {
        return;
    }

    m_windowsProvider->initOnGlobal();
}

IContextSetup* MultiWindowsModule::newContext(const muse::modularity::ContextPtr& ctx) const
{
    return new MultiWindowsContext(ctx);
}

void MultiWindowsContext::resolveImports()
{
    auto ar = ioc()->resolve<muse::ui::IUiActionsRegister>(mname);
    if (ar) {
        ar->reg(std::make_shared<MultiInstancesUiActions>());
    }
}

void MultiWindowsContext::onInit(const IApplication::RunMode& mode)
{
    if (mode != IApplication::RunMode::GuiApp) {
        return;
    }

#ifndef MUSE_MODULE_MULTIWINDOWS_SINGLEPROC_MODE
    //! NOTE The MultiProcessProvider itself must be global.
    //! But it formally uses contextual services
    //! But in this case we always have one context
    //! We need to think about how to do this better.
    auto windowsProvider = modularity::globalIoc()->resolve<IMultiWindowsProvider>(mname);
    if (windowsProvider) {
        auto impl = std::dynamic_pointer_cast<MultiProcessProvider>(windowsProvider);
        if (impl) {
            DO_ASSERT(impl->iocContext() == nullptr); // should be just one context
            impl->setContext(iocContext());
            impl->initOnContext();
        }
    }
#endif
}
