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
#include "testflowmodule.h"

#include "modularity/ioc.h"
#include "interactive/iinteractiveuriregister.h"
#include "ui/iuiactionsregister.h"

#include "internal/testflow.h"
#include "internal/testflowconfiguration.h"

#include "internal/testflowactionscontroller.h"
#include "internal/testflowactions.h"
#include "internal/testflowscriptsrepository.h"

#include "internal/api/testflowapi.h"
#include "internal/api/contextapi.h"

#include "diagnostics/idiagnosticspathsregister.h"

using namespace muse::testflow;
using namespace muse::api;
using namespace muse::modularity;

static const std::string mname("testflow");

std::string TestflowModule::moduleName() const
{
    return mname;
}

void TestflowModule::registerExports()
{
    m_configuration = std::make_shared<TestflowConfiguration>(globalCtx());

    globalIoc()->registerExport<ITestflowConfiguration>(mname, m_configuration);
}

void TestflowModule::resolveImports()
{
    auto ir = globalIoc()->resolve<muse::interactive::IInteractiveUriRegister>(mname);
    if (ir) {
        ir->registerQmlUri(Uri("muse://diagnostics/testflow/scripts"), "Muse.Testflow", "ScriptsDialog");
        ir->registerQmlUri(Uri("muse://testflow/selectfile"), "Muse.Testflow", "TestflowSelectFileDialog");
    }

    auto api = globalIoc()->resolve<IApiRegister>(mname);
    if (api) {
        api->regApiCreator(mname, "MuseInternal.Testflow", new ApiCreator<api::TestflowApi>());
        api->regApiCreator(mname, "MuseInternal.TestflowContext", new ApiCreator<ContextApi>());
    }
}

void TestflowModule::onInit(const IApplication::RunMode&)
{
    //! --- Diagnostics ---
    auto pr = globalIoc()->resolve<muse::diagnostics::IDiagnosticsPathsRegister>(mname);
    if (pr) {
        for (const io::path_t& p : m_configuration->scriptsDirPaths()) {
            pr->reg("testflowScriptsPath", p);
        }
        for (const io::path_t& p : m_configuration->testingFilesDirPaths()) {
            pr->reg("testflowTestingFilesPath", p);
        }
        pr->reg("testflowDataPath", m_configuration->dataPath());
        pr->reg("testflowSavingFilesPath", m_configuration->savingFilesPath());
        pr->reg("testflowReportsPath", m_configuration->reportsPath());
        pr->reg("testflowDrawDataPath", m_configuration->drawDataPath());
    }
}

IContextSetup* TestflowModule::newContext(const muse::modularity::ContextPtr& ctx) const
{
    return new TestflowContext(ctx);
}

// Context

void TestflowContext::registerExports()
{
    m_testflow = std::make_shared<Testflow>(iocContext());
    m_actionsController = std::make_shared<TestflowActionsController>(iocContext());

    ioc()->registerExport<ITestflow>(mname, m_testflow);
    ioc()->registerExport<ITestflowScriptsRepository>(mname, new TestflowScriptsRepository(iocContext()));
}

void TestflowContext::resolveImports()
{
    auto ar = ioc()->resolve<muse::ui::IUiActionsRegister>(mname);
    if (ar) {
        ar->reg(std::make_shared<TestflowActions>());
    }
}

void TestflowContext::onInit(const IApplication::RunMode&)
{
    m_testflow->init();
    m_actionsController->init();
}
