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

namespace muse::vst {
class VstInstancesRegister : public IVstInstancesRegister, public async::Asyncable
{
public:

    // make
    IVstPluginInstancePtr makeAndRegisterInstrPlugin(const muse::audio::AudioResourceId& resourceId,
                                                     const muse::audio::TrackId trackId) override;

    IVstPluginInstancePtr makeAndRegisterFxPlugin(const muse::audio::AudioResourceId& resourceId, const muse::audio::TrackId trackId,
                                                  const muse::audio::AudioFxChainOrder chainOrder) override;

    IVstPluginInstancePtr makeAndRegisterMasterFxPlugin(const muse::audio::AudioResourceId& resourceId,
                                                        const muse::audio::AudioFxChainOrder chainOrder) override;

    // register
    void registerInstrPlugin(const muse::audio::TrackId trackId, IVstPluginInstancePtr instance) override;

    void registerFxPlugin(const muse::audio::TrackId trackId, const muse::audio::AudioFxChainOrder chainOrder,
                          IVstPluginInstancePtr instance) override;

    void registerMasterFxPlugin(const muse::audio::AudioFxChainOrder chainOrder, IVstPluginInstancePtr instance) override;

    // get
    IVstPluginInstancePtr instanceById(const VstPluginInstanceId id) const override;
    IVstPluginInstancePtr instrumentPlugin(const muse::audio::AudioResourceId& resourceId,
                                           const muse::audio::TrackId trackId) const override;
    IVstPluginInstancePtr fxPlugin(const muse::audio::AudioResourceId& resourceId, const muse::audio::TrackId trackId,
                                   const muse::audio::AudioFxChainOrder chainOrder) const override;
    IVstPluginInstancePtr masterFxPlugin(const muse::audio::AudioResourceId& resourceId,
                                         const muse::audio::AudioFxChainOrder chainOrder) const override;

    // unregister
    void unregisterById(const VstPluginInstanceId id) override;
    void unregisterInstrPlugin(const muse::audio::AudioResourceId& resourceId, const muse::audio::TrackId trackId) override;
    void unregisterFxPlugin(const muse::audio::AudioResourceId& resourceId, const muse::audio::TrackId trackId,
                            const muse::audio::AudioFxChainOrder chainOrder) override;
    void unregisterMasterFxPlugin(const muse::audio::AudioResourceId& resourceId, const muse::audio::AudioFxChainOrder chainOrder) override;

    void unregisterAllInstrPlugin() override;
    void unregisterAllFx() override;

private:
    mutable std::mutex m_mutex;

    enum class Type {
        Undefined = 0,
        Instrument,
        Effect
    };

    struct Key {
        Type type = Type::Undefined;
        muse::audio::AudioResourceId resourceId;
        muse::audio::TrackId trackId = -1;
        muse::audio::AudioFxChainOrder chainOrder = 0;

        inline bool operator ==(const Key& k) const
        {
            return type == k.type && trackId == k.trackId && resourceId == k.resourceId && chainOrder == k.chainOrder;
        }

        inline bool operator <(const Key& k) const
        {
            if (type != k.type) {
                return type < k.type;
            }

            if (trackId != k.trackId) {
                return trackId < k.trackId;
            }

            if (resourceId != k.resourceId) {
                return resourceId < k.resourceId;
            }

            return chainOrder < k.chainOrder;
        }
    };

    std::map<Key, IVstPluginInstancePtr> m_instances;
};
}
