//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2020 MuseScore BVBA and others
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
//=============================================================================
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

io::path ExtensionsConfigurationStub::extensionsSharePath() const
{
    return io::path();
}

io::path ExtensionsConfigurationStub::extensionsDataPath() const
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
