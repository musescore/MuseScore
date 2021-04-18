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
#ifndef MU_EXTENSIONS_EXTENSIONSCONFIGURATION_H
#define MU_EXTENSIONS_EXTENSIONSCONFIGURATION_H

#include "modularity/ioc.h"
#include "iextensionsconfiguration.h"
#include "iglobalconfiguration.h"
#include "framework/system/ifilesystem.h"

namespace mu::extensions {
class ExtensionsConfiguration : public IExtensionsConfiguration
{
    INJECT(extensions, framework::IGlobalConfiguration, globalConfiguration)
    INJECT(extensions, system::IFileSystem, fileSystem)

public:
    void init();

    QUrl extensionsUpdateUrl() const override;
    QUrl extensionFileServerUrl(const QString& extensionCode) const override;

    bool needCheckForUpdate() const override;
    void setNeedCheckForUpdate(bool needCheck) override;

    ValCh<ExtensionsHash> extensions() const override;
    Ret setExtensions(const ExtensionsHash& extensions) const override;

    io::path extensionPath(const QString& extensionCode) const override;
    io::path extensionArchivePath(const QString& extensionCode) const override;

    io::paths extensionWorkspaceFiles(const QString& extensionCode) const override;
    io::paths workspacesPaths() const override;

    io::paths extensionInstrumentFiles(const QString& extensionCode) const override;
    io::paths instrumentsPaths() const override;

    io::paths templatesPaths() const override;

    ValCh<io::path> extensionsPath() const override;
    void setExtensionsPath(const io::path& path) override;

private:
    ExtensionsHash parseExtensionConfig(const QByteArray& json) const;

    io::path extensionFileName(const QString& extensionCode) const;
    io::paths fileList(const io::path& directory, const QStringList& filters) const;

    io::path extensionWorkspacesPath(const QString& extensionCode) const;
    io::path extensionInstrumentsPath(const QString& extensionCode) const;
    io::path extensionTemplatesPath(const QString& extensionCode) const;

    io::path extensionsDataPath() const;

    async::Channel<ExtensionsHash> m_extensionHashChanged;
    async::Channel<io::path> m_extensionsPathChanged;
};
}

#endif // MU_EXTENSIONS_EXTENSIONSCONFIGURATION_H
