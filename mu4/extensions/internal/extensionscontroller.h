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
#ifndef MU_EXTENSIONS_EXTENSIONSCONTROLLER_H
#define MU_EXTENSIONS_EXTENSIONSCONTROLLER_H

#include "modularity/ioc.h"
#include "iextensionscontroller.h"
#include "iextensionsconfiguration.h"
#include "iextensionunpacker.h"
#include "framework/system/ifsoperations.h"

namespace mu {
namespace extensions {
class ExtensionsController : public IExtensionsController
{
    INJECT(extensions, IExtensionsConfiguration, configuration)
    INJECT(extensions, IExtensionUnpacker, extensionUnpacker)
    INJECT(extensions, framework::IFsOperations, fsOperation)

public:

    void init();

    Ret refreshExtensions() override;
    ValCh<ExtensionsHash> extensions() const override;
    Ret install(const QString& extensionCode) override;
    Ret uninstall(const QString& extensionCode) override;
    Ret update(const QString& extensionCode) override;

    RetCh<Extension> extensionChanged() const override;

private:
    RetVal<ExtensionsHash> parseExtensionConfig(const QByteArray& json) const;
    bool isExtensionExists(const QString& extensionCode) const;

    RetVal<ExtensionsHash> correctExtensionsStates(ExtensionsHash& extensions) const;

    RetVal<QString> downloadExtension(const QString& extensionCode) const;
    Ret removeExtension(const QString& extensionCode) const;

    Extension::ExtensionTypes extensionTypes(const QString& extensionCode) const;

private:
    async::Channel<Extension> m_extensionChanged;
};
}
}

#endif // MU_EXTENSIONS_EXTENSIONSCONTROLLER_H
