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
#include "extensionsproviderstub.h"

using namespace muse;
using namespace muse::extensions;

void ExtensionsProviderStub::reloadExtensions()
{
}

ManifestList ExtensionsProviderStub::manifestList(Filter) const
{
    return ManifestList();
}

async::Notification ExtensionsProviderStub::manifestListChanged() const
{
    static async::Notification n;
    return n;
}

bool ExtensionsProviderStub::exists(const Uri&) const
{
    return false;
}

const Manifest& ExtensionsProviderStub::manifest(const Uri&) const
{
    static Manifest m;
    return m;
}

async::Channel<Manifest> ExtensionsProviderStub::manifestChanged() const
{
    static async::Channel<Manifest> c;
    return c;
}

Action ExtensionsProviderStub::action(const UriQuery&) const
{
    return Action();
}

KnownCategories ExtensionsProviderStub::knownCategories() const
{
    return KnownCategories();
}

Ret ExtensionsProviderStub::perform(const UriQuery&)
{
    return muse::make_ret(Ret::Code::NotSupported);
}

Ret ExtensionsProviderStub::run(const UriQuery&)
{
    return muse::make_ret(Ret::Code::NotSupported);
}

Ret ExtensionsProviderStub::setExecPoint(const Uri&, const ExecPointName&)
{
    return muse::make_ret(Ret::Code::NotSupported);
}

std::vector<ExecPoint> ExtensionsProviderStub::execPoints(const Uri&) const
{
    return std::vector<ExecPoint>();
}

Ret ExtensionsProviderStub::performPoint(const ExecPointName&)
{
    return muse::make_ret(Ret::Code::NotSupported);
}

void ExtensionsProviderStub::performPointAsync(const ExecPointName&)
{
}
