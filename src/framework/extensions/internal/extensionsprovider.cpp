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
#include "extensionsprovider.h"

#include "global/containers.h"

#include "extensionsloader.h"
#include "legacy/extpluginsloader.h"

using namespace mu::extensions;

const ManifestList& ExtensionsProvider::manifestList() const
{
    if (m_manifests.empty()) {
        ExtensionsLoader loader;
        m_manifests = loader.loadManifesList(configuration()->defaultPath(),
                                             configuration()->userPath());

        legacy::ExtPluginsLoader pluginsLoader;
        ManifestList plugins = pluginsLoader.loadManifesList(configuration()->pluginsDefaultPath(),
                                                             configuration()->pluginsUserPath());

        mu::join(m_manifests, plugins);
    }
    return m_manifests;
}

const Manifest& ExtensionsProvider::manifest(const Uri& uri) const
{
    const ManifestList& list = manifestList();
    auto it = std::find_if(list.begin(), list.end(), [uri](const Manifest& m) {
        return m.uri == uri;
    });

    if (it != list.end()) {
        return *it;
    }

    static Manifest _dymmy;
    return _dymmy;
}
