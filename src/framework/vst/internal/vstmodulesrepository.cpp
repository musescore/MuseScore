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

#include "vstmodulesrepository.h"

using namespace mu;
using namespace mu::vst;

void VstModulesRepository::init()
{
    ONLY_MAIN_THREAD(threadSecurer);

    PluginContextFactory::instance().setPluginContext(&m_pluginContext);
}

void VstModulesRepository::deInit()
{
    for (auto& pair : m_modules) {
        pair.second.reset();
    }

    PluginContextFactory::instance().setPluginContext(nullptr);
}

bool VstModulesRepository::exists(const audio::AudioResourceId& resourceId) const
{
    ONLY_AUDIO_THREAD(threadSecurer);

    std::lock_guard lock(m_mutex);

    return knownPlugins()->exists(resourceId);
}

PluginModulePtr VstModulesRepository::pluginModule(const audio::AudioResourceId& resourceId) const
{
    ONLY_AUDIO_OR_MAIN_THREAD(threadSecurer);

    std::lock_guard lock(m_mutex);

    auto search = m_modules.find(resourceId);

    if (search != m_modules.end()) {
        return search->second;
    }

    LOGE() << "Unable to find vst plugin module, resourceId: " << resourceId;

    return nullptr;
}

void VstModulesRepository::addPluginModule(const audio::AudioResourceId& resourceId)
{
    ONLY_MAIN_THREAD(threadSecurer);

    std::lock_guard lock(m_mutex);

    auto search = m_modules.find(resourceId);
    if (search != m_modules.end()) {
        return;
    }

    PluginModulePtr module = createModule(knownPlugins()->pluginPath(resourceId));
    if (!module) {
        return;
    }

    m_modules.emplace(resourceId, std::move(module));
}

void VstModulesRepository::removePluginModule(const audio::AudioResourceId& resourceId)
{
    ONLY_MAIN_THREAD(threadSecurer);

    std::lock_guard lock(m_mutex);

    auto search = m_modules.find(resourceId);
    if (search == m_modules.end()) {
        return;
    }

    m_modules.erase(search);
}

audio::AudioResourceMetaList VstModulesRepository::instrumentModulesMeta() const
{
    ONLY_AUDIO_THREAD(threadSecurer);

    std::lock_guard lock(m_mutex);

    return modulesMetaList(audio::AudioPluginType::Instrument);
}

audio::AudioResourceMetaList VstModulesRepository::fxModulesMeta() const
{
    ONLY_AUDIO_THREAD(threadSecurer);

    std::lock_guard lock(m_mutex);

    return modulesMetaList(audio::AudioPluginType::Fx);
}

void VstModulesRepository::refresh()
{
}

audio::AudioResourceMetaList VstModulesRepository::modulesMetaList(const audio::AudioPluginType& type) const
{
    auto infoAccepted = [type](const audio::AudioPluginInfo& info) {
        return info.type == type && info.meta.type == audio::AudioResourceType::VstPlugin && info.enabled;
    };

    std::vector<audio::AudioPluginInfo> infoList = knownPlugins()->pluginInfoList(infoAccepted);
    audio::AudioResourceMetaList result;

    for (const audio::AudioPluginInfo& info : infoList) {
        result.push_back(info.meta);
    }

    return result;
}
