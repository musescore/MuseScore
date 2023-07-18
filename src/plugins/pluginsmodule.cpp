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
#include "pluginsmodule.h"

#include <QQmlEngine>

#include "modularity/ioc.h"
#include "ui/iuiengine.h"

#include "internal/pluginsservice.h"
#include "internal/pluginsconfiguration.h"
#include "internal/pluginsuiactions.h"
#include "internal/pluginsactioncontroller.h"

#include "view/pluginsmodel.h"
#include "api/qmlpluginapi.h"

#include "ui/iuiactionsregister.h"

#include "ui/iinteractiveuriregister.h"

using namespace mu::plugins;
using namespace mu::modularity;
using namespace mu::ui;

static void plugins_init_qrc()
{
    Q_INIT_RESOURCE(plugins);
}

std::string PluginsModule::moduleName() const
{
    return "plugins";
}

void PluginsModule::registerExports()
{
    m_configuration = std::make_shared<PluginsConfiguration>();
    m_pluginsService = std::make_shared<PluginsService>();
    m_pluginsUiActions = std::make_shared<PluginsUiActions>(m_pluginsService);
    m_pluginActionController = std::make_shared<PluginsActionController>();

    ioc()->registerExport<IPluginsService>(moduleName(), m_pluginsService);
    ioc()->registerExport<IPluginsConfiguration>(moduleName(), m_configuration);
}

void PluginsModule::resolveImports()
{
    auto ar = ioc()->resolve<ui::IUiActionsRegister>(moduleName());
    if (ar) {
        ar->reg(m_pluginsUiActions);
    }
}

void PluginsModule::registerResources()
{
    plugins_init_qrc();
}

void PluginsModule::registerUiTypes()
{
    qmlRegisterType<PluginsModel>("MuseScore.Plugins", 1, 0, "PluginsModel");

    mu::plugins::api::PluginAPI::registerQmlTypes();

    auto ui = ioc()->resolve<IUiEngine>(moduleName());
    if (ui) {
        ui->addSourceImportPath(plugins_QML_IMPORT);
    }
}

void PluginsModule::onInit(const framework::IApplication::RunMode& mode)
{
    if (mode != framework::IApplication::RunMode::GuiApp) {
        return;
    }

    m_configuration->init();
}

void PluginsModule::onDelayedInit()
{
    //! NOTE: Need to be registered only on delayed init because it depends on the information
    //!       that is stored in the qml and we can only access them after the qml engine has been loaded
    m_pluginsService->init();

    auto ar = ioc()->resolve<ui::IUiActionsRegister>(moduleName());
    if (ar) {
        //! NOTE: Re-registration of actions for new available plugins
        ar->reg(m_pluginsUiActions);

        //! NOTE: Notify about plugins changed for updating actions state
        m_pluginsService->pluginsChanged().notify();
    }

    m_pluginActionController->init();
}
