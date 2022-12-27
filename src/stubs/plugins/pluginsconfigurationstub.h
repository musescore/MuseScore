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
#ifndef MU_PLUGINS_PLUGINSCONFIGURATIONSTUB_H
#define MU_PLUGINS_PLUGINSCONFIGURATIONSTUB_H

#include "plugins/ipluginsconfiguration.h"

namespace mu::plugins {
class PluginsConfigurationStub : public IPluginsConfiguration
{
public:
    io::paths_t availablePluginsPaths() const override;

    io::path_t userPluginsPath() const override;
    void setUserPluginsPath(const io::path_t& path) override;
    async::Channel<io::path_t> userPluginsPathChanged() const override;

    const PluginsConfigurationHash& pluginsConfiguration() const override;
    mu::Ret setPluginsConfiguration(const PluginsConfigurationHash& configuration) override;

    QColor viewBackgroundColor() const override;
};
}

#endif // MU_PLUGINS_PLUGINSCONFIGURATIONSTUB_H
