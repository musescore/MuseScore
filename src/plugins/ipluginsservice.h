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

#ifndef MU_PLUGINS_IPLUGINSSERVICE_H
#define MU_PLUGINS_IPLUGINSSERVICE_H

#include "pluginstypes.h"
#include "types/retval.h"
#include "types/translatablestring.h"

#include "modularity/imoduleinterface.h"

namespace mu::plugins {
class IPluginsService : MODULE_EXPORT_INTERFACE
{
    INTERFACE_ID(IPluginsService)

public:
    enum PluginsStatus {
        Enabled,
        All
    };

    virtual void reloadPlugins() = 0;

    virtual RetVal<PluginInfoMap> plugins(PluginsStatus status = All) const = 0;
    virtual async::Notification pluginsChanged() const = 0;

    using CategoryInfoMap = std::map<std::string /*code*/, TranslatableString /*title*/>;
    virtual CategoryInfoMap categories() const = 0;

    virtual Ret setEnable(const CodeKey& codeKey, bool enable) = 0;

    virtual Ret run(const CodeKey& codeKey) = 0;

    virtual async::Channel<PluginInfo> pluginChanged() const = 0;
};
}

#endif // MU_PLUGINS_IPLUGINSSERVICE_H
