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
#include "pluginsservicestub.h"

using namespace mu::plugins;
using namespace mu;

void PluginsServiceStub::reloadPlugins()
{
}

RetVal<PluginInfoMap> PluginsServiceStub::plugins(PluginsStatus) const
{
    return make_ret(Ret::Code::NotSupported);
}

async::Notification PluginsServiceStub::pluginsChanged() const
{
    return async::Notification();
}

IPluginsService::CategoryInfoMap PluginsServiceStub::categories() const
{
    return {};
}

Ret PluginsServiceStub::setEnable(const CodeKey&, bool)
{
    return make_ret(Ret::Code::NotSupported);
}

Ret PluginsServiceStub::run(const CodeKey&)
{
    return make_ret(Ret::Code::NotSupported);
}

async::Channel<PluginInfo> PluginsServiceStub::pluginChanged() const
{
    return async::Channel<PluginInfo>();
}
