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
#include "vstmodule.h"

#include <QQmlEngine>

#include "settings.h"
#include "internal/vstscanner.h"
#include "devtools/vstdevtools.h"
#include "internal/plugineditorview.h"
#include "internal/vstinstanceregister.h"
#include "view/vstinstanceeditormodel.h"
#include "ui/iinteractiveuriregister.h"
#include "modularity/ioc.h"
#include "log.h"

using namespace mu::vst;
using namespace mu::framework;
using namespace mu::ui;

VSTConfiguration VSTModule::m_configuration = VSTConfiguration();
static std::shared_ptr<VSTScanner> s_vstScanner = std::make_shared<VSTScanner>();
static std::shared_ptr<VSTInstanceRegister> s_register = std::make_shared<VSTInstanceRegister>();

std::string VSTModule::moduleName() const
{
    return "vst";
}

void VSTModule::registerExports()
{
    framework::ioc()->registerExport<VSTScanner>(moduleName(), s_vstScanner);
    framework::ioc()->registerExport<IVSTInstanceRegister>(moduleName(), s_register);
}

void VSTModule::resolveImports()
{
    auto ir = ioc()->resolve<IInteractiveUriRegister>(moduleName());
    if (ir) {
        ir->registerUri(Uri("musescore://vst/editor"),
                        ContainerMeta(ContainerType::QWidgetDialog, PluginEditorView::metaTypeId()));
    }
}

static void vst_init_qrc()
{
    Q_INIT_RESOURCE(vst);
}

void VSTModule::registerResources()
{
    vst_init_qrc();
}

void VSTModule::registerUiTypes()
{
    qmlRegisterType<VSTDevTools>("MuseScore.VST", 1, 0, "VSTDevTools");
    qmlRegisterType<VSTInstanceEditorModel>("MuseScore.VST", 1, 0, "VSTInstanceEditorModel");
    qmlRegisterType<PluginListModel>("MuseScore.VST", 1, 0, "VSTPluginListModel");
    qRegisterMetaType<PluginEditorView>("PluginEditorView");
}

void VSTModule::onInit(const IApplication::RunMode& mode)
{
    if (framework::IApplication::RunMode::Editor != mode) {
        return;
    }

    m_configuration.init();
    s_vstScanner->setPaths(m_configuration.searchPaths());
}
