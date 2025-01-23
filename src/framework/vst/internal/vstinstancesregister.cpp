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

#include "vstinstancesregister.h"

#include "log.h"

#include "vstinstance.h"

using namespace muse::vst;
using namespace muse::audio;

IVstInstancePtr VstInstancesRegister::makeAndRegisterInstrPlugin(const muse::audio::TrackId trackId, const AudioResourceId &resourceId)
{
    std::shared_ptr<VstInstance> instance = std::make_shared<VstInstance>(resourceId);
    registerInstrPlugin(trackId, instance);

    instance->load();

    return instance;
}

IVstInstancePtr VstInstancesRegister::makeAndRegisterFxPlugin(const muse::audio::TrackId trackId,
                                                              const muse::audio::AudioResourceId& resourceId,
                                                              const muse::audio::AudioFxChainOrder chainOrder)
{
    std::shared_ptr<VstInstance> instance = std::make_shared<VstInstance>(resourceId);
    registerFxPlugin(trackId, resourceId, chainOrder, instance);

    instance->load();

    return instance;
}

IVstInstancePtr VstInstancesRegister::makeAndRegisterMasterFxPlugin(const muse::audio::AudioResourceId& resourceId,
                                                                    const muse::audio::AudioFxChainOrder chainOrder)
{
    std::shared_ptr<VstInstance> instance = std::make_shared<VstInstance>(resourceId);
    registerMasterFxPlugin(resourceId, chainOrder, instance);

    instance->load();

    return instance;
}

void VstInstancesRegister::registerInstrPlugin(const muse::audio::TrackId trackId, IVstInstancePtr instance)
{
    ONLY_AUDIO_THREAD(threadSecurer);

    std::lock_guard lock(m_mutex);

    m_vstiPluginsMap.insert_or_assign(trackId, instance);
}

void VstInstancesRegister::registerFxPlugin(const muse::audio::TrackId trackId, const muse::audio::AudioResourceId& resourceId,
                                            const AudioFxChainOrder chainOrder, IVstInstancePtr instance)
{
    ONLY_AUDIO_THREAD(threadSecurer);

    std::lock_guard lock(m_mutex);

    VstFxInstancesMap& instancesMap = m_vstFxPluginsMap[trackId];
    instancesMap.insert_or_assign(std::make_pair(resourceId, chainOrder), instance);
}

void VstInstancesRegister::registerMasterFxPlugin(const muse::audio::AudioResourceId& resourceId, const AudioFxChainOrder chainOrder,
                                                  IVstInstancePtr instance)
{
    ONLY_AUDIO_THREAD(threadSecurer);

    std::lock_guard lock(m_mutex);

    m_masterPluginsMap.insert_or_assign(std::make_pair(resourceId, chainOrder), instance);
}

IVstInstancePtr VstInstancesRegister::instrumentPlugin(const muse::audio::TrackId trackId, const muse::audio::AudioResourceId& resourceId) const
{
    ONLY_MAIN_THREAD(threadSecurer);

    std::lock_guard lock(m_mutex);

    auto search = m_vstiPluginsMap.find(trackId);
    if (search != m_vstiPluginsMap.cend()
        && search->second->resourceId() == resourceId) {
        return search->second;
    }

    LOGE() << "Unable to find instrument plugin, trackId: " << trackId
           << " , resourceId: " << resourceId;

    return nullptr;
}

IVstInstancePtr VstInstancesRegister::fxPlugin(const muse::audio::TrackId trackId, const muse::audio::AudioResourceId& resourceId,
                                               const AudioFxChainOrder chainOrder) const
{
    ONLY_MAIN_THREAD(threadSecurer);

    std::lock_guard lock(m_mutex);

    auto mapSearch = m_vstFxPluginsMap.find(trackId);
    if (mapSearch == m_vstFxPluginsMap.end()) {
        LOGE() << "Unable to find fx plugin, trackId: " << trackId
               << " , resourceId: " << resourceId;

        return nullptr;
    }

    const VstFxInstancesMap& instancesMap = mapSearch->second;

    auto pluginSearch = instancesMap.find({ resourceId, chainOrder });
    if (pluginSearch == instancesMap.end()) {
        LOGE() << "Unable to find fx plugin, trackId: " << trackId
               << " , resourceId: " << resourceId;

        return nullptr;
    }

    return pluginSearch->second;
}

IVstInstancePtr VstInstancesRegister::masterFxPlugin(const muse::audio::AudioResourceId& resourceId, const AudioFxChainOrder chainOrder) const
{
    ONLY_MAIN_THREAD(threadSecurer);

    std::lock_guard lock(m_mutex);

    auto pluginSearch = m_masterPluginsMap.find({ resourceId, chainOrder });
    if (pluginSearch == m_masterPluginsMap.end()) {
        LOGE() << "Unable to find master plugin, resourceId: " << resourceId << ", chainOrder: " << chainOrder;

        return nullptr;
    }

    return pluginSearch->second;
}

void VstInstancesRegister::unregisterInstrPlugin(const muse::audio::TrackId trackId, const muse::audio::AudioResourceId& resourceId)
{
    ONLY_AUDIO_THREAD(threadSecurer);

    std::lock_guard lock(m_mutex);

    auto search = m_vstiPluginsMap.find(trackId);
    if (search != m_vstiPluginsMap.end()
        && search->second->resourceId() == resourceId) {
        m_vstiPluginsMap.erase(search);
        return;
    }

    LOGE() << "Unable to find plugin, trackId: " << trackId
           << " , resourceId: " << resourceId;
    return;
}

void VstInstancesRegister::unregisterFxPlugin(const muse::audio::TrackId trackId, const muse::audio::AudioResourceId& resourceId,
                                              const AudioFxChainOrder chainOrder)
{
    ONLY_AUDIO_THREAD(threadSecurer);

    std::lock_guard lock(m_mutex);

    auto mapSearch = m_vstFxPluginsMap.find(trackId);
    if (mapSearch == m_vstFxPluginsMap.end()) {
        LOGE() << "Unable to find plugin, trackId: " << trackId
               << " , resourceId: " << resourceId;
        return;
    }

    VstFxInstancesMap& instancesMap = mapSearch->second;
    instancesMap.erase({ resourceId, chainOrder });
}

void VstInstancesRegister::unregisterMasterFxPlugin(const muse::audio::AudioResourceId& resourceId, const AudioFxChainOrder chainOrder)
{
    ONLY_AUDIO_THREAD(threadSecurer);

    std::lock_guard lock(m_mutex);

    m_masterPluginsMap.erase({ resourceId, chainOrder });
}

void VstInstancesRegister::unregisterAllInstrPlugin()
{
    ONLY_MAIN_THREAD(threadSecurer);

    std::lock_guard lock(m_mutex);

    m_vstiPluginsMap.clear();
}

void VstInstancesRegister::unregisterAllFx()
{
    ONLY_MAIN_THREAD(threadSecurer);

    std::lock_guard lock(m_mutex);

    m_vstFxPluginsMap.clear();
    m_masterPluginsMap.clear();
}
