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

#include "pluginstestmodel.h"

#include "log.h"

using namespace mu::plugins;

void PluginsTestModel::load()
{
    RetVal<PluginList> plugins = service()->plugins(IPluginsService::Installed);
    if (!plugins.ret) {
        LOGE() << plugins.ret.toString();
        return;
    }

    m_installedPlugins = plugins.val;

    emit loaded();
}

void PluginsTestModel::open(int index)
{
    if (index < 0 || index >= m_installedPlugins.size()) {
        return;
    }

    Plugin plugin = m_installedPlugins[index];

    QStringList params {
        QString("name=%1").arg(plugin.name),
        QString("url=%2").arg(plugin.url.toString())
    };

    std::string uri = QString("musescore://plugins/open?%1").arg(params.join('&')).toStdString();
    interactive()->open(uri);
}

QStringList PluginsTestModel::installedPluginsNames() const
{
    QStringList names;

    for (const Plugin& plugin: m_installedPlugins) {
        names << plugin.name;
    }

    return names;
}
