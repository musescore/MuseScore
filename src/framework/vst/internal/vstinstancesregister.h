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

#include <map>
#include <mutex>

#include "../ivstinstancesregister.h"
#include "async/asyncable.h"

#include "modularity/ioc.h"
#include "audio/iaudiothreadsecurer.h"

namespace muse::vst {
class VstInstancesRegister : public IVstInstancesRegister, public async::Asyncable
{
    muse::GlobalInject<muse::audio::IAudioThreadSecurer> threadSecurer;

public:

    IVstInstancePtr makeAndRegisterInstrPlugin(const muse::audio::TrackId trackId,
                                               const muse::audio::AudioResourceId& resourceId) override;
    IVstInstancePtr makeAndRegisterFxPlugin(const muse::audio::TrackId trackId,
                                            const muse::audio::AudioResourceId& resourceId,
                                            const muse::audio::AudioFxChainOrder chainOrder) override;
    IVstInstancePtr makeAndRegisterMasterFxPlugin(const muse::audio::AudioResourceId& resourceId,
                                                  const muse::audio::AudioFxChainOrder chainOrder) override;

    void registerInstrPlugin(const muse::audio::TrackId trackId, IVstInstancePtr instance) override;
    void registerFxPlugin(const muse::audio::TrackId trackId,
                          const muse::audio::AudioResourceId& resourceId,
                          const muse::audio::AudioFxChainOrder chainOrder,
                          IVstInstancePtr instance) override;
    void registerMasterFxPlugin(const muse::audio::AudioResourceId& resourceId,
                                const muse::audio::AudioFxChainOrder chainOrder,
                                IVstInstancePtr instance) override;

    IVstInstancePtr instrumentPlugin(const muse::audio::TrackId trackId, const muse::audio::AudioResourceId& resourceId) const override;
    IVstInstancePtr fxPlugin(const muse::audio::TrackId trackId, const muse::audio::AudioResourceId& resourceId,
                             const muse::audio::AudioFxChainOrder chainOrder) const override;
    IVstInstancePtr masterFxPlugin(const muse::audio::AudioResourceId& resourceId,
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
    using VstFxInstancesMap = std::map<FxPluginKey, IVstInstancePtr>;

    std::map<muse::audio::TrackId, IVstInstancePtr> m_vstiPluginsMap;
    std::map<muse::audio::TrackId, VstFxInstancesMap> m_vstFxPluginsMap;
    VstFxInstancesMap m_masterPluginsMap;
};
}

