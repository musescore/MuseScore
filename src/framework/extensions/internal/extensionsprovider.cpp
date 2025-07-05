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
#include "global/async/async.h"
#include "global/io/path.h"

#include "extensionsloader.h"
#include "legacy/extpluginsloader.h"

#include "extensionrunner.h"
#include "legacy/extpluginrunner.h"

#include "../extensionserrors.h"

#include "log.h"

using namespace muse::extensions;

KnownCategories ExtensionsProvider::knownCategories() const
{
    static KnownCategories categories {
        { "composing-arranging-tools", TranslatableString("extensions", "Composing/arranging tools") },
        { "color-notes", TranslatableString("extensions", "Color notes") },
        { "playback", TranslatableString("extensions", "Playback") },
        { "lyrics", TranslatableString("extensions", "Lyrics") }
    };

    return categories;
}

void ExtensionsProvider::reloadExtensions()
{
    ExtensionsLoader loader;
    m_manifests = loader.loadManifestList(configuration()->defaultPath(),
                                          configuration()->userPath());

    legacy::ExtPluginsLoader pluginsLoader;
    ManifestList plugins = pluginsLoader.loadManifestList(configuration()->pluginsDefaultPath(),
                                                          configuration()->pluginsUserPath());

    muse::join(m_manifests, plugins);

    std::map<Uri, Manifest::Config> configs = configuration()->manifestConfigs();
    for (Manifest& m : m_manifests) {
        m.config = muse::value(configs, m.uri);
    }

    m_manifestListChanged.notify();
}

ManifestList ExtensionsProvider::manifestList(Filter filter) const
{
    if (filter == Filter::Enabled) {
        ManifestList list;
        for (const Manifest& m : m_manifests) {
            if (m.enabled()) {
                list.push_back(m);
            }
        }
        return list;
    }

    return m_manifests;
}

muse::async::Notification ExtensionsProvider::manifestListChanged() const
{
    return m_manifestListChanged;
}

bool ExtensionsProvider::exists(const Uri& uri) const
{
    auto it = std::find_if(m_manifests.begin(), m_manifests.end(), [uri](const Manifest& m) {
        return m.uri == uri;
    });

    if (it != m_manifests.end()) {
        return true;
    }

    return false;
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

muse::async::Channel<Manifest> ExtensionsProvider::manifestChanged() const
{
    return m_manifestChanged;
}

Action ExtensionsProvider::action(const UriQuery& q) const
{
    const Manifest& m = manifest(q.uri());
    IF_ASSERT_FAILED(m.actions.size() > 0) {
        return Action();
    }

    if (m.actions.size() == 1) {
        return m.actions.at(0);
    }

    std::string code = q.param("action").toString();
    for (const Action& a : m.actions) {
        if (a.code == code) {
            return a;
        }
    }

    LOGE() << "not found action: " << code << ", uri: " << q.toString();
    return Action();
}

muse::Ret ExtensionsProvider::perform(const UriQuery& uri)
{
    Action a = action(uri);
    switch (a.type) {
    case Type::Form: {
        UriQuery q = uri;
        if (!q.contains("modal")) {
            q.addParam("modal", Val(a.modal));
        }
        return interactive()->openSync(q).ret;
    } break;
    case Type::Macros:
        return run(uri);
    default:
        break;
    }

    return make_ret(Ret::Code::UnknownError);
}

muse::Ret ExtensionsProvider::run(const UriQuery& uri)
{
    Action a = action(uri);
    return run(a);
}

muse::Ret ExtensionsProvider::run(const Action& a)
{
    if (!a.isValid()) {
        return make_ret(Err::ExtNotFound);
    }

    //! TODO Add check of type

    Ret ret;
    if (a.legacyPlugin) {
        legacy::ExtPluginRunner runner(iocContext());
        ret = runner.run(a);
    } else {
        ExtensionRunner runner(iocContext());
        ret = runner.run(a);
    }

    return ret;
}

muse::Ret ExtensionsProvider::setExecPoint(const Uri& uri, const ExecPointName& name)
{
    bool ok = false;
    std::map<Uri, Manifest::Config> allconfigs;
    for (Manifest& m : m_manifests) {
        if (m.uri == uri) {
            for (const Action& a : m.actions) {
                Action::Config& ac = m.config.actions[a.code];
                ac.execPoint = name;
            }

            m_manifestChanged.send(m);
            ok = true;
        }

        allconfigs[m.uri] = m.config;
    }

    Ret ret;
    if (ok) {
        ret = configuration()->setManifestConfigs(allconfigs);
    } else {
        ret = make_ret(Ret::Code::UnknownError);
    }

    return ret;
}

std::vector<ExecPoint> ExtensionsProvider::execPoints(const Uri& uri) const
{
    UNUSED(uri);
    return execPointsRegister()->allPoints();
}

muse::Ret ExtensionsProvider::performPoint(const ExecPointName& name)
{
    Ret ret = make_ok();
    for (const Manifest& m : m_manifests) {
        for (const Action& a : m.actions) {
            if (m.config.aconfig(a.code).execPoint != name) {
                continue;
            }

            Ret r = perform(makeActionQuery(m.uri, a.code));
            if (ret) {
                ret = r;
            }
        }
    }

    return ret;
}

void ExtensionsProvider::performPointAsync(const ExecPointName& name)
{
    async::Async::call(this, [this, name]() {
        performPoint(name);
    });
}
