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

#ifndef VSTPLUGINREPOSITORY_H
#define VSTPLUGINREPOSITORY_H

#include <unordered_map>

#include "async/channel.h"
#include "async/asyncable.h"
#include "modularity/ioc.h"
#include "system/ifilesystem.h"

#include "ivstpluginrepository.h"
#include "ivstconfiguration.h"
#include "vsttypes.h"

namespace mu::vst {
class VstPluginRepository : public IVstPluginRepository, public async::Asyncable
{
    INJECT(vst, IVstConfiguration, configuration)
    INJECT(vst, system::IFileSystem, fileSystem)

public:
    VstPluginRepository() = default;

    Ret loadAvailablePlugins() override;
    RetVal<VstPluginPtr> findPluginById(const PluginId& id) const override;
    RetValCh<VstPluginMetaList> pluginsMetaList() const override;

private:
    RetVal<io::paths> pluginPathsFromCustomLocation(const io::path& customPath) const;
    PluginModule::PathList pluginPathsFromDefaultLocation() const;

    Ret instantiatePlugin(const std::string& path);

    VstPluginMetaList availablePluginsMetaList() const;

    std::unordered_map<PluginId, VstPluginPtr> m_pluginsMap;

    async::Channel<VstPluginMetaList> m_pluginMetaListChannel;
};
}

#endif // VSTPLUGINREPOSITORY_H
