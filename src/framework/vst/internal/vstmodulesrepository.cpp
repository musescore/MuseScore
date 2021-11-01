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

static const std::map<VstPluginType, std::string> PLUGIN_TYPE_MAP = {
    { VstPluginType::Instrument, "Instrument" },
    { VstPluginType::Fx, "Fx" }
};

void VstModulesRepository::init()
{
    ONLY_MAIN_THREAD(threadSecurer);

    PluginContextFactory::instance().setPluginContext(&m_pluginContext);

    refresh();
}

void VstModulesRepository::deInit()
{
    for (auto& pair : m_modules) {
        pair.second.reset();
    }

    PluginContextFactory::instance().setPluginContext(nullptr);
}

PluginModulePtr VstModulesRepository::pluginModule(const audio::AudioResourceId& resourceId) const
{
    ONLY_AUDIO_THREAD(threadSecurer);

    std::lock_guard lock(m_mutex);

    auto search = m_modules.find(resourceId);

    if (search != m_modules.end()) {
        return search->second;
    }

    LOGE() << "Unable to find vst plugin module, resourceId: " << resourceId;

    return nullptr;
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
        addModule(pluginPath);
    }

    for (const io::path& pluginPath : pluginPathsFromCustomLocation(configuration()->customSearchPath()).val) {
        addModule(pluginPath);
    }
}

void VstModulesRepository::addModule(const io::path& path)
{
    std::string errorString;

    PluginModulePtr module = PluginModule::create(path.toStdString(), errorString);

    if (!module) {
        LOGE() << errorString;
        return;
    }

    m_modules.emplace(io::basename(path).toStdString(), module);
}

audio::AudioResourceMetaList VstModulesRepository::modulesMetaList(const VstPluginType& type) const
{
    audio::AudioResourceMetaList result;

    static auto hasNativeEditorSupport = []() {
#ifdef Q_OS_LINUX
        //!Note Host applications on Linux should provide their own event loop via VST3 API,
        //!     otherwise it'll be impossible to launch native VST editor views
        return false;
#else
        return true;
#endif
    };

    for (const auto& pair : m_modules) {
        PluginModulePtr module = pair.second;
        const auto& factory = module->getFactory();

        for (auto& classInfo : factory.classInfos()) {
            if (classInfo.category() != kVstAudioEffectClass) {
                continue;
            }

            std::string subCategoriesStr = classInfo.subCategoriesString();
            if (subCategoriesStr.find(PLUGIN_TYPE_MAP.at(type)) != std::string::npos) {
                audio::AudioResourceMeta meta;
                meta.id = pair.first;
                meta.type = audio::AudioResourceType::VstPlugin;
                meta.vendor = factory.info().vendor();
                meta.hasNativeEditorSupport = hasNativeEditorSupport();
                result.emplace_back(std::move(meta));
                break;
            }
        }
    }

    return result;
}

RetVal<io::paths> VstModulesRepository::pluginPathsFromCustomLocation(const io::path& customPath) const
{
    RetVal<io::paths> pluginPaths = fileSystem()->scanFiles(customPath, QStringList(QString::fromStdString(VST3_PACKAGE_EXTENSION)));

    if (!pluginPaths.ret) {
        return pluginPaths;
    }

    return pluginPaths;
}

/**
 * @brief Scanning for plugins in the default VST locations, considering the current architechture (i386, x86_64, arm, etc.)
 * @see https://developer.steinberg.help/pages/viewpage.action?pageId=9798275
 **/
PluginModule::PathList VstModulesRepository::pluginPathsFromDefaultLocation() const
{
    return PluginModule::getModulePaths();
}
