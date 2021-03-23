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
#ifndef MU_PLUGINS_IPLUGINSCONFIGURATION_H
#define MU_PLUGINS_IPLUGINSCONFIGURATION_H

#include "pluginstypes.h"
#include "modularity/imoduleexport.h"
#include "retval.h"

#include "io/path.h"

namespace mu::plugins {
class IPluginsConfiguration : MODULE_EXPORT_INTERFACE
{
    INTERFACE_ID(IPluginsConfiguration)

public:
    virtual ~IPluginsConfiguration() = default;

    virtual io::paths availablePluginsPaths() const = 0;

    virtual ValCh<io::path> pluginsPath() const = 0;
    virtual void setPluginsPath(const io::path& path) = 0;

    virtual ValCh<CodeKeyList> installedPlugins() const = 0;
    virtual void setInstalledPlugins(const CodeKeyList& codeKeyList) = 0;
};
}

#endif // MU_PLUGINS_IPLUGINSCONFIGURATION_H
