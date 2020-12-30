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
#ifndef MU_VST_PLUGINLOADER_H
#define MU_VST_PLUGINLOADER_H

#include <QLibrary>

#include "pluginterfaces/base/ipluginbase.h"
#include "plugin.h"

namespace mu {
namespace vst {
/*! Load plugins from file
    Each file can contain multiply plugins.
    After loading all plugins will be stored in m_plugins and available
*/
class PluginLoader
{
public:
    PluginLoader(std::string folder, std::string filename);
    ~PluginLoader();

    bool load();
    void unload();

    //! return vector of loaded plugins from file
    const std::vector<Plugin>& getPlugins() const;

private:
    std::string m_folder = "";
    std::string m_filename = "";

    QLibrary m_library;
    Steinberg::IPluginFactory3* m_factory = nullptr;

    std::vector<Plugin> m_plugins = {};

    void initPlugins();
};
}
}
#endif // MU_VST_PLUGINLOADER_H
