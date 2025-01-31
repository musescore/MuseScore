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
#include "vstmodule.h"

#include <QQmlEngine>

#include "ui/iinteractiveuriregister.h"
#include "ui/iuiengine.h"

#include "modularity/ioc.h"
#include "audio/isynthresolver.h"
#include "audio/ifxresolver.h"

#include "audioplugins/iaudiopluginsscannerregister.h"
#include "audioplugins/iaudiopluginmetareaderregister.h"

#include "ui/iuiactionsregister.h"

#include "internal/vstconfiguration.h"
#include "internal/vstinstancesregister.h"
#include "internal/vstmodulesrepository.h"
#include "internal/synth/vstsynthesiser.h"
#include "internal/synth/vstiresolver.h"
#include "internal/fx/vstfxresolver.h"
#include "internal/vstpluginsscanner.h"
#include "internal/vstpluginmetareader.h"
#include "internal/vstactionscontroller.h"
#include "internal/vstuiactions.h"

#include "view/vstview.h"
#include "view/vstviewdialog_qwidget.h"

#include "log.h"

using namespace muse::vst;
using namespace muse::modularity;
using namespace muse::audio::synth;
using namespace muse::audio::fx;
using namespace muse::audio;
using namespace muse::audioplugins;
using namespace muse::ui;

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
    m_configuration = std::make_shared<VstConfiguration>();
    m_pluginModulesRepo = std::make_shared<VstModulesRepository>();
    m_pluginInstancesRegister = std::make_shared<VstInstancesRegister>();
    m_actionsController = std::make_shared<VstActionsController>();

    ioc()->registerExport<IVstConfiguration>(moduleName(), m_configuration);
    ioc()->registerExport<IVstModulesRepository>(moduleName(), m_pluginModulesRepo);
    ioc()->registerExport<IVstInstancesRegister>(moduleName(), m_pluginInstancesRegister);
}

void VSTModule::resolveImports()
{
    //! NOTE Now we can switch which view to use in runtime.
    //! switches the action controller, so registration is there now.
    //! as soon as the new view is stabilized, we need to remove the old one and do as usual

    // auto ir = ioc()->resolve<IInteractiveUriRegister>(moduleName());
    // if (ir) {
    //     ir->registerWidgetUri<VstViewDialog>(Uri("muse://vst/editor"));
    //     ir->registerQmlUri(Uri("muse://vst/editor"), "Muse/Vst/VstEditorDialog.qml");
    // }

    auto ar = ioc()->resolve<ui::IUiActionsRegister>(moduleName());
    if (ar) {
        ar->reg(std::make_shared<VstUiActions>(m_actionsController));
    }

    auto synthResolver = ioc()->resolve<ISynthResolver>(moduleName());
    if (synthResolver) {
        synthResolver->registerResolver(AudioSourceType::Vsti, std::make_shared<VstiResolver>(iocContext()));
    }

    auto fxResolver = ioc()->resolve<IFxResolver>(moduleName());
    if (fxResolver) {
        fxResolver->registerResolver(AudioFxType::VstFx, std::make_shared<VstFxResolver>());
    }

    auto scannerRegister = ioc()->resolve<IAudioPluginsScannerRegister>(moduleName());
    if (scannerRegister) {
        scannerRegister->registerScanner(std::make_shared<VstPluginsScanner>());
    }

    auto metaReaderRegister = ioc()->resolve<IAudioPluginMetaReaderRegister>(moduleName());
    if (metaReaderRegister) {
        metaReaderRegister->registerReader(std::make_shared<VstPluginMetaReader>());
    }
}

void VSTModule::registerResources()
{
    vst_init_qrc();
}

void VSTModule::registerUiTypes()
{
    qmlRegisterType<VstView>("Muse.Vst", 1, 0, "VstView");

    ioc()->resolve<muse::ui::IUiEngine>(moduleName())->addSourceImportPath(muse_vst_QML_IMPORT);
}

void VSTModule::onInit(const IApplication::RunMode&)
{
    m_configuration->init();
    m_actionsController->init();
    m_pluginModulesRepo->init();
    m_actionsController->setupUsedView();
}

void VSTModule::onDeinit()
{
    m_pluginModulesRepo->deInit();
}
