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
#include "shortcutsmodule.h"

#include "modularity/ioc.h"

#include "internal/shortcutsregister.h"
#include "internal/shortcutscontroller.h"
#include "internal/midiremote.h"
#include "internal/shortcutsconfiguration.h"

#include "global/api/iapiregister.h"
#include "api/shortcutsapi.h"

#include "muse_framework_config.h"

#ifdef MUSE_MODULE_DIAGNOSTICS
#include "diagnostics/idiagnosticspathsregister.h"
#endif

using namespace muse::shortcuts;
using namespace muse::modularity;
using namespace muse::ui;

static const std::string mname("shortcuts");

std::string ShortcutsModule::moduleName() const
{
    return mname;
}

void ShortcutsModule::registerExports()
{
    m_configuration = std::make_shared<ShortcutsConfiguration>(globalCtx());

    globalIoc()->registerExport<IShortcutsConfiguration>(mname, m_configuration);
}

void ShortcutsModule::registerApi()
{
    using namespace muse::api;

    auto api = globalIoc()->resolve<IApiRegister>(mname);
    if (api) {
        api->regApiCreator(mname, "MuseInternal.Shortcuts", new ApiCreator<api::ShortcutsApi>());
    }
}

void ShortcutsModule::onInit(const IApplication::RunMode&)
{
    m_configuration->init();
}

IContextSetup* ShortcutsModule::newContext(const modularity::ContextPtr& ctx) const
{
    return new ShortcutsContext(ctx);
}

void ShortcutsContext::registerExports()
{
    m_shortcutsController = std::make_shared<ShortcutsController>(iocContext());
    m_shortcutsRegister = std::make_shared<ShortcutsRegister>(iocContext());
    m_midiRemote = std::make_shared<MidiRemote>(iocContext());

    ioc()->registerExport<IShortcutsRegister>(mname, m_shortcutsRegister);
    ioc()->registerExport<IShortcutsController>(mname, m_shortcutsController);
    ioc()->registerExport<IMidiRemote>(mname, m_midiRemote);
}

void ShortcutsContext::onInit(const IApplication::RunMode&)
{
    m_shortcutsController->init();
    m_midiRemote->init();

#ifdef MUSE_MODULE_DIAGNOSTICS
    auto configuration = ioc()->resolve<IShortcutsConfiguration>(mname);
    auto pr = ioc()->resolve<muse::diagnostics::IDiagnosticsPathsRegister>(mname);
    if (pr && configuration) {
        pr->reg("shortcutsUserAppDataPath", configuration->shortcutsUserAppDataPath());
        pr->reg("shortcutsAppDataPath", configuration->shortcutsAppDataPath());
        pr->reg("midiMappingUserAppDataPath", configuration->midiMappingUserAppDataPath());
    }
#endif
}

void ShortcutsContext::onAllInited(const IApplication::RunMode&)
{
    m_shortcutsRegister->init();
}
