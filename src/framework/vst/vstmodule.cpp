/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-CLA-applies
 *
 * MuseScore
 * Music Composition & Notation
 *
 * Copyright (C) 2021 MuseScore Limited and others
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

#include "modularity/ioc.h"
#include "audio/engine/isynthresolver.h"
#include "audio/engine/ifxresolver.h"

#include "audioplugins/iaudiopluginsscannerregister.h"
#include "audioplugins/iaudiopluginmetareaderregister.h"

#include "ui/iuiactionsregister.h"

#include "internal/vstconfiguration.h"
#include "internal/vstinstancesregister.h"
#include "internal/vstmodulesrepository.h"
#include "internal/synth/vstiresolver.h"
#include "internal/fx/vstfxresolver.h"
#include "internal/vstpluginsscanner.h"
#include "internal/vstpluginmetareader.h"
#include "internal/vstactionscontroller.h"
#include "internal/vstuiactions.h"

using namespace muse::vst;
using namespace muse::modularity;
using namespace muse::audio::synth;
using namespace muse::audio::fx;
using namespace muse::audio;
using namespace muse::audioplugins;

std::string VSTModule::moduleName() const
{
    return "vst";
}

void VSTModule::registerExports()
{
    m_configuration = std::make_shared<VstConfiguration>();
    m_pluginModulesRepo = std::make_shared<VstModulesRepository>(globalCtx());
    m_pluginInstancesRegister = std::make_shared<VstInstancesRegister>(globalCtx());
    m_actionsController = std::make_shared<VstActionsController>(globalCtx());

    globalIoc()->registerExport<IVstConfiguration>(moduleName(), m_configuration);
    globalIoc()->registerExport<IVstModulesRepository>(moduleName(), m_pluginModulesRepo);
    globalIoc()->registerExport<IVstInstancesRegister>(moduleName(), m_pluginInstancesRegister);
}

void VSTModule::resolveImports()
{
    //! NOTE Now we can switch which view to use in runtime.
    //! switches the action controller, so registration is there now.
    //! as soon as the new view is stabilized, we need to remove the old one and do as usual

    // auto ir = globalIoc()->resolve<IInteractiveUriRegister>(moduleName());
    // if (ir) {
    //     ir->registerWidgetUri<VstViewDialog>(Uri("muse://vst/editor"));
    //     ir->registerQmlUri(Uri("muse://vst/editor"), "Muse.Vst", "VstEditorDialog");
    // }

    auto ar = globalIoc()->resolve<ui::IUiActionsRegister>(moduleName());
    if (ar) {
        ar->reg(std::make_shared<VstUiActions>(m_actionsController));
    }

    auto synthResolver = globalIoc()->resolve<ISynthResolver>(moduleName());
    if (synthResolver) {
        synthResolver->registerResolver(AudioSourceType::Vsti, std::make_shared<VstiResolver>(globalCtx()));
    }

    auto fxResolver = globalIoc()->resolve<IFxResolver>(moduleName());
    if (fxResolver) {
        fxResolver->registerResolver(AudioFxType::VstFx, std::make_shared<VstFxResolver>(globalCtx()));
    }

    auto scannerRegister = globalIoc()->resolve<IAudioPluginsScannerRegister>(moduleName());
    if (scannerRegister) {
        scannerRegister->registerScanner(std::make_shared<VstPluginsScanner>());
    }

    auto metaReaderRegister = globalIoc()->resolve<IAudioPluginMetaReaderRegister>(moduleName());
    if (metaReaderRegister) {
        metaReaderRegister->registerReader(std::make_shared<VstPluginMetaReader>());
    }
}

void VSTModule::onInit(const IApplication::RunMode& mode)
{
    m_configuration->init();
    m_pluginModulesRepo->init();

    if (mode == IApplication::RunMode::GuiApp) {
        m_actionsController->init();
        m_actionsController->setupUsedView();
    }
}

void VSTModule::onDeinit()
{
    m_pluginModulesRepo->deInit();
}
