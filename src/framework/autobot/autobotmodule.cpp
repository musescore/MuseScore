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
#include "autobotmodule.h"

#include "modularity/ioc.h"
#include "interactive/iinteractiveuriregister.h"
#include "ui/iuiactionsregister.h"

#include "internal/autobot.h"
#include "internal/autobotconfiguration.h"

#include "internal/autobotactionscontroller.h"
#include "internal/autobotactions.h"
#include "internal/autobotscriptsrepository.h"

#include "internal/api/autobotapi.h"
#include "internal/api/contextapi.h"

#include "diagnostics/idiagnosticspathsregister.h"

using namespace muse::autobot;
using namespace muse::api;
using namespace muse::modularity;

static const std::string mname("autobot");

std::string AutobotModule::moduleName() const
{
    return mname;
}

void AutobotModule::registerExports()
{
    m_configuration = std::make_shared<AutobotConfiguration>(globalCtx());

    globalIoc()->registerExport<IAutobotConfiguration>(mname, m_configuration);
}

void AutobotModule::resolveImports()
{
    auto ir = globalIoc()->resolve<muse::interactive::IInteractiveUriRegister>(mname);
    if (ir) {
        ir->registerQmlUri(Uri("muse://diagnostics/autobot/scripts"), "Muse.Autobot", "ScriptsDialog");
        ir->registerQmlUri(Uri("muse://autobot/selectfile"), "Muse.Autobot", "AutobotSelectFileDialog");
    }
}

IContextSetup* AutobotModule::newContext(const muse::modularity::ContextPtr& ctx) const
{
    return new AutobotContext(ctx);
}

// Context

void AutobotContext::registerExports()
{
    m_autobot = std::make_shared<Autobot>(iocContext());
    m_actionsController = std::make_shared<AutobotActionsController>(iocContext());

    ioc()->registerExport<IAutobot>(mname, m_autobot);
    ioc()->registerExport<IAutobotScriptsRepository>(mname, new AutobotScriptsRepository(iocContext()));
}

void AutobotContext::resolveImports()
{
    auto ar = ioc()->resolve<muse::ui::IUiActionsRegister>(mname);
    if (ar) {
        ar->reg(std::make_shared<AutobotActions>());
    }

    auto api = ioc()->resolve<IApiRegister>(mname);
    if (api) {
        api->regApiCreator(mname, "api.autobot", new ApiCreator<api::AutobotApi>());
        api->regApiCreator(mname, "api.context", new ApiCreator<ContextApi>());
    }
}

void AutobotContext::onInit(const IApplication::RunMode&)
{
    m_autobot->init();
    m_actionsController->init();

    //! --- Diagnostics ---
    auto pr = ioc()->resolve<muse::diagnostics::IDiagnosticsPathsRegister>(mname);
    if (pr) {
        auto configuration = globalIoc()->resolve<IAutobotConfiguration>(mname);
        for (const io::path_t& p : configuration->scriptsDirPaths()) {
            pr->reg("autobotScriptsPath", p);
        }
        for (const io::path_t& p : configuration->testingFilesDirPaths()) {
            pr->reg("autobotTestingFilesPath", p);
        }
        pr->reg("autobotDataPath", configuration->dataPath());
        pr->reg("autobotSavingFilesPath", configuration->savingFilesPath());
        pr->reg("autobotReportsPath", configuration->reportsPath());
        pr->reg("autobotDrawDataPath", configuration->drawDataPath());
    }
}
