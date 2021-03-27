//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2021 MuseScore BVBA and others
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
#include "vstmodule.h"

#include <QQmlEngine>

#include "ui/iinteractiveuriregister.h"
#include "ui/iuiengine.h"
#include "log.h"
#include "settings.h"
#include "modularity/ioc.h"

#include "internal/vstconfiguration.h"
#include "internal/vstpluginrepository.h"

#include "devtools/vstpluginlistmodelexample.h"
#include "view/vstplugineditorview.h"

using namespace mu::vst;
using namespace mu::framework;
using namespace mu::ui;

static std::shared_ptr<IVstConfiguration> s_configuration = std::make_shared<VstConfiguration>();
static std::shared_ptr<IVstPluginRepository> s_pluginRepo = std::make_shared<VstPluginRepository>();

static void vst_init_qrc()
{
    Q_INIT_RESOURCE(vst);
}

std::string VSTModule::moduleName() const
{
    return "vst";
}

void VSTModule::registerExports()
{
    ioc()->registerExport<IVstConfiguration>(moduleName(), s_configuration);
    ioc()->registerExport<IVstPluginRepository>(moduleName(), s_pluginRepo);
}

void VSTModule::resolveImports()
{
    auto ir = ioc()->resolve<IInteractiveUriRegister>(moduleName());
    if (ir) {
        ir->registerUri(Uri("musescore://vst/editor"),
                        ContainerMeta(ContainerType::QWidgetDialog, qRegisterMetaType<VstPluginEditorView>("VstPluginEditorView")));
    }
}

void VSTModule::registerResources()
{
    vst_init_qrc();
}

void VSTModule::registerUiTypes()
{
    qmlRegisterType<VstPluginListModelExample>("MuseScore.Vst", 1, 0, "VstPluginListModelExample");

    ioc()->resolve<ui::IUiEngine>(moduleName())->addSourceImportPath(vst_QML_IMPORT);
}

void VSTModule::onInit(const IApplication::RunMode& mode)
{
    if (framework::IApplication::RunMode::Editor != mode) {
        return;
    }

    s_pluginRepo->loadAvailablePlugins();
}
