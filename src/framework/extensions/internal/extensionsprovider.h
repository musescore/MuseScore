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
#ifndef MU_EXTENSIONS_EXTENSIONSPROVIDER_H
#define MU_EXTENSIONS_EXTENSIONSPROVIDER_H

#include "modularity/ioc.h"
#include "../iextensionsconfiguration.h"
#include "../iextensionsprovider.h"
#include "global/iinteractive.h"

namespace mu::extensions {
class ExtensionsProvider : public IExtensionsProvider
{
    Inject<IExtensionsConfiguration> configuration;
    Inject<IInteractive> interactive;

public:
    ExtensionsProvider() = default;

    void reloadPlugins() override;

    ManifestList manifestList(Filter filter = Filter::All) const override;
    async::Notification manifestListChanged() const override;

    const Manifest& manifest(const Uri& uri) const override;
    async::Channel<Manifest> manifestChanged() const override;
    Action action(const UriQuery& q) const override;

    KnownCategories knownCategories() const override;

    Ret setEnable(const Uri& uri, bool enable) override;

    Ret perform(const UriQuery& uri) override;
    Ret run(const UriQuery& uri) override;

private:

    Ret run(const Action& a);

    mutable ManifestList m_manifests;
    async::Notification m_manifestListChanged;
    async::Channel<Manifest> m_manifestChanged;
};
}

#endif // MU_EXTENSIONS_EXTENSIONSPROVIDER_H
