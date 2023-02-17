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
#include "pluginsstubmodule.h"

#include "modularity/ioc.h"
#include "ui/iuiengine.h"

#include "pluginsservicestub.h"
#include "pluginsconfigurationstub.h"

using namespace mu::plugins;
using namespace mu::modularity;
using namespace mu::ui;

static void plugins_init_qrc()
{
    Q_INIT_RESOURCE(plugins);
}

std::string PluginsModule::moduleName() const
{
    return "plugins_stub";
}

void PluginsModule::registerExports()
{
    ioc()->registerExport<IPluginsService>(moduleName(), new PluginsServiceStub());
    ioc()->registerExport<IPluginsConfiguration>(moduleName(), new PluginsConfigurationStub());
}

void PluginsModule::registerResources()
{
    plugins_init_qrc();
}

void PluginsModule::registerUiTypes()
{
    std::shared_ptr<ui::IUiEngine> ui = ioc()->resolve<ui::IUiEngine>(moduleName());
    if (ui) {
        ui->addSourceImportPath(plugins_QML_IMPORT);
    }
}
