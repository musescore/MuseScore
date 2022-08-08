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
    if (!pluginModulesRepo()->exists(fxParams.resourceMeta.id)) {
        return nullptr;
    }

    VstPluginPtr pluginPtr = std::make_shared<VstPlugin>(fxParams.resourceMeta.id);
    pluginsRegister()->registerMasterFxPlugin(fxParams.resourceMeta.id, fxParams.chainOrder, pluginPtr);

    pluginPtr->load();

    std::shared_ptr<VstFxProcessor> fx = std::make_shared<VstFxProcessor>(std::move(pluginPtr), fxParams);
    fx->init();

    return fx;
}

VstFxPtr VstFxResolver::createTrackFx(const audio::TrackId trackId, const audio::AudioFxParams& fxParams) const
{
    if (!pluginModulesRepo()->exists(fxParams.resourceMeta.id)) {
        LOGE() << "Unable to create VST plugin"
               << ", pluginId: " << fxParams.resourceMeta.id
               << ", trackId: " << trackId;
        return nullptr;
    }

    VstPluginPtr pluginPtr = std::make_shared<VstPlugin>(fxParams.resourceMeta.id);
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
    fxChainToRemove(currentFxChain, newFxChain, fxToRemove);

    for (const auto& pair : fxToRemove) {
        pluginsRegister()->unregisterFxPlugin(trackId, pair.second.resourceMeta.id, pair.second.chainOrder);
        currentFxChain.erase(pair.first);
        fxMap.erase(pair.first);
    }

    audio::AudioFxChain fxToCreate;
    fxChainToCreate(currentFxChain, newFxChain, fxToCreate);

    for (const auto& pair : fxToCreate) {
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
    fxChainToRemove(currentFxChain, newFxChain, fxToRemove);

    for (const auto& pair : fxToRemove) {
        pluginsRegister()->unregisterMasterFxPlugin(pair.second.resourceMeta.id, pair.second.chainOrder);
        currentFxChain.erase(pair.first);
        m_masterFxMap.erase(pair.first);
    }

    audio::AudioFxChain fxToCreate;
    fxChainToCreate(currentFxChain, newFxChain, fxToCreate);

    for (const auto& pair : fxToCreate) {
        m_masterFxMap.emplace(pair.first, createMasterFx(pair.second));
    }
}

void VstFxResolver::fxChainToRemove(const AudioFxChain& currentFxChain,
                                    const AudioFxChain& newFxChain,
                                    AudioFxChain& resultChain)
{
    for (auto it = currentFxChain.cbegin(); it != currentFxChain.cend(); ++it) {
        auto newIt = newFxChain.find(it->first);

        if (newIt == newFxChain.cend()) {
            resultChain.insert({ it->first, it->second });
            continue;
        }

        if (it->second != newIt->second) {
            resultChain.insert({ it->first, it->second });
        }
    }
}

void VstFxResolver::fxChainToCreate(const AudioFxChain& currentFxChain,
                                    const AudioFxChain& newFxChain,
                                    AudioFxChain& resultChain)
{
    for (auto it = newFxChain.cbegin(); it != newFxChain.cend(); ++it) {
        if (currentFxChain.find(it->first) != currentFxChain.cend()) {
            continue;
        }

        if (it->second.isValid()) {
            resultChain.insert({ it->first, it->second });
        }
    }
}
