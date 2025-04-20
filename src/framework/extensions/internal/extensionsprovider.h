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
#ifndef MUSE_EXTENSIONS_EXTENSIONSPROVIDER_H
#define MUSE_EXTENSIONS_EXTENSIONSPROVIDER_H

#include "global/async/asyncable.h"

#include "modularity/ioc.h"
#include "../iextensionsconfiguration.h"
#include "../iextensionsprovider.h"
#include "../iextensionsexecpointsregister.h"
#include "global/iinteractive.h"
#include "io/ifilesystem.h"

namespace muse::extensions {
class ExtensionsProvider : public IExtensionsProvider, public Injectable, public async::Asyncable
{
    Inject<IExtensionsConfiguration> configuration = { this };
    Inject<IExtensionsExecPointsRegister> execPointsRegister = { this };
    Inject<IInteractive> interactive = { this };
    Inject<io::IFileSystem> fileSystem  = { this };

public:
    ExtensionsProvider(const modularity::ContextPtr& iocCtx)
        : Injectable(iocCtx) {}

    void reloadExtensions() override;
    Ret removeExtension(const Uri& uri) override;

    ManifestList manifestList(Filter filter = Filter::All) const override;
    async::Notification manifestListChanged() const override;

    bool exists(const Uri& uri) const override;
    const Manifest& manifest(const Uri& uri) const override;
    async::Channel<Manifest> manifestChanged() const override;
    Action action(const UriQuery& q) const override;

    KnownCategories knownCategories() const override;

    Ret perform(const UriQuery& uri) override;
    Ret run(const UriQuery& uri) override;

    Ret setExecPoint(const Uri& uri, const ExecPointName& name) override;
    std::vector<ExecPoint> execPoints(const Uri& uri) const override;
    Ret performPoint(const ExecPointName& name) override;
    void performPointAsync(const ExecPointName& name) override;

private:

    Ret run(const Action& a);

    mutable ManifestList m_manifests;
    async::Notification m_manifestListChanged;
    async::Channel<Manifest> m_manifestChanged;
};
}

#endif // MUSE_EXTENSIONS_EXTENSIONSPROVIDER_H
