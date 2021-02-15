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
#ifndef MU_VST_PLUGIN_H
#define MU_VST_PLUGIN_H

#include "pluginterfaces/base/ipluginbase.h"
#include "ivstinstanceregister.h"
#include "modularity/ioc.h"

namespace mu {
namespace vst {
class PluginLoader;
class PluginInstance;
class Plugin
{
public:
    enum Type {
        UNKNOWN,
        Instrument,
        //Audio, //for the future
    };

    Plugin();
    Plugin(const Plugin& second);
    friend PluginInstance;
    friend PluginLoader;

    INJECT(vst, IVSTInstanceRegister, vstInstanceRegister)

    //! return unique id of the plugin
    std::string getId() const;

    //! return original plugin's name
    std::string getName() const;

    //! return plugin's type (Instrument, Audio etc) or UNKNOWN
    Type getType() const;

    //! create an instance that could be used for processing
    std::shared_ptr<PluginInstance> createInstance();

private:
    Plugin(Steinberg::PClassInfo2 effectClass, Steinberg::IPluginFactory3* factory);

    //! static map of supported plugin's types
    static const std::map<std::string, Type> subCategoriesMap;

    //! information about AudioEffectClass of plugin
    Steinberg::PClassInfo2 m_effectClass;

    //! pointer to the factory function of plugin
    Steinberg::IPluginFactory3* m_factory = nullptr;
};
}
}
#endif // MU_VST_PLUGIN_H
