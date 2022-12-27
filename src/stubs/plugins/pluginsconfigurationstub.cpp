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
#include "pluginsconfigurationstub.h"

#include <QColor>

using namespace mu::plugins;
using namespace mu;

io::paths_t PluginsConfigurationStub::availablePluginsPaths() const
{
    return {};
}

io::path_t PluginsConfigurationStub::userPluginsPath() const
{
    return io::path_t();
}

void PluginsConfigurationStub::setUserPluginsPath(const io::path_t&)
{
}

async::Channel<io::path_t> PluginsConfigurationStub::userPluginsPathChanged() const
{
    return async::Channel<io::path_t>();
}

const IPluginsConfiguration::PluginsConfigurationHash& PluginsConfigurationStub::pluginsConfiguration() const
{
    static const PluginsConfigurationHash h;
    return h;
}

Ret PluginsConfigurationStub::setPluginsConfiguration(const PluginsConfigurationHash&)
{
    return make_ret(Ret::Code::NotSupported);
}

QColor PluginsConfigurationStub::viewBackgroundColor() const
{
    return QColor();
}
