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
#ifndef MU_PLUGINS_PLUGINSSERVICESTUB_H
#define MU_PLUGINS_PLUGINSSERVICESTUB_H

#include "plugins/ipluginsservice.h"

namespace mu::plugins {
class PluginsServiceStub : public IPluginsService
{
public:
    void reloadPlugins() override;

    RetVal<PluginInfoMap> plugins(PluginsStatus status = All) const override;
    async::Notification pluginsChanged() const override;

    CategoryInfoMap categories() const override;

    Ret setEnable(const CodeKey& codeKey, bool enable) override;

    Ret run(const CodeKey& codeKey) override;

    async::Channel<PluginInfo> pluginChanged() const override;
};
}

#endif // MU_PLUGINS_IPLUGINSSERVICE_H
