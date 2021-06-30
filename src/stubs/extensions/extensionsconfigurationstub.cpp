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
#include "extensionsconfigurationstub.h"

using namespace mu;
using namespace mu::extensions;

QUrl ExtensionsConfigurationStub::extensionsUpdateUrl() const
{
    return QUrl();
}

QUrl ExtensionsConfigurationStub::extensionFileServerUrl(const QString&) const
{
    return QUrl();
}

bool ExtensionsConfigurationStub::needCheckForUpdate() const
{
    return false;
}

void ExtensionsConfigurationStub::setNeedCheckForUpdate(bool)
{
}

ValCh<ExtensionsHash> ExtensionsConfigurationStub::extensions() const
{
    return ValCh<ExtensionsHash>();
}

Ret ExtensionsConfigurationStub::setExtensions(const ExtensionsHash&) const
{
    return make_ret(Ret::Code::NotSupported);
}

io::path ExtensionsConfigurationStub::extensionPath(const QString&) const
{
    return io::path();
}

io::path ExtensionsConfigurationStub::extensionArchivePath(const QString&) const
{
    return io::path();
}

io::paths ExtensionsConfigurationStub::extensionWorkspaceFiles(const QString&) const
{
    return {};
}

io::paths ExtensionsConfigurationStub::workspacesPaths() const
{
    return {};
}

io::paths ExtensionsConfigurationStub::extensionInstrumentFiles(const QString&) const
{
    return {};
}

io::paths ExtensionsConfigurationStub::instrumentsPaths() const
{
    return {};
}

io::paths ExtensionsConfigurationStub::templatesPaths() const
{
    return {};
}

ValCh<mu::io::path> mu::extensions::ExtensionsConfigurationStub::extensionsPath() const
{
    return ValCh<io::path>();
}

void ExtensionsConfigurationStub::setExtensionsPath(const io::path&)
{
}
