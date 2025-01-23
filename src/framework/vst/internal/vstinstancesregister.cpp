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

#include "global/containers.h"

#include "vstplugininstance.h"

#include "log.h"

using namespace muse::vst;
using namespace muse::audio;

IVstPluginInstancePtr VstInstancesRegister::makeAndRegisterInstrPlugin(const AudioResourceId& resourceId,
                                                                       const muse::audio::TrackId trackId)
{
    std::shared_ptr<VstPluginInstance> instance = std::make_shared<VstPluginInstance>(resourceId);

    registerInstrPlugin(trackId, instance);

    instance->load();

    return instance;
}

IVstPluginInstancePtr VstInstancesRegister::makeAndRegisterFxPlugin(const muse::audio::AudioResourceId& resourceId,
                                                                    const muse::audio::TrackId trackId,
                                                                    const muse::audio::AudioFxChainOrder chainOrder)
{
    std::shared_ptr<VstPluginInstance> instance = std::make_shared<VstPluginInstance>(resourceId);

    registerFxPlugin(trackId, chainOrder, instance);

    instance->load();

    return instance;
}

IVstPluginInstancePtr VstInstancesRegister::makeAndRegisterMasterFxPlugin(const muse::audio::AudioResourceId& resourceId,
                                                                          const muse::audio::AudioFxChainOrder chainOrder)
{
    std::shared_ptr<VstPluginInstance> instance = std::make_shared<VstPluginInstance>(resourceId);

    registerMasterFxPlugin(chainOrder, instance);

    instance->load();

    return instance;
}

void VstInstancesRegister::registerInstrPlugin(const muse::audio::TrackId trackId, IVstPluginInstancePtr instance)
{
    IF_ASSERT_FAILED(instance) {
        return;
    }

    std::lock_guard lock(m_mutex);

    m_instances.insert_or_assign({ Type::Instrument, instance->resourceId(), trackId, 0 }, instance);
}

void VstInstancesRegister::registerFxPlugin(const muse::audio::TrackId trackId,
                                            const AudioFxChainOrder chainOrder,
                                            IVstPluginInstancePtr instance)
{
    IF_ASSERT_FAILED(instance) {
        return;
    }

    std::lock_guard lock(m_mutex);

    m_instances.insert_or_assign({ Type::Effect, instance->resourceId(), trackId, chainOrder }, instance);
}

void VstInstancesRegister::registerMasterFxPlugin(const AudioFxChainOrder chainOrder, IVstPluginInstancePtr instance)
{
    registerFxPlugin(-1, chainOrder, instance);
}

IVstPluginInstancePtr VstInstancesRegister::instanceById(const VstPluginInstanceId id) const
{
    std::lock_guard lock(m_mutex);
    for (const auto& p : m_instances) {
        if (p.second->id() == id) {
            return p.second;
        }
    }
    return nullptr;
}

IVstPluginInstancePtr VstInstancesRegister::instrumentPlugin(const muse::audio::AudioResourceId& resourceId,
                                                             const muse::audio::TrackId trackId) const
{
    std::lock_guard lock(m_mutex);

    auto it = m_instances.find({ Type::Instrument, resourceId, trackId, 0 });
    if (it != m_instances.end()) {
        return it->second;
    }

    LOGE() << "Unable to find instrument plugin, trackId: " << trackId
           << " , resourceId: " << resourceId;

    return nullptr;
}

IVstPluginInstancePtr VstInstancesRegister::fxPlugin(const muse::audio::AudioResourceId& resourceId,
                                                     const muse::audio::TrackId trackId,
                                                     const AudioFxChainOrder chainOrder) const
{
    std::lock_guard lock(m_mutex);

    auto it = m_instances.find({ Type::Effect, resourceId, trackId, chainOrder });
    if (it != m_instances.end()) {
        return it->second;
    }

    LOGE() << "Unable to find fx plugin, trackId: " << trackId
           << ", resourceId: " << resourceId
           << ", chainOrder: " << chainOrder;

    return nullptr;
}

IVstPluginInstancePtr VstInstancesRegister::masterFxPlugin(const muse::audio::AudioResourceId& resourceId,
                                                           const AudioFxChainOrder chainOrder) const
{
    std::lock_guard lock(m_mutex);

    auto it = m_instances.find({ Type::Effect, resourceId, -1, chainOrder });
    if (it != m_instances.end()) {
        return it->second;
    }

    LOGE() << "Unable to find master fx plugin"
           << ", resourceId: " << resourceId
           << ", chainOrder: " << chainOrder;

    return nullptr;
}

void VstInstancesRegister::unregisterById(const VstPluginInstanceId id)
{
    std::lock_guard lock(m_mutex);

    muse::remove_if(m_instances, [id](const auto& p) {
        return p.second->id() == id;
    });
}

void VstInstancesRegister::unregisterInstrPlugin(const muse::audio::AudioResourceId& resourceId, const muse::audio::TrackId trackId)
{
    std::lock_guard lock(m_mutex);

    m_instances.erase({ Type::Instrument, resourceId, trackId, 0 });
}

void VstInstancesRegister::unregisterFxPlugin(const muse::audio::AudioResourceId& resourceId,
                                              const muse::audio::TrackId trackId,
                                              const AudioFxChainOrder chainOrder)
{
    std::lock_guard lock(m_mutex);

    m_instances.erase({ Type::Effect, resourceId, trackId, chainOrder });
}

void VstInstancesRegister::unregisterMasterFxPlugin(const muse::audio::AudioResourceId& resourceId, const AudioFxChainOrder chainOrder)
{
    unregisterFxPlugin(resourceId, -1, chainOrder);
}

void VstInstancesRegister::unregisterAllInstrPlugin()
{
    std::lock_guard lock(m_mutex);

    muse::remove_if(m_instances, [](const auto& p) {
        return p.first.type == Type::Instrument;
    });
}

void VstInstancesRegister::unregisterAllFx()
{
    std::lock_guard lock(m_mutex);

    muse::remove_if(m_instances, [](const auto& p) {
        return p.first.type == Type::Effect;
    });
}
