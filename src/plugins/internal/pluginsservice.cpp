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

#include "shortcuts/shortcutstypes.h"

#include "pluginserrors.h"
#include "pluginsuiactions.h"

#include "view/pluginview.h"

#include "log.h"

using namespace mu::plugins;
using namespace mu::framework;
using namespace mu::async;
using namespace mu::shortcuts;

using PluginConfiguration = IPluginsConfiguration::PluginConfiguration;
using PluginConfigurationHash = IPluginsConfiguration::PluginsConfigurationHash;

static const std::string PLUGINS_CONTEXT = "plugins";

void PluginsService::init()
{
    TRACEFUNC;

    reloadPlugins();

    registerShortcuts();
}

void PluginsService::reloadPlugins()
{
    m_plugins = readPlugins();
    m_pluginsChanged.notify();
}

mu::RetVal<PluginInfoList> PluginsService::plugins(PluginsStatus status) const
{
    PluginInfoList result;

    for (const PluginInfo& plugin: m_plugins) {
        if (isAccepted(plugin.codeKey, status)) {
            result << plugin;
        }
    }

    return RetVal<PluginInfoList>::make_ok(result);
}

Notification PluginsService::pluginsChanged() const
{
    return m_pluginsChanged;
}

bool PluginsService::isAccepted(const CodeKey& codeKey, PluginsStatus status) const
{
    switch (status) {
    case PluginsStatus::All: return true;
    case PluginsStatus::Enabled: return pluginsConfiguration().value(codeKey).enabled;
    }

    return false;
}

PluginInfoList PluginsService::readPlugins() const
{
    TRACEFUNC;

    PluginInfoList result;
    io::paths pluginsPaths = scanFileSystemForPlugins();

    const PluginConfigurationHash& pluginsConfigurationHash = pluginsConfiguration();

    for (const io::path& pluginPath: pluginsPaths) {
        QUrl url = QUrl::fromLocalFile(pluginPath.toQString());
        PluginView view(url);

        PluginInfo info;
        info.codeKey = io::basename(pluginPath).toQString();
        info.url = url;
        info.name = view.name();
        info.description = view.description();
        info.version = view.version();
        info.enabled = pluginsConfigurationHash.value(info.codeKey).enabled;

        auto sequences = shortcutsRegister()->shortcut(info.codeKey.toStdString()).sequences;
        info.shortcuts = Shortcut::sequencesToString(sequences);

        if (info.isValid()) {
            result << info;
        }
    }

    return result;
}

mu::io::paths PluginsService::scanFileSystemForPlugins() const
{
    TRACEFUNC;

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

mu::Ret PluginsService::setEnable(const CodeKey& codeKey, bool enable)
{
    TRACEFUNC;

    PluginInfo& info = pluginInfo(codeKey);
    if (!info.isValid()) {
        LOGW() << QString("Plugin %1 not found").arg(codeKey);
        return make_ret(Err::PluginNotFound);
    }

    if (info.enabled == enable) {
        return make_ok();
    }

    PluginConfigurationHash pluginsConfigurationHash = this->pluginsConfiguration();
    if (pluginsConfigurationHash.contains(codeKey)) {
        pluginsConfigurationHash[codeKey].enabled = enable;
    } else if (enable) {
        PluginConfiguration plugin;
        plugin.codeKey = codeKey;
        plugin.enabled = enable;

        pluginsConfigurationHash[codeKey] = plugin;
    }

    setPluginsConfiguration(pluginsConfigurationHash);

    info.enabled = enable;
    m_pluginChanged.send(info);

    return make_ok();
}

PluginInfo& PluginsService::pluginInfo(const CodeKey& codeKey)
{
    for (PluginInfo& plugin: m_plugins) {
        if (plugin.codeKey == codeKey) {
            return plugin;
        }
    }

    static PluginInfo _dummy;
    return _dummy;
}

void PluginsService::registerShortcuts()
{
    TRACEFUNC;

    const PluginConfigurationHash& pluginsConfigurationHash = pluginsConfiguration();

    ShortcutList shortcuts;

    for (const PluginInfo& plugin : m_plugins) {
        Shortcut shortcut;
        shortcut.action = plugin.codeKey.toStdString();

        if (pluginsConfigurationHash.contains(plugin.codeKey)
            && !pluginsConfigurationHash[plugin.codeKey].shortcuts.empty()) {
            shortcut.sequences = Shortcut::sequencesFromString(pluginsConfigurationHash[plugin.codeKey].shortcuts);
        }

        shortcut.context = "notation-opened";

        shortcuts.push_back(shortcut);
    }

    if (!shortcuts.empty()) {
        shortcutsRegister()->setAdditionalShortcuts(PLUGINS_CONTEXT, shortcuts);
    }

    shortcutsRegister()->shortcutsChanged().onNotify(this, [this](){
        onShortcutsChanged();
    });
}

const PluginConfigurationHash& PluginsService::pluginsConfiguration() const
{
    return configuration()->pluginsConfiguration();
}

void PluginsService::setPluginsConfiguration(const PluginConfigurationHash& pluginsConfiguration)
{
    configuration()->setPluginsConfiguration(pluginsConfiguration);
}

mu::Ret PluginsService::run(const CodeKey& codeKey)
{
    PluginInfo& info = pluginInfo(codeKey);
    if (!info.isValid()) {
        LOGW() << QString("Plugin %1 not found").arg(codeKey);
        return make_ret(Err::PluginNotFound);
    }

    PluginView* view = new PluginView(info.url);
    view->run();

    QObject::connect(view, &PluginView::finished, view, &QObject::deleteLater);

    m_pluginChanged.send(info);

    return true;
}

Channel<PluginInfo> PluginsService::pluginChanged() const
{
    return m_pluginChanged;
}

void PluginsService::onShortcutsChanged()
{
    TRACEFUNC;

    PluginConfigurationHash pluginsConfigurationHash = this->pluginsConfiguration();

    PluginInfoList changedPlugins;

    for (PluginInfo& plugin : m_plugins) {
        const Shortcut shortcut = shortcutsRegister()->shortcut(plugin.codeKey.toStdString());
        if (shortcut.sequences.empty() && !pluginsConfigurationHash.contains(plugin.codeKey)) {
            continue;
        }

        std::string shortcuts = Shortcut::sequencesToString(shortcut.sequences);

        if (pluginsConfigurationHash.contains(plugin.codeKey)) {
            pluginsConfigurationHash[plugin.codeKey].shortcuts = shortcuts;
        } else {
            PluginConfiguration config;
            config.codeKey = plugin.codeKey;
            config.shortcuts = shortcuts;

            pluginsConfigurationHash[plugin.codeKey] = config;
        }

        plugin.shortcuts = shortcuts;
        changedPlugins.push_back(plugin);
    }

    setPluginsConfiguration(pluginsConfigurationHash);

    for (const PluginInfo& plugin : changedPlugins) {
        m_pluginChanged.send(plugin);
    }
}
