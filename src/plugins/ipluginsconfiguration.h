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
#ifndef MU_PLUGINS_IPLUGINSCONFIGURATION_H
#define MU_PLUGINS_IPLUGINSCONFIGURATION_H

#include "pluginstypes.h"
#include "modularity/imoduleinterface.h"
#include "types/retval.h"

#include "io/path.h"
#include "async/channel.h"

namespace mu::plugins {
class IPluginsConfiguration : MODULE_EXPORT_INTERFACE
{
    INTERFACE_ID(IPluginsConfiguration)

public:
    virtual ~IPluginsConfiguration() = default;

    virtual io::paths_t availablePluginsPaths() const = 0;

    virtual io::path_t userPluginsPath() const = 0;
    virtual void setUserPluginsPath(const io::path_t& path) = 0;
    virtual async::Channel<io::path_t> userPluginsPathChanged() const = 0;

    struct PluginConfiguration {
        CodeKey codeKey;
        bool enabled = false;
        std::string shortcuts;

        bool isValid() const
        {
            return !codeKey.isEmpty();
        }
    };
    using PluginsConfigurationHash = QHash<CodeKey, PluginConfiguration>;

    virtual const PluginsConfigurationHash& pluginsConfiguration() const = 0;
    virtual mu::Ret setPluginsConfiguration(const PluginsConfigurationHash& configuration) = 0;

    virtual QColor viewBackgroundColor() const = 0;
};
}

#endif // MU_PLUGINS_IPLUGINSCONFIGURATION_H
