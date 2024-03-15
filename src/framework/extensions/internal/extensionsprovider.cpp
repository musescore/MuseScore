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

#include "extensionrunner.h"
#include "legacy/extpluginrunner.h"

using namespace mu::extensions;

KnownCategories ExtensionsProvider::knownCategories() const
{
    static KnownCategories categories {
        { "composing-arranging-tools", TranslatableString("plugins", "Composing/arranging tools") },
        { "color-notes", TranslatableString("plugins", "Color notes") },
        { "playback", TranslatableString("plugins", "Playback") },
        { "lyrics", TranslatableString("plugins", "Lyrics") }
    };

    return categories;
}

void ExtensionsProvider::reloadPlugins()
{
    ExtensionsLoader loader;
    m_manifests = loader.loadManifesList(configuration()->defaultPath(),
                                         configuration()->userPath());

    legacy::ExtPluginsLoader pluginsLoader;
    ManifestList plugins = pluginsLoader.loadManifesList(configuration()->pluginsDefaultPath(),
                                                         configuration()->pluginsUserPath());

    mu::join(m_manifests, plugins);

    m_manifestListChanged.notify();
}

ManifestList ExtensionsProvider::manifestList(Filter filter) const
{
    if (filter == Filter::Enabled) {
        ManifestList list;
        for (const Manifest& m : m_manifests) {
            if (m.config.enabled) {
                list.push_back(m);
            }
        }
        return list;
    }

    return m_manifests;
}

mu::async::Notification ExtensionsProvider::manifestListChanged() const
{
    return m_manifestListChanged;
}

const Manifest& ExtensionsProvider::manifest(const Uri& uri) const
{
    auto it = std::find_if(m_manifests.begin(), m_manifests.end(), [uri](const Manifest& m) {
        return m.uri == uri;
    });

    if (it != m_manifests.end()) {
        return *it;
    }

    static Manifest _dymmy;
    return _dymmy;
}

mu::async::Channel<Manifest> ExtensionsProvider::manifestChanged() const
{
    return m_manifestChanged;
}

mu::Ret ExtensionsProvider::setEnable(const Uri& uri, bool enable)
{
    for (Manifest& m : m_manifests) {
        if (m.uri == uri) {
            m.config.enabled = enable;
            m_manifestChanged.send(m);

            //! TODO Add save config
            return make_ok();
        }
    }

    return make_ret(Ret::Code::UnknownError);
}

mu::Ret ExtensionsProvider::perform(const Uri& uri)
{
    const Manifest& m = manifest(uri);
    switch (m.type) {
    case Type::Form:
        return interactive()->open(uri).ret;
        break;
    case Type::Macros:
        return run(uri);
    default:
        break;
    }

    return make_ret(Ret::Code::UnknownError);
}

mu::Ret ExtensionsProvider::run(const Uri& uri)
{
    const Manifest& m = manifest(uri);
    if (!m.isValid()) {
        return make_ret(Ret::Code::UnknownError);
    }

    //! TODO Add check of type

    Ret ret;
    if (m.apiversion == 1) {
        legacy::ExtPluginRunner runner;
        ret = runner.run(m);
    } else {
        ExtensionRunner runner;
        ret = runner.run(m);
    }

    return ret;
}
