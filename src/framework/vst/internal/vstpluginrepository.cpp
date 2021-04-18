//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2021 MuseScore BVBA and others
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
//=============================================================================

#include "vstpluginrepository.h"

#include "vstplugin.h"
#include "log.h"

using namespace mu;
using namespace io;
using namespace vst;

//@see https://developer.steinberg.help/pages/viewpage.action?pageId=9798275
static const std::string VST3_PACKAGE_EXTENSION = "*.vst3";

Ret VstPluginRepository::loadAvailablePlugins()
{
    Ret result(true);

    for (const std::string& pluginPath : pluginPathsFromDefaultLocation()) {
        result = instantiatePlugin(pluginPath);

        if (!result) {
            return result;
        }
    }

    for (const io::path& pluginPath : pluginPathsFromCustomLocation(configuration()->customSearchPath()).val) {
        result = instantiatePlugin(pluginPath.toStdString());

        if (!result) {
            return result;
        }
    }

    m_pluginMetaListChannel.send(availablePluginsMetaList());

    return result;
}

RetVal<VstPluginPtr> VstPluginRepository::findPluginById(const PluginId& id) const
{
    RetVal<VstPluginPtr> result;
    result.ret = make_ret(Ret::Code::Ok);

    auto it = m_pluginsMap.find(id);
    if (it != m_pluginsMap.end()) {
        result.val = it->second;
    } else {
        result.ret = make_ret(Err::NoPluginWithId);
        LOGE() << result.ret.text()
               << "\nplugin id: "
               << id;
    }

    return result;
}

RetValCh<VstPluginMetaList> VstPluginRepository::pluginsMetaList() const
{
    RetValCh<VstPluginMetaList> result;

    result.ret = make_ret(Ret::Code::Ok);
    result.val = availablePluginsMetaList();
    result.ch = m_pluginMetaListChannel;

    return result;
}

RetVal<io::paths> VstPluginRepository::pluginPathsFromCustomLocation(const io::path& customPath) const
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
PluginModule::PathList VstPluginRepository::pluginPathsFromDefaultLocation() const
{
    return PluginModule::getModulePaths();
}

Ret VstPluginRepository::instantiatePlugin(const std::string& path)
{
    Ret result(true);

    VstPluginPtr plugin = std::make_shared<VstPlugin>();

    result = plugin->load(path);

    if (!result) {
        return result;
    }

    m_pluginsMap.insert(std::make_pair(plugin->id(), plugin));

    return result;
}

VstPluginMetaList VstPluginRepository::availablePluginsMetaList() const
{
    VstPluginMetaList result;

    for (auto& pluginElement : m_pluginsMap) {
        VstPluginPtr pluginPtr = pluginElement.second;

        if (pluginPtr->isValid()) {
            result.push_back(pluginPtr->meta());
        }
    }

    return result;
}
