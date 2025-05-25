/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-CLA-applies
 *
 * MuseScore
 * Music Composition & Notation
 *
 * Copyright (C) 2025 MuseScore BVBA and others
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

#include "extensions/iextensionsprovider.h"

namespace muse::extensions {
class ExtensionsProviderStub : public IExtensionsProvider
{
public:
    ExtensionsProviderStub() = default;

    void reloadExtensions() override;

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
};
}
