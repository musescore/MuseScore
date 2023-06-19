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

#ifndef MU_AUDIO_KNOWNAUDIOPLUGINSREGISTER_H
#define MU_AUDIO_KNOWNAUDIOPLUGINSREGISTER_H

#include "audio/iknownaudiopluginsregister.h"

#include "modularity/ioc.h"
#include "audio/iaudioconfiguration.h"
#include "io/ifilesystem.h"

namespace mu::audio {
class KnownAudioPluginsRegister : public IKnownAudioPluginsRegister
{
    INJECT(IAudioConfiguration, configuration)
    INJECT(io::IFileSystem, fileSystem)

public:
    Ret load() override;

    std::vector<AudioPluginInfo> pluginInfoList(PluginInfoAccepted accepted = PluginInfoAccepted()) const override;
    const io::path_t& pluginPath(const AudioResourceId& resourceId) const override;

    bool exists(const io::path_t& pluginPath) const override;
    bool exists(const AudioResourceId& resourceId) const override;

    Ret registerPlugin(const AudioPluginInfo& info) override;
    Ret unregisterPlugin(const AudioResourceId& resourceId) override;

private:
    mu::io::path_t pluginInfoPath(const AudioResourceVendor& vendor, const AudioResourceId& resourceId) const;

    std::unordered_map<AudioResourceId, AudioPluginInfo> m_pluginInfoMap;
    std::set<io::path_t> m_pluginPaths;
};
}

#endif // MU_AUDIO_KNOWNAUDIOPLUGINSREGISTER_H
