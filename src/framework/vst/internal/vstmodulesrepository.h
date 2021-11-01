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

#ifndef MU_VST_VSTMODULESREPOSITORY_H
#define MU_VST_VSTMODULESREPOSITORY_H

#include <unordered_map>
#include <mutex>

#include "modularity/ioc.h"
#include "system/ifilesystem.h"
#include "audio/audiotypes.h"
#include "audio/iaudiothreadsecurer.h"

#include "ivstmodulesrepository.h"
#include "ivstconfiguration.h"
#include "vsttypes.h"

namespace mu::vst {
class VstModulesRepository : public IVstModulesRepository
{
    INJECT(vst, IVstConfiguration, configuration)
    INJECT(vst, system::IFileSystem, fileSystem)
    INJECT_STATIC(vst, audio::IAudioThreadSecurer, threadSecurer)

public:
    VstModulesRepository() = default;

    void init();
    void deInit();

    PluginModulePtr pluginModule(const audio::AudioResourceId& resourceId) const override;

    audio::AudioResourceMetaList instrumentModulesMeta() const override;
    audio::AudioResourceMetaList fxModulesMeta() const override;
    void refresh() override;

private:
    void addModule(const io::path& path);
    audio::AudioResourceMetaList modulesMetaList(const VstPluginType& type) const;

    RetVal<io::paths> pluginPathsFromCustomLocation(const io::path& customPath) const;
    PluginModule::PathList pluginPathsFromDefaultLocation() const;

    PluginContext m_pluginContext;

    mutable std::mutex m_mutex;
    std::unordered_map<audio::AudioResourceId, PluginModulePtr> m_modules;
};
}

#endif // MU_VST_VSTMODULESREPOSITORY_H
