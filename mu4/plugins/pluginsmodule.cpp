//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2020 MuseScore BVBA and others
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
//=============================================================================
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
using namespace mu::framework;

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
    framework::ioc()->registerExport<IPluginsService>(moduleName(), new PluginsService());
    framework::ioc()->registerExport<IPluginsConfiguration>(moduleName(), new PluginsConfiguration());
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

    framework::ioc()->resolve<framework::IUiEngine>(moduleName())->addSourceImportPath(plugins_QML_IMPORT);
}
