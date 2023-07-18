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

#include <QObject>

#include "io/path.h"
#include "async/asyncable.h"

#include "modularity/ioc.h"
#include "io/ifilesystem.h"
#include "shortcuts/ishortcutsregister.h"
#include "ui/iuiactionsregister.h"
#include "ipluginsconfiguration.h"

#include "ipluginsservice.h"

namespace mu::plugins {
class PluginsService : public QObject, public IPluginsService, public async::Asyncable
{
    Q_OBJECT

    INJECT(io::IFileSystem, fileSystem)
    INJECT(shortcuts::IShortcutsRegister, shortcutsRegister)
    INJECT(ui::IUiActionsRegister, uiActionsRegister)
    INJECT(IPluginsConfiguration, configuration)

public:
    void init();

    void reloadPlugins() override;

    mu::RetVal<PluginInfoMap> plugins(PluginsStatus status = PluginsStatus::All) const override;
    async::Notification pluginsChanged() const override;

    CategoryInfoMap categories() const override;

    Ret setEnable(const CodeKey& codeKey, bool enable) override;

    Ret run(const CodeKey& codeKey) override;

    async::Channel<PluginInfo> pluginChanged() const override;

private:
    void onShortcutsChanged();

    bool isAccepted(const CodeKey& codeKey, PluginsStatus status) const;

    const IPluginsConfiguration::PluginsConfigurationHash& pluginsConfiguration() const;
    void setPluginsConfiguration(const IPluginsConfiguration::PluginsConfigurationHash& pluginsConfiguration);

    PluginInfoMap readPlugins() const;
    io::paths_t scanFileSystemForPlugins() const;

    PluginInfo& pluginInfo(const CodeKey& codeKey);

    void registerShortcuts();

    mutable PluginInfoMap m_plugins;
    async::Notification m_pluginsChanged;
    async::Channel<PluginInfo> m_pluginChanged;
};
}

#endif // MU_PLUGINS_PLUGINSSERVICE_H
