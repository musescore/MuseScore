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
#ifndef MU_PLUGINS_PLUGINSCONFIGURATION_H
#define MU_PLUGINS_PLUGINSCONFIGURATION_H

#include "ipluginsconfiguration.h"
#include "modularity/ioc.h"
#include "iglobalconfiguration.h"
#include "val.h"
#include "system/ifilesystem.h"

namespace mu::plugins {
class PluginsConfiguration : public IPluginsConfiguration
{
    INJECT(plugins, framework::IGlobalConfiguration, globalConfiguration)
    INJECT(plugins, system::IFileSystem, fileSystem)

public:
    void init();

    io::paths availablePluginsPaths() const override;

    ValCh<io::path> pluginsPath() const override;
    void setPluginsPath(const io::path& path) override;

    ValCh<CodeKeyList> installedPlugins() const override;
    void setInstalledPlugins(const CodeKeyList& codeKeyList) override;

private:
    CodeKeyList parseInstalledPlugins(const mu::Val& val) const;

    io::path userPluginsPath() const;
    io::path defaultPluginsPath() const;

    async::Channel<CodeKeyList> m_installedPluginsChanged;
    async::Channel<io::path> m_pluginsPathChanged;
};
}

#endif // MU_PLUGINS_PLUGINSCONFIGURATION_H
