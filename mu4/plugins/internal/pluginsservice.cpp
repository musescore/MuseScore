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

#include "api/qmlplugin.h"
#include "log.h"

#include <QQmlComponent>

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

mu::RetVal<PluginList> PluginsService::plugins(PluginsStatus status) const
{
    PluginList readedPlugins = readPlugins();
    PluginList result;

    for (const Plugin& plugin: readedPlugins) {
        if (isAccepted(plugin.codeKey, status)) {
            result << plugin;
        }
    }

    return RetVal<PluginList>::make_ok(result);
}

bool PluginsService::isAccepted(const CodeKey&codeKey, PluginsStatus status) const
{
    switch (status) {
    case PluginsStatus::All: return true;
    case PluginsStatus::Installed: return isInstalled(codeKey);
    }

    return false;
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
        RetVal<io::paths> files = fileSystem()->scanFiles(dirPath, { "*.qml" }, IFileSystem::ScanMode::IncludeSubdirs);

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
    QUrl url = QUrl::fromLocalFile(path.toQString());
    QQmlComponent component(uiEngine()->qmlEngine(), url);
    Ms::QmlPlugin* qmlPlugin = qobject_cast<Ms::QmlPlugin*>(component.create());

    if (!qmlPlugin) {
        return Plugin();
    }

    Plugin plugin;

    plugin.codeKey = path.toQString();
    plugin.url = url;
    plugin.name = qmlPlugin->menuPath().mid(qmlPlugin->menuPath().lastIndexOf(".") + 1);
    plugin.description = qmlPlugin->description();
    plugin.version = QVersionNumber::fromString(qmlPlugin->version());
    plugin.installed = isInstalled(plugin.codeKey);

    delete qmlPlugin;

    return plugin;
}

bool PluginsService::isInstalled(const CodeKey& codeKey) const
{
    return installedPlugins().contains(codeKey);
}

mu::RetValCh<Progress> PluginsService::install(const CodeKey& codeKey)
{
    mu::RetValCh<Progress> result(true);

    CodeKeyList installedPlugins = this->installedPlugins();

    if (installedPlugins.contains(codeKey)) {
        LOGW() << QString("Plugin %1 is already installed").arg(codeKey);
        return result;
    }

    installedPlugins << codeKey;
    setInstalledPlugins(installedPlugins);

    notifyAboutPluginChanged(codeKey);

    return result;
}

void PluginsService::notifyAboutPluginChanged(const CodeKey& codeKey)
{
    Plugin plugin = this->plugin(codeKey);

    if (plugin.isValid()) {
        m_pluginChanged.send(plugin);
    }
}

Plugin PluginsService::plugin(const CodeKey& codeKey) const
{
    for (const Plugin& plugin: plugins().val) {
        if (plugin.codeKey == codeKey) {
            return plugin;
        }
    }

    return Plugin();
}

CodeKeyList PluginsService::installedPlugins() const
{
    return configuration()->installedPlugins().val;
}

void PluginsService::setInstalledPlugins(const CodeKeyList& codeKeyList)
{
    configuration()->setInstalledPlugins(codeKeyList);
}

mu::RetValCh<Progress> PluginsService::update(const CodeKey& codeKey)
{
    NOT_IMPLEMENTED;
    Q_UNUSED(codeKey)
    return mu::RetValCh<Progress>();
}

mu::Ret PluginsService::uninstall(const CodeKey& codeKey)
{
    Ret result(true);

    CodeKeyList installedPlugins = this->installedPlugins();
    installedPlugins.removeOne(codeKey);
    setInstalledPlugins(installedPlugins);

    notifyAboutPluginChanged(codeKey);

    return result;
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
