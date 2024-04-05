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

#ifndef MUSE_VST_VSTPLUGINSREGISTER_H
#define MUSE_VST_VSTPLUGINSREGISTER_H

#include <map>
#include <mutex>

#include "modularity/ioc.h"
#include "audio/iaudiothreadsecurer.h"

#include "ivstpluginsregister.h"
#include "async/asyncable.h"

namespace muse::vst {
class VstPluginsRegister : public IVstPluginsRegister, public async::Asyncable
{
    INJECT_STATIC(muse::audio::IAudioThreadSecurer, threadSecurer)
public:
    void registerInstrPlugin(const muse::audio::TrackId trackId, VstPluginPtr pluginPtr) override;
    void registerFxPlugin(const muse::audio::TrackId trackId, const muse::audio::AudioResourceId& resourceId,
                          const muse::audio::AudioFxChainOrder chainOrder, VstPluginPtr pluginPtr) override;
    void registerMasterFxPlugin(const muse::audio::AudioResourceId& resourceId, const muse::audio::AudioFxChainOrder chainOrder,
                                VstPluginPtr pluginPtr) override;

    VstPluginPtr instrumentPlugin(const muse::audio::TrackId trackId, const muse::audio::AudioResourceId& resourceId) const override;
    VstPluginPtr fxPlugin(const muse::audio::TrackId trackId, const muse::audio::AudioResourceId& resourceId,
                          const muse::audio::AudioFxChainOrder chainOrder) const override;
    VstPluginPtr masterFxPlugin(const muse::audio::AudioResourceId& resourceId,
                                const muse::audio::AudioFxChainOrder chainOrder) const override;

    void unregisterInstrPlugin(const muse::audio::TrackId trackId, const muse::audio::AudioResourceId& resourceId) override;
    void unregisterFxPlugin(const muse::audio::TrackId trackId, const muse::audio::AudioResourceId& resourceId,
                            const muse::audio::AudioFxChainOrder chainOrder) override;
    void unregisterMasterFxPlugin(const muse::audio::AudioResourceId& resourceId, const muse::audio::AudioFxChainOrder chainOrder) override;

    void unregisterAllInstrPlugin() override;
    void unregisterAllFx() override;

private:
    mutable std::mutex m_mutex;

    using FxPluginKey = std::pair<muse::audio::AudioResourceId, muse::audio::AudioFxChainOrder>;
    using VstFxInstancesMap = std::map<FxPluginKey, VstPluginPtr>;

    std::map<muse::audio::TrackId, VstPluginPtr> m_vstiPluginsMap;
    std::map<muse::audio::TrackId, VstFxInstancesMap> m_vstFxPluginsMap;
    VstFxInstancesMap m_masterPluginsMap;
};
}

#endif // MUSE_VST_VSTPLUGINSREGISTER_H
