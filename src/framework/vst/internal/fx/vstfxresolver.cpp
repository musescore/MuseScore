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

std::vector<IFxProcessorPtr> VstFxResolver::resolveFxList(const audio::TrackId trackId, const std::vector<audio::AudioFxParams>& fxParams)
{
    if (fxParams.empty()) {
        LOGE() << "invalid fx params for trackId: " << trackId;
        return {};
    }

    AudioResourceIdList newResourcesidList;

    for (const auto& param : fxParams) {
        newResourcesidList.emplace_back(param.resourceId);
    }

    FxMap& fxMap = m_tracksFxMap[trackId];
    updateTrackFxMap(fxMap, trackId, newResourcesidList);

    std::vector<IFxProcessorPtr> result;

    for (const auto& pair : m_masterFxMap) {
        result.emplace_back(pair.second);
    }

    return result;
}

std::vector<IFxProcessorPtr> VstFxResolver::resolveMasterFxList(const std::vector<audio::AudioFxParams>& fxParams)
{
    if (fxParams.empty()) {
        LOGE() << "invalid master fx params";
        return {};
    }

    AudioResourceIdList newResourcesidList;

    for (const auto& param : fxParams) {
        newResourcesidList.emplace_back(param.resourceId);
    }

    updateMasterFxMap(newResourcesidList);

    std::vector<IFxProcessorPtr> result;

    for (const auto& pair : m_masterFxMap) {
        result.emplace_back(pair.second);
    }

    return result;
}

AudioResourceIdList VstFxResolver::resolveResources() const
{
    return pluginModulesRepo()->moduleIdList(VstPluginType::Fx);
}

void VstFxResolver::refresh()
{
    pluginModulesRepo()->refresh();
}

VstFxPtr VstFxResolver::createMasterFx(const audio::AudioResourceId& resourceId) const
{
    PluginModulePtr modulePtr = pluginModulesRepo()->pluginModule(resourceId);

    IF_ASSERT_FAILED(modulePtr) {
        return nullptr;
    }

    VstPluginPtr pluginPtr = std::make_shared<VstPlugin>(modulePtr);
    pluginsRegister()->registerMasterFxPlugin(resourceId, pluginPtr);

    std::shared_ptr<VstFxProcessor> fx = std::make_shared<VstFxProcessor>(std::move(pluginPtr));
    fx->init();

    return fx;
}

VstFxPtr VstFxResolver::createTrackFx(const audio::TrackId trackId, const audio::AudioResourceId& resourceId) const
{
    PluginModulePtr modulePtr = pluginModulesRepo()->pluginModule(resourceId);

    IF_ASSERT_FAILED(modulePtr) {
        return nullptr;
    }

    VstPluginPtr pluginPtr = std::make_shared<VstPlugin>(modulePtr);
    pluginsRegister()->registerFxPlugin(trackId, resourceId, pluginPtr);

    std::shared_ptr<VstFxProcessor> fx = std::make_shared<VstFxProcessor>(std::move(pluginPtr));
    fx->init();

    return fx;
}

void VstFxResolver::updateTrackFxMap(FxMap& fxMap, const audio::TrackId trackId, const audio::AudioResourceIdList& newResourceIdList)
{
    AudioResourceIdList currentResourceIdList;
    for (const auto& pair : fxMap) {
        currentResourceIdList.emplace_back(pair.first);
    }

    audio::AudioResourceIdList idListToRemove;
    fxListToRemove(newResourceIdList, currentResourceIdList, idListToRemove);

    for (const auto& idToRemove : idListToRemove) {
        pluginsRegister()->unregisterFxPlugin(trackId, idToRemove);
        fxMap.erase(idToRemove);
    }

    audio::AudioResourceIdList idListToCreate;
    fxListToCreate(newResourceIdList, currentResourceIdList, idListToCreate);

    for (const auto& idToCreate : idListToCreate) {
        fxMap.emplace(idToCreate, createTrackFx(trackId, idToCreate));
    }
}

void VstFxResolver::updateMasterFxMap(const audio::AudioResourceIdList& newResourceIdList)
{
    AudioResourceIdList currentResourceIdList;
    for (const auto& pair : m_masterFxMap) {
        currentResourceIdList.emplace_back(pair.first);
    }

    audio::AudioResourceIdList idListToRemove;
    fxListToRemove(newResourceIdList, currentResourceIdList, idListToRemove);

    for (const auto& idToRemove : idListToRemove) {
        pluginsRegister()->unregisterMasterFxPlugin(idToRemove);
        m_masterFxMap.erase(idToRemove);
    }

    audio::AudioResourceIdList idListToCreate;
    fxListToCreate(newResourceIdList, currentResourceIdList, idListToCreate);

    for (const auto& idToCreate : idListToCreate) {
        m_masterFxMap.emplace(idToCreate, createMasterFx(idToCreate));
    }
}

void VstFxResolver::fxListToRemove(const AudioResourceIdList& currentResourceIdList,
                                   const audio::AudioResourceIdList& newResourceIdList,
                                   audio::AudioResourceIdList& resultList)
{
    std::set_difference(newResourceIdList.begin(), newResourceIdList.end(),
                        currentResourceIdList.begin(), currentResourceIdList.end(),
                        std::inserter(resultList, resultList.begin()));
}

void VstFxResolver::fxListToCreate(const AudioResourceIdList& currentResourceIdList,
                                   const audio::AudioResourceIdList& newResourceIdList,
                                   audio::AudioResourceIdList& resultList)
{
    std::set_difference(currentResourceIdList.begin(), currentResourceIdList.end(),
                        newResourceIdList.begin(), newResourceIdList.end(),
                        std::inserter(resultList, resultList.begin()));
}
