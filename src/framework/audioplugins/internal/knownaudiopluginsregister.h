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
#include "global/io/ifilesystem.h"

#include "../iknownaudiopluginsregister.h"
#include "../iaudiopluginsconfiguration.h"

namespace muse::audioplugins {
class KnownAudioPluginsRegister : public IKnownAudioPluginsRegister, public Injectable
{
public:
    Inject<IAudioPluginsConfiguration> configuration = { this };
    Inject<io::IFileSystem> fileSystem = { this };

public:
    KnownAudioPluginsRegister(const modularity::ContextPtr& iocCtx)
        : Injectable(iocCtx) {}

    Ret load() override;

    std::vector<AudioPluginInfo> pluginInfoList(PluginInfoAccepted accepted = PluginInfoAccepted()) const override;
    const io::path_t& pluginPath(const audio::AudioResourceId& resourceId) const override;

    bool exists(const io::path_t& pluginPath) const override;
    bool exists(const audio::AudioResourceId& resourceId) const override;

    Ret registerPlugin(const AudioPluginInfo& info) override;
    Ret unregisterPlugin(const audio::AudioResourceId& resourceId) override;

private:
    Ret writePluginsInfo();

    bool m_loaded = false;
    std::multimap<audio::AudioResourceId, AudioPluginInfo> m_pluginInfoMap;
    std::set<io::path_t> m_pluginPaths;
};
}
