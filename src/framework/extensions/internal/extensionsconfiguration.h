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
#ifndef MU_EXTENSIONS_EXTENSIONSCONFIGURATION_H
#define MU_EXTENSIONS_EXTENSIONSCONFIGURATION_H

#include "../iextensionsconfiguration.h"

#include "modularity/ioc.h"
#include "global/iglobalconfiguration.h"
#include "multiinstances/imultiinstancesprovider.h"

#include "global/async/asyncable.h"

namespace mu::extensions {
class ExtensionsConfiguration : public IExtensionsConfiguration, public mu::async::Asyncable
{
    Inject<IGlobalConfiguration> globalConfiguration;
    Inject<mi::IMultiInstancesProvider> multiInstancesProvider;

public:
    ExtensionsConfiguration() = default;

    void init();

    io::path_t defaultPath() const override;
    io::path_t userPath() const override;

    Ret setManifestConfigs(const std::map<Uri, Manifest::Config>& configs) override;
    std::map<Uri, Manifest::Config> manifestConfigs() const override;

    // legacy plugins
    io::path_t pluginsDefaultPath() const override;
    io::path_t pluginsUserPath() const override;
    void setUserPluginsPath(const io::path_t& path) override;
    async::Channel<io::path_t> pluginsUserPathChanged() const override;

private:

    async::Channel<io::path_t> m_pluginsUserPathChanged;
};
}

#endif // MU_EXTENSIONS_EXTENSIONSCONFIGURATION_H
