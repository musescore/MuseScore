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
#ifndef MU_VST_VSTSCANNER_H
#define MU_VST_VSTSCANNER_H

#include <vector>

#include "plugin.h"
#include "modularity/imoduleexport.h"

namespace mu {
namespace vst {
class VSTScanner : MODULE_EXPORT_INTERFACE
{
    INTERFACE_ID(VSTScanner)

public:
    //! construct with paths
    explicit VSTScanner(std::string paths);
    explicit VSTScanner();

    //! current paths for scan
    std::string paths() const;
    void setPaths(const std::string& path);

    //! scnan for installed VST3 plugins
    void scan();

    //! return all available plugins as a map: [std::string UID] : Plugin
    const std::map<std::string, Plugin>& getPlugins() const;

private:
    //! paths to search installed plugins
    std::vector<std::string> m_paths = {};

    //! all loaded plugins m_plugins[UID] = Plugin
    std::map<std::string, Plugin> m_plugins = {};
};
}
}

#endif // MU_VST_VSTSCANNER_H
