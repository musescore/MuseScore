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
#include "framework/system/ifsoperations.h"

namespace mu {
namespace extensions {
class ExtensionsConfiguration : public IExtensionsConfiguration
{
    INJECT(extensions, framework::IGlobalConfiguration, globalConfiguration)
    INJECT(extensions, framework::IFsOperations, fsOperation)

public:
    void init();

    QUrl extensionsUpdateUrl() const override;
    QUrl extensionFileServerUrl(const QString& extensionCode) const override;

    ValCh<ExtensionsHash> extensions() const override;
    Ret setExtensions(const ExtensionsHash& extensions) const override;

    QString extensionPath(const QString& extensionCode) const override;
    QString extensionArchivePath(const QString& extensionCode) const override;

    QString extensionsSharePath() const override;
    QString extensionsDataPath() const override;

    QStringList extensionWorkspaceFiles(const QString& extensionCode) const override;
    QStringList workspacesPaths() const override;

    QStringList extensionInstrumentFiles(const QString& extensionCode) const override;
    QStringList instrumentsPaths() const override;

    QStringList templatesPaths() const override;

private:
    ExtensionsHash parseExtensionConfig(const QByteArray& json) const;

    QString extensionFileName(const QString& extensionCode) const;
    QStringList fileList(const QString& directory, const QStringList& filters) const;

    QString extensionWorkspacesPath(const QString& extensionCode) const;
    QString extensionInstrumentsPath(const QString& extensionCode) const;
    QString extensionTemplatesPath(const QString& extensionCode) const;

    async::Channel<ExtensionsHash> m_extensionHashChanged;
};
}
}

#endif // MU_EXTENSIONS_EXTENSIONSCONFIGURATION_H
