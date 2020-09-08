//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2020 MuseScore BVBA and others
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

#include "pluginsservice.h"

#include "log.h"

using namespace mu::plugins;
using namespace mu::framework;
using namespace mu::async;

void PluginsService::init()
{
    ValCh<CodeKeyList> installedPluginsCh = configuration()->installedPlugins();
    startPlugins(installedPluginsCh.val);

    installedPluginsCh.ch.onReceive(this, [this](const CodeKeyList& codeKeyList) {
        startPlugins(codeKeyList);
    });
}

void PluginsService::startPlugins(const CodeKeyList& codeKeyList)
{
    for (const CodeKey& codeKey: codeKeyList) {
        Ret ret = start(codeKey);

        if (!ret) {
            LOGE() << ret.toString();
        }
    }
}

mu::RetVal<PluginList> PluginsService::allPlugins() const
{
    PluginList result = readPlugins();
    PluginList downloadedPlugins = downloadPlugins();

    for (const Plugin& plugin: downloadedPlugins) {
        if (!result.contains(plugin)) {
            result << plugin;
        }
    }

    updateInstalled(result);

    return RetVal<PluginList>::make_ok(result);
}

PluginList PluginsService::readPlugins() const
{
    PluginList result;
    io::paths pluginsPaths = scanFileSystemForPlugins();

    for (const io::path& pluginPath: pluginsPaths) {
         Plugin plugin = readPlugin(pluginPath);

         if (plugin.isValid()) {
             result.push_back(plugin);
         }
    }

    return result;
}

mu::io::paths PluginsService::scanFileSystemForPlugins() const
{
    io::paths result;

    for (const io::path& dirPath: configuration()->pluginsDirPaths()) {
        RetVal<io::paths> files = fileSystem()->scanFiles(dirPath, { ".qml" }, IFileSystem::ScanMode::IncludeSubdirs);

        if (!files.ret) {
            LOGE() << files.ret.toString();
            continue;
        }

        result.insert(result.end(), files.val.begin(), files.val.end());
    }

    return result;
}

Plugin PluginsService::readPlugin(const io::path& path) const
{
    NOT_IMPLEMENTED;
    Q_UNUSED(path);
    return Plugin();
}

PluginList PluginsService::downloadPlugins() const
{
    NOT_IMPLEMENTED;
    return PluginList();
}

void PluginsService::updateInstalled(PluginList& plugins) const
{
    CodeKeyList installedPlugins = configuration()->installedPlugins().val;

    for (Plugin& plugin: plugins) {
        plugin.installed = installedPlugins.contains(plugin.codeKey);
    }
}

mu::RetValCh<Progress> PluginsService::install(const CodeKey& codeKey)
{
    NOT_IMPLEMENTED;
    Q_UNUSED(codeKey);
    return mu::RetValCh<Progress>();
}

mu::RetValCh<Progress> PluginsService::update(const CodeKey& codeKey)
{
    NOT_IMPLEMENTED;
    Q_UNUSED(codeKey)
    return mu::RetValCh<Progress>();
}

mu::Ret PluginsService::uninstall(const CodeKey& codeKey)
{
    NOT_IMPLEMENTED;
    Q_UNUSED(codeKey)
    return mu::Ret();
}

mu::Ret PluginsService::start(const CodeKey& codeKey)
{
    NOT_IMPLEMENTED;
    Q_UNUSED(codeKey)
    return mu::Ret();
}

mu::Ret PluginsService::stop(const CodeKey &codeKey)
{
    NOT_IMPLEMENTED;
    Q_UNUSED(codeKey)
    return mu::Ret();
}

Channel<Plugin> PluginsService::pluginChanged() const
{
    return m_pluginChanged;
}
