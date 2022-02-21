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

#include "async/asyncable.h"

#include "modularity/ioc.h"
#include "system/ifilesystem.h"
#include "multiinstances/imultiinstancesprovider.h"
#include "iglobalconfiguration.h"

#include "ipluginsconfiguration.h"

namespace mu::plugins {
class PluginsConfiguration : public IPluginsConfiguration, public async::Asyncable
{
    INJECT(plugins, framework::IGlobalConfiguration, globalConfiguration)
    INJECT(plugins, mi::IMultiInstancesProvider, multiInstancesProvider)
    INJECT(plugins, system::IFileSystem, fileSystem)

public:
    void init();

    io::paths availablePluginsPaths() const override;

    io::path userPluginsPath() const override;
    void setUserPluginsPath(const io::path& path) override;
    async::Channel<io::path> userPluginsPathChanged() const override;

    const PluginsConfigurationHash& pluginsConfiguration() const override;
    Ret setPluginsConfiguration(const PluginsConfigurationHash& configuration) override;

private:
    io::path pluginsDataPath() const;
    io::path pluginsFilePath() const;

    RetVal<QByteArray> readPluginsConfiguration() const;
    Ret writePluginsConfiguration(const QByteArray& data);

    PluginsConfigurationHash parsePluginsConfiguration(const QByteArray& json) const;

    void updatePluginsConfiguration();

    async::Channel<CodeKeyList> m_configuredPluginsChanged;
    async::Channel<io::path> m_userPluginsPathChanged;

    PluginsConfigurationHash m_pluginsConfiguration;
};
}

#endif // MU_PLUGINS_PLUGINSCONFIGURATION_H
