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

#include "view/pluginview.h"
#include "pluginserrors.h"

#include "log.h"

using namespace mu::plugins;
using namespace mu::framework;
using namespace mu::async;

mu::RetVal<PluginInfoList> PluginsService::plugins(PluginsStatus status) const
{
    PluginInfoList readedPlugins = readPlugins();
    PluginInfoList result;

    for (const PluginInfo& plugin: readedPlugins) {
        if (isAccepted(plugin.codeKey, status)) {
            result << plugin;
        }
    }

    return RetVal<PluginInfoList>::make_ok(result);
}

bool PluginsService::isAccepted(const CodeKey& codeKey, PluginsStatus status) const
{
    switch (status) {
    case PluginsStatus::All: return true;
    case PluginsStatus::Installed: return isInstalled(codeKey);
    }

    return false;
}

PluginInfoList PluginsService::readPlugins() const
{
    PluginInfoList result;
    io::paths pluginsPaths = scanFileSystemForPlugins();

    for (const io::path& pluginPath: pluginsPaths) {
        QUrl url = QUrl::fromLocalFile(pluginPath.toQString());
        PluginView view(url);

        PluginInfo info;
        info.codeKey = pluginPath.toQString();
        info.url = url;
        info.name = view.name();
        info.description = view.description();
        info.version = view.version();
        info.installed = isInstalled(info.codeKey);

        if (info.isValid()) {
            result << info;
        }
    }

    return result;
}

mu::io::paths PluginsService::scanFileSystemForPlugins() const
{
    io::paths result;

    for (const io::path& dirPath: configuration()->availablePluginsPaths()) {
        RetVal<io::paths> files = fileSystem()->scanFiles(dirPath, { "*.qml" });

        if (!files.ret) {
            LOGE() << files.ret.toString();
            continue;
        }

        result.insert(result.end(), files.val.begin(), files.val.end());
    }

    return result;
}

bool PluginsService::isInstalled(const CodeKey& codeKey) const
{
    return installedPlugins().contains(codeKey);
}

mu::RetValCh<Progress> PluginsService::install(const CodeKey& codeKey)
{
    RetVal<PluginInfo> info = pluginInfo(codeKey);
    if (!info.ret) {
        return info.ret;
    }

    mu::RetValCh<Progress> result(true);
    CodeKeyList installedPlugins = this->installedPlugins();

    if (installedPlugins.contains(codeKey)) {
        LOGW() << QString("Plugin %1 is already installed").arg(codeKey);
        return result;
    }

    installedPlugins << codeKey;
    setInstalledPlugins(installedPlugins);

    info.val.installed = true;
    m_pluginChanged.send(info.val);

    return result;
}

mu::RetVal<PluginInfo> PluginsService::pluginInfo(const CodeKey& codeKey) const
{
    for (const PluginInfo& plugin: plugins().val) {
        if (plugin.codeKey == codeKey) {
            return RetVal<PluginInfo>::make_ok(plugin);
        }
    }

    return RetVal<PluginInfo>(make_ret(Err::PluginNotFound));
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
    RetVal<PluginInfo> info = pluginInfo(codeKey);
    if (!info.ret) {
        return info.ret;
    }

    CodeKeyList installedPlugins = this->installedPlugins();
    installedPlugins.removeOne(codeKey);
    setInstalledPlugins(installedPlugins);

    info.val.installed = false;
    m_pluginChanged.send(info.val);

    return true;
}

mu::Ret PluginsService::run(const CodeKey& codeKey)
{
    RetVal<PluginInfo> info = pluginInfo(codeKey);
    if (!info.ret) {
        return info.ret;
    }

    PluginView* view = new PluginView(info.val.url);
    view->run();

    QObject::connect(view, &PluginView::finished, view, &QObject::deleteLater);

    m_pluginChanged.send(info.val);

    return true;
}

Channel<PluginInfo> PluginsService::pluginChanged() const
{
    return m_pluginChanged;
}
