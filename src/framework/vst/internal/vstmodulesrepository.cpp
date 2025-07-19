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

using namespace muse;
using namespace muse::vst;

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

bool VstModulesRepository::exists(const muse::audio::AudioResourceId& resourceId) const
{
    ONLY_AUDIO_THREAD(threadSecurer);

    std::lock_guard lock(m_mutex);

    return knownPlugins()->exists(resourceId);
}

PluginModulePtr VstModulesRepository::pluginModule(const muse::audio::AudioResourceId& resourceId) const
{
    ONLY_AUDIO_OR_MAIN_THREAD(threadSecurer);

    std::lock_guard lock(m_mutex);

    auto search = m_modules.find(resourceId);

    if (search != m_modules.end()) {
        return search->second;
    }

    return nullptr;
}

void VstModulesRepository::addPluginModule(const muse::audio::AudioResourceId& resourceId)
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

void VstModulesRepository::removePluginModule(const muse::audio::AudioResourceId& resourceId)
{
    ONLY_MAIN_THREAD(threadSecurer);

    std::lock_guard lock(m_mutex);

    auto search = m_modules.find(resourceId);
    if (search == m_modules.end()) {
        return;
    }

    m_modules.erase(search);
}

muse::audio::AudioResourceMetaList VstModulesRepository::instrumentModulesMeta() const
{
    ONLY_AUDIO_THREAD(threadSecurer);

    std::lock_guard lock(m_mutex);

    return modulesMetaList(audioplugins::AudioPluginType::Instrument);
}

muse::audio::AudioResourceMetaList VstModulesRepository::fxModulesMeta() const
{
    ONLY_AUDIO_THREAD(threadSecurer);

    std::lock_guard lock(m_mutex);

    return modulesMetaList(audioplugins::AudioPluginType::Fx);
}

void VstModulesRepository::refresh()
{
}

muse::audio::AudioResourceMetaList VstModulesRepository::modulesMetaList(const audioplugins::AudioPluginType& type) const
{
    auto infoAccepted = [type](const audioplugins::AudioPluginInfo& info) {
        return info.type == type && info.meta.type == muse::audio::AudioResourceType::VstPlugin && info.enabled;
    };

    std::vector<audioplugins::AudioPluginInfo> infoList = knownPlugins()->pluginInfoList(infoAccepted);
    muse::audio::AudioResourceMetaList result;

    for (const audioplugins::AudioPluginInfo& info : infoList) {
        result.push_back(info.meta);
    }

    return result;
}
