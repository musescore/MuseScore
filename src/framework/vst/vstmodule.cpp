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

#include "internal/vstconfiguration.h"
#include "internal/vstpluginsregister.h"
#include "internal/vstmodulesrepository.h"
#include "internal/synth/vstsynthesiser.h"
#include "internal/synth/vstiresolver.h"
#include "internal/fx/vstfxresolver.h"
#include "internal/vstpluginsscanner.h"
#include "internal/vstpluginmetareader.h"

#include "view/vstieditorview.h"
#include "view/vstfxeditorview.h"

#include "log.h"

using namespace muse::vst;
using namespace muse::modularity;
using namespace muse::audio::synth;
using namespace muse::audio::fx;
using namespace muse::audio;
using namespace muse::audioplugins;
using namespace muse::ui;

static std::shared_ptr<VstConfiguration> s_configuration = std::make_shared<VstConfiguration>();
static std::shared_ptr<VstModulesRepository> s_pluginModulesRepo = std::make_shared<VstModulesRepository>();
static std::shared_ptr<VstPluginsRegister> s_pluginsRegister = std::make_shared<VstPluginsRegister>();

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
    ioc()->registerExport<IVstModulesRepository>(moduleName(), s_pluginModulesRepo);
    ioc()->registerExport<IVstPluginsRegister>(moduleName(), s_pluginsRegister);
}

void VSTModule::resolveImports()
{
    auto ir = ioc()->resolve<IInteractiveUriRegister>(moduleName());
    if (ir) {
        ir->registerUri(Uri("muse://vsti/editor"),
                        ContainerMeta(ContainerType::QWidgetDialog, qRegisterMetaType<VstiEditorView>("VstiEditorView")));

        ir->registerUri(Uri("muse://vstfx/editor"),
                        ContainerMeta(ContainerType::QWidgetDialog, qRegisterMetaType<VstFxEditorView>("VstFxEditorView")));
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
    ioc()->resolve<muse::ui::IUiEngine>(moduleName())->addSourceImportPath(muse_vst_QML_IMPORT);
}

void VSTModule::onInit(const IApplication::RunMode&)
{
    s_configuration->init();
    s_pluginModulesRepo->init();
}

void VSTModule::onDeinit()
{
    s_pluginModulesRepo->deInit();
}
