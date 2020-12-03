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
    RetVal<PluginInfoList> plugins = service()->plugins(IPluginsService::Installed);
    if (!plugins.ret) {
        LOGE() << plugins.ret.toString();
        return;
    }

    m_installedPlugins = plugins.val;

    emit loaded();
}

void PluginsTestModel::run(int index)
{
    if (index < 0 || index >= m_installedPlugins.size()) {
        return;
    }

    Ret ret = service()->run(m_installedPlugins[index].codeKey);
    if (!ret) {
        LOGE() << ret.toString();
    }
}

QStringList PluginsTestModel::installedPluginsNames() const
{
    QStringList names;

    for (const PluginInfo& plugin: m_installedPlugins) {
        names << plugin.name;
    }

    return names;
}
