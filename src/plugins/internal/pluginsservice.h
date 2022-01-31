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

#ifndef MU_PLUGINS_PLUGINSSERVICE_H
#define MU_PLUGINS_PLUGINSSERVICE_H

#include "ipluginsservice.h"
#include "ipluginsconfiguration.h"
#include "system/ifilesystem.h"

#include "modularity/ioc.h"
#include "io/path.h"
#include "async/asyncable.h"

namespace mu::plugins {
class PluginsService : public IPluginsService, public async::Asyncable
{
    INJECT(plugins, IPluginsConfiguration, configuration)
    INJECT(plugins, system::IFileSystem, fileSystem)

public:
    mu::RetVal<PluginInfoList> plugins(PluginsStatus status = PluginsStatus::All) const override;

    RetValCh<framework::Progress> enable(const CodeKey& codeKey) override;
    RetValCh<framework::Progress> update(const CodeKey& codeKey) override;
    Ret disable(const CodeKey& codeKey) override;

    Ret run(const CodeKey& codeKey) override;

    async::Channel<PluginInfo> pluginChanged() const override;

private:
    bool isAccepted(const CodeKey& codeKey, PluginsStatus status) const;

    CodeKeyList enabledPlugins() const;
    void setEnabledPlugins(const CodeKeyList& codeKeyList);

    bool isEnabled(const CodeKey& codeKey) const;

    PluginInfoList readPlugins() const;
    io::paths scanFileSystemForPlugins() const;

    mu::RetVal<PluginInfo> pluginInfo(const CodeKey& codeKey) const;

    async::Channel<PluginInfo> m_pluginChanged;
};
}

#endif // MU_PLUGINS_PLUGINSSERVICE_H
