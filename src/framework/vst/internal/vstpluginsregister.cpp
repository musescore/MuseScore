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

#include "vstpluginsregister.h"

#include "log.h"

#include "vstplugin.h"

using namespace muse::vst;
using namespace muse::audio;

void VstPluginsRegister::registerInstrPlugin(const muse::audio::TrackId trackId, VstPluginPtr pluginPtr)
{
    ONLY_AUDIO_THREAD(threadSecurer);

    std::lock_guard lock(m_mutex);

    m_vstiPluginsMap.insert_or_assign(trackId, pluginPtr);
}

void VstPluginsRegister::registerFxPlugin(const muse::audio::TrackId trackId, const muse::audio::AudioResourceId& resourceId,
                                          const AudioFxChainOrder chainOrder, VstPluginPtr pluginPtr)
{
    ONLY_AUDIO_THREAD(threadSecurer);

    std::lock_guard lock(m_mutex);

    VstFxInstancesMap& instancesMap = m_vstFxPluginsMap[trackId];
    instancesMap.insert_or_assign(std::make_pair(resourceId, chainOrder), pluginPtr);
}

void VstPluginsRegister::registerMasterFxPlugin(const muse::audio::AudioResourceId& resourceId, const AudioFxChainOrder chainOrder,
                                                VstPluginPtr pluginPtr)
{
    ONLY_AUDIO_THREAD(threadSecurer);

    std::lock_guard lock(m_mutex);

    m_masterPluginsMap.insert_or_assign(std::make_pair(resourceId, chainOrder), pluginPtr);
}

VstPluginPtr VstPluginsRegister::instrumentPlugin(const muse::audio::TrackId trackId, const muse::audio::AudioResourceId& resourceId) const
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

VstPluginPtr VstPluginsRegister::fxPlugin(const muse::audio::TrackId trackId, const muse::audio::AudioResourceId& resourceId,
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

VstPluginPtr VstPluginsRegister::masterFxPlugin(const muse::audio::AudioResourceId& resourceId, const AudioFxChainOrder chainOrder) const
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

void VstPluginsRegister::unregisterInstrPlugin(const muse::audio::TrackId trackId, const muse::audio::AudioResourceId& resourceId)
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

void VstPluginsRegister::unregisterFxPlugin(const muse::audio::TrackId trackId, const muse::audio::AudioResourceId& resourceId,
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

void VstPluginsRegister::unregisterMasterFxPlugin(const muse::audio::AudioResourceId& resourceId, const AudioFxChainOrder chainOrder)
{
    ONLY_AUDIO_THREAD(threadSecurer);

    std::lock_guard lock(m_mutex);

    m_masterPluginsMap.erase({ resourceId, chainOrder });
}

void VstPluginsRegister::unregisterAllInstrPlugin()
{
    ONLY_MAIN_THREAD(threadSecurer);

    std::lock_guard lock(m_mutex);

    m_vstiPluginsMap.clear();
}

void VstPluginsRegister::unregisterAllFx()
{
    ONLY_MAIN_THREAD(threadSecurer);

    std::lock_guard lock(m_mutex);

    m_vstFxPluginsMap.clear();
    m_masterPluginsMap.clear();
}
