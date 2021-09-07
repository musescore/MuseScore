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

using namespace mu::vst;
using namespace mu::audio;

void VstPluginsRegister::registerInstrPlugin(const audio::TrackId trackId, const audio::AudioResourceId& resourceId, VstPluginPtr pluginPtr)
{
    ONLY_AUDIO_THREAD(threadSecurer);

    std::lock_guard lock(m_mutex);

    VstiInstancesMap& instancesMap = m_vstiPluginsMap[trackId];
    instancesMap.emplace(resourceId, pluginPtr);
}

void VstPluginsRegister::registerFxPlugin(const audio::TrackId trackId, const audio::AudioResourceId& resourceId,
                                          const AudioFxChainOrder chainOrder, VstPluginPtr pluginPtr)
{
    ONLY_AUDIO_THREAD(threadSecurer);

    std::lock_guard lock(m_mutex);

    VstFxInstancesMap& instancesMap = m_vstFxPluginsMap[trackId];
    instancesMap.emplace(std::make_pair(resourceId, chainOrder), pluginPtr);
}

void VstPluginsRegister::registerMasterFxPlugin(const audio::AudioResourceId& resourceId, const AudioFxChainOrder chainOrder,
                                                VstPluginPtr pluginPtr)
{
    ONLY_AUDIO_THREAD(threadSecurer);

    std::lock_guard lock(m_mutex);

    m_masterPluginsMap.emplace(std::make_pair(resourceId, chainOrder), pluginPtr);
}

VstPluginPtr VstPluginsRegister::instrumentPlugin(const audio::TrackId trackId, const audio::AudioResourceId& resourceId) const
{
    ONLY_MAIN_THREAD(threadSecurer);

    std::lock_guard lock(m_mutex);

    auto mapSearch = m_vstiPluginsMap.find(trackId);
    if (mapSearch == m_vstiPluginsMap.end()) {
        LOGE() << "Unable to find intrument plugin, trackId: " << trackId
               << " , resourceId: " << resourceId;

        return nullptr;
    }

    const VstiInstancesMap& instancesMap = mapSearch->second;

    auto pluginSearch = instancesMap.find(resourceId);
    if (pluginSearch == instancesMap.end()) {
        LOGE() << "Unable to find instrument plugin, trackId: " << trackId
               << " , resourceId: " << resourceId;

        return nullptr;
    }

    return pluginSearch->second;
}

VstPluginPtr VstPluginsRegister::fxPlugin(const audio::TrackId trackId, const audio::AudioResourceId& resourceId,
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

VstPluginPtr VstPluginsRegister::masterFxPlugin(const audio::AudioResourceId& resourceId, const AudioFxChainOrder chainOrder) const
{
    ONLY_MAIN_THREAD(threadSecurer);

    std::lock_guard lock(m_mutex);

    auto pluginSearch = m_masterPluginsMap.find({ resourceId, chainOrder });
    if (pluginSearch == m_masterPluginsMap.end()) {
        LOGE() << "Unable to find master plugin, resourceId: " << resourceId;

        return nullptr;
    }

    return pluginSearch->second;
}

void VstPluginsRegister::unregisterInstrPlugin(const audio::TrackId trackId, const audio::AudioResourceId& resourceId)
{
    ONLY_AUDIO_THREAD(threadSecurer);

    std::lock_guard lock(m_mutex);

    auto mapSearch = m_vstiPluginsMap.find(trackId);
    if (mapSearch == m_vstiPluginsMap.end()) {
        LOGE() << "Unable to find plugin, trackId: " << trackId
               << " , resourceId: " << resourceId;
        return;
    }

    VstiInstancesMap& instancesMap = mapSearch->second;
    instancesMap.erase(resourceId);
}

void VstPluginsRegister::unregisterFxPlugin(const audio::TrackId trackId, const audio::AudioResourceId& resourceId,
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

void VstPluginsRegister::unregisterMasterFxPlugin(const audio::AudioResourceId& resourceId, const AudioFxChainOrder chainOrder)
{
    ONLY_AUDIO_THREAD(threadSecurer);

    std::lock_guard lock(m_mutex);

    m_masterPluginsMap.erase({ resourceId, chainOrder });
}
