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

#ifndef MU_AUDIO_REGISTERAUDIOPLUGINSSCENARIO_H
#define MU_AUDIO_REGISTERAUDIOPLUGINSSCENARIO_H

#include "audio/iregisteraudiopluginsscenario.h"
#include "async/asyncable.h"

#include "modularity/ioc.h"
#include "audio/iknownaudiopluginsregister.h"
#include "audio/iaudiopluginsscannerregister.h"
#include "audio/iaudiopluginmetareaderregister.h"
#include "iprocess.h"
#include "iglobalconfiguration.h"
#include "iinteractive.h"

namespace mu::audio {
class RegisterAudioPluginsScenario : public IRegisterAudioPluginsScenario, public async::Asyncable
{
    INJECT(IKnownAudioPluginsRegister, knownPluginsRegister)
    INJECT(IAudioPluginsScannerRegister, scannerRegister)
    INJECT(IAudioPluginMetaReaderRegister, metaReaderRegister)
    INJECT(framework::IGlobalConfiguration, globalConfiguration)
    INJECT(framework::IInteractive, interactive)
    INJECT(IProcess, process)

public:
    void init();

    Ret registerNewPlugins() override;
    Ret registerPlugin(const io::path_t& pluginPath) override;
    Ret registerFailedPlugin(const io::path_t& pluginPath, int failCode) override;

private:
    void processPluginsRegistration(const io::paths_t& pluginPaths);
    IAudioPluginMetaReaderPtr metaReader(const io::path_t& pluginPath) const;

    framework::Progress m_progress;
    bool m_aborted = false;
};
}

#endif // MU_AUDIO_REGISTERAUDIOPLUGINSSCENARIO_H
