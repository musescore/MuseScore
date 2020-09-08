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
static const std::string PLUGINS_DIR("/plugins");
static const std::string PLUGINS_REPOSITORY("/plugins.xml");

mu::io::paths PluginsConfiguration::pluginsDirPaths() const
{
    mu::io::paths result;

    result.push_back(globalConfiguration()->dataPath() + PLUGINS_DIR);
    result.push_back(globalConfiguration()->sharePath() + PLUGINS_DIR);
    result.push_back(settings()->value(USERPATH_TO_PLUGINS).toString());

    return result;
}

mu::io::path PluginsConfiguration::pluginsRepositoryPath() const
{
    return globalConfiguration()->dataPath() + PLUGINS_REPOSITORY;
}

QUrl PluginsConfiguration::pluginsServerUrl() const
{
    NOT_IMPLEMENTED;
    return QUrl();
}

QUrl PluginsConfiguration::pluginDetailsUrl(const std::string& codeKey) const
{
    NOT_IMPLEMENTED;
    Q_UNUSED(codeKey);
    return QUrl();
}
