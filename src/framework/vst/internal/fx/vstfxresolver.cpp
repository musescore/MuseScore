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

#include "log.h"

using namespace mu::vst;
using namespace mu::audio;
using namespace mu::audio::fx;

bool filterFxParams(const std::pair<AudioFxChainOrder, AudioFxParams>& f,
                    const std::pair<AudioFxChainOrder, AudioFxParams>& s)
{
    return (f.first == s.first) && !(f.second == s.second);
}

std::vector<IFxProcessorPtr> VstFxResolver::resolveFxList(const audio::TrackId trackId, const AudioFxChain& fxChain)
{
    if (fxChain.empty()) {
        LOGE() << "invalid fx chain for trackId: " << trackId;
        return {};
    }

    FxMap& fxMap = m_tracksFxMap[trackId];
    updateTrackFxMap(fxMap, trackId, fxChain);

    std::vector<IFxProcessorPtr> result;

    for (const auto& pair : fxMap) {
        result.emplace_back(pair.second);
    }

    return result;
}

std::vector<IFxProcessorPtr> VstFxResolver::resolveMasterFxList(const AudioFxChain& fxChain)
{
    if (fxChain.empty()) {
        LOGE() << "invalid master fx params";
        return {};
    }

    updateMasterFxMap(fxChain);

    std::vector<IFxProcessorPtr> result;

    for (const auto& pair : m_masterFxMap) {
        result.emplace_back(pair.second);
    }

    return result;
}

AudioResourceMetaList VstFxResolver::resolveResources() const
{
    return pluginModulesRepo()->fxModulesMeta();
}

void VstFxResolver::refresh()
{
    pluginModulesRepo()->refresh();
}

VstFxPtr VstFxResolver::createMasterFx(const audio::AudioFxParams& fxParams) const
{
    PluginModulePtr modulePtr = pluginModulesRepo()->pluginModule(fxParams.resourceMeta.id);

    IF_ASSERT_FAILED(modulePtr) {
        return nullptr;
    }

    VstPluginPtr pluginPtr = std::make_shared<VstPlugin>(modulePtr);
    pluginsRegister()->registerMasterFxPlugin(fxParams.resourceMeta.id, fxParams.chainOrder, pluginPtr);

    std::shared_ptr<VstFxProcessor> fx = std::make_shared<VstFxProcessor>(std::move(pluginPtr), fxParams);
    fx->init();

    return fx;
}

VstFxPtr VstFxResolver::createTrackFx(const audio::TrackId trackId, const audio::AudioFxParams& fxParams) const
{
    PluginModulePtr modulePtr = pluginModulesRepo()->pluginModule(fxParams.resourceMeta.id);

    if (!modulePtr) {
        LOGE() << "Unable to create VST plugin"
               << ", pluginId: " << fxParams.resourceMeta.id
               << ", trackId: " << trackId;
        return nullptr;
    }

    VstPluginPtr pluginPtr = std::make_shared<VstPlugin>(modulePtr);
    pluginsRegister()->registerFxPlugin(trackId, fxParams.resourceMeta.id, fxParams.chainOrder, pluginPtr);

    pluginPtr->load();

    std::shared_ptr<VstFxProcessor> fx = std::make_shared<VstFxProcessor>(std::move(pluginPtr), fxParams);
    fx->init();

    return fx;
}

void VstFxResolver::updateTrackFxMap(FxMap& fxMap, const audio::TrackId trackId, const AudioFxChain& newFxChain)
{
    AudioFxChain currentFxChain;
    for (const auto& pair : fxMap) {
        currentFxChain.emplace(pair.first, pair.second->params());
    }

    audio::AudioFxChain fxToRemove;
    fxChainToRemove(newFxChain, currentFxChain, fxToRemove);

    for (const auto& pair : fxToRemove) {
        pluginsRegister()->unregisterFxPlugin(trackId, pair.second.resourceMeta.id, pair.second.chainOrder);
        fxMap.erase(pair.second.chainOrder);
    }

    audio::AudioFxChain fxToCreate;
    fxChainToCreate(newFxChain, currentFxChain, fxToCreate);

    for (const auto& pair : fxToCreate) {
        if (!pair.second.isValid()) {
            continue;
        }

        VstFxPtr fxPtr = createTrackFx(trackId, pair.second);

        if (fxPtr) {
            fxMap.emplace(pair.first, std::move(fxPtr));
        }
    }
}

void VstFxResolver::updateMasterFxMap(const AudioFxChain& newFxChain)
{
    AudioFxChain currentFxChain;
    for (const auto& pair : m_masterFxMap) {
        currentFxChain.emplace(pair.first, pair.second->params());
    }

    audio::AudioFxChain fxToRemove;
    fxChainToRemove(newFxChain, currentFxChain, fxToRemove);

    for (const auto& pair : fxToRemove) {
        pluginsRegister()->unregisterMasterFxPlugin(pair.second.resourceMeta.id, pair.second.chainOrder);
        m_masterFxMap.erase(pair.second.chainOrder);
    }

    audio::AudioFxChain fxToCreate;
    fxChainToCreate(newFxChain, currentFxChain, fxToCreate);

    for (const auto& pair : fxToCreate) {
        if (!pair.second.isValid()) {
            continue;
        }
        m_masterFxMap.emplace(pair.first, createMasterFx(pair.second));
    }
}

void VstFxResolver::fxChainToRemove(const AudioFxChain& currentFxChain,
                                    const AudioFxChain& newFxChain,
                                    AudioFxChain& resultChain)
{
    std::set_difference(newFxChain.begin(), newFxChain.end(),
                        currentFxChain.begin(), currentFxChain.end(),
                        std::inserter(resultChain, resultChain.begin()),
                        filterFxParams);
}

void VstFxResolver::fxChainToCreate(const AudioFxChain& currentFxChain,
                                    const AudioFxChain& newFxChain,
                                    AudioFxChain& resultChain)
{
    std::set_difference(currentFxChain.begin(), currentFxChain.end(),
                        newFxChain.begin(), newFxChain.end(),
                        std::inserter(resultChain, resultChain.begin()),
                        filterFxParams);
}
