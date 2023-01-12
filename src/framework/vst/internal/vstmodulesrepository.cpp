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

#include <vector>

#include "log.h"

#include "vstplugin.h"

using namespace mu;
using namespace mu::vst;

//@see https://developer.steinberg.help/pages/viewpage.action?pageId=9798275
static const std::string VST3_PACKAGE_EXTENSION = "*.vst3";

void VstModulesRepository::init()
{
    ONLY_MAIN_THREAD(threadSecurer);

    PluginContextFactory::instance().setPluginContext(&m_pluginContext);

    m_knownPlugins.init();

    configuration()->userVstDirectoriesChanged().onReceive(this, [this](const io::paths_t&) {
        refresh();
    });

    refresh();
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

    return m_knownPlugins.exists(resourceId);
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

    PluginModulePtr module = createModule(m_knownPlugins.pluginPath(resourceId));
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

    return modulesMetaList(VstPluginType::Instrument);
}

audio::AudioResourceMetaList VstModulesRepository::fxModulesMeta() const
{
    ONLY_AUDIO_THREAD(threadSecurer);

    std::lock_guard lock(m_mutex);

    return modulesMetaList(VstPluginType::Fx);
}

void VstModulesRepository::refresh()
{
    ONLY_AUDIO_OR_MAIN_THREAD(threadSecurer);

    TRACEFUNC;

    m_modules.clear();

    for (const std::string& pluginPath : pluginPathsFromDefaultLocation()) {
        if (!m_knownPlugins.exists(io::path_t(pluginPath))) {
            addModule(io::path_t(pluginPath));
        }
    }

    for (const io::path_t& pluginPath : pluginPathsFromCustomLocations(configuration()->userVstDirectories())) {
        if (!m_knownPlugins.exists(pluginPath)) {
            addModule(pluginPath);
        }
    }
}

PluginModulePtr VstModulesRepository::createModule(const io::path_t& path)
{
    std::string errorString;
    PluginModulePtr result = nullptr;

    try {
        result = PluginModule::create(path.toStdString(), errorString);
    }  catch (...) {
        LOGE() << "Unable to load a new VST Module, error string: " << errorString;
    }

    return result;
}

void VstModulesRepository::addModule(const io::path_t& path)
{
    audio::AudioResourceId resourceId = io::basename(path).toStdString();

    m_knownPlugins.registerPath(resourceId, path);

    PluginModulePtr module = createModule(path);
    if (!module) {
        return;
    }

    m_knownPlugins.registerPlugin(resourceId, module);
    m_modules.emplace(resourceId, module);
}

audio::AudioResourceMetaList VstModulesRepository::modulesMetaList(const VstPluginType& type) const
{
    return m_knownPlugins.metaList(type);
}

io::paths_t VstModulesRepository::pluginPathsFromCustomLocations(const io::paths_t& customPaths) const
{
    io::paths_t result;

    for (const io::path_t& path : customPaths) {
        RetVal<io::paths_t> paths = fileSystem()->scanFiles(path, { VST3_PACKAGE_EXTENSION });
        if (!paths.ret) {
            LOGW() << paths.ret.toString();
            continue;
        }

        for (const io::path_t& pluginPath : paths.val) {
            result.push_back(pluginPath);
        }
    }

    return result;
}

/**
 * @brief Scanning for plugins in the default VST locations, considering the current architecture (i386, x86_64, arm, etc.)
 * @see https://developer.steinberg.help/pages/viewpage.action?pageId=9798275
 **/
PluginModule::PathList VstModulesRepository::pluginPathsFromDefaultLocation() const
{
    PluginModule::PathList result;

    try {
        result = PluginModule::getModulePaths();
    } catch (...) {
        LOGE() << "Unable to get module paths";
    }

    return result;
}
