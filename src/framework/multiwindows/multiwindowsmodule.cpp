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

#ifdef MUSE_MULTICONTEXT_WIP
#include "internal/singleprocess/singleprocessprovider.h"
#else
#include "internal/multiprocess/multiprocessprovider.h"
#endif

using namespace muse::mi;
using namespace muse::modularity;

std::string MultiInstancesModule::moduleName() const
{
    return "multiwindows";
}

void MultiInstancesModule::registerExports()
{
#ifdef MUSE_MULTICONTEXT_WIP
    m_windowsProvider = std::make_shared<SingleProcessProvider>();
#else
    m_windowsProvider = std::make_shared<MultiProcessProvider>(globalCtx());
    globalIoc()->registerExport<IMultiProcessProvider>(moduleName(), m_windowsProvider);
#endif

    globalIoc()->registerExport<IMultiWindowsProvider>(moduleName(), m_windowsProvider);
}

void MultiInstancesModule::resolveImports()
{
    auto ir = globalIoc()->resolve<muse::interactive::IInteractiveUriRegister>(moduleName());
    if (ir) {
        ir->registerQmlUri(Uri("muse://devtools/multiwindows/info"), "Muse.MultiWindows", "MultiInstancesDevDialog");
    }

    auto ar = globalIoc()->resolve<muse::ui::IUiActionsRegister>(moduleName());
    if (ar) {
        ar->reg(std::make_shared<MultiInstancesUiActions>());
    }
}

void MultiInstancesModule::onPreInit(const IApplication::RunMode& mode)
{
    if (mode != IApplication::RunMode::GuiApp) {
        return;
    }

    m_windowsProvider->init();
}
