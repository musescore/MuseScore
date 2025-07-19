/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-CLA-applies
 *
 * MuseScore
 * Music Composition & Notation
 *
 * Copyright (C) 2024 MuseScore BVBA and others
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
#pragma once

#include "modularity/imoduleinterface.h"

#include "global/io/path.h"
#include "global/types/ret.h"
#include "global/async/channel.h"

#include "extensionstypes.h"

namespace muse::extensions {
class IExtensionsConfiguration : MODULE_EXPORT_INTERFACE
{
    INTERFACE_ID(IExtensionsConfiguration)
public:

    virtual ~IExtensionsConfiguration() = default;

    virtual io::path_t defaultPath() const = 0;
    virtual io::path_t userPath() const = 0;

    virtual Ret setManifestConfigs(const std::map<Uri, Manifest::Config>& configs) = 0;
    virtual std::map<Uri, Manifest::Config> manifestConfigs() const = 0;

    // legacy plugins
    virtual io::path_t pluginsDefaultPath() const = 0;
    virtual io::path_t pluginsUserPath() const = 0;
    virtual void setUserPluginsPath(const io::path_t& path) = 0;
    virtual async::Channel<io::path_t> pluginsUserPathChanged() const = 0;
};
}
