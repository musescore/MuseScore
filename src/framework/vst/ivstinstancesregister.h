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

#include "modularity/imoduleinterface.h"
#include "audio/audiotypes.h"

#include "vsttypes.h"
#include "ivstinstance.h"

namespace muse::vst {
class IVstInstancesRegister : MODULE_EXPORT_INTERFACE
{
    INTERFACE_ID(IVstInstancesRegister)
public:
    virtual ~IVstInstancesRegister() = default;

    virtual IVstInstancePtr makeAndRegisterInstrPlugin(const muse::audio::TrackId trackId,
                                                       const muse::audio::AudioResourceId& resourceId) = 0;
    virtual IVstInstancePtr makeAndRegisterFxPlugin(const muse::audio::TrackId trackId,
                                                    const muse::audio::AudioResourceId& resourceId,
                                                    const muse::audio::AudioFxChainOrder chainOrder) = 0;
    virtual IVstInstancePtr makeAndRegisterMasterFxPlugin(const muse::audio::AudioResourceId& resourceId,
                                                          const muse::audio::AudioFxChainOrder chainOrder) = 0;

    virtual void registerInstrPlugin(const muse::audio::TrackId trackId, IVstInstancePtr instance) = 0;
    virtual void registerFxPlugin(const muse::audio::TrackId trackId,
                                  const muse::audio::AudioResourceId& resourceId,
                                  const muse::audio::AudioFxChainOrder chainOrder,
                                  IVstInstancePtr instance) = 0;

    virtual void registerMasterFxPlugin(const muse::audio::AudioResourceId& resourceId,
                                        const muse::audio::AudioFxChainOrder chainOrder,
                                        IVstInstancePtr instance) = 0;

    virtual IVstInstancePtr instrumentPlugin(const muse::audio::TrackId trackId, const muse::audio::AudioResourceId& resourceId) const = 0;
    virtual IVstInstancePtr fxPlugin(const muse::audio::TrackId trackId,
                                     const muse::audio::AudioResourceId& resourceId,
                                     const muse::audio::AudioFxChainOrder chainOrder) const = 0;
    virtual IVstInstancePtr masterFxPlugin(const muse::audio::AudioResourceId& resourceId,
                                           const muse::audio::AudioFxChainOrder chainOrder) const = 0;

    virtual void unregisterInstrPlugin(const muse::audio::TrackId trackId, const muse::audio::AudioResourceId& resourceId) = 0;
    virtual void unregisterFxPlugin(const muse::audio::TrackId trackId, const muse::audio::AudioResourceId& resourceId,
                                    const muse::audio::AudioFxChainOrder chainOrder) = 0;
    virtual void unregisterMasterFxPlugin(const muse::audio::AudioResourceId& resourceId,
                                          const muse::audio::AudioFxChainOrder chainOrder) = 0;

    virtual void unregisterAllInstrPlugin() = 0;
    virtual void unregisterAllFx() = 0;
};
}
