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

#include "multiinstancesmodule.h"

#include "internal/multiinstancesuiactions.h"
#include "internal/multiinstancesprovider.h"

#include "modularity/ioc.h"
#include "ui/iinteractiveuriregister.h"
#include "ui/iuiactionsregister.h"

using namespace muse::mi;
using namespace muse::modularity;

std::string MultiInstancesModule::moduleName() const
{
    return "multiinstances";
}

void MultiInstancesModule::registerExports()
{
    m_multiInstancesProvider = std::make_shared<MultiInstancesProvider>(iocContext());

    ioc()->registerExport<IMultiInstancesProvider>(moduleName(), m_multiInstancesProvider);
}

void MultiInstancesModule::resolveImports()
{
    auto ir = ioc()->resolve<muse::ui::IInteractiveUriRegister>(moduleName());
    if (ir) {
        ir->registerQmlUri(Uri("muse://devtools/multiinstances/info"), "Muse.MultiInstances", "MultiInstancesDevDialog");
    }

    auto ar = ioc()->resolve<muse::ui::IUiActionsRegister>(moduleName());
    if (ar) {
        ar->reg(std::make_shared<MultiInstancesUiActions>());
    }
}

void MultiInstancesModule::onPreInit(const IApplication::RunMode& mode)
{
    if (mode != IApplication::RunMode::GuiApp) {
        return;
    }

    m_multiInstancesProvider->init();
}
