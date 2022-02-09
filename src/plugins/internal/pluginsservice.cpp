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

#include "pluginsuiactions.h"

#include "log.h"

using namespace mu::plugins;
using namespace mu::framework;
using namespace mu::async;
using namespace mu::shortcuts;

using ConfiguredPlugin = IPluginsConfiguration::ConfiguredPlugin;
using ConfiguredPluginHash = IPluginsConfiguration::ConfiguredPluginHash;

static const std::string PLUGINS_CONTEXT = "plugins";

std::string shortcutsToString(const std::vector<std::string>& shortcuts)
{
    return mu::strings::join(shortcuts, "; ");
}

std::vector<std::string> shortcutsFromString(const std::string& shortcutsStr)
{
    std::vector<std::string> shortcuts;
    mu::strings::split(shortcutsStr, shortcuts, "; ");

    return shortcuts;
}

void PluginsService::init()
{
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
    if (m_plugins.empty()) {
        m_plugins = readPlugins();
    }

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
    case PluginsStatus::Enabled: return isEnabled(configuredPlugins(), codeKey);
    }

    return false;
}

PluginInfoList PluginsService::readPlugins() const
{
    PluginInfoList result;
    io::paths pluginsPaths = scanFileSystemForPlugins();

    ConfiguredPluginHash configuredPluginsList = configuredPlugins();

    for (const io::path& pluginPath: pluginsPaths) {
        QUrl url = QUrl::fromLocalFile(pluginPath.toQString());
        PluginView view(url);

        PluginInfo info;
        info.codeKey = io::basename(pluginPath).toQString();
        info.url = url;
        info.name = view.name();
        info.description = view.description();
        info.version = view.version();
        info.enabled = isEnabled(configuredPluginsList, info.codeKey);

        auto sequences = shortcutsRegister()->shortcut(info.codeKey.toStdString()).sequences;
        info.shortcuts = shortcutsToString(sequences);

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

bool PluginsService::isEnabled(const ConfiguredPluginHash& configuredPluginList, const CodeKey& codeKey) const
{
    for (const ConfiguredPlugin& plugin : configuredPluginList.values()) {
        if (plugin.codeKey == codeKey) {
            return plugin.enabled;
        }
    }

    return false;
}

mu::RetValCh<Progress> PluginsService::enable(const CodeKey& codeKey)
{
    PluginInfo& info = pluginInfo(codeKey);
    if (!info.isValid()) {
        LOGW() << QString("Plugin %1 not found").arg(codeKey);
        return make_ret(Ret::Code::UnknownError);
    }

    mu::RetValCh<Progress> result(true);
    ConfiguredPluginHash configuredPluginHash = this->configuredPlugins();

    if (isEnabled(configuredPluginHash, codeKey)) {
        LOGW() << QString("Plugin %1 is already enabled").arg(codeKey);
        return result;
    }

    if (configuredPluginHash.contains(codeKey)) {
        configuredPluginHash[codeKey].enabled = true;
    } else {
        ConfiguredPlugin plugin;
        plugin.codeKey = codeKey;
        plugin.enabled = true;

        configuredPluginHash[codeKey] = plugin;
    }

    setConfiguredPlugins(configuredPluginHash);

    info.enabled = true;
    m_pluginChanged.send(info);

    return result;
}

PluginInfo& PluginsService::pluginInfo(const CodeKey& codeKey)
{
    for (PluginInfo& plugin: m_plugins) {
        if (plugin.codeKey == codeKey) {
            return plugin;
        }
    }

    static PluginInfo dummy;
    return dummy;
}

void PluginsService::registerShortcuts()
{
    ConfiguredPluginHash configuredPluginHash = configuredPlugins();

    ShortcutList shortcuts;

    for (const PluginInfo& plugin : m_plugins) {
        Shortcut shortcut;
        shortcut.action = plugin.codeKey.toStdString();

        if (configuredPluginHash.contains(plugin.codeKey)) {
            shortcut.sequences = shortcutsFromString(configuredPluginHash[plugin.codeKey].shortcuts);
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

ConfiguredPluginHash PluginsService::configuredPlugins() const
{
    return configuration()->configuredPlugins();
}

void PluginsService::setConfiguredPlugins(const ConfiguredPluginHash& configuredPlugins)
{
    configuration()->setConfiguredPlugins(configuredPlugins);
}

mu::RetValCh<Progress> PluginsService::update(const CodeKey& codeKey)
{
    NOT_IMPLEMENTED;
    Q_UNUSED(codeKey)
    return mu::RetValCh<Progress>();
}

mu::Ret PluginsService::disable(const CodeKey& codeKey)
{
    PluginInfo& info = pluginInfo(codeKey);
    if (!info.isValid()) {
        LOGW() << QString("Plugin %1 not found").arg(codeKey);
        return make_ret(Ret::Code::UnknownError);
    }

    ConfiguredPluginHash configuredPluginHash = configuredPlugins();
    if (!configuredPluginHash.contains(codeKey)) {
        LOGW() << QString("Plugin %1 is already disabled").arg(codeKey);
        return make_ok();
    }

    configuredPluginHash[codeKey].enabled = false;

    setConfiguredPlugins(configuredPluginHash);

    info.enabled = false;
    m_pluginChanged.send(info);

    return true;
}

mu::Ret PluginsService::run(const CodeKey& codeKey)
{
    PluginInfo& info = pluginInfo(codeKey);
    if (!info.isValid()) {
        LOGW() << QString("Plugin %1 not found").arg(codeKey);
        return make_ret(Ret::Code::UnknownError);
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
    ConfiguredPluginHash configuredPluginHash = this->configuredPlugins();

    PluginInfoList changedPlugins;

    for (PluginInfo& plugin : m_plugins) {
        Shortcut shortcut = shortcutsRegister()->shortcut(plugin.codeKey.toStdString());
        if (shortcut.sequences.empty() && !configuredPluginHash.contains(plugin.codeKey)) {
            continue;
        }

        std::string shortcuts = shortcutsToString(shortcut.sequences);

        if (configuredPluginHash.contains(plugin.codeKey)) {
            configuredPluginHash[plugin.codeKey].shortcuts = shortcuts;
        } else {
            ConfiguredPlugin configuredPlugin;
            configuredPlugin.codeKey = plugin.codeKey;
            configuredPlugin.shortcuts = shortcuts;

            configuredPluginHash[plugin.codeKey] = configuredPlugin;
        }

        plugin.shortcuts = shortcuts;
        changedPlugins.push_back(plugin);
    }

    setConfiguredPlugins(configuredPluginHash);

    for (const PluginInfo& plugin : changedPlugins) {
        m_pluginChanged.send(plugin);
    }
}
