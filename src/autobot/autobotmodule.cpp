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
#include "autobotmodule.h"

#include <QQmlEngine>

#include "modularity/ioc.h"
#include "ui/iinteractiveuriregister.h"
#include "ui/iuiactionsregister.h"

#include "internal/autobot.h"
#include "internal/autobotconfiguration.h"
#include "view/autobotmodel.h"
#include "view/autobotscriptsmodel.h"

#include "engraving/infrastructure/draw/painter.h"
#include "internal/draw/abpaintprovider.h"
#include "internal/autobotactionscontroller.h"
#include "internal/autobotactions.h"
#include "internal/autobotscriptsrepository.h"

#include "internal/api/apiregister.h"
#include "internal/api/logapi.h"
#include "internal/api/autobotapi.h"
#include "internal/api/dispatcherapi.h"
#include "internal/api/navigationapi.h"

using namespace mu::autobot;
using namespace mu::api;

static const std::shared_ptr<Autobot> s_autobot = std::make_shared<Autobot>();
static std::shared_ptr<AutobotActionsController> s_actionsController = std::make_shared<AutobotActionsController>();

std::string AutobotModule::moduleName() const
{
    return "autobot";
}

void AutobotModule::registerExports()
{
    modularity::ioc()->registerExport<IAutobot>(moduleName(), s_autobot);
    modularity::ioc()->registerExport<IAutobotConfiguration>(moduleName(), new AutobotConfiguration());
    modularity::ioc()->registerExport<IAutobotScriptsRepository>(moduleName(), new AutobotScriptsRepository());

    modularity::ioc()->registerExport<IApiRegister>(moduleName(), new ApiRegister());

    draw::Painter::extended = AbPaintProvider::instance();
}

void AutobotModule::resolveImports()
{
    auto ir = modularity::ioc()->resolve<ui::IInteractiveUriRegister>(moduleName());
    if (ir) {
        ir->registerQmlUri(Uri("musescore://autobot/batchtests"), "MuseScore/Autobot/BatchTestsDialog.qml");
        ir->registerQmlUri(Uri("musescore://autobot/scripts"), "MuseScore/Autobot/ScriptsDialog.qml");
    }

    auto ar = modularity::ioc()->resolve<ui::IUiActionsRegister>(moduleName());
    if (ar) {
        ar->reg(std::make_shared<AutobotActions>());
    }

    auto api = modularity::ioc()->resolve<IApiRegister>(moduleName());
    if (api) {
        api->regApiCreator("global", "api.log", new ApiCreator<LogApi>());
        api->regApiCreator("autobot", "api.autobot", new ApiCreator<AutobotApi>());
        api->regApiCreator("autobot", "api.dispatcher", new ApiCreator<DispatcherApi>());
        api->regApiCreator("autobot", "api.navigation", new ApiCreator<NavigationApi>());
    }
}

void AutobotModule::registerUiTypes()
{
    qmlRegisterType<AutobotModel>("MuseScore.Autobot", 1, 0, "AutobotModel");
    qmlRegisterType<AutobotScriptsModel>("MuseScore.Autobot", 1, 0, "AutobotScriptsModel");
}

void AutobotModule::onInit(const framework::IApplication::RunMode&)
{
    s_autobot->init();
    s_actionsController->init();
}
