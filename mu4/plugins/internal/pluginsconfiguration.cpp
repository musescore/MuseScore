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

#include "pluginsconfiguration.h"

#include "settings.h"
#include "log.h"

using namespace mu::plugins;
using namespace mu::framework;

static const std::string module_name("plugins");
static const Settings::Key USERPATH_TO_PLUGINS(module_name, "application/paths/myPlugins");
static const Settings::Key INSTALLED_PLUGINS(module_name, "plugins/installedPlugins");
static const std::string PLUGINS_DIR("/plugins");

PluginsConfiguration::PluginsConfiguration()
{
    settings()->valueChanged(INSTALLED_PLUGINS).onReceive(nullptr, [this](const Val& val) {
        CodeKeyList installedPlugins = parseInstalledPlugins(val);
        m_installedPluginsChanged.send(installedPlugins);
    });
}

mu::io::paths PluginsConfiguration::pluginsDirPaths() const
{
    mu::io::paths result;

    result.push_back(globalConfiguration()->dataPath() + PLUGINS_DIR);
    result.push_back(globalConfiguration()->sharePath() + PLUGINS_DIR);
    result.push_back(settings()->value(USERPATH_TO_PLUGINS).toString());

    return result;
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
