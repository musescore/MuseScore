//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2021 MuseScore BVBA and others
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
#ifndef MU_FRAMEWORK_IEXTENSIONPROVIDER_H
#define MU_FRAMEWORK_IEXTENSIONPROVIDER_H

#include "modularity/imoduleexport.h"
#include "io/path.h"
#include "async/channel.h"

namespace mu::framework {
class IExtensionContentProvider : MODULE_EXPORT_INTERFACE
{
    INTERFACE_ID(IExtensionContentProvider)

public:
    virtual ~IExtensionContentProvider() = default;

    enum ExtensionContentType {
        Undefined,
        Workspaces,
        SFZS,
        SoundFonts,
        Templates,
        Instruments
    };

    virtual io::paths extensionPaths(ExtensionContentType extensionType) const = 0;
    virtual async::Channel<ExtensionContentType> extensionPathsChanged() const = 0;
};
}

#endif // MU_FRAMEWORK_IEXTENSIONPROVIDER_H
