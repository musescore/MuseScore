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
static const std::string PLUGINS_DIR("/plugins");

void PluginsConfiguration::init()
{
    settings()->setDefaultValue(USER_PLUGINS_PATH, Val(globalConfiguration()->appDataPath().toStdString() + "Plugins"));
    settings()->valueChanged(USER_PLUGINS_PATH).onReceive(nullptr, [this](const Val& val) {
        m_pluginsPathChanged.send(val.toString());
    });

    settings()->valueChanged(INSTALLED_PLUGINS).onReceive(nullptr, [this](const Val& val) {
        CodeKeyList installedPlugins = parseInstalledPlugins(val);
        m_installedPluginsChanged.send(installedPlugins);
    });

    fileSystem()->makePath(pluginsPath().val);
}

mu::io::paths PluginsConfiguration::availablePluginsPaths() const
{
    io::paths result;

    result.push_back(globalConfiguration()->userDataPath() + PLUGINS_DIR);

    io::path defaultPluginsPath  = this->defaultPluginsPath();
    result.push_back(defaultPluginsPath);

    io::path userPluginsPath = this->userPluginsPath();
    if (!userPluginsPath.empty() && userPluginsPath != defaultPluginsPath) {
        result.push_back(userPluginsPath);
    }

    return result;
}

mu::ValCh<mu::io::path> PluginsConfiguration::pluginsPath() const
{
    ValCh<io::path> result;
    result.ch = m_pluginsPathChanged;
    result.val = userPluginsPath();

    return result;
}

void PluginsConfiguration::setPluginsPath(const io::path& path)
{
    settings()->setValue(USER_PLUGINS_PATH, Val(path.toStdString()));
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

    settings()->setValue(INSTALLED_PLUGINS, Val::fromQVariant(plugins));
}

CodeKeyList PluginsConfiguration::parseInstalledPlugins(const mu::Val& val) const
{
    return val.toQVariant().toStringList();
}

mu::io::path PluginsConfiguration::userPluginsPath() const
{
    return settings()->value(USER_PLUGINS_PATH).toString();
}

mu::io::path PluginsConfiguration::defaultPluginsPath() const
{
    return settings()->defaultValue(USER_PLUGINS_PATH).toString();
}
