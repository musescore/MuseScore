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

    io::path userPluginsPath() const override;
    void setUserPluginsPath(const io::path& path) override;
    async::Channel<io::path> userPluginsPathChanged() const override;

    ValCh<CodeKeyList> installedPlugins() const override;
    void setInstalledPlugins(const CodeKeyList& codeKeyList) override;

private:
    CodeKeyList parseInstalledPlugins(const mu::Val& val) const;

    async::Channel<CodeKeyList> m_installedPluginsChanged;
    async::Channel<io::path> m_userPluginsPathChanged;
};
}

#endif // MU_PLUGINS_PLUGINSCONFIGURATION_H
