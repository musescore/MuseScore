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

#include "settings.h"
#include "log.h"

using namespace mu::plugins;
using namespace mu::framework;

static const std::string module_name("plugins");
static const Settings::Key USER_PLUGINS_PATH(module_name, "application/paths/myPlugins");
static const Settings::Key INSTALLED_PLUGINS(module_name, "plugins/installedPlugins");

void PluginsConfiguration::init()
{
    settings()->setDefaultValue(USER_PLUGINS_PATH, Val(globalConfiguration()->userDataPath() + "/Plugins"));
    settings()->valueChanged(USER_PLUGINS_PATH).onReceive(nullptr, [this](const Val& val) {
        m_userPluginsPathChanged.send(val.toString());
    });
    fileSystem()->makePath(userPluginsPath());

    settings()->valueChanged(INSTALLED_PLUGINS).onReceive(nullptr, [this](const Val& val) {
        CodeKeyList installedPlugins = parseInstalledPlugins(val);
        m_installedPluginsChanged.send(installedPlugins);
    });
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

mu::ValCh<CodeKeyList> PluginsConfiguration::installedPlugins() const
{
    ValCh<CodeKeyList> result;
    result.val = parseInstalledPlugins(settings()->value(INSTALLED_PLUGINS));
    result.ch = m_installedPluginsChanged;

    return result;
}

void PluginsConfiguration::setInstalledPlugins(const CodeKeyList& codeKeyList)
{
    QStringList plugins;

    for (const CodeKey& codeKey: codeKeyList) {
        plugins << codeKey;
    }

    settings()->setSharedValue(INSTALLED_PLUGINS, Val::fromQVariant(plugins));
}

CodeKeyList PluginsConfiguration::parseInstalledPlugins(const mu::Val& val) const
{
    return val.toQVariant().toStringList();
}
