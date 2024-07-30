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

#pragma once

#include "global/modularity/ioc.h"
#include "global/iprocess.h"
#include "global/iglobalconfiguration.h"
#include "global/iinteractive.h"
#include "global/async/asyncable.h"

#include "../iregisteraudiopluginsscenario.h"
#include "../iknownaudiopluginsregister.h"
#include "../iaudiopluginsscannerregister.h"
#include "../iaudiopluginmetareaderregister.h"

namespace muse::audioplugins {
class RegisterAudioPluginsScenario : public IRegisterAudioPluginsScenario, public Injectable, public async::Asyncable
{
public:
    Inject<IKnownAudioPluginsRegister> knownPluginsRegister = { this };
    Inject<IAudioPluginsScannerRegister> scannerRegister = { this };
    Inject<IAudioPluginMetaReaderRegister> metaReaderRegister = { this };
    Inject<IGlobalConfiguration> globalConfiguration = { this };
    Inject<IInteractive> interactive = { this };
    Inject<IProcess> process = { this };

public:
    RegisterAudioPluginsScenario(const modularity::ContextPtr& iocCtx)
        : Injectable(iocCtx) {}

    void init();

    Ret registerNewPlugins() override;
    Ret registerPlugin(const io::path_t& pluginPath) override;
    Ret registerFailedPlugin(const io::path_t& pluginPath, int failCode) override;

private:
    void processPluginsRegistration(const io::paths_t& pluginPaths);
    IAudioPluginMetaReaderPtr metaReader(const io::path_t& pluginPath) const;

    Progress m_progress;
    bool m_aborted = false;
};
}
