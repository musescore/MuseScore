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
#ifndef MUSE_VST_IVSTPLUGINSREGISTER_H
#define MUSE_VST_IVSTPLUGINSREGISTER_H

#include "modularity/imoduleinterface.h"
#include "audio/audiotypes.h"

#include "vsttypes.h"

namespace muse::vst {
class IVstPluginsRegister : MODULE_EXPORT_INTERFACE
{
    INTERFACE_ID(IVstPluginsRegister)
public:
    virtual ~IVstPluginsRegister() = default;

    virtual void registerInstrPlugin(const muse::audio::TrackId trackId, VstPluginPtr pluginPtr) = 0;
    virtual void registerFxPlugin(const muse::audio::TrackId trackId, const muse::audio::AudioResourceId& resourceId,
                                  const muse::audio::AudioFxChainOrder chainOrder, VstPluginPtr pluginPtr) = 0;
    virtual void registerMasterFxPlugin(const muse::audio::AudioResourceId& resourceId, const muse::audio::AudioFxChainOrder chainOrder,
                                        VstPluginPtr pluginPtr) = 0;

    virtual VstPluginPtr instrumentPlugin(const muse::audio::TrackId trackId, const muse::audio::AudioResourceId& resourceId) const = 0;
    virtual VstPluginPtr fxPlugin(const muse::audio::TrackId trackId, const muse::audio::AudioResourceId& resourceId,
                                  const muse::audio::AudioFxChainOrder chainOrder) const = 0;
    virtual VstPluginPtr masterFxPlugin(const muse::audio::AudioResourceId& resourceId,
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

#endif // MUSE_VST_IVSTPLUGINSREGISTER_H
