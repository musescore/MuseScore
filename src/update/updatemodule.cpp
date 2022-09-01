/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-CLA-applies
 *
 * MuseScore
 * Music Composition & Notation
 *
 * Copyright (C) 2021 MuseScore BVBA and others
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

#include <QQmlEngine>

#include "modularity/ioc.h"

#include "framework/ui/iinteractiveuriregister.h"
#include "framework/ui/iuiengine.h"
#include "framework/ui/iuiactionsregister.h"

#include "internal/updateconfiguration.h"
#include "internal/updatescenario.h"
#include "internal/updateservice.h"
#include "internal/updateactioncontroller.h"
#include "internal/updateuiactions.h"

#include "view/updatemodel.h"

#include "diagnostics/idiagnosticspathsregister.h"

using namespace mu::update;
using namespace mu::modularity;
using namespace mu::ui;

static std::shared_ptr<UpdateScenario> s_scenario = std::make_shared<UpdateScenario>();
static std::shared_ptr<UpdateService> s_service = std::make_shared<UpdateService>();
static std::shared_ptr<UpdateConfiguration> s_configuration = std::make_shared<UpdateConfiguration>();
static std::shared_ptr<UpdateActionController> s_actionController = std::make_shared<UpdateActionController>();

static void update_init_qrc()
{
    Q_INIT_RESOURCE(update);
}

std::string UpdateModule::moduleName() const
{
    return "update";
}

void UpdateModule::registerExports()
{
    ioc()->registerExport<IUpdateScenario>(moduleName(), s_scenario);
    ioc()->registerExport<IUpdateService>(moduleName(), s_service);
    ioc()->registerExport<IUpdateConfiguration>(moduleName(), s_configuration);
}

void UpdateModule::resolveImports()
{
    auto ar = ioc()->resolve<ui::IUiActionsRegister>(moduleName());
    if (ar) {
        ar->reg(std::make_shared<UpdateUiActions>(s_actionController));
    }

    auto ir = ioc()->resolve<IInteractiveUriRegister>(moduleName());
    if (ir) {
        ir->registerQmlUri(Uri("musescore://update/releaseinfo"), "MuseScore/Update/ReleaseInfoDialog.qml");
        ir->registerQmlUri(Uri("musescore://update"), "MuseScore/Update/UpdateProgressDialog.qml");
    }
}

void UpdateModule::registerResources()
{
    update_init_qrc();
}

void UpdateModule::registerUiTypes()
{
    qmlRegisterType<UpdateModel>("MuseScore.Update", 1, 0, "UpdateModel");

    ioc()->resolve<IUiEngine>(moduleName())->addSourceImportPath(update_QML_IMPORT);
}

void UpdateModule::onInit(const framework::IApplication::RunMode& mode)
{
    if (framework::IApplication::RunMode::Converter == mode) {
        return;
    }

    s_configuration->init();
    s_service->init();
    s_actionController->init();
}

void UpdateModule::onDelayedInit()
{
    s_scenario->delayedInit();
}
