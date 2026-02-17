/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-CLA-applies
 *
 * MuseScore
 * Music Composition & Notation
 *
 * Copyright (C) 2025 MuseScore Limited and others
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
#include "updatemodule.h"

#include "modularity/ioc.h"

#include "interactive/iinteractiveuriregister.h"
#include "ui/iuiactionsregister.h"

#include "internal/updateconfiguration.h"
#include "internal/updateactioncontroller.h"
#include "internal/updateuiactions.h"

#include "internal/appupdatescenario.h"
#include "internal/appupdateservice.h"

using namespace muse::update;
using namespace muse::modularity;

static const std::string mname("update");

std::string UpdateModule::moduleName() const
{
    return mname;
}

void UpdateModule::registerExports()
{
    m_configuration = std::make_shared<UpdateConfiguration>(globalCtx());

    globalIoc()->registerExport<IUpdateConfiguration>(mname, m_configuration);
}

void UpdateModule::resolveImports()
{
    auto ir = globalIoc()->resolve<interactive::IInteractiveUriRegister>(mname);
    if (ir) {
        ir->registerQmlUri(Uri("muse://update/appreleaseinfo"), "Muse.Update", "AppReleaseInfoDialog");
        ir->registerQmlUri(Uri("muse://update/app"), "Muse.Update", "AppUpdateProgressDialog");
    }
}

void UpdateModule::onInit(const IApplication::RunMode&)
{
    m_configuration->init();
}

IContextSetup* UpdateModule::newContext(const muse::modularity::ContextPtr& ctx) const
{
    return new UpdateContext(ctx);
}

// Context

void UpdateContext::registerExports()
{
    m_appUpdateScenario = std::make_shared<AppUpdateScenario>(iocContext());
    m_appUpdateService = std::make_shared<AppUpdateService>(iocContext());
    m_actionController = std::make_shared<UpdateActionController>(iocContext());

    ioc()->registerExport<IAppUpdateScenario>(mname, m_appUpdateScenario);
    ioc()->registerExport<IAppUpdateService>(mname, m_appUpdateService);
}

void UpdateContext::resolveImports()
{
    auto ar = ioc()->resolve<ui::IUiActionsRegister>(mname);
    if (ar) {
        ar->reg(std::make_shared<UpdateUiActions>(m_actionController, iocContext()));
    }
}

void UpdateContext::onInit(const IApplication::RunMode&)
{
    m_appUpdateService->init();
    m_actionController->init();
}
