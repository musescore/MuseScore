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

#include "retval.h"
#include "modularity/imoduleexport.h"

#include "extensionstypes.h"

namespace mu {
namespace extensions {
class IExtensionsConfiguration : MODULE_EXPORT_INTERFACE
{
    INTERFACE_ID(IExtensionsConfiguration)

public:
    virtual ~IExtensionsConfiguration() = default;

    virtual QUrl extensionsUpdateUrl() const = 0;
    virtual QUrl extensionsFileServerUrl() const = 0;

    virtual ValCh<ExtensionsHash> extensions() const = 0;
    virtual Ret setExtensions(const ExtensionsHash& extensions) const = 0;

    virtual QString extensionsSharePath() const = 0;
    virtual QString extensionsDataPath() const = 0;

    virtual QStringList workspacesPaths() const = 0;
};
}
}

#endif // MU_EXTENSIONS_IEXTENSIONSCONFIGURATION_H
