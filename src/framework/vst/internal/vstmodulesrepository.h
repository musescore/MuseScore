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

#include "ivstmodulesrepository.h"

#include "modularity/ioc.h"
#include "audio/iknownaudiopluginsregister.h"
#include "audio/iaudiothreadsecurer.h"

#include "audio/audiotypes.h"
#include "vsttypes.h"

namespace mu::vst {
class VstModulesRepository : public IVstModulesRepository
{
    INJECT(vst, audio::IKnownAudioPluginsRegister, knownPlugins)
    INJECT_STATIC(vst, audio::IAudioThreadSecurer, threadSecurer)

public:
    VstModulesRepository() = default;

    void init();
    void deInit();

    bool exists(const audio::AudioResourceId& resourceId) const override;
    PluginModulePtr pluginModule(const audio::AudioResourceId& resourceId) const override;
    void addPluginModule(const audio::AudioResourceId& resourceId) override;
    void removePluginModule(const audio::AudioResourceId& resourceId) override;

    audio::AudioResourceMetaList instrumentModulesMeta() const override;
    audio::AudioResourceMetaList fxModulesMeta() const override;
    void refresh() override;

private:
    audio::AudioResourceMetaList modulesMetaList(const audio::AudioPluginType& type) const;

    PluginContext m_pluginContext;

    mutable std::mutex m_mutex;
    mutable std::unordered_map<audio::AudioResourceId, PluginModulePtr> m_modules;
};
}

#endif // MU_VST_VSTMODULESREPOSITORY_H
