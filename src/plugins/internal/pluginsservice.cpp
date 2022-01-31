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
    case PluginsStatus::Enabled: return isEnabled(codeKey);
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
        info.enabled = isEnabled(info.codeKey);

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

bool PluginsService::isEnabled(const CodeKey& codeKey) const
{
    return enabledPlugins().contains(codeKey);
}

mu::RetValCh<Progress> PluginsService::enable(const CodeKey& codeKey)
{
    RetVal<PluginInfo> info = pluginInfo(codeKey);
    if (!info.ret) {
        return info.ret;
    }

    mu::RetValCh<Progress> result(true);
    CodeKeyList enabledPlugins = this->enabledPlugins();

    if (enabledPlugins.contains(codeKey)) {
        LOGW() << QString("Plugin %1 is already enabled").arg(codeKey);
        return result;
    }

    enabledPlugins << codeKey;
    setEnabledPlugins(enabledPlugins);

    info.val.enabled = true;
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

CodeKeyList PluginsService::enabledPlugins() const
{
    return configuration()->enabledPlugins().val;
}

void PluginsService::setEnabledPlugins(const CodeKeyList& codeKeyList)
{
    configuration()->setEnabledPlugins(codeKeyList);
}

mu::RetValCh<Progress> PluginsService::update(const CodeKey& codeKey)
{
    NOT_IMPLEMENTED;
    Q_UNUSED(codeKey)
    return mu::RetValCh<Progress>();
}

mu::Ret PluginsService::disable(const CodeKey& codeKey)
{
    RetVal<PluginInfo> info = pluginInfo(codeKey);
    if (!info.ret) {
        return info.ret;
    }

    CodeKeyList enabledPlugins = this->enabledPlugins();
    enabledPlugins.removeOne(codeKey);
    setEnabledPlugins(enabledPlugins);

    info.val.enabled = false;
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
