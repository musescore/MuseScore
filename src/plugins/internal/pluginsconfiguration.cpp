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

#include "pluginsconfiguration.h"

#include <QJsonDocument>
#include <QJsonArray>

#include "multiinstances/resourcelockguard.h"

#include "settings.h"
#include "log.h"

using namespace mu::plugins;
using namespace mu::framework;

static const std::string module_name("plugins");
static const Settings::Key USER_PLUGINS_PATH(module_name, "application/paths/myPlugins");

static const mu::io::path PLUGINS_FILE("/plugins.json");

void PluginsConfiguration::init()
{
    settings()->setDefaultValue(USER_PLUGINS_PATH, Val(globalConfiguration()->userDataPath() + "/Plugins"));
    settings()->valueChanged(USER_PLUGINS_PATH).onReceive(nullptr, [this](const Val& val) {
        m_userPluginsPathChanged.send(val.toString());
    });
    fileSystem()->makePath(userPluginsPath());

    fileSystem()->makePath(pluginsDataPath());
}

mu::io::paths PluginsConfiguration::availablePluginsPaths() const
{
    io::paths result;

    io::path appPluginsPath  = globalConfiguration()->appDataPath() + "/plugins";
    result.push_back(appPluginsPath);

    io::path userPluginsPath = this->userPluginsPath();
    if (!userPluginsPath.empty() && userPluginsPath != appPluginsPath) {
        result.push_back(userPluginsPath);
    }

    return result;
}

mu::io::path PluginsConfiguration::userPluginsPath() const
{
    return settings()->value(USER_PLUGINS_PATH).toPath();
}

void PluginsConfiguration::setUserPluginsPath(const io::path& path)
{
    settings()->setSharedValue(USER_PLUGINS_PATH, Val(path));
}

mu::async::Channel<mu::io::path> PluginsConfiguration::userPluginsPathChanged() const
{
    return m_userPluginsPathChanged;
}

IPluginsConfiguration::ConfiguredPluginHash PluginsConfiguration::configuredPlugins() const
{
    RetVal<QByteArray> retVal = readConfiguredPlugins();
    if (!retVal.ret) {
        LOGE() << retVal.ret.toString();
        return {};
    }

    return parseConfiguredPlugins(retVal.val);
}

mu::Ret PluginsConfiguration::setConfiguredPlugins(const ConfiguredPluginHash& pluginList)
{
    QJsonArray jsonArray;
    for (const ConfiguredPlugin& plugin : pluginList.values()) {
        QVariantMap value;
        value["codeKey"] = plugin.codeKey;
        value["enabled"] = plugin.enabled;
        value["shortcuts"] = QString::fromStdString(plugin.shortcuts);

        jsonArray << QJsonValue::fromVariant(value);
    }

    QByteArray data = QJsonDocument(jsonArray).toJson();
    return writeConfiguredPlugins(data);
}

mu::io::path PluginsConfiguration::pluginsDataPath() const
{
    return globalConfiguration()->userAppDataPath() + "/plugins";
}

mu::io::path PluginsConfiguration::pluginsFilePath() const
{
    return pluginsDataPath() + PLUGINS_FILE;
}

mu::RetVal<QByteArray> PluginsConfiguration::readConfiguredPlugins() const
{
    mi::ResourceLockGuard lock_guard(multiInstancesProvider(), "PLUGINS_FILE");
    return fileSystem()->readFile(pluginsFilePath());
}

mu::Ret PluginsConfiguration::writeConfiguredPlugins(const QByteArray& data)
{
    mi::ResourceLockGuard lock_guard(multiInstancesProvider(), "PLUGINS_FILE");
    return fileSystem()->writeToFile(pluginsFilePath(), data);
}

IPluginsConfiguration::ConfiguredPluginHash PluginsConfiguration::parseConfiguredPlugins(const QByteArray& json) const
{
    QJsonParseError err;
    QJsonDocument jsodDoc = QJsonDocument::fromJson(json, &err);
    if (err.error != QJsonParseError::NoError || !jsodDoc.isArray()) {
        LOGE() << "failed parse, err: " << err.errorString();
        return {};
    }

    ConfiguredPluginHash result;
    const QVariantList pluginList = jsodDoc.array().toVariantList();
    for (const QVariant& pluginVal : pluginList) {
        QVariantMap pluginMap = pluginVal.toMap();

        ConfiguredPlugin plugin;
        plugin.codeKey = pluginMap.value("codeKey").toString();
        plugin.enabled = pluginMap.value("enabled").toBool();
        plugin.shortcuts = pluginMap.value("shortcuts").toString().toStdString();

        if (plugin.isValid()) {
            result[plugin.codeKey] = plugin;
        }
    }

    return result;
}
