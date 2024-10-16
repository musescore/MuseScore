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
#include "internal/updateactioncontroller.h"
#include "internal/updateuiactions.h"
#include "internal/appupdateservice.h"

#include "internal/musesoundscheckupdatescenario.h"
#include "internal/musesoundscheckupdateservice.h"

#include "view/updatemodel.h"

#include "diagnostics/idiagnosticspathsregister.h"

using namespace muse::update;
using namespace muse::modularity;
using namespace muse::ui;

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
    m_scenario = std::make_shared<UpdateScenario>(iocContext());
    m_configuration = std::make_shared<UpdateConfiguration>(iocContext());
    m_actionController = std::make_shared<UpdateActionController>(iocContext());
    m_appUpdateService = std::make_shared<AppUpdateService>(iocContext());

    m_museSoundsCheckUpdateScenario = std::make_shared<MuseSoundsCheckUpdateScenario>(iocContext());
    m_museSamplerUpdateService = std::make_shared<MuseSoundsCheckUpdateService>(iocContext());

    ioc()->registerExport<IUpdateScenario>(moduleName(), m_scenario);
    ioc()->registerExport<IUpdateConfiguration>(moduleName(), m_configuration);
    ioc()->registerExport<IAppUpdateService>(moduleName(), m_appUpdateService);

    ioc()->registerExport<IMuseSoundsCheckUpdateScenario>(moduleName(), m_museSoundsCheckUpdateScenario);
    ioc()->registerExport<IMuseSoundsCheckUpdateService>(moduleName(), m_museSamplerUpdateService);
}

void UpdateModule::resolveImports()
{
    auto ar = ioc()->resolve<IUiActionsRegister>(moduleName());
    if (ar) {
        ar->reg(std::make_shared<UpdateUiActions>(m_actionController, iocContext()));
    }

    auto ir = ioc()->resolve<IInteractiveUriRegister>(moduleName());
    if (ir) {
        ir->registerQmlUri(Uri("muse://update/appreleaseinfo"), "Muse/Update/AppReleaseInfoDialog.qml");
        ir->registerQmlUri(Uri("muse://update"), "Muse/Update/UpdateProgressDialog.qml");
        ir->registerQmlUri(Uri("muse://update/musesoundsreleaseinfo"), "Muse/Update/MuseSoundsReleaseInfoDialog.qml");
    }
}

void UpdateModule::registerResources()
{
    update_init_qrc();
}

void UpdateModule::registerUiTypes()
{
    qmlRegisterType<UpdateModel>("Muse.Update", 1, 0, "UpdateModel");

    ioc()->resolve<IUiEngine>(moduleName())->addSourceImportPath(muse_update_QML_IMPORT);
}

void UpdateModule::onInit(const IApplication::RunMode& mode)
{
    if (mode != IApplication::RunMode::GuiApp) {
        return;
    }

    m_configuration->init();
    m_appUpdateService->init();
    m_actionController->init();
}

void UpdateModule::onDelayedInit()
{
    m_scenario->delayedInit();
    m_museSoundsCheckUpdateScenario->delayedInit();
}
