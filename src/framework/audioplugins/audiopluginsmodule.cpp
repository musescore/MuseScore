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
#include "audiopluginsmodule.h"

#include "internal/audiopluginsconfiguration.h"
#include "internal/knownaudiopluginsregister.h"
#include "internal/audiopluginsscannerregister.h"
#include "internal/audiopluginmetareaderregister.h"
#include "internal/registeraudiopluginsscenario.h"

#include "diagnostics/idiagnosticspathsregister.h"

using namespace muse;
using namespace muse::modularity;
using namespace muse::audioplugins;

static const std::string mname("vst");

std::string AudioPluginsModule::moduleName() const
{
    return mname;
}

void AudioPluginsModule::registerExports()
{
    m_configuration = std::make_shared<AudioPluginsConfiguration>(globalCtx());

    globalIoc()->registerExport<IAudioPluginsConfiguration>(moduleName(), m_configuration);
    globalIoc()->registerExport<IKnownAudioPluginsRegister>(moduleName(), std::make_shared<KnownAudioPluginsRegister>());
    globalIoc()->registerExport<IAudioPluginsScannerRegister>(moduleName(), std::make_shared<AudioPluginsScannerRegister>());
    globalIoc()->registerExport<IAudioPluginMetaReaderRegister>(moduleName(), std::make_shared<AudioPluginMetaReaderRegister>());
}

void AudioPluginsModule::resolveImports()
{
    //! --- Diagnostics ---
    auto pr = globalIoc()->resolve<muse::diagnostics::IDiagnosticsPathsRegister>(moduleName());
    if (pr) {
        pr->reg("known_audio_plugins", m_configuration->knownAudioPluginsFilePath());
    }
}

modularity::IContextSetup* AudioPluginsModule::newContext(const muse::modularity::ContextPtr& ctx) const
{
    return new AudioPluginsContext(ctx);
}

void AudioPluginsContext::registerExports()
{
    m_registerAudioPluginsScenario = std::make_shared<RegisterAudioPluginsScenario>(iocContext());

    ioc()->registerExport<IRegisterAudioPluginsScenario>(mname, m_registerAudioPluginsScenario);
}

void AudioPluginsContext::onInit(const IApplication::RunMode&)
{
    m_registerAudioPluginsScenario->init();
}
