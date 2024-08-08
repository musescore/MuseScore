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
#include "audiopluginsmodule.h"


#include "log.h"

using namespace muse;
using namespace muse::modularity;
using namespace muse::audioplugins;

AudioPluginsModule::AudioPluginsModule()
{

}

std::string AudioPluginsModule::moduleName() const
{
    return "audioplugins";
}

void AudioPluginsModule::registerExports()
{

}

void AudioPluginsModule::resolveImports()
{

}

void AudioPluginsModule::onInit(const IApplication::RunMode& mode)
{


    //! --- Diagnostics ---
    // auto pr = ioc()->resolve<muse::diagnostics::IDiagnosticsPathsRegister>(moduleName());
    // if (pr) {
    //     std::vector<io::path_t> paths = m_configuration->soundFontDirectories();
    //     for (const io::path_t& p : paths) {
    //         pr->reg("soundfonts", p);
    //     }
    //     pr->reg("known_audio_plugins", m_configuration->knownAudioPluginsFilePath());
    // }
}

void AudioPluginsModule::onDelayedInit()
{
    // Ret ret = m_registerAudioPluginsScenario->registerNewPlugins();
    // if (!ret) {
    //     LOGE() << ret.toString();
    // }
}

