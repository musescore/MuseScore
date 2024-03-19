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

#include "internal/pluginsconfiguration.h"
#include "internal/pluginsuiactions.h"
#include "internal/pluginsactioncontroller.h"

#include "view/pluginsmodel.h"

#include "ui/iuiactionsregister.h"

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
    m_pluginActionController = std::make_shared<PluginsActionController>();

    ioc()->registerExport<IPluginsConfiguration>(moduleName(), m_configuration);
}

void PluginsModule::resolveImports()
{
}

void PluginsModule::registerResources()
{
    plugins_init_qrc();
}

void PluginsModule::registerUiTypes()
{
    qmlRegisterType<PluginsModel>("MuseScore.Plugins", 1, 0, "PluginsModel");

    auto ui = ioc()->resolve<IUiEngine>(moduleName());
    if (ui) {
        ui->addSourceImportPath(plugins_QML_IMPORT);
    }
}

void PluginsModule::onInit(const IApplication::RunMode& mode)
{
    if (mode != IApplication::RunMode::GuiApp) {
        return;
    }

    m_configuration->init();

    m_pluginActionController->init();
}

void PluginsModule::onDelayedInit()
{
}
