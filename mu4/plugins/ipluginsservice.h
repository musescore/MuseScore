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

#ifndef MU_PLUGINS_IPLUGINSSERVICE_H
#define MU_PLUGINS_IPLUGINSSERVICE_H

#include "pluginstypes.h"
#include "retval.h"
#include "network/networktypes.h"

#include "modularity/imoduleexport.h"

namespace mu {
namespace plugins {
class IPluginsService : MODULE_EXPORT_INTERFACE
{
    INTERFACE_ID(IPluginsService)

public:
    enum PluginsStatus {
        Installed,
        All
    };

    virtual RetVal<PluginInfoList> plugins(PluginsStatus status = All) const = 0;

    virtual RetValCh<framework::Progress> install(const CodeKey& codeKey) = 0;
    virtual RetValCh<framework::Progress> update(const CodeKey& codeKey) = 0;
    virtual Ret uninstall(const CodeKey& codeKey) = 0;

    virtual Ret run(const CodeKey& codeKey) = 0;

    virtual async::Channel<PluginInfo> pluginChanged() const = 0;
};
}
}

#endif // MU_PLUGINS_IPLUGINSSERVICE_H
