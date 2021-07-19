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
#include "modularity/imoduleexport.h"
#include "retval.h"

#include "io/path.h"
#include "async/channel.h"

namespace mu::plugins {
class IPluginsConfiguration : MODULE_EXPORT_INTERFACE
{
    INTERFACE_ID(IPluginsConfiguration)

public:
    virtual ~IPluginsConfiguration() = default;

    virtual io::paths availablePluginsPaths() const = 0;

    virtual io::path userPluginsPath() const = 0;
    virtual void setUserPluginsPath(const io::path& path) = 0;
    virtual async::Channel<io::path> userPluginsPathChanged() const = 0;

    virtual ValCh<CodeKeyList> installedPlugins() const = 0;
    virtual void setInstalledPlugins(const CodeKeyList& codeKeyList) = 0;
};
}

#endif // MU_PLUGINS_IPLUGINSCONFIGURATION_H
