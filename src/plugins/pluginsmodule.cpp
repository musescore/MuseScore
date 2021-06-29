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
#include "view/pluginsmodel.h"
#include "view/pluginview.h"
#include "view/dev/pluginstestmodel.h"
#include "api/qmlpluginapi.h"

#include "ui/iinteractiveuriregister.h"

using namespace mu::plugins;
using namespace mu::modularity;
using namespace mu::ui;

static std::shared_ptr<PluginsConfiguration> s_configuration = std::make_shared<PluginsConfiguration>();

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
    ioc()->registerExport<IPluginsService>(moduleName(), new PluginsService());
    ioc()->registerExport<IPluginsConfiguration>(moduleName(), s_configuration);
}

void PluginsModule::registerResources()
{
    plugins_init_qrc();
}

void PluginsModule::registerUiTypes()
{
    qmlRegisterType<PluginsModel>("MuseScore.Plugins", 1, 0, "PluginsModel");
    qmlRegisterType<PluginsTestModel>("MuseScore.Plugins", 1, 0, "PluginsTestModel");

    Ms::PluginAPI::PluginAPI::registerQmlTypes();

    ioc()->resolve<IUiEngine>(moduleName())->addSourceImportPath(plugins_QML_IMPORT);
}

void PluginsModule::onInit(const framework::IApplication::RunMode& mode)
{
    if (framework::IApplication::RunMode::Converter == mode) {
        return;
    }
    s_configuration->init();
}
