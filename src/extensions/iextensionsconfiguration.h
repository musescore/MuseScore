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
#ifndef MU_EXTENSIONS_IEXTENSIONSCONFIGURATION_H
#define MU_EXTENSIONS_IEXTENSIONSCONFIGURATION_H

#include "modularity/imoduleexport.h"
#include "retval.h"
#include "io/path.h"
#include "extensionstypes.h"

namespace mu::extensions {
class IExtensionsConfiguration : MODULE_EXPORT_INTERFACE
{
    INTERFACE_ID(IExtensionsConfiguration)

public:
    virtual ~IExtensionsConfiguration() = default;

    virtual QUrl extensionsUpdateUrl() const = 0;
    virtual QUrl extensionFileServerUrl(const QString& extensionCode) const = 0;

    virtual bool needCheckForUpdate() const = 0;
    virtual void setNeedCheckForUpdate(bool needCheck) = 0;

    virtual ValCh<ExtensionsHash> extensions() const = 0;
    virtual Ret setExtensions(const ExtensionsHash& extensions) const = 0;

    virtual io::path extensionPath(const QString& extensionCode) const = 0;
    virtual io::path extensionArchivePath(const QString& extensionCode) const = 0;

    virtual io::paths extensionWorkspacesFiles(const QString& extensionCode) const = 0;
    virtual io::paths extensionSoundFontsFiles(const QString& extensionCode) const = 0;
    virtual io::paths extensionInstrumentsFiles(const QString& extensionCode) const = 0;
    virtual io::paths extensionTemplatesFiles(const QString& extensionCode) const = 0;

    virtual io::path userExtensionsPath() const = 0;
    virtual void setUserExtensionsPath(const io::path& path) = 0;
};
}

#endif // MU_EXTENSIONS_IEXTENSIONSCONFIGURATION_H
