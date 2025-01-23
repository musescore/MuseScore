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

#include "vstfxresolver.h"

#include "vstfxprocessor.h"

#include "log.h"

using namespace muse::vst;
using namespace muse::audio;
using namespace muse::audio::fx;

AudioResourceMetaList VstFxResolver::resolveResources() const
{
    return pluginModulesRepo()->fxModulesMeta();
}

void VstFxResolver::refresh()
{
    pluginModulesRepo()->refresh();
}

void VstFxResolver::clearAllFx()
{
    instancesRegister()->unregisterAllFx();

    AbstractFxResolver::clearAllFx();
}

IFxProcessorPtr VstFxResolver::createMasterFx(const AudioFxParams& fxParams) const
{
    if (!pluginModulesRepo()->exists(fxParams.resourceMeta.id)) {
        return nullptr;
    }

    IVstPluginInstancePtr pluginPtr = instancesRegister()->makeAndRegisterMasterFxPlugin(fxParams.resourceMeta.id, fxParams.chainOrder);

    std::shared_ptr<VstFxProcessor> fx = std::make_shared<VstFxProcessor>(std::move(pluginPtr), fxParams);
    fx->init();

    return fx;
}

IFxProcessorPtr VstFxResolver::createTrackFx(const TrackId trackId, const AudioFxParams& fxParams) const
{
    if (!pluginModulesRepo()->exists(fxParams.resourceMeta.id)) {
        LOGE() << "Unable to create VST plugin"
               << ", pluginId: " << fxParams.resourceMeta.id
               << ", trackId: " << trackId;
        return nullptr;
    }

    IVstPluginInstancePtr pluginPtr = instancesRegister()->makeAndRegisterFxPlugin(fxParams.resourceMeta.id, trackId, fxParams.chainOrder);

    std::shared_ptr<VstFxProcessor> fx = std::make_shared<VstFxProcessor>(std::move(pluginPtr), fxParams);
    fx->init();

    return fx;
}

void VstFxResolver::removeMasterFx(const AudioResourceId& resoureId, AudioFxChainOrder chainOrder)
{
    instancesRegister()->unregisterMasterFxPlugin(resoureId, chainOrder);
}

void VstFxResolver::removeTrackFx(const TrackId trackId, const AudioResourceId& resoureId, AudioFxChainOrder chainOrder)
{
    instancesRegister()->unregisterFxPlugin(resoureId, trackId, chainOrder);
}
